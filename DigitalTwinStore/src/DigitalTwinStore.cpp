#include "DigitalTwinStore.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include <pqxx/pqxx>

#include "EmbeddedSchema.h"
#include "RowQueue.h"

namespace DigitalTwin::Store {
	namespace {
		/** Key of the advisory lock that serialises the application of the schema. */
		constexpr int64_t SCHEMA_LOCK_KEY = 0x4454535431LL;   // "DTST1"
		/** Numbers per block of measurement_change_no_seq (INCREMENT BY 1000). */
		constexpr int64_t CHANGE_NO_BLOCK = 1000;
		constexpr int SCHEMA_VERSION = 1;
		constexpr size_t ELEMENT_INSERT_CHUNK = 500;
		/** After a refused checkpoint the next one is asked for after this time. */
		constexpr Micros GAP_RETRY_INTERVAL = MICROS_PER_SECOND;

		bool isVariable(Model::ElementKind kind)
		{
			return static_cast<int>(kind) >= static_cast<int>(Model::ElementKind::Measurable);
		}

		std::string lowerCase(std::string text)
		{
			std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return text;
		}
	}

	/** What the store knows about one model version, guarded by Impl::Mutex. */
	struct VersionState {
		std::unordered_map<std::string, int32_t> ElementNos;
		MonotonicClock Clock;
		CheckpointTracker Tracker;
	};

	struct DigitalTwinStore::Impl {
		explicit Impl(StoreSettings settings) :
			Settings(std::move(settings)),
			IntervalMicros(static_cast<Micros>(Settings.CheckpointIntervalSeconds) * MICROS_PER_SECOND),
			Queue(Settings.QueueCapacityRows)
		{
			if (Settings.BatchSizeRows == 0)
				Settings.BatchSizeRows = 1;
			if (Settings.BackoffInitialMs == 0)
				Settings.BackoffInitialMs = 1;
			if (Settings.BackoffMaxMs < Settings.BackoffInitialMs)
				Settings.BackoffMaxMs = Settings.BackoffInitialMs;
			if (Settings.MaxDataErrorAttempts == 0)
				Settings.MaxDataErrorAttempts = 1;
		}

		StoreSettings Settings;
		const Micros IntervalMicros;

		mutable std::mutex Mutex;   // guards Queue, Versions, Stopping, Deadline, WriterFinished
		std::condition_variable Wake;       // the writer waits on it
		std::condition_variable Finished;   // shutdown waits on it
		RowQueue Queue;
		std::map<int32_t, VersionState> Versions;
		/** Items (change sets and checkpoints) of a version that are queued or in the hands of the writer. */
		std::map<int32_t, size_t> PendingItems;
		std::condition_variable Drained;    // registerTwin waits on it
		bool Stopping = false;
		std::chrono::steady_clock::time_point Deadline;
		bool WriterFinished = false;
		bool ShutdownDone = false;
		bool Joined = false;

		std::atomic<bool> Connected{false};
		std::atomic<uint64_t> Written{0};
		std::atomic<uint64_t> Dropped{0};
		std::atomic<uint64_t> Retries{0};
		std::atomic<uint64_t> Checkpoints{0};

		std::thread Writer;

		// Only the writer thread:
		std::set<YearMonth> KnownPartitions;
		std::string LastLogged;
		std::chrono::steady_clock::time_point LastLoggedAt;

		// -----------------------------------------------------------------------------------------------------
		// Logging
		// -----------------------------------------------------------------------------------------------------

		void log(const std::string& message) const
		{
			const std::string line = "[DigitalTwinStore] " + message;
			if (Settings.Log)
			{
				try { Settings.Log(line); } catch (...) {}
				return;
			}
			std::cerr << line << std::endl;
		}

		std::string describe(const std::exception& e) const
		{
			return redactSecrets(e.what(), Settings.DatabaseUrl);
		}

		/** Logs a message unless the same one was logged less than a minute ago (the writer retries in a loop). */
		void logThrottled(const std::string& message)
		{
			const auto now = std::chrono::steady_clock::now();
			if (message == LastLogged && now - LastLoggedAt < std::chrono::minutes(1))
				return;
			LastLogged = message;
			LastLoggedAt = now;
			log(message);
		}

		// -----------------------------------------------------------------------------------------------------
		// Connection and schema
		// -----------------------------------------------------------------------------------------------------

