// Bounded queue of the rows that the writer has to write. The capacity is counted in rows, not in items: one change
// set has one row per changed variable, one checkpoint one row per variable of the model.
#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

#include "Model/State/StateTypes.h"
#include "StoreLogic.h"

namespace DigitalTwin::Store {
	struct QueuedRow {
		int32_t ElementNo = 0;
		int16_t Source = 1;   // 1 client, 2 derived, 3 checkpoint
		Model::VariableValue Value;
	};

	/** One change set (one change number) or one checkpoint. */
	struct QueueItem {
		enum class Kind { Changes, Checkpoint };

		Kind ItemKind = Kind::Changes;
		int32_t VersionNo = 0;
		Micros ReceivedAt = 0;   // for a checkpoint: taken_at
		Micros Ts = 0;
		int16_t Reason = 0;      // checkpoints only
		int64_t ChangeNo = -1;   // assigned by the writer, in queue order
		std::vector<QueuedRow> Rows;
	};

	/** Not thread safe, the store guards it with its mutex. */
	class RowQueue {
	public:
		explicit RowQueue(size_t capacityRows) : Capacity(capacityRows) {}

		/**
		 * Appends a change set if its rows fit into the capacity (together with everything that is queued or being
		 * written). Otherwise nothing changes and the caller counts the rows as dropped.
		 */
		bool tryPushChanges(QueueItem&& item)
		{
			if (PendingRows + item.Rows.size() > Capacity)
				return false;
			push(std::move(item));
			return true;
		}

		/**
		 * Appends a checkpoint if the queue is not full. A checkpoint is as large as the model, so it is not
		 * measured against the capacity: it may take the queue above it by one checkpoint, but it never waits
		 * for room that a large model could not give it.
		 */
		bool tryPushCheckpoint(QueueItem&& item)
		{
			if (PendingRows >= Capacity)
				return false;
			push(std::move(item));
			return true;
		}

		/**
		 * Takes whole items in queue order until at least maxRows rows are taken (always one item if there is one).
		 * The rows stay counted as pending until finish() is called for them.
		 */
		std::vector<QueueItem> popBatch(size_t maxRows)
		{
			std::vector<QueueItem> batch;
			size_t rows = 0;
			while (!Items.empty() && (batch.empty() || rows < maxRows))
			{
				rows += Items.front().Rows.size();
				QueuedRows -= Items.front().Rows.size();
				batch.push_back(std::move(Items.front()));
				Items.pop_front();
			}
			return batch;
		}

		/** The rows of a popped batch are written or dropped. */
		void finish(size_t rows) { PendingRows = rows > PendingRows ? 0 : PendingRows - rows; }

		/** Rows that are queued or in the hands of the writer. */
		size_t pendingRows() const { return PendingRows; }
		/** Rows that are queued and not yet taken. */
		size_t queuedRows() const { return QueuedRows; }
		bool empty() const { return Items.empty(); }
		size_t capacity() const { return Capacity; }

	private:
		void push(QueueItem&& item)
		{
			PendingRows += item.Rows.size();
			QueuedRows += item.Rows.size();
			Items.push_back(std::move(item));
		}

		size_t Capacity;
		size_t PendingRows = 0;
		size_t QueuedRows = 0;
		std::deque<QueueItem> Items;
	};
}
