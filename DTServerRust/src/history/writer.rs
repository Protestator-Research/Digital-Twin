//! The write side of the recorder: batches rows from the queue and inserts them with ONE statement per
//! batch, and the periodic maintenance (partitions and retention).
//!
//! Failure handling of the writer:
//! - transient errors (connection lost, timeouts, locks): the batch is kept and retried with exponential
//!   backoff. Memory stays bounded: the writer holds at most `batch_size` rows and the queue holds at most
//!   `queue_capacity`; beyond that the ingest side drops and counts rows.
//! - rows the database refuses (SQLSTATE class 22/23): the batch is split in halves until the offending row
//!   stands alone; only that row is dropped (and counted), so one bad row never poisons a batch.
//! - a missing partition (a concurrent retention drop): the partitions are created again and the batch retried.
//! - partitions are created on demand for the months of the rows (within the accepted time window, see
//!   `partition`); if a month cannot be created for a permanent reason its rows are dropped and counted.
//! - shutdown: the queue is drained and flushed, but not longer than `shutdown_flush_limit` after the shutdown
//!   flag was set; rows still unwritten then are counted as lost.

use std::collections::{BTreeSet, HashSet};
use std::sync::Arc;
use std::time::Duration;

use chrono::{DateTime, Utc};
use tokio::sync::{mpsc, watch};
use tokio::time::Instant;
use tracing::{debug, error, info, warn};

use super::partition::{months_to_ensure, partitions_to_drop, retention_cutoff, Month};
use super::recorder::HistoryStats;
use super::row::{BatchColumns, Row};
use super::store::{ErrorKind, Store, StoreError};

#[derive(Debug, Clone)]
pub struct WriterSettings {
    pub batch_size: usize,
    pub flush_interval: Duration,
    pub backoff_initial: Duration,
    pub backoff_max: Duration,
    pub shutdown_flush_limit: Duration,
    /// A batch the database keeps refusing for other than data or transient reasons (permissions, missing
    /// table) is dropped after this many attempts.
    pub permanent_attempts: u32,
}

enum Outcome {
    Done,
    /// The batch has a row the database refuses; try the halves.
    Split(Vec<Row>),
    /// Shutdown time limit reached.
    Abandoned,
}

struct Writer<S: Store> {
    store: Arc<S>,
    settings: WriterSettings,
    stats: Arc<HistoryStats>,
    shutdown: watch::Receiver<bool>,
    /// Partitions known to exist.
    partitions: HashSet<Month>,
    deadline: Option<Instant>,
}

/// Reads rows from `queue` until it is closed and empty, and inserts them in batches.
pub async fn run_writer<S: Store>(
    store: Arc<S>,
    mut queue: mpsc::Receiver<Row>,
    settings: WriterSettings,
    stats: Arc<HistoryStats>,
    shutdown: watch::Receiver<bool>,
) {
    let batch_size = settings.batch_size.max(1);
    let flush_interval = settings.flush_interval;
    let mut writer = Writer {
        store,
        settings,
        stats,
        shutdown,
        partitions: HashSet::new(),
        deadline: None,
    };
    let mut closed = false;
    while !closed {
        let Some(first) = queue.recv().await else {
            break;
        };
        let mut batch = Vec::with_capacity(batch_size);
        batch.push(first);
        let flush_at = Instant::now() + flush_interval;
        while batch.len() < batch_size {
            tokio::select! {
                row = queue.recv() => match row {
                    Some(row) => batch.push(row),
                    None => { closed = true; break; }
                },
                _ = tokio::time::sleep_until(flush_at) => break,
            }
        }
        writer.flush(batch).await;
    }
    debug!("history writer stopped");
}

impl<S: Store> Writer<S> {
    async fn flush(&mut self, rows: Vec<Row>) {
        let mut work = vec![rows];
        while let Some(chunk) = work.pop() {
            if chunk.is_empty() {
                continue;
            }
            match self.insert_with_retry(chunk).await {
                Outcome::Done => {}
                Outcome::Abandoned => {
                    // everything still queued in `work` is lost as well
                    let lost: usize = work.iter().map(Vec::len).sum();
                    HistoryStats::add(&self.stats.lost_on_shutdown, lost as u64);
                    return;
                }
                Outcome::Split(chunk) => {
                    let right = chunk.len() / 2;
                    let mut left = chunk;
                    let right = left.split_off(right);
                    // `pop` takes from the end: push the right half first so the left half is written first
                    work.push(right);
                    work.push(left);
                }
            }
        }
    }