		std::unique_ptr<pqxx::connection> openConnection()
		{
			std::map<std::string, std::string> parameters;
			const auto lowered = lowerCase(Settings.DatabaseUrl);
			if (lowered.find("connect_timeout") == std::string::npos)
				parameters["connect_timeout"] = std::to_string(std::max<uint32_t>(1, Settings.ConnectTimeoutSeconds));
			if (lowered.find("application_name") == std::string::npos)
				parameters["application_name"] = "digital-twin-store";
			// A network that dies silently must not hold a statement until the keepalive of the operating system
			// (hours). The server side limits are set below, these are the client side ones.
			const std::pair<const char*, const char*> keepAlives[] = {
				{"keepalives", "1"}, {"keepalives_idle", "30"}, {"keepalives_interval", "10"},
				{"keepalives_count", "3"}, {"tcp_user_timeout", "30000"}};
			for (const auto& [key, value] : keepAlives)
				if (lowered.find(key) == std::string::npos)
					parameters[key] = value;
			auto connection = std::make_unique<pqxx::connection>(Settings.DatabaseUrl, parameters);

			// Every database call has a time limit.
			pqxx::work tx(*connection);
			// The time limits first, so that they also apply to the advisory lock below.
			tx.exec("SET statement_timeout = " + std::to_string(Settings.StatementTimeoutMs));
			tx.exec("SET lock_timeout = " + std::to_string(Settings.LockTimeoutMs));
			tx.exec("SET idle_in_transaction_session_timeout = " + std::to_string(std::max<uint32_t>(Settings.StatementTimeoutMs, 60000)));
			if (!Settings.SchemaName.empty())   // CREATE SCHEMA IF NOT EXISTS is not safe against a concurrent one
				tx.exec("SELECT pg_advisory_xact_lock(" + std::to_string(SCHEMA_LOCK_KEY) + ")");
			if (!Settings.SchemaName.empty())
			{
				const auto quoted = connection->quote_name(Settings.SchemaName);
				tx.exec("CREATE SCHEMA IF NOT EXISTS " + quoted);
				tx.exec("SET search_path TO " + quoted);
			}
			tx.commit();
			return connection;
		}

		/**
		 * Applies sql/0001_schema.sql if schema_version does not list it. The advisory lock makes this safe against a
		 * second process that starts at the same time.
		 */
		void ensureSchema(pqxx::connection& connection)
		{
			pqxx::work tx(connection);
			tx.exec("SELECT pg_advisory_xact_lock(" + std::to_string(SCHEMA_LOCK_KEY) + ")");
			if (!tx.exec("SELECT to_regclass('schema_version') IS NOT NULL").one_field().as<bool>())
			{
				// Tables of ours without a version table are from another writer (the removed Rust recorder): applying
				// the schema would fail on the first CREATE TABLE, over and over. Say what is wrong, change nothing.
				for (const char* name : {"project", "twin", "model_version", "element", "checkpoint", "measurement"})
					if (tx.exec("SELECT to_regclass($1) IS NOT NULL", pqxx::params{std::string(name)}).one_field().as<bool>())
						throw std::runtime_error(
							std::string("the database has a table '") + name + "' but no schema_version: it is the schema of an "
							"older writer (the Rust recorder). The old tables (project, twin, model_version, element, checkpoint, "
							"measurement) must be dropped by hand before the store can create its schema; nothing was changed");
			}
			tx.exec("CREATE TABLE IF NOT EXISTS schema_version (version integer PRIMARY KEY, applied_at timestamptz NOT NULL DEFAULT now())");
			const auto applied = tx.exec("SELECT 1 FROM schema_version WHERE version = " + std::to_string(SCHEMA_VERSION));
			if (applied.empty())
			{
				for (const auto& statement : splitSqlStatements(embeddedSchemaSql()))
					tx.exec(statement);
				tx.exec("INSERT INTO schema_version (version) VALUES (" + std::to_string(SCHEMA_VERSION) + ")");
			}
			tx.commit();
		}

		void ensurePartition(pqxx::connection& connection, YearMonth month)
		{
			if (KnownPartitions.count(month))
				return;
			pqxx::work tx(connection);
			tx.exec(createPartitionSql(month));
			tx.commit();
			KnownPartitions.insert(month);
		}

		/** The partitions of the current and the next month: all that writing needs. */
		void ensureCurrentPartitions(pqxx::connection& connection)
		{
			const YearMonth current = monthOf(toMicros(std::chrono::system_clock::now()));
			ensurePartition(connection, current);
			ensurePartition(connection, addMonths(current, 1));
		}

