// The logic of the store that needs no database: time and month arithmetic, partition names and DDL, retention,
// the clamp that keeps received_at monotonic, the checkpoint decision, the change number blocks, the encoding of
// values and timestamps for the COPY stream, splitting of the schema file and removal of secrets from messages.
// Kept apart from DigitalTwinStore.cpp so that all of it can be tested without PostgreSQL.
#pragma once

#include <chrono>
#include <compare>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "cpp_digital_twin_store_global.h"
#include "Model/State/StateTypes.h"

namespace DigitalTwin::Store {
	/** Microseconds since 1970-01-01 00:00:00 UTC. */
	using Micros = int64_t;

	constexpr Micros MICROS_PER_SECOND = 1'000'000;
	constexpr Micros MICROS_PER_DAY = 86'400 * MICROS_PER_SECOND;

	/** Why a checkpoint is written. The numbers are stored in checkpoint.reason. */
	enum class CheckpointReason : int16_t {
		Load = 1,
		Interval = 2,
		NewMonth = 3,       // first write into a new monthly partition
		AfterDataLoss = 4,
		Shutdown = 5
	};

	CPPDIGITALTWINSTORE_EXPORT Micros toMicros(std::chrono::system_clock::time_point time);
	CPPDIGITALTWINSTORE_EXPORT std::chrono::system_clock::time_point toTimePoint(Micros micros);

	// ---------------------------------------------------------------------------------------------------------
	// Months and partitions
	// ---------------------------------------------------------------------------------------------------------

	struct YearMonth {
		int Year = 1970;
		int Month = 1;   // 1..12
		friend bool operator==(const YearMonth&, const YearMonth&) = default;
		friend auto operator<=>(const YearMonth&, const YearMonth&) = default;
	};

	/** The month (UTC) that contains the time. */
	CPPDIGITALTWINSTORE_EXPORT YearMonth monthOf(Micros time);
	/** Months may be negative. */
	CPPDIGITALTWINSTORE_EXPORT YearMonth addMonths(YearMonth month, int months);
	/** First instant of the month (inclusive lower bound of its partition). */
	CPPDIGITALTWINSTORE_EXPORT Micros startOf(YearMonth month);
	/** First instant of the next month (exclusive upper bound of its partition). */
	CPPDIGITALTWINSTORE_EXPORT Micros endOf(YearMonth month);

	/** measurement_yYYYYmMM */
	CPPDIGITALTWINSTORE_EXPORT std::string partitionName(YearMonth month);
	/** The month of a name made by partitionName, nothing else. */
	CPPDIGITALTWINSTORE_EXPORT std::optional<YearMonth> parsePartitionName(std::string_view name);
	/** CREATE TABLE IF NOT EXISTS ... PARTITION OF measurement ...; built from numbers only. */
	CPPDIGITALTWINSTORE_EXPORT std::string createPartitionSql(YearMonth month);
	CPPDIGITALTWINSTORE_EXPORT std::string dropPartitionSql(YearMonth month);

	/** Time before which data is dropped: now - retentionDays. Empty for 0 (keep forever). */
	CPPDIGITALTWINSTORE_EXPORT std::optional<Micros> retentionCutoff(Micros now, uint32_t retentionDays);
	/** True if the whole partition lies before the cutoff. */
	CPPDIGITALTWINSTORE_EXPORT bool partitionExpired(YearMonth month, Micros cutoff);

	/** "YYYY-MM-DD HH:MM:SS.ffffff+00", accepted by a timestamptz column (also in COPY text format). */
	CPPDIGITALTWINSTORE_EXPORT std::string formatTimestamp(Micros time);

	// ---------------------------------------------------------------------------------------------------------
	// Monotonic received_at and the checkpoint decision
	// ---------------------------------------------------------------------------------------------------------

	/**
	 * Keeps received_at monotonic for one model version.
	 *
	 * The state at a time t is the newest row of every element, newest meaning the largest (received_at, change_no),
	 * and it starts at the last checkpoint before t. That is only the latest value if the order of the change
	 * numbers (the order of the updates in the model) and the order of received_at agree. The arrival times come
	 * from several concurrent tasks and from a clock that may step back, so a time that is older than the last one is
	 * raised to the last one.
	 */
	class CPPDIGITALTWINSTORE_EXPORT MonotonicClock {
	public:
		MonotonicClock() = default;
		explicit MonotonicClock(Micros last) : Last(last), HasLast(true) {}

		Micros clamp(Micros time);
		/** Raises the last time, for the times found in the database. */
		void seed(Micros time);
		std::optional<Micros> last() const;

	private:
		Micros Last = 0;
		bool HasLast = false;
	};

