// Writes the history of the digital twins (change log and checkpoints) to PostgreSQL.
// See Markdownfiles/Plan_Datenbank_CppKern.md and sql/0001_schema.sql.
//
// Threads: registerTwin runs on the caller's thread with a connection of its own. All the other writes go through a
// bounded queue to one writer thread that owns its connection. enqueueChanges, enqueueCheckpoint, checkpointDue and
// stats never block on the database.
#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "cpp_digital_twin_store_global.h"
#include "Model/State/StateTypes.h"
#include "StoreLogic.h"
#include "StoreSettings.h"

namespace DigitalTwin::Store {
	/** Result of registerTwin. */
	struct TwinRegistration {
		int32_t VersionNo = 0;
		/** The commit of the twin was loaded before. */
		bool KnownCommit = false;
		/** element_no of the variables of the tree. */
		std::map<std::string, int32_t> ElementNoByPath;
		/** Known commit only: the last stored state (the newest checkpoint and everything after it). */
		std::vector<Model::VariableState> LastState;
	};

	struct StoreStats {
		bool Connected = false;
		uint64_t Pending = 0;       // rows in the queue or in the hands of the writer
		uint64_t Written = 0;       // rows written
		uint64_t Dropped = 0;       // rows lost: queue full, refused by the database, not written at shutdown
		uint64_t Retries = 0;
		uint64_t Checkpoints = 0;   // checkpoints written
	};

	class CPPDIGITALTWINSTORE_EXPORT DigitalTwinStore {
	public:
		/**
		 * Starts the writer thread. Never fails because the database is down: the thread connects with backoff.
		 * Throws std::invalid_argument for settings that are wrong in themselves (invalid schema name).
		 */
		explicit DigitalTwinStore(StoreSettings settings);
		~DigitalTwinStore();
		DigitalTwinStore(const DigitalTwinStore&) = delete;
		DigitalTwinStore& operator=(const DigitalTwinStore&) = delete;

		/**
		 * Registers the twin and the commit it was loaded from, synchronously: project, twin and model_version are
		 * upserted, the elements of the tree that are missing are inserted. For a known commit the last stored state
		 * is returned as well, after waiting (a few seconds at most) until the queue holds nothing of the version.
		 * With readLastState false neither the wait nor the read happens and LastState stays empty: for a caller
		 * that has the live state elsewhere.
		 * The ids are UUIDs in the usual text form.
		 * @throws std::runtime_error on database errors (the message never contains the URL or the password),
		 *         std::invalid_argument for a malformed id or tree.
		 */
		TwinRegistration registerTwin(const std::string& projectId, const std::string& twinId, const std::string& twinName,
		                              const std::string& commitId, const std::vector<Model::ModelElementInfo>& tree,
		                              bool readLastState = true);

		/**
		 * Queues one change set (one change number): the rows share the number. Does not block.
		 * `receivedAt` is clamped to be monotonic per version. If the queue is full the set is dropped, counted, and
		 * the version is marked as having a gap, so that checkpointDue asks for a checkpoint.
		 * @return false if the set was dropped (or the store is shut down or the version unknown).
		 */
		bool enqueueChanges(int32_t versionNo, const std::vector<Model::VariableChange>& changes,
		                    std::chrono::system_clock::time_point receivedAt, std::chrono::system_clock::time_point ts);

		/**
		 * Queues a checkpoint: one checkpoint row and one measurement row (source 3) per variable, written in one
		 * transaction with a change number of its own, taken in queue order. Does not block. `takenAt` is clamped
		 * like receivedAt.
		 * @return false if the queue refused it (it is full), the store is shut down or the version unknown.
		 */
		bool enqueueCheckpoint(int32_t versionNo, CheckpointReason reason, std::chrono::system_clock::time_point takenAt,
		                       const std::vector<Model::VariableState>& state);

		/**
		 * Is a checkpoint due for the version? See decideCheckpoint. Empty if not (or the version is unknown).
		 */
		std::optional<CheckpointReason> checkpointDue(int32_t versionNo, std::chrono::system_clock::time_point receivedAt);

		StoreStats stats() const;

		/**
		 * Stops accepting rows, lets the writer drain the queue for at most `timeout` and joins it. Rows that could
		 * not be written are counted as dropped. Later calls do nothing.
		 */
		void shutdown(std::chrono::milliseconds timeout);

	private:
		struct Impl;
		Impl* Pimpl;
	};
}
