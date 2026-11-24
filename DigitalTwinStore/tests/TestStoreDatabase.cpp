// Tests against a real PostgreSQL. They run only if DT_TEST_DATABASE_URL is set (a libpq URI of a database in which
// the role may create and drop schemas); otherwise each of them is reported as skipped and the run still passes.
// Every test works in a schema of its own with a unique name, which is dropped afterwards, so that the tests do not
// disturb each other or the data of the database.
//
// NOTE: written without a PostgreSQL at hand, so these tests have been compiled but never run by the author.
#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <pqxx/pqxx>

#include "DigitalTwinStore.h"
#include "Model/State/StateTypes.h"
#include "StoreLogic.h"

using namespace DigitalTwin::Store;
using namespace DigitalTwin::Model;
using Clock = std::chrono::system_clock;

namespace {
	const char* PROJECT = "11111111-1111-4111-8111-111111111111";
	const char* TWIN = "22222222-2222-4222-8222-222222222222";
	const char* COMMIT_A = "33333333-3333-4333-8333-333333333333";
	const char* COMMIT_B = "44444444-4444-4444-8444-444444444444";

	ModelElementInfo element(const std::string& path, const std::string& parent, ElementKind kind, std::optional<ValueType> type = std::nullopt) {
		ModelElementInfo info;
		info.path = path;
		info.parentPath = parent;
		info.name = path.substr(path.rfind('/') == std::string::npos ? 0 : path.rfind('/') + 1);
		info.kind = kind;
		info.valueType = type;
		return info;
	}

	/** Package Pkg with three variables, a component Pkg/inst below it with three more: 8 elements, 6 variables. */
	std::vector<ModelElementInfo> sampleTree() {
		return {
			element("Pkg", "", ElementKind::Package),
			element("Pkg/temp", "Pkg", ElementKind::Measurable, ValueType::Real),
			element("Pkg/count", "Pkg", ElementKind::Controllable, ValueType::Integer),
			element("Pkg/label", "Pkg", ElementKind::Attribute, ValueType::String),
			element("Pkg/inst", "Pkg", ElementKind::Component),
			element("Pkg/inst/ratio", "Pkg/inst", ElementKind::Measurable, ValueType::Rational),
			element("Pkg/inst/z", "Pkg/inst", ElementKind::Measurable, ValueType::Complex),
			element("Pkg/inst/flag", "Pkg/inst", ElementKind::Controllable, ValueType::Boolean),
		};
	}

	VariableValue real(double v) { VariableValue x; x.type = ValueType::Real; x.num = v; return x; }
	VariableValue integer(double v) { VariableValue x; x.type = ValueType::Integer; x.num = v; return x; }
	VariableValue boolean(bool v) { VariableValue x; x.type = ValueType::Boolean; x.num = v ? 1.0 : 0.0; return x; }
	VariableValue text(ValueType type, const std::string& v) { VariableValue x; x.type = type; x.text = v; return x; }
	VariableValue complex(const std::string& json) { VariableValue x; x.type = ValueType::Complex; x.json = json; return x; }

	std::vector<VariableState> initialState() {
		return {
			{"Pkg/temp", real(20.0)},
			{"Pkg/count", integer(1)},
			{"Pkg/label", text(ValueType::String, "initial")},
			{"Pkg/inst/ratio", text(ValueType::Rational, "1/2")},
			{"Pkg/inst/z", complex(R"({"imaginaryValue":0.0,"realValue":1.0})")},
			{"Pkg/inst/flag", boolean(false)},
		};
	}

	const VariableState* findState(const std::vector<VariableState>& state, const std::string& path) {
		for (const auto& entry : state)
			if (entry.path == path)
				return &entry;
		return nullptr;
	}

	Clock::time_point micros(Micros value) { return toTimePoint(value); }