		/**
		 * The periodic work: the partitions, and what the retention drops. A failure here (a lock that a reader
		 * holds on the table, a missing privilege) is logged by the caller and does not stop the writing.
		 */
		void maintain(pqxx::connection& connection)
		{
			ensureCurrentPartitions(connection);
			const Micros now = toMicros(std::chrono::system_clock::now());

			const auto cutoff = retentionCutoff(now, Settings.RetentionDays);
			if (!cutoff)
				return;

			std::vector<YearMonth> existing;
			{
				pqxx::work tx(connection);
				const auto names = tx.exec(
					"SELECT c.relname FROM pg_inherits i JOIN pg_class c ON c.oid = i.inhrelid "
					"WHERE i.inhparent = 'measurement'::regclass");
				for (const auto& row : names)
					if (const auto month = parsePartitionName(row[0].as<std::string>()))
						existing.push_back(*month);
				tx.commit();
			}
			std::sort(existing.begin(), existing.end());

			std::optional<YearMonth> oldestRemaining;
			for (const auto& month : existing)
			{
				if (partitionExpired(month, *cutoff))
				{
					pqxx::work tx(connection);
					tx.exec(dropPartitionSql(month));
					tx.commit();
					KnownPartitions.erase(month);
					log("retention: dropped partition " + partitionName(month));
				}
				else if (!oldestRemaining)
					oldestRemaining = month;
			}
			if (oldestRemaining)
			{
				// The checkpoints whose rows were in the dropped partitions are of no use any more.
				pqxx::work tx(connection);
				tx.exec("DELETE FROM checkpoint WHERE taken_at < $1::timestamptz",
				        pqxx::params{formatTimestamp(startOf(*oldestRemaining))});
				tx.commit();
			}
		}

		// -----------------------------------------------------------------------------------------------------
		// registerTwin
		// -----------------------------------------------------------------------------------------------------

		static std::map<std::string, int32_t> loadElementNos(pqxx::work& tx, int32_t versionNo)
		{
			std::map<std::string, int32_t> elementNos;
			const auto rows = tx.exec("SELECT path, element_no FROM element WHERE version_no = $1", pqxx::params{versionNo});
			for (const auto& row : rows)
				elementNos[row[0].as<std::string>()] = row[1].as<int32_t>();
			return elementNos;
		}

		/** Inserts the elements of the tree that the version does not have yet, parents first. */
		static void insertMissingElements(pqxx::work& tx, int32_t versionNo, const std::vector<Model::ModelElementInfo>& tree,
		                                  std::map<std::string, int32_t>& elementNos)
		{
			// Depth of every element, to insert level by level (a child needs the element_no of its parent).
			std::map<std::string, size_t> depthOf;
			std::vector<std::vector<const Model::ModelElementInfo*>> levels;
			for (const auto& element : tree)
			{
				if (depthOf.count(element.path))
					continue;   // a path is inserted once
				size_t depth = 0;
				if (!element.parentPath.empty())
				{
					const auto parent = depthOf.find(element.parentPath);
					if (parent == depthOf.end())
						throw std::invalid_argument("the model tree lists '" + element.path + "' before its parent");
					depth = parent->second + 1;
				}
				depthOf[element.path] = depth;
				if (levels.size() <= depth)
					levels.resize(depth + 1);
				levels[depth].push_back(&element);
			}

			for (const auto& level : levels)
			{
				std::vector<const Model::ModelElementInfo*> missing;
				for (const auto* element : level)
					if (!elementNos.count(element->path))
						missing.push_back(element);

				for (size_t begin = 0; begin < missing.size(); begin += ELEMENT_INSERT_CHUNK)
				{
					const size_t end = std::min(missing.size(), begin + ELEMENT_INSERT_CHUNK);
					std::string sql = "INSERT INTO element (version_no, parent_no, kind, name, path, value_type) VALUES ";
					pqxx::params parameters;
					for (size_t i = begin; i < end; ++i)
					{
						const auto& element = *missing[i];
						const size_t base = (i - begin) * 6;
						if (i > begin)
							sql += ",";
						sql += "($" + std::to_string(base + 1) + "::integer,$" + std::to_string(base + 2) + "::integer,$" +
							std::to_string(base + 3) + "::smallint,$" + std::to_string(base + 4) + "::text,$" +
							std::to_string(base + 5) + "::text,$" + std::to_string(base + 6) + "::smallint)";
						parameters.append(versionNo);
						if (element.parentPath.empty())
							parameters.append();
						else
							parameters.append(elementNos.at(element.parentPath));
						parameters.append(static_cast<int>(element.kind));
						parameters.append(element.name);
						parameters.append(element.path);
						if (element.valueType.has_value())
							parameters.append(static_cast<int>(*element.valueType));
						else
							parameters.append();
					}
					sql += " ON CONFLICT (version_no, path) DO NOTHING";
					tx.exec(sql, parameters);
				}
				if (!missing.empty())
					elementNos = loadElementNos(tx, versionNo);   // the children of the next level need them
			}
		}

