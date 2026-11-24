// Tests of the logic of the store that needs no database, plus the behaviour of the store when there is none.
#include <gtest/gtest.h>

#include <chrono>
#include <fstream>
#include <limits>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "DigitalTwinStore.h"
#include "EmbeddedSchema.h"
#include "RowQueue.h"
#include "StoreLogic.h"

using namespace DigitalTwin::Store;
using DigitalTwin::Model::VariableValue;
using DigitalTwin::Model::ValueType;

namespace {
	constexpr Micros SECOND = MICROS_PER_SECOND;
	constexpr Micros Y2000 = 946684800 * SECOND;   // 2000-01-01T00:00:00Z

	// ---------------------------------------------------------------------------------------------------------
	// Months and partitions
	// ---------------------------------------------------------------------------------------------------------

	TEST(StoreLogicMonths, CalendarAnchors) {
		EXPECT_EQ(startOf(YearMonth{2000, 1}), Y2000);
		EXPECT_EQ(startOf(YearMonth{1970, 1}), 0);
		EXPECT_EQ(monthOf(0), (YearMonth{1970, 1}));
		EXPECT_EQ(monthOf(Y2000 + 12 * 3600 * SECOND + 5), (YearMonth{2000, 1}));
		EXPECT_EQ(monthOf(-1), (YearMonth{1969, 12}));
	}

	TEST(StoreLogicMonths, LengthOfFebruaryFollowsTheLeapYearRules) {
		const auto days = [](YearMonth month) { return (endOf(month) - startOf(month)) / MICROS_PER_DAY; };
		EXPECT_EQ(days(YearMonth{2024, 2}), 29);
		EXPECT_EQ(days(YearMonth{2023, 2}), 28);
		EXPECT_EQ(days(YearMonth{2100, 2}), 28);   // not a leap year
		EXPECT_EQ(days(YearMonth{2000, 2}), 29);   // divisible by 400
		EXPECT_EQ(days(YearMonth{2026, 11}), 30);
		EXPECT_EQ(days(YearMonth{2026, 12}), 31);
	}

	TEST(StoreLogicMonths, BoundsBelongToTheRightMonth) {
		for (const YearMonth month : {YearMonth{2026, 11}, YearMonth{2026, 12}, YearMonth{2027, 1}, YearMonth{2024, 2}}) {
			EXPECT_EQ(monthOf(startOf(month)), month);
			EXPECT_EQ(monthOf(startOf(month) - 1), addMonths(month, -1));
			EXPECT_EQ(monthOf(endOf(month) - 1), month);
			EXPECT_EQ(monthOf(endOf(month)), addMonths(month, 1));
		}
	}

	TEST(StoreLogicMonths, AddMonthsCrossesYears) {
		EXPECT_EQ(addMonths(YearMonth{2026, 11}, 1), (YearMonth{2026, 12}));
		EXPECT_EQ(addMonths(YearMonth{2026, 11}, 2), (YearMonth{2027, 1}));
		EXPECT_EQ(addMonths(YearMonth{2026, 1}, -1), (YearMonth{2025, 12}));
		EXPECT_EQ(addMonths(YearMonth{2026, 11}, 0), (YearMonth{2026, 11}));
		EXPECT_EQ(addMonths(YearMonth{2026, 11}, 25), (YearMonth{2028, 12}));
		EXPECT_EQ(addMonths(YearMonth{2026, 11}, -11), (YearMonth{2025, 12}));
		EXPECT_EQ(addMonths(YearMonth{2026, 11}, -23), (YearMonth{2024, 12}));
	}

	TEST(StoreLogicPartitions, NamesAreMeasurementYYYYmMM) {
		EXPECT_EQ(partitionName(YearMonth{2026, 11}), "measurement_y2026m11");
		EXPECT_EQ(partitionName(YearMonth{2027, 1}), "measurement_y2027m01");
		EXPECT_EQ(partitionName(YearMonth{999, 5}), "measurement_y0999m05");
	}