    /// `Some(deadline)` once the shutdown flag is set.
    fn shutdown_deadline(&mut self) -> Option<Instant> {
        if self.deadline.is_none() && *self.shutdown.borrow() {
            self.deadline = Some(Instant::now() + self.settings.shutdown_flush_limit);
        }
        self.deadline
    }

    async fn insert_with_retry(&mut self, mut rows: Vec<Row>) -> Outcome {
        let mut backoff = self.settings.backoff_initial;
        let mut missing_partition_seen = false;
        let mut permanent_failures = 0u32;
        loop {
            let error = match self.ensure_partitions(&mut rows).await {
                Err(e) => e,
                Ok(()) if rows.is_empty() => return Outcome::Done,
                Ok(()) => {
                    let columns = BatchColumns::from_rows(&rows);
                    let result = bounded(
                        &mut self.shutdown,
                        &mut self.deadline,
                        self.settings.shutdown_flush_limit,
                        self.store.insert_batch(&columns),
                    )
                    .await;
                    match result {
                        Ok(()) => {
                            HistoryStats::add(&self.stats.recorded, rows.len() as u64);
                            HistoryStats::add(&self.stats.batches, 1);
                            return Outcome::Done;
                        }
                        Err(e) => e,
                    }
                }
            };
            let mut kind = error.kind;
            if kind == ErrorKind::MissingPartition {
                if missing_partition_seen {
                    // Creating the partition did not help (a detached table of the same name, a range that
                    // overlaps something else): the rows are the problem, isolate and drop them by splitting.
                    kind = ErrorKind::BadData;
                } else {
                    missing_partition_seen = true;
                }
            }
            match kind {
                ErrorKind::BadData if rows.len() == 1 => {
                    HistoryStats::add(&self.stats.rejected, 1);
                    warn!(path = %rows[0].element_path, "the database refused a measurement, it is dropped: {error}");
                    return Outcome::Done;
                }
                ErrorKind::BadData => return Outcome::Split(rows),
                ErrorKind::MissingPartition => {
                    // somebody dropped a partition: create them again on the next attempt (once)
                    self.partitions.clear();
                    warn!("a partition disappeared during the insert, creating it again: {error}");
                }
                ErrorKind::Permanent => {
                    permanent_failures += 1;
                    if permanent_failures >= self.settings.permanent_attempts.max(1) {
                        HistoryStats::add(&self.stats.rejected, rows.len() as u64);
                        error!(
                            rows = rows.len(),
                            attempts = permanent_failures,
                            "the database keeps refusing a batch, it is dropped: {error}"
                        );
                        return Outcome::Done;
                    }
                    warn!(rows = rows.len(), retry_in = ?backoff, "the database refused a batch: {error}");
                }
                ErrorKind::Transient => {
                    warn!(rows = rows.len(), retry_in = ?backoff, "cannot write measurements to the database: {error}");
                }
            }
            HistoryStats::add(&self.stats.flush_retries, 1);

            let wait = match self.shutdown_deadline() {
                Some(deadline) => {
                    let now = Instant::now();
                    if now >= deadline {
                        HistoryStats::add(&self.stats.lost_on_shutdown, rows.len() as u64);
                        error!(
                            rows = rows.len(),
                            "shutdown: the database did not accept the measurements in time, they are lost"
                        );
                        return Outcome::Abandoned;
                    }
                    backoff.min(deadline - now)
                }
                None => backoff,
            };
            if self.deadline.is_some() {
                tokio::time::sleep(wait).await;
            } else {
                tokio::select! {
                    _ = tokio::time::sleep(wait) => {}
                    changed = self.shutdown.changed() => {
                        // sender gone: no shutdown signal will ever come, just wait
                        if changed.is_err() {
                            tokio::time::sleep(wait).await;
                        }
                    }
                }
            }
            backoff = (backoff * 2).min(self.settings.backoff_max);
        }
    }