		/** The newest checkpoint of the version and every row after it, newest per element. */
		static std::vector<Model::VariableState> loadLastState(pqxx::work& tx, int32_t versionNo, std::optional<Micros>& newestTime)
		{
			// change_no >= the checkpoint's: rows of change sets that were written before the checkpoint (and are
			// part of it) share its received_at but are not newer than it.
			const auto rows = tx.exec(
				"WITH cp AS (SELECT taken_at, change_no FROM checkpoint WHERE version_no = $1 "
				"            ORDER BY taken_at DESC, checkpoint_no DESC LIMIT 1) "
				"SELECT DISTINCT ON (m.element_no) e.path, e.value_type, m.value_num, m.value_text, m.value_json::text, "
				"       (extract(epoch from m.received_at) * 1000000)::bigint, "
				"       (extract(epoch from cp.taken_at) * 1000000)::bigint "
				"FROM cp "
				"JOIN measurement m ON m.received_at >= cp.taken_at AND m.change_no >= cp.change_no "
				"JOIN element e ON e.element_no = m.element_no AND e.version_no = $1 "
				"ORDER BY m.element_no, m.received_at DESC, m.change_no DESC",
				pqxx::params{versionNo});
			std::vector<Model::VariableState> state;
			state.reserve(rows.size());
			for (const auto& row : rows)
			{
				if (row[1].is_null())
					continue;   // not a variable
				Model::VariableState variable;
				variable.path = row[0].as<std::string>();
				variable.value = valueFromColumns(static_cast<Model::ValueType>(row[1].as<int>()),
				                                  row[2].as<std::optional<double>>(), row[3].as<std::optional<std::string>>(),
				                                  row[4].as<std::optional<std::string>>());
				state.push_back(std::move(variable));
				for (const size_t column : {5, 6})
				{
					const auto time = row[static_cast<pqxx::row::size_type>(column)].as<std::optional<int64_t>>();
					if (time && (!newestTime || *time > *newestTime))
						newestTime = *time;
				}
			}
			return state;
		}

		/**
		 * Waits, for at most RegisterDrainWaitMs, until the queue and the writer hold no item of the version. If the
		 * time runs out (the database is slow or down) the caller goes on with what is committed: the queued tail then
		 * sorts before the checkpoint that the caller writes next and is hidden by it, which loses that tail but keeps
		 * the model and the database consistent.
		 */
		void waitForDrain(int32_t versionNo)
		{
			bool drained;
			{
				std::unique_lock lock(Mutex);
				drained = Drained.wait_for(lock, std::chrono::milliseconds(Settings.RegisterDrainWaitMs), [&] {
					const auto pending = PendingItems.find(versionNo);
					return pending == PendingItems.end() || pending->second == 0 || WriterFinished;
				});
			}
			if (!drained)
				log("version " + std::to_string(versionNo) + ": rows are still queued, the state is restored from what is committed");
		}

		/** The rows of a batch are written or dropped: they leave the queue and the pending counts. */
		void releaseBatch(const std::vector<QueueItem>& batch, size_t rows, bool markGap)
		{
			std::lock_guard lock(Mutex);
			Queue.finish(rows);
			for (const auto& item : batch)
			{
				const auto pending = PendingItems.find(item.VersionNo);
				if (pending != PendingItems.end() && --pending->second == 0)
					PendingItems.erase(pending);
				if (markGap)
				{
					const auto version = Versions.find(item.VersionNo);
					if (version != Versions.end())
						version->second.Tracker.Gap = true;   // so that a checkpoint after the data loss follows
				}
			}
			Drained.notify_all();
		}