	TEST(StoreLogicPartitions, ParseAcceptsOnlyItsOwnNames) {
		for (const YearMonth month : {YearMonth{2026, 11}, YearMonth{2000, 1}, YearMonth{2099, 12}}) {
			const auto parsed = parsePartitionName(partitionName(month));
			ASSERT_TRUE(parsed.has_value());
			EXPECT_EQ(*parsed, month);
		}
		for (const char* bad : {"", "measurement", "measurement_y2026m13", "measurement_y2026m00", "measurement_y2026m1",
		                        "measurement_y2026m111", "xmeasurement_y2026m11", "measurement_y20a6m11", "measurement_y2026n11",
		                        "measurement_default", "measurement_y2026m11 ", "measurement_y2026m11; drop table x"})
			EXPECT_FALSE(parsePartitionName(bad).has_value()) << bad;
	}

	TEST(StoreLogicPartitions, CreateStatementHasTheMonthBounds) {
		EXPECT_EQ(createPartitionSql(YearMonth{2026, 12}),
		          "CREATE TABLE IF NOT EXISTS measurement_y2026m12 PARTITION OF measurement FOR VALUES FROM "
		          "('2026-12-01 00:00:00.000000+00') TO ('2027-01-01 00:00:00.000000+00')");
		EXPECT_EQ(dropPartitionSql(YearMonth{2026, 12}), "DROP TABLE IF EXISTS measurement_y2026m12");
	}

	TEST(StoreLogicPartitions, TimestampFormat) {
		EXPECT_EQ(formatTimestamp(0), "1970-01-01 00:00:00.000000+00");
		EXPECT_EQ(formatTimestamp(Y2000 + 1), "2000-01-01 00:00:00.000001+00");
		EXPECT_EQ(formatTimestamp(-1), "1969-12-31 23:59:59.999999+00");
		EXPECT_EQ(formatTimestamp(Y2000 + (3 * 3600 + 4 * 60 + 5) * SECOND + 678901), "2000-01-01 03:04:05.678901+00");
		EXPECT_EQ(formatTimestamp(startOf(YearMonth{2024, 2}) + 28 * MICROS_PER_DAY), "2024-02-29 00:00:00.000000+00");
	}

	TEST(StoreLogicPartitions, TimePointConversionIsExactToTheMicrosecond) {
		const Micros value = Y2000 + 123456;
		EXPECT_EQ(toMicros(toTimePoint(value)), value);
	}

	TEST(StoreLogicRetention, CutoffAndExpiry) {
		const Micros now = startOf(YearMonth{2026, 11}) + 10 * MICROS_PER_DAY;
		EXPECT_FALSE(retentionCutoff(now, 0).has_value());
		ASSERT_TRUE(retentionCutoff(now, 30).has_value());
		EXPECT_EQ(*retentionCutoff(now, 30), now - 30 * MICROS_PER_DAY);

		// A partition goes only when it ends at or before the cutoff.
		const YearMonth october{2026, 10};
		EXPECT_TRUE(partitionExpired(october, endOf(october)));
		EXPECT_FALSE(partitionExpired(october, endOf(october) - 1));
		const auto cutoff = *retentionCutoff(now, 30);   // Oct 11
		EXPECT_FALSE(partitionExpired(october, cutoff));
		EXPECT_TRUE(partitionExpired(YearMonth{2026, 9}, cutoff));
		EXPECT_FALSE(partitionExpired(YearMonth{2026, 11}, cutoff));
	}

	// ---------------------------------------------------------------------------------------------------------
	// Monotonic clock and checkpoint decision
	// ---------------------------------------------------------------------------------------------------------

	TEST(MonotonicClockTest, ClampsOlderTimesToTheLastOne) {
		MonotonicClock clock;
		EXPECT_FALSE(clock.last().has_value());
		EXPECT_EQ(clock.clamp(100), 100);
		EXPECT_EQ(clock.clamp(200), 200);
		EXPECT_EQ(clock.clamp(150), 200);
		EXPECT_EQ(clock.clamp(200), 200);
		EXPECT_EQ(clock.clamp(201), 201);
		ASSERT_TRUE(clock.last().has_value());
		EXPECT_EQ(*clock.last(), 201);
	}

