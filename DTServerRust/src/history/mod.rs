//! History of the measurements in PostgreSQL (plan section 6): twin registry, recorder, monthly partitions and
//! retention.
//!
//! Pipeline: hub (`#`, a small subscription of at most [`HUB_CAPACITY`] messages) -> [`recorder::run_ingest`]
//! (classify, map topic to twin; only compact rows go on) -> bounded queue (`queue_capacity` rows) ->
//! [`writer::run_writer`] (batches, one `INSERT ... UNNEST` per batch, retry with backoff) -> PostgreSQL.
//! [`writer::run_maintenance`] keeps the partitions and applies the retention. Every database call has a time
//! limit (server side `statement_timeout` / `lock_timeout` plus [`store::TimedStore`]).
//!
//! Memory bound: the hub subscription holds at most `HUB_CAPACITY` x max packet size bytes (the messages are
//! dropped and counted when it is full), the queue and the writer batch hold rows of at most
//! [`recorder::MAX_ROW_TEXT_BYTES`] of text plus the topic path (512 bytes).
//!
//! The history is enabled only when `run.history.database_url` (or `DT_HISTORY_DATABASE_URL`) is set. The
//! server's own initial values are published with `Origin::Server` and are never recorded.
//!
//! Database trouble never takes the server down: the connection (and the migrations) are retried with backoff in
//! the background while the queue fills up and then drops; rows are counted in [`HistoryStats`].

pub mod partition;
pub mod pg;
pub mod recorder;
pub mod registry;
pub mod row;
pub mod store;
pub mod writer;

/// Capacity of the hub subscription of the recorder (messages that still carry their whole payload).
pub const HUB_CAPACITY: usize = 256;

#[cfg(test)]
mod fake;

use std::sync::{Arc, RwLock};
use std::time::Duration;

use thiserror::Error;
use tokio::sync::{mpsc, watch};
use tokio::task::JoinHandle;
use tracing::{info, warn};

pub use recorder::{HistoryStats, StatsSnapshot};
pub use registry::{DatabaseMerge, Registration, Registry, TwinRecord};
pub use store::{ErrorKind, Store, StoreError, TimedStore};

use crate::config::HistoryConfig;
use crate::hub::{Hub, HubError};
use crate::twin::{LoadedTwin, TwinObserver};
use writer::WriterSettings;

#[derive(Debug, Error)]
pub enum HistoryError {
    #[error("invalid history configuration: {0}")]
    Config(String),
    #[error("cannot subscribe to the hub: {0}")]
    Hub(#[from] HubError),
}

/// Tunables of the pipeline (from `run.history.*`, the rest are constants that tests shorten).
#[derive(Debug, Clone)]
pub struct HistorySettings {
    pub queue_capacity: usize,
    pub batch_size: usize,
    pub flush_interval: Duration,
    pub retention_days: u32,
    pub maintenance_interval: Duration,
    pub backoff_initial: Duration,
    pub backoff_max: Duration,
    pub shutdown_flush_limit: Duration,
    /// Attempts for a batch the database refuses for other than data or transient reasons.
    pub permanent_attempts: u32,
    /// Time limit of one database call.
    pub call_timeout: Duration,
    /// Capacity of the queue of twin registrations waiting to be written to the database.
    pub registry_queue: usize,
    /// Interval of the statistics log line (only logged when something changed).
    pub report_interval: Duration,
}

impl HistorySettings {
    pub fn from_config(config: &HistoryConfig) -> Result<HistorySettings, HistoryError> {
        if config.queue_capacity == 0 || config.batch_size == 0 || config.flush_interval_ms == 0 {
            return Err(HistoryError::Config(
                "queue_capacity, batch_size and flush_interval_ms must be greater than 0".into(),
            ));
        }
        Ok(HistorySettings {
            queue_capacity: config.queue_capacity,
            batch_size: config.batch_size,
            flush_interval: Duration::from_millis(config.flush_interval_ms),
            retention_days: config.retention_days,
            maintenance_interval: Duration::from_secs(3600),
            backoff_initial: Duration::from_millis(500),
            backoff_max: Duration::from_secs(30),
            shutdown_flush_limit: Duration::from_secs(5),
            permanent_attempts: 5,
            call_timeout: Duration::from_secs(45),
            registry_queue: 1024,
            report_interval: Duration::from_secs(60),
        })
    }