    /// Creates the partitions of all months in `rows`. Rows of a month that cannot be created for a permanent
    /// reason are removed (and counted). Transient errors are returned.
    async fn ensure_partitions(&mut self, rows: &mut Vec<Row>) -> Result<(), StoreError> {
        let months: BTreeSet<Month> = rows.iter().map(|r| Month::of(r.ts)).collect();
        for month in months {
            if self.partitions.contains(&month) {
                continue;
            }
            let result = bounded(
                &mut self.shutdown,
                &mut self.deadline,
                self.settings.shutdown_flush_limit,
                self.store.create_partition(month),
            )
            .await;
            match result {
                Ok(()) => {
                    self.partitions.insert(month);
                }
                Err(e) if matches!(e.kind, ErrorKind::Transient | ErrorKind::MissingPartition) => {
                    return Err(e)
                }
                Err(e) => {
                    let before = rows.len();
                    rows.retain(|r| Month::of(r.ts) != month);
                    let dropped = (before - rows.len()) as u64;
                    HistoryStats::add(&self.stats.rejected, dropped);
                    warn!(partition = %month.partition_name(), dropped, "cannot create the partition, its measurements are dropped: {e}");
                }
            }
        }
        Ok(())
    }
}

/// Runs a store call. Once the shutdown flag is set the call may take at most until the shutdown deadline, so a
/// hanging call cannot hold up the shutdown; an overrun is a transient error.
async fn bounded<T>(
    shutdown: &mut watch::Receiver<bool>,
    deadline: &mut Option<Instant>,
    limit: Duration,
    call: impl std::future::Future<Output = Result<T, StoreError>>,
) -> Result<T, StoreError> {
    tokio::pin!(call);
    if deadline.is_none() {
        tokio::select! {
            result = &mut call => return result,
            // (a closed channel returns at once with an error: then there is simply no deadline)
            _ = shutdown.wait_for(|stop| *stop) => {}
        }
        if *shutdown.borrow() {
            *deadline = Some(Instant::now() + limit);
        }
    }
    match *deadline {
        Some(deadline) => match tokio::time::timeout_at(deadline, call).await {
            Ok(result) => result,
            Err(_) => Err(StoreError::new(
                ErrorKind::Transient,
                "shutdown: the database call did not finish in time",
            )),
        },
        None => call.await,
    }
}

#[derive(Debug, Default, PartialEq, Eq)]
pub struct MaintenanceReport {
    pub ensured: Vec<Month>,
    pub dropped: Vec<Month>,
    /// Messages of the steps that failed. A failing step never stops the others.
    pub errors: Vec<String>,
}

/// Creates the partitions of the current and the next month and, if `retention_days > 0`, drops the
/// partitions that lie entirely before the cutoff. Every step runs even if another one failed: an error while
/// creating a partition does not stop the retention, and one failing drop does not stop the other drops.
pub async fn maintenance_once<S: Store>(
    store: &S,
    now: DateTime<Utc>,
    retention_days: u32,
) -> MaintenanceReport {
    let mut report = MaintenanceReport::default();
    for month in months_to_ensure(now) {
        match store.create_partition(month).await {
            Ok(()) => report.ensured.push(month),
            Err(e) => report
                .errors
                .push(format!("partition {}: {e}", month.partition_name())),
        }
    }
    if let Some(cutoff) = retention_cutoff(now, retention_days) {
        match store.list_partitions().await {
            Ok(existing) => {
                for month in partitions_to_drop(&existing, cutoff) {
                    match store.drop_partition(month).await {
                        Ok(()) => {
                            info!(partition = %month.partition_name(), "retention: partition dropped");
                            report.dropped.push(month);
                        }
                        Err(e) => report
                            .errors
                            .push(format!("drop {}: {e}", month.partition_name())),
                    }
                }
            }
            Err(e) => report.errors.push(format!("listing partitions: {e}")),
        }
    }
    report
}

/// How soon the maintenance is repeated after it had errors.
const MAINTENANCE_RETRY: Duration = Duration::from_secs(60);