		TwinRegistration registerTwin(const std::string& projectId, const std::string& twinId, const std::string& twinName,
		                              const std::string& commitId, const std::vector<Model::ModelElementInfo>& tree,
		                              bool readLastState)
		{
			if (!looksLikeUuid(projectId) || !looksLikeUuid(twinId) || !looksLikeUuid(commitId))
				throw std::invalid_argument("project id, twin id and commit id must be UUIDs");
			{
				std::lock_guard lock(Mutex);
				if (Stopping)
					throw std::runtime_error("the store is shut down");
			}

			try
			{
				auto connection = openConnection();
				ensureSchema(*connection);

				pqxx::work tx(*connection);
				tx.exec("INSERT INTO project (project_id) VALUES ($1::uuid) ON CONFLICT (project_id) DO NOTHING",
				        pqxx::params{projectId});
				const auto twinNo = tx.exec(
					"INSERT INTO twin (twin_id, project_id, name) VALUES ($1::uuid, $2::uuid, $3) "
					"ON CONFLICT (twin_id) DO UPDATE SET name = EXCLUDED.name, project_id = EXCLUDED.project_id, last_seen = now() "
					"RETURNING twin_no",
					pqxx::params{twinId, projectId, twinName}).one_field().as<int32_t>();
				// xmax is 0 for a row that was inserted, not for one that was updated.
				const auto version = tx.exec(
					"INSERT INTO model_version (twin_no, commit_id) VALUES ($1, $2::uuid) "
					"ON CONFLICT (twin_no, commit_id) DO UPDATE SET last_loaded = now() "
					"RETURNING version_no, (xmax = 0) AS inserted",
					pqxx::params{twinNo, commitId}).one_row();

				TwinRegistration registration;
				registration.VersionNo = version[0].as<int32_t>();
				registration.KnownCommit = !version[1].as<bool>();

				auto elementNos = loadElementNos(tx, registration.VersionNo);
				insertMissingElements(tx, registration.VersionNo, tree, elementNos);
				tx.commit();

				std::optional<Micros> newestTime;
				if (registration.KnownCommit && readLastState)
				{
					// The last state must include what this store has already accepted for the version (for example
					// the changes of a model that is being replaced by this load): they are in the queue, not yet in
					// the database. A state read before them would be restored into the new model and a checkpoint of
					// it would hide the newer rows. So wait until nothing of the version is queued, then read in a
					// new transaction (a statement sees what was committed up to its start).
					waitForDrain(registration.VersionNo);
					pqxx::work readTx(*connection);
					registration.LastState = loadLastState(readTx, registration.VersionNo, newestTime);
					readTx.commit();
				}

				std::unordered_map<std::string, int32_t> variableNos;
				for (const auto& element : tree)
				{
					if (!isVariable(element.kind))
						continue;
					const auto found = elementNos.find(element.path);
					if (found != elementNos.end())
					{
						registration.ElementNoByPath[element.path] = found->second;
						variableNos[element.path] = found->second;
					}
				}

				std::lock_guard lock(Mutex);
				auto& state = Versions[registration.VersionNo];
				state.ElementNos = std::move(variableNos);
				state.Tracker = CheckpointTracker();   // the caller writes the checkpoint of the load
				if (newestTime)
					state.Clock.seed(*newestTime);
				return registration;
			}
			catch (const std::invalid_argument&)
			{
				throw;
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error("registering the twin failed: " + describe(e));
			}
		}

		// -----------------------------------------------------------------------------------------------------
		// Queue side
		// -----------------------------------------------------------------------------------------------------

		bool enqueueChanges(int32_t versionNo, const std::vector<Model::VariableChange>& changes,
		                    std::chrono::system_clock::time_point receivedAt, std::chrono::system_clock::time_point ts)
		{
			std::lock_guard lock(Mutex);
			if (Stopping)
				return false;
			const auto found = Versions.find(versionNo);
			if (found == Versions.end())
				return false;
			auto& version = found->second;

			QueueItem item;
			item.ItemKind = QueueItem::Kind::Changes;
			item.VersionNo = versionNo;
			item.ReceivedAt = version.Clock.clamp(toMicros(receivedAt));
			item.Ts = toMicros(ts);
			item.Rows.reserve(changes.size());
			for (const auto& change : changes)
			{
				const auto element = version.ElementNos.find(change.path);
				if (element == version.ElementNos.end())
					continue;   // not part of the tree that was registered
				item.Rows.push_back(QueuedRow{element->second, static_cast<int16_t>(change.derived ? 2 : 1), change.value});
			}
			if (item.Rows.empty())
				return true;

			const size_t rows = item.Rows.size();
			if (!Queue.tryPushChanges(std::move(item)))
			{
				Dropped += rows;
				version.Tracker.Gap = true;
				return false;
			}
			version.Tracker.ChangedSinceCheckpoint = true;
			++PendingItems[versionNo];
			if (Queue.queuedRows() >= Settings.BatchSizeRows)
				Wake.notify_one();
			return true;
		}

		bool enqueueCheckpoint(int32_t versionNo, CheckpointReason reason, std::chrono::system_clock::time_point takenAt,
		                       const std::vector<Model::VariableState>& state)
		{
			std::lock_guard lock(Mutex);
			if (Stopping)
				return false;
			const auto found = Versions.find(versionNo);
			if (found == Versions.end())
				return false;
			auto& version = found->second;

			QueueItem item;
			item.ItemKind = QueueItem::Kind::Checkpoint;
			item.VersionNo = versionNo;
			item.ReceivedAt = version.Clock.clamp(toMicros(takenAt));
			item.Ts = item.ReceivedAt;
			item.Reason = static_cast<int16_t>(reason);
			item.Rows.reserve(state.size());
			for (const auto& variable : state)
			{
				const auto element = version.ElementNos.find(variable.path);
				if (element == version.ElementNos.end())
					continue;
				item.Rows.push_back(QueuedRow{element->second, 3, variable.value});
			}

			const size_t rows = item.Rows.size();
			const Micros stamp = item.ReceivedAt;
			if (!Queue.tryPushCheckpoint(std::move(item)))
			{
				Dropped += rows;
				version.Tracker.Gap = true;
				version.Tracker.GapRetryNotBefore = stamp + GAP_RETRY_INTERVAL;
				return false;
			}
			version.Tracker.HasCheckpoint = true;
			version.Tracker.LastCheckpointAt = stamp;
			version.Tracker.ChangedSinceCheckpoint = false;
			version.Tracker.Gap = false;
			++PendingItems[versionNo];
			Wake.notify_one();
			return true;
		}