    fn writer(&self) -> WriterSettings {
        WriterSettings {
            batch_size: self.batch_size,
            flush_interval: self.flush_interval,
            backoff_initial: self.backoff_initial,
            backoff_max: self.backoff_max,
            shutdown_flush_limit: self.shutdown_flush_limit,
            permanent_attempts: self.permanent_attempts,
        }
    }
}

/// Handle to the running history.
pub struct HistoryService {
    shutdown_tx: watch::Sender<bool>,
    tasks: Vec<JoinHandle<()>>,
    registry: Arc<RwLock<Registry>>,
    stats: Arc<HistoryStats>,
    observer: Arc<RegistryObserver>,
    pending: Arc<PendingUpserts>,
    wait_limit: Duration,
}

/// Twin registrations that are not in the database yet, newest version per twin. Bounded.
pub struct PendingUpserts {
    map: std::sync::Mutex<std::collections::HashMap<uuid::Uuid, TwinRecord>>,
    wake: tokio::sync::Notify,
    capacity: usize,
}

impl PendingUpserts {
    pub fn new(capacity: usize) -> Self {
        Self {
            map: Default::default(),
            wake: tokio::sync::Notify::new(),
            capacity: capacity.max(1),
        }
    }

    /// Stores (or replaces) the record of a twin. `false` if the bound is reached for a new twin.
    pub fn push(&self, record: TwinRecord) -> bool {
        let mut map = self.map.lock().unwrap();
        if map.len() >= self.capacity && !map.contains_key(&record.twin_id) {
            return false;
        }
        map.insert(record.twin_id, record);
        drop(map);
        self.wake.notify_one();
        true
    }

    /// A copy of one pending record (it stays pending until [`PendingUpserts::finish`], so a call that is
    /// cancelled or fails loses nothing).
    fn next(&self) -> Option<TwinRecord> {
        self.map.lock().unwrap().values().next().cloned()
    }

    /// Removes the record after it was stored (or refused for good), unless a newer version arrived meanwhile.
    fn finish(&self, record: &TwinRecord) {
        let mut map = self.map.lock().unwrap();
        if map.get(&record.twin_id) == Some(record) {
            map.remove(&record.twin_id);
        }
    }

    pub fn len(&self) -> usize {
        self.map.lock().unwrap().len()
    }

    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
}

/// Applies the collision rule in memory at once and queues the database upsert.
struct RegistryObserver {
    registry: Arc<RwLock<Registry>>,
    pending: Arc<PendingUpserts>,
}

impl TwinObserver for RegistryObserver {
    fn twin_loaded(&self, twin: &LoadedTwin) {
        let record = TwinRecord {
            twin_id: twin.twin_id,
            project_id: twin.project_id,
            name: twin.name.clone(),
        };
        let outcome = self.registry.write().unwrap().register(&record);
        match outcome {
            Registration::NameTaken { owner } => {
                warn!(
                    twin = %record.twin_id,
                    name = %record.name,
                    %owner,
                    "the twin name is already used by another twin; values published under this name \
                     stay assigned to the first twin"
                );
                return;
            }
            Registration::Renamed { old_name } => {
                info!(twin = %record.twin_id, from = %old_name, to = %record.name, "twin was renamed");
            }
            Registration::Added => {
                info!(twin = %record.twin_id, name = %record.name, "twin registered")
            }
            Registration::Refreshed => {}
        }
        if !self.pending.push(record) {
            warn!(
                twin = %twin.twin_id,
                "too many twin registrations wait for the database, this one is only kept in memory \
                 until the twin connects again"
            );
        }
    }
}

impl HistoryService {
    /// Starts the history against PostgreSQL: the connection, the migrations and everything after them run in
    /// a background task that retries with backoff; an invalid URL is an error right away.
    pub fn start(
        hub: Hub,
        config: &HistoryConfig,
        url: &str,
    ) -> Result<HistoryService, HistoryError> {
        let settings = HistorySettings::from_config(config)?;
        let options = pg::parse_options(url).map_err(|e| HistoryError::Config(e.message))?;
        Self::start_with(hub, settings, move |mut shutdown| async move {
            let mut backoff = Duration::from_millis(500);
            loop {
                let attempt = async {
                    // connecting and migrating together may take at most a minute
                    tokio::time::timeout(Duration::from_secs(60), async {
                        let store = pg::PgStore::connect(options.clone(), None).await?;
                        store.migrate().await?;
                        Ok::<_, StoreError>(store)
                    })
                    .await
                    .unwrap_or_else(|_| {
                        Err(StoreError::new(
                            ErrorKind::Transient,
                            "connecting to the history database took longer than 60 seconds",
                        ))
                    })
                };
                // a connection attempt must not delay the shutdown
                let result = tokio::select! {
                    result = attempt => result,
                    _ = shutdown.wait_for(|stop| *stop) => return None,
                };
                match result {
                    Ok(store) => return Some(store),
                    Err(e) => warn!("history database not ready, retrying in {backoff:?}: {e}"),
                }
                tokio::select! {
                    _ = tokio::time::sleep(backoff) => {}
                    _ = shutdown.wait_for(|stop| *stop) => return None,
                }
                backoff = (backoff * 2).min(Duration::from_secs(30));
            }
        })
    }