	bool waitFor(const std::function<bool()>& condition, std::chrono::milliseconds limit = std::chrono::seconds(15)) {
		const auto end = std::chrono::steady_clock::now() + limit;
		while (std::chrono::steady_clock::now() < end) {
			if (condition())
				return true;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		return condition();
	}

	class StoreDatabase : public ::testing::Test {
	protected:
		void SetUp() override {
			const char* url = std::getenv("DT_TEST_DATABASE_URL");
			if (!url || !*url)
				GTEST_SKIP() << "SKIPPED: DT_TEST_DATABASE_URL is not set, the database test is skipped.";
			Url = url;
			std::random_device device;
			char name[48];
			std::snprintf(name, sizeof(name), "dt_test_%08x%08x", device(), device());
			Schema = name;
		}

		void TearDown() override {
			if (Schema.empty())
				return;
			try {
				pqxx::connection connection(Url);
				pqxx::work tx(connection);
				tx.exec("DROP SCHEMA IF EXISTS " + Schema + " CASCADE");
				tx.commit();
			}
			catch (const std::exception& e) {
				ADD_FAILURE() << "could not drop the test schema " << Schema << ": " << e.what();
			}
		}

		StoreSettings settings() const {
			StoreSettings s;
			s.DatabaseUrl = Url;
			s.SchemaName = Schema;
			s.FlushIntervalMs = 20;
			s.BatchSizeRows = 100;
			s.BackoffInitialMs = 100;
			s.BackoffMaxMs = 500;
			s.CheckpointIntervalSeconds = 300;
			s.Log = [](const std::string&) {};
			return s;
		}

		/** Runs a query in the schema of the test. */
		pqxx::result query(const std::string& sql) {
			pqxx::connection connection(Url);
			pqxx::work tx(connection);
			tx.exec("SET search_path TO " + Schema);
			auto result = tx.exec(sql);
			tx.commit();
			return result;
		}

		long long count(const std::string& sql) { return query(sql).one_field().as<long long>(); }

		std::string Url;
		std::string Schema;
	};

	TEST_F(StoreDatabase, NewCommitInsertsProjectTwinVersionAndTree) {
		DigitalTwinStore store(settings());
		const auto registration = store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree());

		EXPECT_FALSE(registration.KnownCommit);
		EXPECT_GT(registration.VersionNo, 0);
		EXPECT_TRUE(registration.LastState.empty());
		EXPECT_EQ(registration.ElementNoByPath.size(), 6u);   // variables only
		EXPECT_TRUE(registration.ElementNoByPath.count("Pkg/inst/z"));
		EXPECT_FALSE(registration.ElementNoByPath.count("Pkg"));

		EXPECT_EQ(count("SELECT count(*) FROM project"), 1);
		EXPECT_EQ(query("SELECT name FROM twin").one_field().as<std::string>(), "Boiler");
		EXPECT_EQ(count("SELECT count(*) FROM model_version"), 1);
		EXPECT_EQ(count("SELECT count(*) FROM element"), 8);
		EXPECT_EQ(count("SELECT count(*) FROM schema_version WHERE version = 1"), 1);

		// parent_no is resolved: the root has none, every other element points at the element of its parent path.
		EXPECT_EQ(count("SELECT count(*) FROM element WHERE parent_no IS NULL"), 1);
		EXPECT_EQ(count("SELECT count(*) FROM element c JOIN element p ON p.element_no = c.parent_no "
		                "WHERE c.path = 'Pkg/inst/ratio' AND p.path = 'Pkg/inst'"), 1);
		EXPECT_EQ(count("SELECT count(*) FROM element WHERE kind >= 4 AND value_type IS NOT NULL"), 6);
		EXPECT_EQ(count("SELECT count(*) FROM element WHERE kind < 4 AND value_type IS NULL"), 2);
		EXPECT_EQ(query("SELECT value_type FROM element WHERE path = 'Pkg/inst/z'").one_field().as<int>(), 5);
		store.shutdown(std::chrono::seconds(5));
	}

	TEST_F(StoreDatabase, SameCommitIsKnownAndKeepsItsVersion) {
		DigitalTwinStore store(settings());
		const auto first = store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree());
		const auto second = store.registerTwin(PROJECT, TWIN, "Boiler 2", COMMIT_A, sampleTree());

		EXPECT_TRUE(second.KnownCommit);
		EXPECT_EQ(second.VersionNo, first.VersionNo);
		EXPECT_EQ(second.ElementNoByPath, first.ElementNoByPath);
		EXPECT_TRUE(second.LastState.empty());   // no checkpoint was written yet
		EXPECT_EQ(count("SELECT count(*) FROM element"), 8);
		EXPECT_EQ(count("SELECT count(*) FROM twin"), 1);
		EXPECT_EQ(query("SELECT name FROM twin").one_field().as<std::string>(), "Boiler 2");   // the name is updated