	/**
	 * The time for the checkpoint decision: `now`, but not before the last (clamped) time of the version. Without
	 * this a single received_at in the future, or a clock that steps back over a month boundary, would make every
	 * later call compare an older `now` with the clamped time of the last checkpoint and write a checkpoint each
	 * time until real time catches up.
	 */
	CPPDIGITALTWINSTORE_EXPORT Micros effectiveNow(Micros now, const MonotonicClock& clock);

	/** What the store knows about the checkpoints of one version. */
	struct CheckpointTracker {
		bool HasCheckpoint = false;
		Micros LastCheckpointAt = 0;
		bool ChangedSinceCheckpoint = false;
		/** Change sets were dropped (or a checkpoint was), the stored history has a gap. */
		bool Gap = false;
		/** After a refused checkpoint no new one is asked for before this time (time base of received_at). */
		Micros GapRetryNotBefore = 0;
	};

	/**
	 * Is a checkpoint due for the version, and why? Due if
	 *  - the history has a gap (AfterDataLoss), at most once per second while the queue refuses it, or
	 *  - the month (UTC) of `now` differs from the month of the last checkpoint (NewMonth), or
	 *  - the interval has passed since the last checkpoint and something changed since (Interval).
	 * A version without any checkpoint is due (Interval).
	 */
	CPPDIGITALTWINSTORE_EXPORT std::optional<CheckpointReason> decideCheckpoint(const CheckpointTracker& tracker, Micros now,
	                                                                           Micros intervalMicros);

	// ---------------------------------------------------------------------------------------------------------
	// Change numbers
	// ---------------------------------------------------------------------------------------------------------

	/**
	 * Hands out change numbers from blocks: `fetchBlockStart` returns the first number of a block (the sequence
	 * measurement_change_no_seq increments by 1000), the allocator uses blockSize numbers from it. A fetch that
	 * throws leaves the allocator unchanged.
	 */
	class CPPDIGITALTWINSTORE_EXPORT ChangeNoAllocator {
	public:
		ChangeNoAllocator(int64_t blockSize, std::function<int64_t()> fetchBlockStart);
		int64_t next();

	private:
		int64_t BlockSize;
		std::function<int64_t()> Fetch;
		int64_t Current = 0;
		int64_t End = 0;   // exclusive
	};

	// ---------------------------------------------------------------------------------------------------------
	// Values and text
	// ---------------------------------------------------------------------------------------------------------

	/** The three value columns of a row. */
	struct ValueColumns {
		std::optional<double> Num;
		std::optional<std::string> Text;
		std::optional<std::string> Json;
	};

	/**
	 * The columns of a value. A non-finite number cannot be stored and becomes 0. NUL bytes are removed from the
	 * text (a text column cannot hold them: 22021, which would fail the batch and every later checkpoint). A value
	 * without any column (which the model does not produce) becomes a NULL row.
	 */
	CPPDIGITALTWINSTORE_EXPORT ValueColumns columnsOf(const Model::VariableValue& value);

	/** The value that was read back from the columns, for the type that the element has. */
	CPPDIGITALTWINSTORE_EXPORT Model::VariableValue valueFromColumns(Model::ValueType type, std::optional<double> num,
	                                                                std::optional<std::string> text,
	                                                                std::optional<std::string> json);

	/**
	 * The statements of a SQL file: split at semicolons outside of quotes and comments, comments removed, empty
	 * statements left out. The schema file contains plain statements only (no dollar quoting).
	 */
	CPPDIGITALTWINSTORE_EXPORT std::vector<std::string> splitSqlStatements(std::string_view sql);

	/**
	 * `message` without the URL and without the password in it (also in its percent-encoded form).
	 */
	CPPDIGITALTWINSTORE_EXPORT std::string redactSecrets(std::string message, const std::string& databaseUrl);

	/** True for 8-4-4-4-12 hexadecimal digits. */
	CPPDIGITALTWINSTORE_EXPORT bool looksLikeUuid(std::string_view text);

	/** A schema name that can be used as search_path without quoting problems: [A-Za-z_][A-Za-z0-9_]{0,62}. */
	CPPDIGITALTWINSTORE_EXPORT bool isValidSchemaName(std::string_view name);

	// ---------------------------------------------------------------------------------------------------------
	// Errors
	// ---------------------------------------------------------------------------------------------------------

	enum class ErrorClass {
		/** The database or the connection is (temporarily) unavailable: retry for as long as it takes. */
		Transient,
		/** The database refuses the data or the statement: retry a bounded number of times, then drop the batch. */
		Data
	};

	/** Classifies an SQLSTATE (empty if the error has none, which is a connection problem). */
	CPPDIGITALTWINSTORE_EXPORT ErrorClass classifySqlState(std::string_view sqlState);
}