    /// Starts the history on a store that `connect` provides (`None`: shutdown before the store was ready).
    /// Tests use this with a fake store.
    pub fn start_with<S, F, Fut>(
        hub: Hub,
        settings: HistorySettings,
        connect: F,
    ) -> Result<HistoryService, HistoryError>
    where
        S: Store,
        F: FnOnce(watch::Receiver<bool>) -> Fut + Send + 'static,
        Fut: std::future::Future<Output = Option<S>> + Send + 'static,
    {
        let subscription = hub.subscribe("#", HUB_CAPACITY.min(settings.queue_capacity))?;
        let registry = Arc::new(RwLock::new(Registry::new()));
        let stats = Arc::new(HistoryStats::default());
        let (shutdown_tx, shutdown_rx) = watch::channel(false);
        let (row_tx, row_rx) = mpsc::channel(settings.queue_capacity);
        let pending = Arc::new(PendingUpserts::new(settings.registry_queue));

        let ingest = tokio::spawn(recorder::run_ingest(
            subscription,
            Arc::clone(&registry),
            Arc::clone(&stats),
            row_tx,
            settings.retention_days,
            shutdown_rx.clone(),
        ));
        let wait_limit = settings.shutdown_flush_limit + Duration::from_secs(5);
        let pipeline = tokio::spawn(run_pipeline(
            connect(shutdown_rx.clone()),
            settings,
            Arc::clone(&registry),
            Arc::clone(&stats),
            row_rx,
            Arc::clone(&pending),
            shutdown_rx,
        ));
        Ok(HistoryService {
            shutdown_tx,
            tasks: vec![ingest, pipeline],
            observer: Arc::new(RegistryObserver {
                registry: Arc::clone(&registry),
                pending: Arc::clone(&pending),
            }),
            pending,
            registry,
            stats,
            wait_limit,
        })
    }

    /// Hand this to the twin logic: it registers every loaded twin.
    pub fn observer(&self) -> Arc<dyn TwinObserver> {
        Arc::clone(&self.observer) as Arc<dyn TwinObserver>
    }

    pub fn stats(&self) -> Arc<HistoryStats> {
        Arc::clone(&self.stats)
    }

    /// Registrations that are not stored in the database yet.
    pub fn pending_registrations(&self) -> usize {
        self.pending.len()
    }

    /// Number of registered twins.
    pub fn registered_twins(&self) -> usize {
        self.registry.read().unwrap().len()
    }