	TEST(MonotonicClockTest, SeedOnlyRaises) {
		MonotonicClock clock;
		clock.seed(500);
		EXPECT_EQ(clock.clamp(100), 500);
		clock.seed(300);
		EXPECT_EQ(clock.clamp(400), 500);
		clock.seed(900);
		EXPECT_EQ(clock.clamp(600), 900);
	}

	TEST(MonotonicClockTest, FirstTimeMayBeNegative) {
		MonotonicClock clock;
		EXPECT_EQ(clock.clamp(-50), -50);
		EXPECT_EQ(clock.clamp(-60), -50);
	}

	TEST(CheckpointDecision, VersionWithoutCheckpointIsDue) {
		CheckpointTracker tracker;
		EXPECT_EQ(decideCheckpoint(tracker, Y2000, 300 * SECOND), CheckpointReason::Interval);
	}

	TEST(CheckpointDecision, IntervalNeedsTimeAndAChange) {
		CheckpointTracker tracker;
		tracker.HasCheckpoint = true;
		tracker.LastCheckpointAt = Y2000;
		const Micros interval = 300 * SECOND;

		EXPECT_FALSE(decideCheckpoint(tracker, Y2000 + 10 * SECOND, interval).has_value());
		tracker.ChangedSinceCheckpoint = true;
		EXPECT_FALSE(decideCheckpoint(tracker, Y2000 + 299 * SECOND, interval).has_value());
		EXPECT_EQ(decideCheckpoint(tracker, Y2000 + 300 * SECOND, interval), CheckpointReason::Interval);

		// Time has passed but nothing changed: an idle twin writes nothing.
		tracker.ChangedSinceCheckpoint = false;
		EXPECT_FALSE(decideCheckpoint(tracker, Y2000 + 3000 * SECOND, interval).has_value());
	}

	TEST(CheckpointDecision, IntervalZeroSwitchesTheIntervalRuleOff) {
		CheckpointTracker tracker;
		tracker.HasCheckpoint = true;
		tracker.LastCheckpointAt = Y2000;
		tracker.ChangedSinceCheckpoint = true;
		EXPECT_FALSE(decideCheckpoint(tracker, Y2000 + 100000 * SECOND, 0).has_value());
	}

	TEST(CheckpointDecision, NewMonthIsDueEvenWhenIdle) {
		CheckpointTracker tracker;
		tracker.HasCheckpoint = true;
		tracker.LastCheckpointAt = endOf(YearMonth{2026, 11}) - SECOND;   // Nov 30, 23:59:59
		tracker.ChangedSinceCheckpoint = false;
		EXPECT_FALSE(decideCheckpoint(tracker, endOf(YearMonth{2026, 11}) - 1, 300 * SECOND).has_value());
		EXPECT_EQ(decideCheckpoint(tracker, endOf(YearMonth{2026, 11}), 300 * SECOND), CheckpointReason::NewMonth);
		// Across a year boundary as well.
		tracker.LastCheckpointAt = endOf(YearMonth{2026, 12}) - SECOND;
		EXPECT_EQ(decideCheckpoint(tracker, endOf(YearMonth{2026, 12}) + SECOND, 300 * SECOND), CheckpointReason::NewMonth);
	}

	TEST(CheckpointDecision, GapIsDueAfterDataLossButNotMoreThanOncePerSecond) {
		CheckpointTracker tracker;
		tracker.HasCheckpoint = true;
		tracker.LastCheckpointAt = Y2000;
		tracker.Gap = true;
		EXPECT_EQ(decideCheckpoint(tracker, Y2000 + SECOND, 300 * SECOND), CheckpointReason::AfterDataLoss);

		tracker.GapRetryNotBefore = Y2000 + 5 * SECOND;
		EXPECT_FALSE(decideCheckpoint(tracker, Y2000 + 4 * SECOND, 300 * SECOND).has_value());
		EXPECT_EQ(decideCheckpoint(tracker, Y2000 + 5 * SECOND, 300 * SECOND), CheckpointReason::AfterDataLoss);
	}