		std::optional<CheckpointReason> checkpointDue(int32_t versionNo, std::chrono::system_clock::time_point receivedAt)
		{
			std::lock_guard lock(Mutex);
			if (Stopping)
				return std::nullopt;
			const auto found = Versions.find(versionNo);
			if (found == Versions.end())
				return std::nullopt;
			return decideCheckpoint(found->second.Tracker, effectiveNow(toMicros(receivedAt), found->second.Clock), IntervalMicros);
		}

		StoreStats stats() const
		{
			StoreStats result;
			result.Connected = Connected.load();
			{
				std::lock_guard lock(Mutex);
				result.Pending = Queue.pendingRows();
			}
			result.Written = Written.load();
			result.Dropped = Dropped.load();
			result.Retries = Retries.load();
			result.Checkpoints = Checkpoints.load();
			return result;
		}

		// -----------------------------------------------------------------------------------------------------
		// Writer thread
		// -----------------------------------------------------------------------------------------------------

		void sleepInterruptible(uint32_t milliseconds)
		{
			std::unique_lock lock(Mutex);
			if (Stopping)
				Wake.wait_for(lock, std::chrono::milliseconds(std::min<uint32_t>(milliseconds, 200)));   // drain as long as the deadline allows
			else
				Wake.wait_for(lock, std::chrono::milliseconds(milliseconds), [this] { return Stopping; });
		}

		static int64_t nextChangeNoBlock(pqxx::connection& connection)
		{
			pqxx::work tx(connection);
			const auto start = tx.exec("SELECT nextval('measurement_change_no_seq')").one_field().as<int64_t>();
			tx.commit();
			return start;
		}

		/** Writes the batch in one transaction. Throws on every error, nothing is written then. */
		void writeBatch(pqxx::connection& connection, std::vector<QueueItem>& batch, ChangeNoAllocator& changeNos)
		{
			// The numbers follow the queue order and stay with the item if the batch has to be retried.
			for (auto& item : batch)
				if (item.ChangeNo < 0)
					item.ChangeNo = changeNos.next();

			// A partition must exist before a row for it is written. DDL commits on its own so that it survives a
			// failed batch.
			std::set<YearMonth> months;
			for (const auto& item : batch)
				months.insert(monthOf(item.ReceivedAt));
			for (const auto& month : months)
				ensurePartition(connection, month);

			pqxx::work tx(connection);
			const std::initializer_list<std::string_view> columns{"element_no", "change_no", "received_at", "ts", "source",
			                                                     "checkpoint_no", "value_num", "value_text", "value_json"};
			size_t changeRows = 0;
			for (const auto& item : batch)
				if (item.ItemKind == QueueItem::Kind::Changes)
					changeRows += item.Rows.size();
			if (changeRows > 0)
			{
				auto stream = pqxx::stream_to::table(tx, {"measurement"}, columns);
				for (const auto& item : batch)
				{
					if (item.ItemKind != QueueItem::Kind::Changes)
						continue;
					const auto receivedAt = formatTimestamp(item.ReceivedAt);
					const auto ts = formatTimestamp(item.Ts);
					for (const auto& row : item.Rows)
					{
						const auto value = columnsOf(row.Value);
						stream.write_values(row.ElementNo, item.ChangeNo, receivedAt, ts, static_cast<int>(row.Source),
						                    std::optional<int64_t>(), value.Num, value.Text, value.Json);
					}
				}
				stream.complete();
			}

			for (const auto& item : batch)
			{
				if (item.ItemKind != QueueItem::Kind::Checkpoint)
					continue;
				const auto takenAt = formatTimestamp(item.ReceivedAt);
				const auto checkpointNo = tx.exec(
					"INSERT INTO checkpoint (version_no, taken_at, change_no, reason, variables) "
					"VALUES ($1, $2::timestamptz, $3, $4, $5) RETURNING checkpoint_no",
					pqxx::params{item.VersionNo, takenAt, item.ChangeNo, static_cast<int>(item.Reason),
					             static_cast<int>(item.Rows.size())}).one_field().as<int64_t>();
				if (item.Rows.empty())
					continue;
				auto stream = pqxx::stream_to::table(tx, {"measurement"}, columns);
				for (const auto& row : item.Rows)
				{
					const auto value = columnsOf(row.Value);
					stream.write_values(row.ElementNo, item.ChangeNo, takenAt, takenAt, static_cast<int>(row.Source),
					                    std::optional<int64_t>(checkpointNo), value.Num, value.Text, value.Json);
				}
				stream.complete();
			}
			tx.commit();
		}