    /// Stops the history: the queue is flushed for at most `shutdown_flush_limit`, then the connections are
    /// closed.
    pub async fn shutdown(self) {
        let _ = self.shutdown_tx.send(true);
        let all = join_all(self.tasks);
        if tokio::time::timeout(self.wait_limit, all).await.is_err() {
            warn!("the history did not stop in time");
        }
        let s = self.stats.snapshot();
        info!(?s, "history stopped");
    }
}

async fn join_all(tasks: Vec<JoinHandle<()>>) {
    for task in tasks {
        let _ = task.await;
    }
}

/// Everything that needs the database: registry load, upserts, writer, maintenance.
async fn run_pipeline<S, Fut>(
    connect: Fut,
    settings: HistorySettings,
    registry: Arc<RwLock<Registry>>,
    stats: Arc<HistoryStats>,
    mut rows: mpsc::Receiver<row::Row>,
    pending: Arc<PendingUpserts>,
    shutdown: watch::Receiver<bool>,
) where
    S: Store,
    Fut: std::future::Future<Output = Option<S>>,
{
    let Some(store) = connect.await else {
        // Shutdown before the database was ever reached: whatever is queued can never be written. Count it.
        rows.close();
        let mut lost = 0u64;
        while rows.recv().await.is_some() {
            lost += 1;
        }
        HistoryStats::add(&stats.lost_on_shutdown, lost);
        if lost > 0 {
            warn!(
                rows = lost,
                "shutdown before the history database was reached, queued measurements are lost"
            );
        }
        return;
    };
    let store = Arc::new(TimedStore::new(store, settings.call_timeout));
    info!("history database ready");

    // The registry first: recording after a restart works before any new connectToTwin.
    load_registry(&*store, &registry, &settings, shutdown.clone()).await;

    let maintenance = tokio::spawn(writer::run_maintenance(
        Arc::clone(&store),
        settings.retention_days,
        settings.maintenance_interval,
        shutdown.clone(),
    ));
    let upsert_task = tokio::spawn(run_upserts(
        Arc::clone(&store),
        Arc::clone(&pending),
        settings.clone(),
        shutdown.clone(),
    ));
    let reporter = tokio::spawn(run_reporter(
        Arc::clone(&stats),
        settings.report_interval,
        shutdown.clone(),
    ));
    // The writer ends when the queue is closed (the ingest ended) and drained.
    writer::run_writer(
        Arc::clone(&store),
        rows,
        settings.writer(),
        Arc::clone(&stats),
        shutdown,
    )
    .await;
    // The upsert task drains the pending registrations (bounded by its own deadline), the others just stop.
    maintenance.abort();
    reporter.abort();
    let _ = tokio::time::timeout(
        settings.shutdown_flush_limit + Duration::from_secs(1),
        upsert_task,
    )
    .await;
    store.close().await;
}

/// Loads the registered twins (retrying with backoff) and merges them into the in-memory registry with the
/// rule "the database owner of a name wins" (see [`Registry::apply_database`]): a twin that is already known in
/// memory keeps its newer name, and an in-memory twin that holds a name of a database twin loses it and is no
/// longer recorded.
async fn load_registry<S: Store>(
    store: &S,
    registry: &RwLock<Registry>,
    settings: &HistorySettings,
    mut shutdown: watch::Receiver<bool>,
) {
    let mut backoff = settings.backoff_initial;
    loop {
        match store.load_twins().await {
            Ok(twins) => {
                let events = registry.write().unwrap().apply_database(&twins);
                for event in &events {
                    if let DatabaseMerge::Evicted {
                        twin_id,
                        name,
                        owner,
                    } = event
                    {
                        warn!(
                            twin = %twin_id, %name, %owner,
                            "the twin name belongs to another twin in the database; nothing is recorded for \
                             this twin any more"
                        );
                    }
                }
                info!(twins = twins.len(), "twin registry loaded");
                return;
            }
            Err(e) => warn!("cannot load the twin registry, retrying in {backoff:?}: {e}"),
        }
        tokio::select! {
            _ = tokio::time::sleep(backoff) => {}
            _ = shutdown.wait_for(|stop| *stop) => return,
        }
        backoff = (backoff * 2).min(settings.backoff_max);
    }
}

/// One pass over the pending registrations. Returns `true` if a transient error stopped it (the record stays
/// pending). Records the database refuses are logged and dropped.
async fn upsert_pass<S: Store>(store: &S, pending: &PendingUpserts) -> bool {
    while let Some(record) = pending.next() {
        match store.upsert_twin(&record).await {
            Ok(()) => pending.finish(&record),
            Err(e) if matches!(e.kind, ErrorKind::BadData | ErrorKind::Permanent) => {
                warn!(twin = %record.twin_id, name = %record.name, "twin not stored: {e}");
                pending.finish(&record);
            }
            Err(e) => {
                warn!(twin = %record.twin_id, "cannot store the twin, will retry: {e}");
                return true;
            }
        }
    }
    false
}

/// Writes the twin registrations to the database: whenever one arrives, and again with backoff after a
/// transient error. At shutdown one last pass is made (at most `shutdown_flush_limit`); what is still not
/// stored then is logged (the rows are recreated by the next `connectToTwin` of those twins).
async fn run_upserts<S: Store>(
    store: Arc<S>,
    pending: Arc<PendingUpserts>,
    settings: HistorySettings,
    mut shutdown: watch::Receiver<bool>,
) {
    let mut backoff = settings.backoff_initial;
    loop {
        // a call in progress must not hold up the shutdown (the record stays pending)
        let failed = tokio::select! {
            failed = upsert_pass(&*store, &pending) => failed,
            _ = shutdown.wait_for(|stop| *stop) => break,
        };
        if *shutdown.borrow() {
            break;
        }
        let wait_for_work = async {
            if failed {
                tokio::time::sleep(backoff).await;
            } else {
                pending.wake.notified().await;
            }
        };
        tokio::select! {
            _ = wait_for_work => {}
            _ = shutdown.wait_for(|stop| *stop) => break,
        }
        backoff = if failed {
            (backoff * 2).min(settings.backoff_max)
        } else {
            settings.backoff_initial
        };
    }
    // final drain: retry until everything is stored or the time is up
    let drain = async {
        let mut backoff = settings.backoff_initial;
        while upsert_pass(&*store, &pending).await {
            tokio::time::sleep(backoff).await;
            backoff = (backoff * 2).min(settings.backoff_max);
        }
    };
    let drained = tokio::time::timeout(settings.shutdown_flush_limit, drain).await;
    let left = pending.len();
    if drained.is_err() || left > 0 {
        warn!(
            registrations = left,
            "twin registrations could not be stored before shutdown"
        );
    }
}

/// Logs the counters when they changed since the last report.
async fn run_reporter(
    stats: Arc<HistoryStats>,
    interval: Duration,
    mut shutdown: watch::Receiver<bool>,
) {
    let mut last = StatsSnapshot::default();
    loop {
        tokio::select! {
            _ = tokio::time::sleep(interval) => {}
            _ = shutdown.wait_for(|stop| *stop) => return,
        }
        let now = stats.snapshot();
        if now != last {
            info!(
                recorded = now.recorded,
                batches = now.batches,
                unknown_twin = now.unknown_twin,
                invalid = now.invalid,
                out_of_window = now.out_of_window,
                queue_full = now.queue_full,
                hub_dropped = now.hub_dropped,
                rejected = now.rejected,
                flush_retries = now.flush_retries,
                "history statistics (totals)"
            );
            last = now;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::broker::{Broker, BrokerLimits};
    use crate::history::fake::FakeStore;
    use crate::history::row::value_type;
    use chrono::Utc;
    use uuid::Uuid;

    fn test_settings() -> HistorySettings {
        HistorySettings {
            queue_capacity: 100,
            batch_size: 50,
            flush_interval: Duration::from_millis(20),
            retention_days: 0,
            maintenance_interval: Duration::from_secs(3600),
            backoff_initial: Duration::from_millis(10),
            backoff_max: Duration::from_millis(50),
            shutdown_flush_limit: Duration::from_millis(500),
            permanent_attempts: 3,
            call_timeout: Duration::from_secs(5),
            registry_queue: 16,
            report_interval: Duration::from_secs(3600),
        }
    }

    fn value_json(value: f64) -> String {
        format!(
            r#"{{"time_point":"{}","value":{value}}}"#,
            crate::payload::time::format_time_point(Utc::now())
        )
    }

    async fn wait_until(mut condition: impl FnMut() -> bool) {
        for _ in 0..500 {
            if condition() {
                return;
            }
            tokio::time::sleep(Duration::from_millis(10)).await;
        }
        panic!("condition not reached in time");
    }

    #[tokio::test]
    async fn from_config_rejects_zero_sizes() {
        let mut config = crate::config::Config::from_args(Vec::<String>::new())
            .unwrap()
            .history;
        assert!(HistorySettings::from_config(&config).is_ok());
        config.batch_size = 0;
        assert!(HistorySettings::from_config(&config).is_err());
    }

    #[tokio::test]
    async fn end_to_end_with_a_fake_store() {
        let broker = Broker::start(0, BrokerLimits::default()).await.unwrap();
        let hub = broker.hub();
        let store = Arc::new(FakeStore::default());
        let twin_id = Uuid::from_u128(7);
        let project_id = Uuid::from_u128(8);
        // a twin stored in a previous run
        store.state.lock().unwrap().twins.push(TwinRecord {
            twin_id,
            project_id,
            name: "Pump".into(),
        });
        let shared = Arc::clone(&store);
        let service =
            HistoryService::start_with(hub.clone(), test_settings(), move |_| async move {
                Some(SharedStore(shared))
            })
            .unwrap();
        wait_until(|| service.registered_twins() == 1).await;

        // a measurement from a client for the twin of the previous run, one for an unknown twin, a bad one,
        // and the server's own initial value
        hub.publish("Pump/Motor/initial", value_json(0.0)).unwrap();
        client_publish(
            broker.port(),
            &[
                ("Pump/Motor/temp", value_json(21.5)),
                ("Nobody/x", value_json(1.0)),
                ("Pump/Motor/bad", "{value=0,\r\n timepoint=\"\"}".to_owned()),
                ("connectToTwin", "{}".to_owned()),
            ],
        )
        .await;

        let stats = service.stats();
        wait_until(|| stats.snapshot().recorded == 1).await;
        let s = stats.snapshot();
        assert_eq!((s.recorded, s.unknown_twin, s.invalid), (1, 1, 1));
        {
            let state = store.state.lock().unwrap();
            let batch = &state.inserts[0];
            assert_eq!(batch.twin_id, vec![twin_id]);
            assert_eq!(batch.project_id, vec![project_id]);
            assert_eq!(batch.element_path, vec!["Motor/temp".to_owned()]);
            assert_eq!(batch.value_type, vec![value_type::REAL]);
            assert_eq!(batch.value_num, vec![Some(21.5)]);
            // the maintenance created the current and the next month
            assert!(state.partitions.len() >= 2);
        }

        // a twin loaded by the twin logic is registered and stored, and the name rule holds
        let observer = service.observer();
        let loaded = |id: u128, name: &str| LoadedTwin {
            twin_id: Uuid::from_u128(id),
            project_id,
            name: name.into(),
        };
        observer.twin_loaded(&loaded(9, "Valve"));
        observer.twin_loaded(&loaded(10, "Valve")); // name taken by twin 9
        wait_until(|| !store.state.lock().unwrap().upserts.is_empty()).await;
        tokio::time::sleep(Duration::from_millis(50)).await;
        {
            let state = store.state.lock().unwrap();
            assert_eq!(state.upserts.len(), 1);
            assert_eq!(state.upserts[0].twin_id, Uuid::from_u128(9));
        }
        assert_eq!(service.registered_twins(), 2);

        broker.shutdown().await;
        service.shutdown().await;
        assert!(store.state.lock().unwrap().closed);
    }

    fn record(n: u128, name: &str) -> TwinRecord {
        TwinRecord {
            twin_id: Uuid::from_u128(n),
            project_id: Uuid::from_u128(100),
            name: name.into(),
        }
    }

    #[tokio::test(start_paused = true)]
    async fn pending_registrations_are_bounded_and_keep_the_newest_version() {
        let pending = PendingUpserts::new(2);
        assert!(pending.push(record(1, "A")));
        assert!(pending.push(record(2, "B")));
        assert!(
            !pending.push(record(3, "C")),
            "bound reached for a new twin"
        );
        assert!(pending.push(record(1, "A2")), "a known twin is replaced");
        assert_eq!(pending.len(), 2);
        let mut names = Vec::new();
        while let Some(r) = pending.next() {
            pending.finish(&r);
            names.push(r.name);
        }
        names.sort();
        assert_eq!(names, ["A2", "B"]);
    }

    #[tokio::test(start_paused = true)]
    async fn failed_upserts_stay_pending_and_are_retried() {
        let store = Arc::new(FakeStore::default());
        store.state.lock().unwrap().fail_next_upserts =
            [ErrorKind::Transient, ErrorKind::Transient]
                .into_iter()
                .collect();
        let pending = Arc::new(PendingUpserts::new(10));
        let (tx, rx) = watch::channel(false);
        let task = tokio::spawn(run_upserts(
            Arc::clone(&store),
            Arc::clone(&pending),
            test_settings(),
            rx,
        ));
        pending.push(record(1, "A"));
        tokio::time::sleep(Duration::from_millis(500)).await;
        assert_eq!(store.state.lock().unwrap().upserts, vec![record(1, "A")]);
        assert!(pending.is_empty());
        // a registration that the database refuses is dropped, not retried forever
        store
            .state
            .lock()
            .unwrap()
            .fail_next_upserts
            .push_back(ErrorKind::BadData);
        pending.push(record(2, "B"));
        tokio::time::sleep(Duration::from_millis(500)).await;
        assert!(pending.is_empty());
        assert_eq!(store.state.lock().unwrap().upserts.len(), 1);
        tx.send(true).unwrap();
        task.await.unwrap();
    }

    #[tokio::test(start_paused = true)]
    async fn pending_registrations_are_drained_at_shutdown() {
        let store = Arc::new(FakeStore::default());
        store
            .state
            .lock()
            .unwrap()
            .fail_next_upserts
            .push_back(ErrorKind::Transient);
        let pending = Arc::new(PendingUpserts::new(10));
        pending.push(record(1, "A"));
        pending.push(record(2, "B"));
        let (tx, rx) = watch::channel(true); // shutdown is already requested
        run_upserts(
            Arc::clone(&store),
            Arc::clone(&pending),
            test_settings(),
            rx,
        )
        .await;
        drop(tx);
        assert_eq!(store.state.lock().unwrap().upserts.len(), 2);
        assert!(pending.is_empty());
    }

    #[tokio::test(start_paused = true)]
    async fn the_shutdown_drain_is_bounded_when_the_database_hangs() {
        let store = Arc::new(FakeStore::default());
        store.state.lock().unwrap().hang = true;
        let pending = Arc::new(PendingUpserts::new(10));
        pending.push(record(1, "A"));
        let (tx, rx) = watch::channel(true);
        let started = tokio::time::Instant::now();
        run_upserts(
            Arc::clone(&store),
            Arc::clone(&pending),
            test_settings(),
            rx,
        )
        .await;
        drop(tx);
        assert!(started.elapsed() <= Duration::from_secs(1));
        assert_eq!(
            pending.len(),
            1,
            "nothing is lost silently: it is reported as not stored"
        );
    }

    #[tokio::test]
    async fn rows_queued_before_the_database_was_reached_are_counted_at_shutdown() {
        let broker = Broker::start(0, BrokerLimits::default()).await.unwrap();
        let service =
            HistoryService::start_with(broker.hub(), test_settings(), |mut stop| async move {
                let _ = stop.wait_for(|s| *s).await;
                None::<SharedStore>
            })
            .unwrap();
        let observer = service.observer();
        observer.twin_loaded(&LoadedTwin {
            twin_id: Uuid::from_u128(1),
            project_id: Uuid::from_u128(2),
            name: "Pump".into(),
        });
        let stats = service.stats();
        client_publish(
            broker.port(),
            &[
                ("Pump/a", value_json(1.0)),
                ("Pump/b", value_json(2.0)),
                ("Pump/c", value_json(3.0)),
            ],
        )
        .await;
        wait_until(|| stats.snapshot().queued == 3).await;
        broker.shutdown().await;
        service.shutdown().await;
        assert_eq!(stats.snapshot().lost_on_shutdown, 3);
        assert_eq!(stats.snapshot().recorded, 0);
    }

    #[tokio::test]
    async fn registry_load_merges_with_the_state_made_in_memory_meanwhile() {
        let broker = Broker::start(0, BrokerLimits::default()).await.unwrap();
        let store = Arc::new(FakeStore::default());
        {
            let mut s = store.state.lock().unwrap();
            // database: twin 1 owns "Pump", twin 3 is stored as "Old"
            s.twins.push(record(1, "Pump"));
            s.twins.push(record(3, "Old"));
        }
        let shared = Arc::clone(&store);
        // the database becomes reachable only after the twin logic already loaded twins
        let (go_tx, go_rx) = tokio::sync::oneshot::channel::<()>();
        let service =
            HistoryService::start_with(broker.hub(), test_settings(), move |_| async move {
                let _ = go_rx.await;
                Some(SharedStore(shared))
            })
            .unwrap();
        let observer = service.observer();
        let loaded = |id: u128, name: &str| LoadedTwin {
            twin_id: Uuid::from_u128(id),
            project_id: Uuid::from_u128(100),
            name: name.into(),
        };
        observer.twin_loaded(&loaded(2, "Pump")); // takes the name before the database is known
        observer.twin_loaded(&loaded(3, "New")); // renamed in memory
        assert_eq!(service.registered_twins(), 2);
        go_tx.send(()).unwrap();
        // after the load: the database owner (1) has "Pump", twin 2 is gone, twin 3 keeps "New"
        wait_until(|| {
            service.registered_twins() == 2 && store.state.lock().unwrap().partitions.len() >= 2
        })
        .await;
        let stats = service.stats();
        client_publish(
            broker.port(),
            &[
                ("Pump/x", value_json(1.0)),
                ("New/x", value_json(2.0)),
                ("Old/x", value_json(3.0)),
            ],
        )
        .await;
        wait_until(|| stats.snapshot().recorded == 2).await;
        let s = stats.snapshot();
        assert_eq!(
            (s.recorded, s.unknown_twin),
            (2, 1),
            "Old/x is unknown (twin 3 was renamed)"
        );
        {
            let state = store.state.lock().unwrap();
            let mut owners: Vec<(String, Uuid)> = state
                .inserts
                .iter()
                .flat_map(|c| {
                    c.element_path
                        .iter()
                        .cloned()
                        .zip(c.twin_id.iter().copied())
                })
                .collect();
            owners.sort();
            assert_eq!(
                owners,
                vec![
                    ("x".to_owned(), Uuid::from_u128(1)),
                    ("x".to_owned(), Uuid::from_u128(3))
                ]
            );
        }
        broker.shutdown().await;
        service.shutdown().await;
    }

    /// Publishes QoS 0 messages as an MQTT client (rumqttc) and waits until they are sent.
    async fn client_publish(port: u16, messages: &[(&str, String)]) {
        use rumqttc::{AsyncClient, MqttOptions, QoS};
        let mut options = MqttOptions::new("history-test", "127.0.0.1", port);
        options.set_keep_alive(Duration::from_secs(30));
        let (client, mut eventloop) = AsyncClient::new(options, 16);
        let poller = tokio::spawn(async move { while eventloop.poll().await.is_ok() {} });
        for (topic, payload) in messages {
            client
                .publish(*topic, QoS::AtMostOnce, false, payload.clone())
                .await
                .unwrap();
        }
        tokio::time::sleep(Duration::from_millis(300)).await;
        client.disconnect().await.ok();
        poller.abort();
    }

    /// Lets the test keep a handle on the store the service owns.
    struct SharedStore(Arc<FakeStore>);

    impl Store for SharedStore {
        fn insert_batch(
            &self,
            rows: &row::BatchColumns,
        ) -> impl std::future::Future<Output = Result<(), StoreError>> + Send {
            self.0.insert_batch(rows)
        }
        fn create_partition(
            &self,
            month: partition::Month,
        ) -> impl std::future::Future<Output = Result<(), StoreError>> + Send {
            self.0.create_partition(month)
        }
        fn list_partitions(
            &self,
        ) -> impl std::future::Future<Output = Result<Vec<partition::Month>, StoreError>> + Send
        {
            self.0.list_partitions()
        }
        fn drop_partition(
            &self,
            month: partition::Month,
        ) -> impl std::future::Future<Output = Result<(), StoreError>> + Send {
            self.0.drop_partition(month)
        }
        fn upsert_twin(
            &self,
            twin: &TwinRecord,
        ) -> impl std::future::Future<Output = Result<(), StoreError>> + Send {
            self.0.upsert_twin(twin)
        }
        fn load_twins(
            &self,
        ) -> impl std::future::Future<Output = Result<Vec<TwinRecord>, StoreError>> + Send {
            self.0.load_twins()
        }
        fn close(&self) -> impl std::future::Future<Output = ()> + Send {
            self.0.close()
        }
    }
}