/// Runs the maintenance right away and then every `interval` (after errors: sooner) until shutdown.
pub async fn run_maintenance<S: Store>(
    store: Arc<S>,
    retention_days: u32,
    interval: Duration,
    mut shutdown: watch::Receiver<bool>,
) {
    loop {
        let report = maintenance_once(&*store, Utc::now(), retention_days).await;
        let wait = if report.errors.is_empty() {
            interval
        } else {
            for e in &report.errors {
                warn!("history maintenance: {e}");
            }
            interval.min(MAINTENANCE_RETRY)
        };
        tokio::select! {
            _ = tokio::time::sleep(wait) => {}
            _ = shutdown.wait_for(|stop| *stop) => return,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::history::fake::FakeStore;
    use crate::history::row::value_type;
    use chrono::{Duration as ChronoDuration, TimeZone};
    use uuid::Uuid;

    fn settings() -> WriterSettings {
        WriterSettings {
            batch_size: 10,
            flush_interval: Duration::from_millis(200),
            backoff_initial: Duration::from_millis(100),
            backoff_max: Duration::from_secs(1),
            shutdown_flush_limit: Duration::from_secs(2),
            permanent_attempts: 3,
        }
    }

    fn row_at(path: &str, ts: DateTime<Utc>) -> Row {
        Row {
            twin_id: Uuid::from_u128(1),
            project_id: Uuid::from_u128(2),
            element_path: path.to_owned(),
            ts,
            received_at: ts,
            value_type: value_type::REAL,
            value_num: Some(1.0),
            value_text: None,
            value_json: None,
        }
    }

    fn row(path: &str) -> Row {
        row_at(path, Utc.with_ymd_and_hms(2026, 11, 17, 9, 0, 0).unwrap())
    }

    struct Setup {
        store: Arc<FakeStore>,
        stats: Arc<HistoryStats>,
        tx: Option<mpsc::Sender<Row>>,
        shutdown_tx: watch::Sender<bool>,
        task: tokio::task::JoinHandle<()>,
    }

    fn start(settings: WriterSettings) -> Setup {
        let store = Arc::new(FakeStore::default());
        let stats = Arc::new(HistoryStats::default());
        let (tx, rx) = mpsc::channel(1000);
        let (shutdown_tx, shutdown_rx) = watch::channel(false);
        let task = tokio::spawn(run_writer(
            Arc::clone(&store),
            rx,
            settings,
            Arc::clone(&stats),
            shutdown_rx,
        ));
        Setup {
            store,
            stats,
            tx: Some(tx),
            shutdown_tx,
            task,
        }
    }

    impl Setup {
        async fn send(&self, rows: impl IntoIterator<Item = Row>) {
            for r in rows {
                self.tx.as_ref().unwrap().send(r).await.unwrap();
            }
        }

        /// Closes the queue like the ingest does at shutdown and waits for the writer.
        async fn finish(mut self) -> (Arc<FakeStore>, Arc<HistoryStats>) {
            self.tx = None;
            self.task.await.unwrap();
            (self.store, self.stats)
        }
    }

    #[tokio::test(start_paused = true)]
    async fn one_statement_per_batch_in_arrival_order() {
        let s = start(settings());
        s.send((0..25).map(|i| row(&format!("e{i}")))).await;
        let (store, stats) = s.finish().await;
        let state = store.state.lock().unwrap();
        let sizes: Vec<usize> = state.inserts.iter().map(|c| c.len()).collect();
        assert_eq!(sizes, vec![10, 10, 5]);
        assert_eq!(state.insert_calls, 3);
        drop(state);
        let paths = store.recorded_paths();
        assert_eq!(paths, (0..25).map(|i| format!("e{i}")).collect::<Vec<_>>());
        let snap = stats.snapshot();
        assert_eq!(
            (snap.recorded, snap.batches, snap.flush_retries),
            (25, 3, 0)
        );
    }

    #[tokio::test(start_paused = true)]
    async fn partial_batch_is_flushed_after_the_interval() {
        let s = start(settings());
        s.send([row("a"), row("b")]).await;
        // nothing before the interval is over ...
        tokio::time::sleep(Duration::from_millis(150)).await;
        assert_eq!(s.store.recorded_rows(), 0);
        // ... and the batch is written right after
        tokio::time::sleep(Duration::from_millis(100)).await;
        assert_eq!(s.store.recorded_rows(), 2);
        assert_eq!(s.store.state.lock().unwrap().insert_calls, 1);
        let _ = s.finish().await;
    }

    #[tokio::test(start_paused = true)]
    async fn transient_errors_are_retried_with_backoff_without_loss() {
        let s = start(settings());
        {
            let mut st = s.store.state.lock().unwrap();
            st.fail_next_inserts = [
                ErrorKind::Transient,
                ErrorKind::Transient,
                ErrorKind::Permanent,
            ]
            .into_iter()
            .collect();
        }
        s.send((0..3).map(|i| row(&format!("e{i}")))).await;
        let started = Instant::now();
        let (store, stats) = s.finish().await;
        // 3 failures: waits of 100 + 200 + 400 ms (plus the 200 ms batch interval) elapsed in paused time
        assert!(started.elapsed() >= Duration::from_millis(700));
        assert_eq!(store.recorded_paths(), vec!["e0", "e1", "e2"]);
        let snap = stats.snapshot();
        assert_eq!(
            (snap.recorded, snap.flush_retries, snap.rejected),
            (3, 3, 0)
        );
    }

    #[tokio::test(start_paused = true)]
    async fn bad_row_is_isolated_by_splitting() {
        let s = start(settings());
        s.store
            .state
            .lock()
            .unwrap()
            .bad_paths
            .insert("bad".to_owned());
        let mut rows: Vec<Row> = (0..7).map(|i| row(&format!("ok{i}"))).collect();
        rows.insert(3, row("bad"));
        s.send(rows).await;
        let (store, stats) = s.finish().await;
        let mut recorded = store.recorded_paths();
        assert_eq!(recorded.len(), 7);
        assert!(!recorded.contains(&"bad".to_owned()));
        recorded.sort();
        assert_eq!(
            recorded,
            (0..7).map(|i| format!("ok{i}")).collect::<Vec<_>>()
        );
        let snap = stats.snapshot();
        assert_eq!((snap.recorded, snap.rejected), (7, 1));
        // no retries: a data error is not transient
        assert_eq!(snap.flush_retries, 0);
    }

    #[tokio::test(start_paused = true)]
    async fn partitions_are_created_on_demand_once_per_month() {
        let s = start(settings());
        let nov = Utc.with_ymd_and_hms(2026, 11, 30, 23, 59, 59).unwrap();
        let dec = nov + ChronoDuration::seconds(1);
        let jan = Utc.with_ymd_and_hms(2027, 1, 15, 0, 0, 0).unwrap();
        s.send([
            row_at("a", nov),
            row_at("b", dec),
            row_at("c", jan),
            row_at("d", nov),
        ])
        .await;
        s.send((0..20).map(|i| row_at(&format!("x{i}"), nov))).await;
        let (store, _) = s.finish().await;
        let state = store.state.lock().unwrap();
        assert_eq!(
            state.create_calls,
            vec![
                Month::new(2026, 11).unwrap(),
                Month::new(2026, 12).unwrap(),
                Month::new(2027, 1).unwrap()
            ]
        );
    }

    #[tokio::test(start_paused = true)]
    async fn rows_of_a_month_that_cannot_be_created_do_not_block_the_others() {
        let s = start(settings());
        let nov = Utc.with_ymd_and_hms(2026, 11, 5, 0, 0, 0).unwrap();
        let dec = Utc.with_ymd_and_hms(2026, 12, 5, 0, 0, 0).unwrap();
        s.store
            .state
            .lock()
            .unwrap()
            .create_failures
            .insert(Month::new(2026, 12).unwrap(), ErrorKind::Permanent);
        s.send([row_at("n1", nov), row_at("d1", dec), row_at("n2", nov)])
            .await;
        let (store, stats) = s.finish().await;
        assert_eq!(store.recorded_paths(), vec!["n1", "n2"]);
        assert_eq!(stats.snapshot().rejected, 1);
    }

    #[tokio::test(start_paused = true)]
    async fn missing_partition_is_created_again_and_the_batch_retried() {
        let s = start(settings());
        s.store
            .state
            .lock()
            .unwrap()
            .fail_next_inserts
            .push_back(ErrorKind::MissingPartition);
        s.send([row("a")]).await;
        let (store, stats) = s.finish().await;
        let state = store.state.lock().unwrap();
        // created before the first attempt and again after the 23514
        assert_eq!(state.create_calls.len(), 2);
        assert_eq!(state.inserts.len(), 1);
        drop(state);
        assert_eq!(stats.snapshot().recorded, 1);
    }

    #[tokio::test(start_paused = true)]
    async fn shutdown_flush_is_bounded_when_the_database_is_down() {
        let s = start(settings());
        s.store.state.lock().unwrap().fail_all_inserts = Some(ErrorKind::Transient);
        s.send((0..25).map(|i| row(&format!("e{i}")))).await;
        // let the writer run into the failure, then ask for shutdown and close the queue
        tokio::time::sleep(Duration::from_millis(50)).await;
        let started = Instant::now();
        s.shutdown_tx.send(true).unwrap();
        let shutdown_tx = s.shutdown_tx.clone();
        let (store, stats) = s.finish().await;
        let elapsed = started.elapsed();
        assert!(elapsed <= Duration::from_secs(3), "took {elapsed:?}");
        assert_eq!(store.recorded_rows(), 0);
        let snap = stats.snapshot();
        // every row is accounted for: nothing is silently gone
        assert_eq!(snap.lost_on_shutdown, 25);
        drop(shutdown_tx);
    }

    #[tokio::test(start_paused = true)]
    async fn shutdown_flushes_what_is_possible() {
        let s = start(settings());
        s.send((0..4).map(|i| row(&format!("e{i}")))).await;
        s.shutdown_tx.send(true).unwrap();
        let (store, stats) = s.finish().await;
        assert_eq!(store.recorded_rows(), 4);
        assert_eq!(stats.snapshot().lost_on_shutdown, 0);
    }

    #[tokio::test]
    async fn maintenance_creates_current_and_next_month() {
        let store = FakeStore::default();
        let now = Utc.with_ymd_and_hms(2026, 12, 31, 23, 0, 0).unwrap();
        let report = maintenance_once(&store, now, 0).await;
        assert_eq!(
            report,
            MaintenanceReport {
                ensured: vec![Month::new(2026, 12).unwrap(), Month::new(2027, 1).unwrap()],
                dropped: vec![],
                errors: vec![]
            }
        );
        // idempotent
        maintenance_once(&store, now, 0).await;
        assert_eq!(store.state.lock().unwrap().partitions.len(), 2);
    }

    #[tokio::test]
    async fn retention_drops_only_entirely_old_partitions() {
        let store = FakeStore::default();
        {
            let mut s = store.state.lock().unwrap();
            for (y, m) in [(2026, 7), (2026, 8), (2026, 9), (2026, 10), (2026, 11)] {
                s.partitions.insert(Month::new(y, m).unwrap());
            }
        }
        let now = Utc.with_ymd_and_hms(2026, 11, 17, 9, 0, 0).unwrap();
        // 30 days: cutoff 2026-10-18; October still holds younger rows
        let report = maintenance_once(&store, now, 30).await;
        assert_eq!(
            report.dropped,
            vec![
                Month::new(2026, 7).unwrap(),
                Month::new(2026, 8).unwrap(),
                Month::new(2026, 9).unwrap()
            ]
        );
        // 0 keeps everything
        let store2 = FakeStore::default();
        store2
            .state
            .lock()
            .unwrap()
            .partitions
            .insert(Month::new(2001, 1).unwrap());
        let report = maintenance_once(&store2, now, 0).await;
        assert!(report.dropped.is_empty());
        assert!(store2
            .state
            .lock()
            .unwrap()
            .partitions
            .contains(&Month::new(2001, 1).unwrap()));
    }

    #[tokio::test(start_paused = true)]
    async fn a_missing_partition_that_creation_does_not_fix_does_not_stall_the_writer() {
        // e.g. a detached table of the same name: CREATE IF NOT EXISTS succeeds but the insert still gets 23514
        let s = start(settings());
        s.store.state.lock().unwrap().fail_all_inserts = Some(ErrorKind::MissingPartition);
        s.send((0..5).map(|i| row(&format!("e{i}")))).await;
        let started = Instant::now();
        let (store, stats) = s.finish().await;
        // one re-create attempt per (sub)batch, then the split isolates and drops every row: no endless loop
        assert!(started.elapsed() < Duration::from_secs(60));
        assert_eq!(store.recorded_rows(), 0);
        let snap = stats.snapshot();
        assert_eq!((snap.rejected, snap.recorded), (5, 0));
    }

    #[tokio::test(start_paused = true)]
    async fn later_rows_are_written_after_a_poisoned_batch() {
        let s = start(settings());
        s.store
            .state
            .lock()
            .unwrap()
            .fail_next_inserts
            .extend([ErrorKind::MissingPartition, ErrorKind::MissingPartition]);
        s.send([row("poison")]).await;
        tokio::time::sleep(Duration::from_secs(1)).await;
        s.send([row("after")]).await;
        let (store, stats) = s.finish().await;
        assert_eq!(store.recorded_paths(), vec!["after"]);
        assert_eq!(stats.snapshot().rejected, 1);
    }

    #[tokio::test(start_paused = true)]
    async fn permanent_insert_errors_drop_the_batch_after_a_bounded_number_of_attempts() {
        let s = start(settings());
        s.store.state.lock().unwrap().fail_all_inserts = Some(ErrorKind::Permanent);
        s.send((0..4).map(|i| row(&format!("e{i}")))).await;
        tokio::time::sleep(Duration::from_secs(10)).await;
        // the database works again: data written afterwards is not blocked
        s.store.state.lock().unwrap().fail_all_inserts = None;
        s.send([row("later")]).await;
        let (store, stats) = s.finish().await;
        assert_eq!(store.state.lock().unwrap().insert_calls, 3 + 1);
        assert_eq!(store.recorded_paths(), vec!["later"]);
        let snap = stats.snapshot();
        assert_eq!((snap.rejected, snap.recorded), (4, 1));
    }

    #[tokio::test(start_paused = true)]
    async fn transient_errors_are_never_given_up_while_running() {
        let s = start(settings());
        s.store.state.lock().unwrap().fail_all_inserts = Some(ErrorKind::Transient);
        s.send([row("a")]).await;
        tokio::time::sleep(Duration::from_secs(120)).await;
        assert_eq!(s.stats.snapshot().rejected, 0);
        s.store.state.lock().unwrap().fail_all_inserts = None;
        let (store, _) = s.finish().await;
        assert_eq!(store.recorded_paths(), vec!["a"]);
    }

    #[tokio::test(start_paused = true)]
    async fn shutdown_is_bounded_even_when_a_database_call_hangs() {
        let s = start(settings());
        s.send([row("a"), row("b")]).await;
        // the writer starts its flush after the interval and hangs in the insert
        tokio::time::sleep(Duration::from_millis(250)).await;
        s.store.state.lock().unwrap().hang = true;
        s.send([row("c")]).await;
        tokio::time::sleep(Duration::from_millis(250)).await;
        let started = Instant::now();
        s.shutdown_tx.send(true).unwrap();
        let keep = s.shutdown_tx.clone();
        let (_, stats) = s.finish().await;
        assert!(
            started.elapsed() <= Duration::from_secs(3),
            "{:?}",
            started.elapsed()
        );
        assert_eq!(stats.snapshot().lost_on_shutdown, 1);
        drop(keep);
    }

    #[tokio::test]
    async fn maintenance_continues_after_an_error() {
        let store = FakeStore::default();
        let now = Utc.with_ymd_and_hms(2026, 11, 17, 9, 0, 0).unwrap();
        {
            let mut s = store.state.lock().unwrap();
            // the current month fails (a race that was not benign), the next one works, and old partitions exist
            s.create_failures
                .insert(Month::new(2026, 11).unwrap(), ErrorKind::Permanent);
            for m in [3, 4] {
                s.partitions.insert(Month::new(2026, m).unwrap());
            }
        }
        let report = maintenance_once(&store, now, 30).await;
        assert_eq!(report.ensured, vec![Month::new(2026, 12).unwrap()]);
        assert_eq!(report.errors.len(), 1);
        // the retention still ran
        assert_eq!(report.dropped.len(), 2);
    }
}