	TEST(CheckpointDecision, FutureReceivedAtDoesNotCauseACheckpointStorm) {
		// One update arrived with a time in the next month (a clock that is ahead): the clamped clock and the
		// checkpoint that follows are in that month. Later calls with the real time must not see a "new month"
		// every time.
		const YearMonth november{2026, 11};
		const Micros future = endOf(november) + 3600 * SECOND;   // December 1st, 01:00
		const Micros real = endOf(november) - 3600 * SECOND;     // November 30th, 23:00
		MonotonicClock clock;
		clock.clamp(future);
		CheckpointTracker tracker;
		tracker.HasCheckpoint = true;
		tracker.LastCheckpointAt = future;
		tracker.ChangedSinceCheckpoint = true;

		// The unclamped comparison: a different month, so every call would be due.
		EXPECT_EQ(decideCheckpoint(tracker, real, 300 * SECOND), CheckpointReason::NewMonth);
		// With the effective time it is not due until the interval has passed in the clamped time.
		EXPECT_EQ(effectiveNow(real, clock), future);
		EXPECT_FALSE(decideCheckpoint(tracker, effectiveNow(real, clock), 300 * SECOND).has_value());
		EXPECT_EQ(decideCheckpoint(tracker, effectiveNow(future + 300 * SECOND, clock), 300 * SECOND), CheckpointReason::Interval);
		// Real time ahead of the clock is used as it is.
		EXPECT_EQ(effectiveNow(future + 10, clock), future + 10);
		EXPECT_EQ(effectiveNow(5, MonotonicClock()), 5);
	}

	// ---------------------------------------------------------------------------------------------------------
	// Change numbers
	// ---------------------------------------------------------------------------------------------------------

	TEST(ChangeNoAllocatorTest, TakesBlocksInOrder) {
		int fetches = 0;
		int64_t nextStart = 1000;
		ChangeNoAllocator allocator(1000, [&] {
			++fetches;
			const auto start = nextStart;
			nextStart += 1000;
			return start;
		});
		EXPECT_EQ(fetches, 0);
		EXPECT_EQ(allocator.next(), 1000);
		EXPECT_EQ(fetches, 1);
		for (int i = 1; i < 1000; ++i)
			EXPECT_EQ(allocator.next(), 1000 + i);
		EXPECT_EQ(fetches, 1);
		EXPECT_EQ(allocator.next(), 2000);   // the next block
		EXPECT_EQ(fetches, 2);
	}

	TEST(ChangeNoAllocatorTest, FailedFetchLeavesTheAllocatorUsable) {
		int calls = 0;
		ChangeNoAllocator allocator(1000, [&]() -> int64_t {
			if (++calls == 1)
				throw std::runtime_error("database down");
			return 5000;
		});
		EXPECT_THROW(allocator.next(), std::runtime_error);
		EXPECT_EQ(allocator.next(), 5000);
		EXPECT_EQ(allocator.next(), 5001);
	}

	TEST(ChangeNoAllocatorTest, GapsBetweenBlocksKeepTheOrder) {
		// Another process (or a crash) may have used blocks in between: numbers only ever grow.
		std::vector<int64_t> starts{1000, 4000};
		size_t index = 0;
		ChangeNoAllocator allocator(2, [&] { return starts[index++]; });
		EXPECT_EQ(allocator.next(), 1000);
		EXPECT_EQ(allocator.next(), 1001);
		EXPECT_EQ(allocator.next(), 4000);
	}

	// ---------------------------------------------------------------------------------------------------------
	// Values
	// ---------------------------------------------------------------------------------------------------------

	TEST(ValueColumnsTest, ColumnsFollowTheEncoding) {
		VariableValue value;
		value.type = ValueType::Real;
		value.num = 1.5;
		auto columns = columnsOf(value);
		EXPECT_EQ(columns.Num, std::optional<double>(1.5));
		EXPECT_FALSE(columns.Text.has_value());
		EXPECT_FALSE(columns.Json.has_value());

		value = VariableValue();
		value.type = ValueType::Complex;
		value.json = R"({"imaginaryValue":1.0,"realValue":2.0})";
		columns = columnsOf(value);
		EXPECT_FALSE(columns.Num.has_value());
		EXPECT_EQ(columns.Json, value.json);

		value = VariableValue();
		value.type = ValueType::Integer;
		value.text = "9007199254740993";
		columns = columnsOf(value);
		EXPECT_EQ(columns.Text, value.text);
	}

