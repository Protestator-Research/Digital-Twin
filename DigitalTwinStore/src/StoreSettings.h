#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace DigitalTwin::Store {
	struct StoreSettings {
		/** libpq URI. May contain a password: it is never logged and never part of an error message. */
		std::string DatabaseUrl;
		/** Partitions that lie entirely older than this many days are dropped. 0 keeps everything. */
		uint32_t RetentionDays = 0;
		/** Minimum time between two checkpoints of a version while it changes. 0 switches the interval rule off. */
		uint32_t CheckpointIntervalSeconds = 300;
		/** Capacity of the queue, in rows (one row per variable value). */
		size_t QueueCapacityRows = 100000;
		/** The writer takes this many rows (at least one whole change set) into one transaction. */
		size_t BatchSizeRows = 1000;
		/** The writer flushes a partial batch after this time. */
		uint32_t FlushIntervalMs = 500;
		/** Optional schema, used as search_path (created if missing). Meant for tests. */
		std::string SchemaName;

		// Tuning, the defaults are right for production.
		uint32_t StatementTimeoutMs = 30000;
		uint32_t LockTimeoutMs = 10000;
		uint32_t ConnectTimeoutSeconds = 10;
		uint32_t MaintenanceIntervalSeconds = 3600;
		uint32_t BackoffInitialMs = 500;
		uint32_t BackoffMaxMs = 30000;
		/** Attempts for a batch that the database refuses for a data reason, before it is dropped. */
		uint32_t MaxDataErrorAttempts = 3;
		/** registerTwin of a known commit waits this long for the queue to hold no row of the version (see there). */
		uint32_t RegisterDrainWaitMs = 5000;

		/** Receives the log lines (never containing the URL). Default: standard error. */
		std::function<void(const std::string&)> Log;
	};
}