		static size_t rowsOf(const std::vector<QueueItem>& batch)
		{
			size_t rows = 0;
			for (const auto& item : batch)
				rows += item.Rows.size();
			return rows;
		}

		void writerLoop()
		{
			std::unique_ptr<pqxx::connection> connection;
			std::vector<QueueItem> batch;
			// Items of a batch that the database refused, written one by one so that only a bad item is lost.
			std::deque<QueueItem> carry;
			uint32_t dataAttempts = 0;
			uint32_t backoffMs = Settings.BackoffInitialMs;
			auto nextMaintenance = std::chrono::steady_clock::now();
			ChangeNoAllocator changeNos(CHANGE_NO_BLOCK, [&connection] { return nextChangeNoBlock(*connection); });

			const auto onTransient = [&](const std::exception& e, const char* what) {
				if (!connection || !connection->is_open())
					connection.reset();
				Connected = false;
				++Retries;
				logThrottled(std::string(what) + ": " + describe(e));
				sleepInterruptible(backoffMs);
				backoffMs = std::min(Settings.BackoffMaxMs, backoffMs * 2);
			};

			for (;;)
			{
				{
					std::lock_guard lock(Mutex);
					if (Stopping && (std::chrono::steady_clock::now() >= Deadline || (Queue.empty() && batch.empty() && carry.empty())))
						return;
				}

				if (!connection)
				{
					try
					{
						connection = openConnection();
						ensureSchema(*connection);
						KnownPartitions.clear();
						ensureCurrentPartitions(*connection);
						nextMaintenance = std::chrono::steady_clock::now();   // the retention runs on the next turn
						Connected = true;
						backoffMs = Settings.BackoffInitialMs;
						log("connected to the database");
					}
					catch (const std::exception& e)
					{
						onTransient(e, "cannot connect");
						connection.reset();
						continue;
					}
				}

				if (std::chrono::steady_clock::now() >= nextMaintenance)
				{
					try
					{
						maintain(*connection);
					}
					catch (const std::exception& e)
					{
						logThrottled(std::string("maintenance failed: ") + describe(e));
						if (!connection->is_open())
						{
							connection.reset();
							Connected = false;
							continue;
						}
					}
					nextMaintenance = std::chrono::steady_clock::now() + std::chrono::seconds(Settings.MaintenanceIntervalSeconds);
				}

				if (batch.empty() && !carry.empty())
				{
					batch.push_back(std::move(carry.front()));
					carry.pop_front();
				}
				if (batch.empty())
				{
					std::unique_lock lock(Mutex);
					if (!Stopping && Queue.queuedRows() < Settings.BatchSizeRows)
						Wake.wait_for(lock, std::chrono::milliseconds(Settings.FlushIntervalMs),
						              [this] { return Stopping || Queue.queuedRows() >= Settings.BatchSizeRows; });
					if (Queue.empty())
						continue;
					batch = Queue.popBatch(Settings.BatchSizeRows);
				}

				const size_t rows = rowsOf(batch);
				try
				{
					writeBatch(*connection, batch, changeNos);
					size_t checkpoints = 0;
					for (const auto& item : batch)
						if (item.ItemKind == QueueItem::Kind::Checkpoint)
							++checkpoints;
					Written += rows;
					Checkpoints += checkpoints;
					releaseBatch(batch, rows, false);
					batch.clear();
					dataAttempts = 0;
					backoffMs = Settings.BackoffInitialMs;
				}
				catch (const pqxx::broken_connection& e)
				{
					onTransient(e, "connection lost");
				}
				catch (const pqxx::in_doubt_error& e)
				{
					// The commit may or may not have happened. The batch is written again: a row that exists twice
					// does not change the state, which takes the newest row of an element.
					connection.reset();
					onTransient(e, "commit outcome unknown");
				}
				catch (const pqxx::sql_error& e)
				{
					KnownPartitions.clear();
					if (classifySqlState(e.sqlstate()) == ErrorClass::Transient)
						onTransient(e, "database busy");
					else
					{
						if (e.sqlstate() == "42P01")
							connection.reset();   // a table is gone: connect again, which checks the schema
						failBatchOnData(batch, rows, dataAttempts, carry, e);
					}
				}
				catch (const std::exception& e)
				{
					failBatchOnData(batch, rows, dataAttempts, carry, e);
				}
			}
		}