	TEST(ValueColumnsTest, NonFiniteNumbersBecomeZero) {
		VariableValue value;
		value.num = std::numeric_limits<double>::quiet_NaN();
		EXPECT_EQ(columnsOf(value).Num, std::optional<double>(0.0));
		value.num = -std::numeric_limits<double>::infinity();
		EXPECT_EQ(columnsOf(value).Num, std::optional<double>(0.0));
	}

	TEST(ValueColumnsTest, NulBytesAreRemovedFromText) {
		VariableValue value;
		value.type = ValueType::String;
		value.text = std::string("a\0b\0", 4);
		const auto columns = columnsOf(value);
		ASSERT_TRUE(columns.Text.has_value());
		EXPECT_EQ(*columns.Text, "ab");
		EXPECT_EQ(columns.Text->find('\0'), std::string::npos);
		value.text = std::string(3, '\0');
		EXPECT_EQ(columnsOf(value).Text, std::optional<std::string>(""));
	}

	TEST(ValueColumnsTest, ReadBackKeepsTheTypeOfTheElement) {
		const auto value = valueFromColumns(ValueType::Rational, std::nullopt, "3/4", std::nullopt);
		EXPECT_EQ(value.type, ValueType::Rational);
		EXPECT_EQ(value.text, std::optional<std::string>("3/4"));
	}

	// ---------------------------------------------------------------------------------------------------------
	// Text helpers
	// ---------------------------------------------------------------------------------------------------------

	TEST(SqlSplit, SplitsAtSemicolonsOutsideQuotesAndComments) {
		const auto statements = splitSqlStatements(
			"-- a comment; with a semicolon\n"
			"CREATE TABLE a (x text DEFAULT 'a;b'); /* block; comment */ \n"
			"CREATE TABLE \"we;ird\" (y int)\n"
			";\n"
			"  ;  \n"
			"SELECT 'it''s; fine' -- trailing; comment\n");
		ASSERT_EQ(statements.size(), 3u);
		EXPECT_EQ(statements[0], "CREATE TABLE a (x text DEFAULT 'a;b')");
		EXPECT_EQ(statements[1], "CREATE TABLE \"we;ird\" (y int)");
		EXPECT_EQ(statements[2], "SELECT 'it''s; fine'");
	}

	TEST(SqlSplit, EmptyInputGivesNoStatements) {
		EXPECT_TRUE(splitSqlStatements("").empty());
		EXPECT_TRUE(splitSqlStatements("-- only a comment\n").empty());
	}