		const auto other = store.registerTwin(PROJECT, TWIN, "Boiler 2", COMMIT_B, sampleTree());
		EXPECT_FALSE(other.KnownCommit);
		EXPECT_NE(other.VersionNo, first.VersionNo);
		EXPECT_EQ(count("SELECT count(*) FROM element"), 16);   // elements are per commit
		store.shutdown(std::chrono::seconds(5));
	}

	TEST_F(StoreDatabase, ElementsThatAreMissingAreAddedToAKnownCommit) {
		DigitalTwinStore store(settings());
		auto small = sampleTree();
		small.resize(5);   // Pkg, three variables, the component
		const auto first = store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, small);
		EXPECT_EQ(first.ElementNoByPath.size(), 3u);

		const auto second = store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree());
		EXPECT_TRUE(second.KnownCommit);
		EXPECT_EQ(second.ElementNoByPath.size(), 6u);
		EXPECT_EQ(count("SELECT count(*) FROM element"), 8);
		// The ones that existed keep their numbers.
		EXPECT_EQ(second.ElementNoByPath.at("Pkg/temp"), first.ElementNoByPath.at("Pkg/temp"));
		// The new ones are attached to the existing parent.
		EXPECT_EQ(count("SELECT count(*) FROM element c JOIN element p ON p.element_no = c.parent_no "
		                "WHERE c.path = 'Pkg/inst/flag' AND p.path = 'Pkg/inst'"), 1);
		store.shutdown(std::chrono::seconds(5));
	}

	TEST_F(StoreDatabase, ChangesAndCheckpointAreWrittenAndTheLastStateIsRestored) {
		const auto t0 = Clock::now();
		int32_t versionNo = 0;
		{
			DigitalTwinStore store(settings());
			const auto registration = store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree());
			versionNo = registration.VersionNo;

			ASSERT_TRUE(store.enqueueCheckpoint(versionNo, CheckpointReason::Load, t0, initialState()));
			// One change set: the client sets temp, the model derives count.
			ASSERT_TRUE(store.enqueueChanges(versionNo,
			                                 {{"Pkg/temp", real(21.5), false}, {"Pkg/count", integer(7), true}},
			                                 t0 + std::chrono::seconds(1), t0 + std::chrono::milliseconds(900)));
			// Another one with the values that need the other columns: exact integer digits, complex, rational, string.
			ASSERT_TRUE(store.enqueueChanges(versionNo,
			                                 {{"Pkg/temp", real(22.25), false},
			                                  {"Pkg/count", text(ValueType::Integer, "9007199254740993"), true},
			                                  {"Pkg/inst/z", complex(R"({"imaginaryValue":-2.5,"realValue":4.0})"), false},
			                                  {"Pkg/inst/ratio", text(ValueType::Rational, "-3/4"), false},
			                                  {"Pkg/label", text(ValueType::String, "h\xC3\xA4llo\tquote\"\\"), false}},
			                                 t0 + std::chrono::seconds(2), t0 + std::chrono::seconds(2)));
			store.shutdown(std::chrono::seconds(20));

			const auto stats = store.stats();
			EXPECT_EQ(stats.Written, 6u + 2u + 5u);
			EXPECT_EQ(stats.Dropped, 0u);
			EXPECT_EQ(stats.Pending, 0u);
			EXPECT_EQ(stats.Checkpoints, 1u);
		}

		// Rows: the checkpoint's six, the two change sets.
		EXPECT_EQ(count("SELECT count(*) FROM measurement"), 13);
		EXPECT_EQ(count("SELECT count(*) FROM measurement WHERE source = 3 AND checkpoint_no IS NOT NULL"), 6);
		// Client rows: one in the first change set, four in the second.
		EXPECT_EQ(count("SELECT count(*) FROM measurement WHERE source = 1"), 5);
		EXPECT_EQ(count("SELECT count(*) FROM measurement WHERE source = 2"), 2);
		EXPECT_EQ(count("SELECT count(*) FROM checkpoint WHERE variables = 6 AND reason = 1"), 1);
		// All rows of one change set share the number, the sets differ and follow the queue order.
		EXPECT_EQ(count("SELECT count(DISTINCT change_no) FROM measurement"), 3);
		EXPECT_EQ(count("SELECT count(DISTINCT change_no) FROM measurement WHERE source IN (1, 2) "
		                "AND ts = (SELECT min(ts) FROM measurement WHERE source IN (1, 2))"), 1);
		EXPECT_EQ(count("SELECT count(*) FROM measurement WHERE source IN (1, 2) "
		                "AND ts = (SELECT min(ts) FROM measurement WHERE source IN (1, 2))"), 2);
		const auto numbers = query("SELECT change_no FROM (SELECT change_no, min(received_at) AS r FROM measurement GROUP BY change_no) s ORDER BY r");
		ASSERT_EQ(numbers.size(), 3u);
		EXPECT_LT(numbers[0][0].as<long long>(), numbers[1][0].as<long long>());
		EXPECT_LT(numbers[1][0].as<long long>(), numbers[2][0].as<long long>());
		// The checkpoint's change number is the one of its rows.
		EXPECT_EQ(count("SELECT count(*) FROM checkpoint c JOIN measurement m ON m.checkpoint_no = c.checkpoint_no "
		                "WHERE m.change_no <> c.change_no"), 0);
		// The monthly partitions of the current and the next month exist.
		const auto now = toMicros(Clock::now());
		for (const auto month : {monthOf(now), addMonths(monthOf(now), 1)})
			EXPECT_EQ(count("SELECT count(*) FROM pg_class c JOIN pg_namespace n ON n.oid = c.relnamespace "
			                "WHERE n.nspname = '" + Schema + "' AND c.relname = '" + partitionName(month) + "'"), 1);

		// A second store: the same commit is known and its last state is the newest value of each variable.
		DigitalTwinStore second(settings());
		const auto registration = second.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree());
		EXPECT_TRUE(registration.KnownCommit);
		EXPECT_EQ(registration.VersionNo, versionNo);
		ASSERT_EQ(registration.LastState.size(), 6u);

		const auto* temp = findState(registration.LastState, "Pkg/temp");
		ASSERT_NE(temp, nullptr);
		EXPECT_EQ(temp->value.type, ValueType::Real);
		EXPECT_DOUBLE_EQ(*temp->value.num, 22.25);
		const auto* counter = findState(registration.LastState, "Pkg/count");
		ASSERT_NE(counter, nullptr);
		EXPECT_EQ(counter->value.type, ValueType::Integer);
		EXPECT_FALSE(counter->value.num.has_value());
		EXPECT_EQ(counter->value.text, std::optional<std::string>("9007199254740993"));
		const auto* ratio = findState(registration.LastState, "Pkg/inst/ratio");
		ASSERT_NE(ratio, nullptr);
		EXPECT_EQ(ratio->value.text, std::optional<std::string>("-3/4"));
		const auto* label = findState(registration.LastState, "Pkg/label");
		ASSERT_NE(label, nullptr);
		EXPECT_EQ(label->value.text, std::optional<std::string>("h\xC3\xA4llo\tquote\"\\"));
		const auto* z = findState(registration.LastState, "Pkg/inst/z");
		ASSERT_NE(z, nullptr);
		ASSERT_TRUE(z->value.json.has_value());
		EXPECT_NE(z->value.json->find("-2.5"), std::string::npos);   // jsonb normalises the text, not the content
		const auto* flag = findState(registration.LastState, "Pkg/inst/flag");
		ASSERT_NE(flag, nullptr);
		EXPECT_DOUBLE_EQ(*flag->value.num, 0.0);   // only in the checkpoint
		second.shutdown(std::chrono::seconds(5));
	}

	TEST_F(StoreDatabase, RowsBeforeALaterCheckpointDoNotShadowIt) {
		// The state of the checkpoint wins over rows that were written before it (smaller change number), even when
		// they have the same received_at.
		const auto t0 = Clock::now();
		DigitalTwinStore store(settings());
		const auto registration = store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree());
		const auto v = registration.VersionNo;
		ASSERT_TRUE(store.enqueueCheckpoint(v, CheckpointReason::Load, t0, initialState()));
		ASSERT_TRUE(store.enqueueChanges(v, {{"Pkg/temp", real(50.0), false}}, t0 + std::chrono::seconds(1), t0 + std::chrono::seconds(1)));
		auto state = initialState();
		state[0].value = real(60.0);   // the snapshot that follows the change
		ASSERT_TRUE(store.enqueueCheckpoint(v, CheckpointReason::Interval, t0 + std::chrono::seconds(1), state));
		store.shutdown(std::chrono::seconds(20));

		DigitalTwinStore second(settings());
		const auto again = second.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree());
		const auto* temp = findState(again.LastState, "Pkg/temp");
		ASSERT_NE(temp, nullptr);
		EXPECT_DOUBLE_EQ(*temp->value.num, 60.0);
		second.shutdown(std::chrono::seconds(5));
	}

	TEST_F(StoreDatabase, ReceivedAtIsMonotonicPerVersion) {
		const auto t0 = Clock::now();
		DigitalTwinStore store(settings());
		const auto v = store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree()).VersionNo;
		const auto later = t0 + std::chrono::seconds(10);
		ASSERT_TRUE(store.enqueueChanges(v, {{"Pkg/temp", real(1.0), false}}, later, later));
		// Arrives with an older time: stored with the last one, but its own ts stays.
		ASSERT_TRUE(store.enqueueChanges(v, {{"Pkg/temp", real(2.0), false}}, t0, t0));
		// A checkpoint is clamped as well.
		ASSERT_TRUE(store.enqueueCheckpoint(v, CheckpointReason::Interval, t0, initialState()));
		store.shutdown(std::chrono::seconds(20));

		EXPECT_EQ(count("SELECT count(DISTINCT received_at) FROM measurement"), 1);
		EXPECT_EQ(count("SELECT count(DISTINCT ts) FROM measurement WHERE source = 1"), 2);
		EXPECT_EQ(count("SELECT count(*) FROM checkpoint WHERE taken_at = (SELECT max(received_at) FROM measurement)"), 1);
	}

	TEST_F(StoreDatabase, CheckpointDueFollowsIntervalMonthAndGap) {
		auto s = settings();
		s.CheckpointIntervalSeconds = 1000;
		s.QueueCapacityRows = 3;
		DigitalTwinStore store(s);
		const auto v = store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree()).VersionNo;
		const auto start = micros(startOf(YearMonth{2030, 3}) + 5 * MICROS_PER_DAY);

		EXPECT_EQ(store.checkpointDue(v, start), CheckpointReason::Interval);   // none yet
		ASSERT_TRUE(store.enqueueCheckpoint(v, CheckpointReason::Load, start, {{"Pkg/temp", real(1.0)}}));
		EXPECT_FALSE(store.checkpointDue(v, start + std::chrono::seconds(5000)).has_value());   // idle

		ASSERT_TRUE(store.enqueueChanges(v, {{"Pkg/temp", real(2.0), false}}, start + std::chrono::seconds(10), start));
		EXPECT_FALSE(store.checkpointDue(v, start + std::chrono::seconds(999)).has_value());
		EXPECT_EQ(store.checkpointDue(v, start + std::chrono::seconds(1000)), CheckpointReason::Interval);
		EXPECT_EQ(store.checkpointDue(v, micros(startOf(YearMonth{2030, 4}))), CheckpointReason::NewMonth);

		// A change set that does not fit the queue (4 rows > 3) is dropped and leaves a gap.
		const auto before = store.stats().Dropped;
		EXPECT_FALSE(store.enqueueChanges(v,
		                                  {{"Pkg/temp", real(3.0), false}, {"Pkg/count", integer(3), true},
		                                   {"Pkg/inst/flag", boolean(true), true}, {"Pkg/label", text(ValueType::String, "x"), true}},
		                                  start + std::chrono::seconds(20), start));
		EXPECT_EQ(store.stats().Dropped, before + 4);
		EXPECT_EQ(store.checkpointDue(v, start + std::chrono::seconds(21)), CheckpointReason::AfterDataLoss);
		// The checkpoint that follows closes the gap.
		ASSERT_TRUE(store.enqueueCheckpoint(v, CheckpointReason::AfterDataLoss, start + std::chrono::seconds(21), initialState()));
		EXPECT_FALSE(store.checkpointDue(v, start + std::chrono::seconds(22)).has_value());
		store.shutdown(std::chrono::seconds(20));
		EXPECT_EQ(count("SELECT count(*) FROM checkpoint WHERE reason = 4"), 1);
	}

	TEST_F(StoreDatabase, MonthlyPartitionIsCreatedOnDemand) {
		DigitalTwinStore store(settings());
		const auto v = store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree()).VersionNo;
		// Far from the current and the next month: only the on-demand creation can have made the partition.
		const auto when = micros(startOf(YearMonth{2031, 7}) + 12345);
		ASSERT_TRUE(store.enqueueChanges(v, {{"Pkg/temp", real(5.0), false}}, when, when));
		store.shutdown(std::chrono::seconds(20));
		EXPECT_EQ(count("SELECT count(*) FROM measurement_y2031m07"), 1);
		EXPECT_EQ(store.stats().Dropped, 0u);
	}

	TEST_F(StoreDatabase, RetentionDropsOldPartitionsAndTheirCheckpoints) {
		{
			DigitalTwinStore store(settings());   // applies the schema
			store.registerTwin(PROJECT, TWIN, "Boiler", COMMIT_A, sampleTree());
			store.shutdown(std::chrono::seconds(5));
		}
		query("CREATE TABLE measurement_y2001m01 PARTITION OF measurement FOR VALUES FROM ('2001-01-01 00:00:00+00') TO ('2001-02-01 00:00:00+00')");
		const auto elementNo = count("SELECT min(element_no) FROM element");
		const auto versionNo = count("SELECT min(version_no) FROM model_version");
		query("INSERT INTO measurement (element_no, change_no, received_at, ts, source, checkpoint_no, value_num) VALUES (" +
		      std::to_string(elementNo) + ", 1, '2001-01-15 00:00:00+00', '2001-01-15 00:00:00+00', 1, NULL, 1.0)");
		query("INSERT INTO checkpoint (version_no, taken_at, change_no, reason, variables) VALUES (" + std::to_string(versionNo) +
		      ", '2001-01-10 00:00:00+00', 1, 1, 0)");
		ASSERT_EQ(count("SELECT count(*) FROM checkpoint"), 1);

		auto s = settings();
		s.RetentionDays = 30;
		DigitalTwinStore store(s);   // the writer connects and runs the maintenance
		EXPECT_TRUE(waitFor([&] { return count("SELECT count(*) FROM pg_class WHERE relname = 'measurement_y2001m01'") == 0; }));
		EXPECT_EQ(count("SELECT count(*) FROM checkpoint"), 0);
		// The partitions of the current and the next month stay.
		const auto now = toMicros(Clock::now());
		EXPECT_EQ(count("SELECT count(*) FROM pg_class c JOIN pg_namespace n ON n.oid = c.relnamespace "
		                "WHERE n.nspname = '" + Schema + "' AND c.relname = '" + partitionName(monthOf(now)) + "'"), 1);
		store.shutdown(std::chrono::seconds(5));
	}

	TEST_F(StoreDatabase, TwoStoresApplyTheSchemaOnceAtTheSameTime) {
		DigitalTwinStore a(settings());
		DigitalTwinStore b(settings());
		std::string errorA;
		std::string errorB;
		std::thread first([&] {
			try { a.registerTwin(PROJECT, TWIN, "A", COMMIT_A, sampleTree()); } catch (const std::exception& e) { errorA = e.what(); }
		});
		std::thread second([&] {
			try { b.registerTwin(PROJECT, "55555555-5555-4555-8555-555555555555", "B", COMMIT_A, sampleTree()); } catch (const std::exception& e) { errorB = e.what(); }
		});
		first.join();
		second.join();
		EXPECT_EQ(errorA, "");
		EXPECT_EQ(errorB, "");
		EXPECT_EQ(count("SELECT count(*) FROM schema_version"), 1);
		EXPECT_EQ(count("SELECT count(*) FROM twin"), 2);
		a.shutdown(std::chrono::seconds(5));
		b.shutdown(std::chrono::seconds(5));
	}

	TEST_F(StoreDatabase, StatsReportTheConnection) {
		DigitalTwinStore store(settings());
		EXPECT_TRUE(waitFor([&] { return store.stats().Connected; }));
		store.shutdown(std::chrono::seconds(5));
		EXPECT_FALSE(store.stats().Connected);
	}
}