		/**
		 * The database refuses a batch for a data reason. A batch of several items is split: its items are written
		 * one by one (`carry`), so that a bad item does not take the rows of other twins with it. An item that is
		 * refused MaxDataErrorAttempts times is dropped, counted, and its version is marked as having a gap.
		 */
		void failBatchOnData(std::vector<QueueItem>& batch, size_t rows, uint32_t& attempts, std::deque<QueueItem>& carry,
		                     const std::exception& e)
		{
			if (batch.size() > 1)
			{
				for (auto& item : batch)
					carry.push_back(std::move(item));
				batch.clear();
				attempts = 0;
				++Retries;
				logThrottled("batch refused, writing its items one by one: " + describe(e));
				return;
			}
			++attempts;
			if (attempts >= Settings.MaxDataErrorAttempts)
			{
				Dropped += rows;
				releaseBatch(batch, rows, true);
				batch.clear();
				attempts = 0;
				log("dropped " + std::to_string(rows) + " rows that the database refuses: " + describe(e));
				return;
			}
			++Retries;
			logThrottled("batch refused, trying again: " + describe(e));
			sleepInterruptible(Settings.BackoffInitialMs);
		}

		void writerMain()
		{
			try
			{
				writerLoop();
			}
			catch (const std::exception& e)
			{
				log(std::string("the writer stopped: ") + describe(e));
			}
			catch (...)
			{
				log("the writer stopped on an unknown error");
			}
			Connected = false;
			std::lock_guard lock(Mutex);
			// What is left was not written: count it. (A batch in the hands of the writer is part of pendingRows.)
			Dropped += Queue.pendingRows();
			Queue.finish(Queue.pendingRows());
			PendingItems.clear();
			WriterFinished = true;
			Finished.notify_all();
			Drained.notify_all();
		}

		void start()
		{
			Writer = std::thread([this] { writerMain(); });
		}

		/** @return true if the writer thread was joined (so that the object may be deleted). */
		bool shutdown(std::chrono::milliseconds timeout)
		{
			{
				std::lock_guard lock(Mutex);
				if (ShutdownDone)
					return Joined;
				ShutdownDone = true;
				Stopping = true;
				Deadline = std::chrono::steady_clock::now() + timeout;
			}
			Wake.notify_all();

			// The writer stops at the deadline, after the statement that it is in (bounded by statement_timeout).
			// If even that does not come back in time, the thread is detached and the object is left to the process.
			std::unique_lock lock(Mutex);
			const bool finished = Finished.wait_until(lock, Deadline + std::chrono::seconds(2), [this] { return WriterFinished; });
			lock.unlock();
			if (finished && Writer.joinable())
			{
				Writer.join();
				Joined = true;
				return true;
			}
			if (Writer.joinable())
				Writer.detach();
			return false;
		}
	};

	namespace {
		StoreSettings validated(StoreSettings settings)
		{
			if (!settings.SchemaName.empty() && !isValidSchemaName(settings.SchemaName))
				throw std::invalid_argument("invalid schema name");
			return settings;
		}
	}

	DigitalTwinStore::DigitalTwinStore(StoreSettings settings) : Pimpl(new Impl(validated(std::move(settings))))
	{
		Pimpl->start();
	}

	DigitalTwinStore::~DigitalTwinStore()
	{
		// A writer that does not return (a database call that hangs beyond its timeouts) is detached and Impl is
		// deliberately leaked: the thread still uses it.
		if (Pimpl->shutdown(std::chrono::seconds(2)))
			delete Pimpl;
	}

	TwinRegistration DigitalTwinStore::registerTwin(const std::string& projectId, const std::string& twinId,
	                                                const std::string& twinName, const std::string& commitId,
	                                                const std::vector<Model::ModelElementInfo>& tree, bool readLastState)
	{
		return Pimpl->registerTwin(projectId, twinId, twinName, commitId, tree, readLastState);
	}

	bool DigitalTwinStore::enqueueChanges(int32_t versionNo, const std::vector<Model::VariableChange>& changes,
	                                      std::chrono::system_clock::time_point receivedAt,
	                                      std::chrono::system_clock::time_point ts)
	{
		return Pimpl->enqueueChanges(versionNo, changes, receivedAt, ts);
	}

	bool DigitalTwinStore::enqueueCheckpoint(int32_t versionNo, CheckpointReason reason,
	                                         std::chrono::system_clock::time_point takenAt,
	                                         const std::vector<Model::VariableState>& state)
	{
		return Pimpl->enqueueCheckpoint(versionNo, reason, takenAt, state);
	}

	std::optional<CheckpointReason> DigitalTwinStore::checkpointDue(int32_t versionNo,
	                                                                std::chrono::system_clock::time_point receivedAt)
	{
		return Pimpl->checkpointDue(versionNo, receivedAt);
	}

	StoreStats DigitalTwinStore::stats() const
	{
		return Pimpl->stats();
	}

	void DigitalTwinStore::shutdown(std::chrono::milliseconds timeout)
	{
		Pimpl->shutdown(timeout);
	}
}