	std::string readFile(const std::string& path) {
		std::ifstream file(path, std::ios::binary);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	std::string withoutCarriageReturns(std::string text) {
		text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
		return text;
	}

	TEST(EmbeddedSchemaTest, IsTheTextOfTheSchemaFile) {
		const auto file = readFile(DT_STORE_SCHEMA_FILE);
		ASSERT_FALSE(file.empty()) << DT_STORE_SCHEMA_FILE;
		EXPECT_EQ(withoutCarriageReturns(embeddedSchemaSql()), withoutCarriageReturns(file));
	}

	TEST(EmbeddedSchemaTest, SplitsIntoTheNineStatementsOfTheSchema) {
		const auto statements = splitSqlStatements(embeddedSchemaSql());
		ASSERT_EQ(statements.size(), 9u);
		for (const auto& statement : statements) {
			EXPECT_EQ(statement.rfind("CREATE ", 0), 0u) << statement;
			EXPECT_EQ(statement.find("--"), std::string::npos) << statement;
		}
		EXPECT_NE(statements[7].find("PARTITION BY RANGE (received_at)"), std::string::npos);
	}

	TEST(Redaction, RemovesUrlAndPassword) {
		const std::string url = "postgresql://writer:p%40ss%2Fw0rd@db.example.org:5432/twins?sslmode=require";
		const auto cleaned = redactSecrets("connection to " + url + " failed: bad password p@ss/w0rd (p%40ss%2Fw0rd)", url);
		EXPECT_EQ(cleaned.find("p@ss/w0rd"), std::string::npos);
		EXPECT_EQ(cleaned.find("p%40ss"), std::string::npos);
		EXPECT_EQ(cleaned.find(url), std::string::npos);
		EXPECT_NE(cleaned.find("connection to"), std::string::npos);
	}

	TEST(Redaction, KeywordFormAndQueryParameter) {
		const std::string keywordForm = "host=h dbname=d password='my secret' user=u";
		EXPECT_EQ(redactSecrets("failed for my secret", keywordForm).find("my secret"), std::string::npos);
		const std::string parameter = "postgresql://u@h/d?password=hunter2&sslmode=require";
		EXPECT_EQ(redactSecrets("rejected hunter2", parameter).find("hunter2"), std::string::npos);
	}

	TEST(Redaction, PasswordWithUnencodedSeparators) {
		for (const std::string password : {"s3cr/et", "s3cr?et", "s3cr#et", "pa/ss?w#rd", "a:b/c", "x@y/z"}) {
			const std::string url = "postgresql://writer:" + password + "@db.example.org:5432/twins?sslmode=require";
			const auto cleaned = redactSecrets("could not parse " + url + " near " + password + " and " + password.substr(0, password.find_first_of("/?#")), url);
			EXPECT_EQ(cleaned.find(password), std::string::npos) << password << ": " << cleaned;
			EXPECT_EQ(cleaned.find("s3cr"), std::string::npos) << cleaned;
			EXPECT_EQ(cleaned.find("writer:"), std::string::npos) << cleaned;
		}
		// The host stays readable, it is what a message is about.
		EXPECT_NE(redactSecrets("cannot reach db.example.org", "postgresql://u:a/b@db.example.org/d").find("db.example.org"), std::string::npos);
	}

	TEST(Redaction, AtSignInTheQueryDoesNotHideTheRealPassword) {
		const std::string url = "postgresql://u:pw@host/db?user=a@b";
		const auto cleaned = redactSecrets("rejected pw for u at host, " + url + ", pw", url);
		EXPECT_EQ(cleaned.find("pw"), std::string::npos) << cleaned;
		EXPECT_NE(cleaned.find("host"), std::string::npos) << cleaned;
		// A password with a slash and an '@' after it in the query: both readings are removed.
		const std::string slashed = "postgresql://u:ab/cd@host/db?user=a@b";
		const auto cleanedSlashed = redactSecrets("bad ab/cd and ab", slashed);
		EXPECT_EQ(cleanedSlashed.find("ab/cd"), std::string::npos) << cleanedSlashed;
	}

	TEST(Redaction, UrlWithoutPasswordAndEmptyUrlLeaveTheMessage) {
		EXPECT_EQ(redactSecrets("no secret here", "postgresql://u@h/d"), "no secret here");
		EXPECT_EQ(redactSecrets("no secret here", ""), "no secret here");
	}

	TEST(Validation, Uuid) {
		EXPECT_TRUE(looksLikeUuid("123e4567-e89b-12d3-a456-426614174000"));
		EXPECT_TRUE(looksLikeUuid("123E4567-E89B-12D3-A456-426614174000"));
		EXPECT_FALSE(looksLikeUuid("123e4567e89b12d3a456426614174000"));
		EXPECT_FALSE(looksLikeUuid("123e4567-e89b-12d3-a456-42661417400"));
		EXPECT_FALSE(looksLikeUuid("123e4567-e89b-12d3-a456-42661417400g"));
		EXPECT_FALSE(looksLikeUuid("'; drop table twin; --"));
		EXPECT_FALSE(looksLikeUuid(""));
	}

	TEST(Validation, SchemaName) {
		EXPECT_TRUE(isValidSchemaName("dt_test_1"));
		EXPECT_TRUE(isValidSchemaName("_x"));
		EXPECT_FALSE(isValidSchemaName(""));
		EXPECT_FALSE(isValidSchemaName("1abc"));
		EXPECT_FALSE(isValidSchemaName("a-b"));
		EXPECT_FALSE(isValidSchemaName("a\"b"));
		EXPECT_FALSE(isValidSchemaName("a b"));
		EXPECT_FALSE(isValidSchemaName(std::string(64, 'a')));
		EXPECT_TRUE(isValidSchemaName(std::string(63, 'a')));
	}

	TEST(ErrorClassification, ConnectionAndResourceProblemsAreRetriedDataProblemsAreNot) {
		for (const char* transient : {"", "08006", "08001", "40001", "40P01", "53100", "53300", "55P03", "57014", "57P01"})
			EXPECT_EQ(classifySqlState(transient), ErrorClass::Transient) << transient;
		for (const char* data : {"22P02", "22001", "23505", "23514", "42P01", "42601", "42501", "XX000"})
			EXPECT_EQ(classifySqlState(data), ErrorClass::Data) << data;
	}

	// ---------------------------------------------------------------------------------------------------------
	// Queue
	// ---------------------------------------------------------------------------------------------------------

	QueueItem items(size_t rows, QueueItem::Kind kind = QueueItem::Kind::Changes, int32_t version = 1) {
		QueueItem item;
		item.ItemKind = kind;
		item.VersionNo = version;
		item.Rows.resize(rows);
		return item;
	}

	TEST(RowQueueTest, ChangeSetThatDoesNotFitIsRefusedWholeAndNothingChanges) {
		RowQueue queue(10);
		EXPECT_TRUE(queue.tryPushChanges(items(6)));
		EXPECT_FALSE(queue.tryPushChanges(items(5)));   // 6 + 5 > 10
		EXPECT_EQ(queue.pendingRows(), 6u);
		EXPECT_TRUE(queue.tryPushChanges(items(4)));    // exactly full
		EXPECT_EQ(queue.pendingRows(), 10u);
		EXPECT_FALSE(queue.tryPushChanges(items(1)));
		EXPECT_EQ(queue.queuedRows(), 10u);
	}

	TEST(RowQueueTest, SetLargerThanTheCapacityIsNeverAccepted) {
		RowQueue queue(3);
		EXPECT_FALSE(queue.tryPushChanges(items(4)));
		EXPECT_TRUE(queue.empty());
	}

	TEST(RowQueueTest, CheckpointIsAcceptedUnlessTheQueueIsFull) {
		RowQueue queue(10);
		EXPECT_TRUE(queue.tryPushChanges(items(9)));
		// Larger than the capacity, but the queue is not full: a checkpoint is as large as the model.
		EXPECT_TRUE(queue.tryPushCheckpoint(items(500, QueueItem::Kind::Checkpoint)));
		EXPECT_EQ(queue.pendingRows(), 509u);
		EXPECT_FALSE(queue.tryPushCheckpoint(items(1, QueueItem::Kind::Checkpoint)));   // full now
	}

	TEST(RowQueueTest, RowsStayPendingUntilFinished) {
		RowQueue queue(10);
		ASSERT_TRUE(queue.tryPushChanges(items(6)));
		ASSERT_TRUE(queue.tryPushChanges(items(4)));
		const auto batch = queue.popBatch(5);
		ASSERT_EQ(batch.size(), 1u);              // the first item already reaches 5 rows
		EXPECT_EQ(queue.queuedRows(), 4u);
		EXPECT_EQ(queue.pendingRows(), 10u);       // the writer still holds 6 rows
		EXPECT_FALSE(queue.tryPushChanges(items(1)));
		queue.finish(6);
		EXPECT_EQ(queue.pendingRows(), 4u);
		EXPECT_TRUE(queue.tryPushChanges(items(6)));
	}

	TEST(RowQueueTest, BatchIsInQueueOrderAndTakesAtLeastOneItem) {
		RowQueue queue(100);
		for (int32_t version = 1; version <= 4; ++version)
			ASSERT_TRUE(queue.tryPushChanges(items(3, QueueItem::Kind::Changes, version)));
		auto first = queue.popBatch(1);
		ASSERT_EQ(first.size(), 1u);
		EXPECT_EQ(first[0].VersionNo, 1);
		auto second = queue.popBatch(6);
		ASSERT_EQ(second.size(), 2u);
		EXPECT_EQ(second[0].VersionNo, 2);
		EXPECT_EQ(second[1].VersionNo, 3);
		auto third = queue.popBatch(1000);
		ASSERT_EQ(third.size(), 1u);
		EXPECT_EQ(third[0].VersionNo, 4);
		EXPECT_TRUE(queue.popBatch(10).empty());
		EXPECT_TRUE(queue.empty());
	}

	// ---------------------------------------------------------------------------------------------------------
	// The store without a database
	// ---------------------------------------------------------------------------------------------------------

	// Nothing listens on port 1 of the loopback address, the connection is refused at once.
	const char* UNREACHABLE_URL = "postgresql://nobody:S3cr3tPassw0rd@127.0.0.1:1/none";

	StoreSettings unreachableSettings(std::vector<std::string>& lines, std::mutex& linesMutex) {
		StoreSettings settings;
		settings.DatabaseUrl = UNREACHABLE_URL;
		settings.ConnectTimeoutSeconds = 1;
		settings.BackoffInitialMs = 20;
		settings.BackoffMaxMs = 50;
		settings.Log = [&lines, &linesMutex](const std::string& line) {
			std::lock_guard lock(linesMutex);
			lines.push_back(line);
		};
		return settings;
	}

	TEST(StoreWithoutDatabase, ConstructionDoesNotFailAndShutdownReturns) {
		std::vector<std::string> lines;
		std::mutex linesMutex;
		const auto begin = std::chrono::steady_clock::now();
		{
			DigitalTwinStore store(unreachableSettings(lines, linesMutex));
			const auto stats = store.stats();
			EXPECT_FALSE(stats.Connected);
			EXPECT_EQ(stats.Pending, 0u);
			store.shutdown(std::chrono::milliseconds(300));
			store.shutdown(std::chrono::milliseconds(300));   // later calls do nothing
		}
		EXPECT_LT(std::chrono::steady_clock::now() - begin, std::chrono::seconds(15));
		std::lock_guard lock(linesMutex);
		for (const auto& line : lines) {
			EXPECT_EQ(line.find("S3cr3tPassw0rd"), std::string::npos) << line;
			EXPECT_EQ(line.find(UNREACHABLE_URL), std::string::npos) << line;
		}
	}

	TEST(StoreWithoutDatabase, RegisterThrowsWithoutLeakingThePassword) {
		std::vector<std::string> lines;
		std::mutex linesMutex;
		DigitalTwinStore store(unreachableSettings(lines, linesMutex));
		try {
			store.registerTwin("123e4567-e89b-12d3-a456-426614174000", "123e4567-e89b-12d3-a456-426614174001", "twin",
			                   "123e4567-e89b-12d3-a456-426614174002", {});
			FAIL() << "registerTwin must fail without a database";
		}
		catch (const std::runtime_error& e) {
			const std::string message = e.what();
			EXPECT_FALSE(message.empty());
			EXPECT_EQ(message.find("S3cr3tPassw0rd"), std::string::npos) << message;
			EXPECT_EQ(message.find(UNREACHABLE_URL), std::string::npos) << message;
		}
		store.shutdown(std::chrono::milliseconds(300));
		std::lock_guard lock(linesMutex);
		for (const auto& line : lines)
			EXPECT_EQ(line.find("S3cr3tPassw0rd"), std::string::npos) << line;
	}

	TEST(StoreWithoutDatabase, MalformedIdsAndUnknownVersionsAreRejectedWithoutTouchingTheDatabase) {
		std::vector<std::string> lines;
		std::mutex linesMutex;
		DigitalTwinStore store(unreachableSettings(lines, linesMutex));
		EXPECT_THROW(store.registerTwin("x", "y", "twin", "z", {}), std::invalid_argument);
		const auto now = std::chrono::system_clock::now();
		EXPECT_FALSE(store.enqueueChanges(42, {}, now, now));
		EXPECT_FALSE(store.enqueueCheckpoint(42, CheckpointReason::Load, now, {}));
		EXPECT_FALSE(store.checkpointDue(42, now).has_value());
		store.shutdown(std::chrono::milliseconds(100));
	}

	TEST(StoreWithoutDatabase, InvalidSchemaNameIsRefused) {
		StoreSettings settings;
		settings.DatabaseUrl = UNREACHABLE_URL;
		settings.SchemaName = "bad name; drop schema public";
		EXPECT_THROW(DigitalTwinStore store(settings), std::invalid_argument);
	}
}
