//! The database operations the history needs, behind a trait so the writer, the maintenance and the
//! retry logic can be tested without PostgreSQL.

use std::future::Future;
use std::time::Duration;

use thiserror::Error;

use super::partition::Month;
use super::registry::TwinRecord;
use super::row::BatchColumns;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ErrorKind {
    /// Connection, timeout, resource and lock problems: retry later with backoff.
    Transient,
    /// The data of (some) rows is rejected (SQLSTATE class 22 or 23): split the batch, drop the bad rows.
    BadData,
    /// "no partition of relation found for row" (23514): the partition was dropped meanwhile, create it again.
    MissingPartition,
    /// Everything else the database refuses (permissions, syntax, missing objects).
    Permanent,
}

#[derive(Debug, Clone, Error)]
#[error("{message}")]
pub struct StoreError {
    pub kind: ErrorKind,
    /// Never contains the database URL or a password.
    pub message: String,
}

impl StoreError {
    pub fn new(kind: ErrorKind, message: impl Into<String>) -> Self {
        Self {
            kind,
            message: message.into(),
        }
    }
}

/// Classifies an SQLSTATE. `None` means the error did not come from the database server (I/O, pool timeout).
pub fn classify_sqlstate(sqlstate: Option<&str>) -> ErrorKind {
    match sqlstate {
        None => ErrorKind::Transient,
        Some("23514") => ErrorKind::MissingPartition,
        Some(code) if code.starts_with("22") || code.starts_with("23") => ErrorKind::BadData,
        Some(code)
            if ["08", "40", "53", "55", "57", "58"]
                .iter()
                .any(|class| code.starts_with(class)) =>
        {
            ErrorKind::Transient
        }
        Some(_) => ErrorKind::Permanent,
    }
}

/// SQLSTATEs that `CREATE TABLE IF NOT EXISTS ... PARTITION OF` returns when another session created the same
/// partition at the same moment (42P07 duplicate table, 23505 unique violation in the catalog). They are only a
/// success if the partition is really attached to `measurement` afterwards (a detached table of the same name
/// is not).
pub fn is_partition_race(sqlstate: Option<&str>) -> bool {
    matches!(sqlstate, Some("42P07" | "23505"))
}

/// Wraps a store: every call gets a time limit, an overrun is a transient error. This bounds a hanging
/// connection or a lock wait the server side timeouts did not catch.
pub struct TimedStore<S> {
    inner: S,
    limit: Duration,
}

impl<S> TimedStore<S> {
    pub fn new(inner: S, limit: Duration) -> Self {
        Self { inner, limit }
    }

    async fn bounded<T>(
        &self,
        what: &str,
        call: impl Future<Output = Result<T, StoreError>>,
    ) -> Result<T, StoreError> {
        match tokio::time::timeout(self.limit, call).await {
            Ok(result) => result,
            Err(_) => Err(StoreError::new(
                ErrorKind::Transient,
                format!("{what} did not finish within {:?}", self.limit),
            )),
        }
    }
}

impl<S: Store> Store for TimedStore<S> {
    async fn insert_batch(&self, rows: &BatchColumns) -> Result<(), StoreError> {
        self.bounded("insert of a batch", self.inner.insert_batch(rows))
            .await
    }
    async fn create_partition(&self, month: Month) -> Result<(), StoreError> {
        self.bounded("creating a partition", self.inner.create_partition(month))
            .await
    }
    async fn list_partitions(&self) -> Result<Vec<Month>, StoreError> {
        self.bounded("listing the partitions", self.inner.list_partitions())
            .await
    }
    async fn drop_partition(&self, month: Month) -> Result<(), StoreError> {
        self.bounded("dropping a partition", self.inner.drop_partition(month))
            .await
    }
    async fn upsert_twin(&self, twin: &TwinRecord) -> Result<(), StoreError> {
        self.bounded("storing a twin", self.inner.upsert_twin(twin))
            .await
    }
    async fn load_twins(&self) -> Result<Vec<TwinRecord>, StoreError> {
        self.bounded("loading the twin registry", self.inner.load_twins())
            .await
    }
    async fn close(&self) {
        let _ = tokio::time::timeout(self.limit, self.inner.close()).await;
    }
}

/// Operations on the history database. Implemented by `PgStore` and by fakes in the tests.
pub trait Store: Send + Sync + 'static {
    /// Inserts all rows in ONE statement, or none.
    fn insert_batch(
        &self,
        rows: &BatchColumns,
    ) -> impl Future<Output = Result<(), StoreError>> + Send;
    /// `CREATE TABLE IF NOT EXISTS ... PARTITION OF`; safe to call for an existing partition.
    fn create_partition(&self, month: Month)
        -> impl Future<Output = Result<(), StoreError>> + Send;
    fn list_partitions(&self) -> impl Future<Output = Result<Vec<Month>, StoreError>> + Send;
    fn drop_partition(&self, month: Month) -> impl Future<Output = Result<(), StoreError>> + Send;
    /// Inserts the twin or updates project, name and `last_seen`.
    fn upsert_twin(&self, twin: &TwinRecord)
        -> impl Future<Output = Result<(), StoreError>> + Send;
    /// All registered twins, oldest first.
    fn load_twins(&self) -> impl Future<Output = Result<Vec<TwinRecord>, StoreError>> + Send;
    fn close(&self) -> impl Future<Output = ()> + Send;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn partition_race_sqlstates() {
        assert!(is_partition_race(Some("42P07")));
        assert!(is_partition_race(Some("23505")));
        assert!(!is_partition_race(Some("42501")));
        assert!(!is_partition_race(None));
    }

    #[tokio::test(start_paused = true)]
    async fn timed_store_turns_a_hanging_call_into_a_transient_error() {
        use crate::history::fake::FakeStore;
        let fake = FakeStore::default();
        fake.state.lock().unwrap().hang = true;
        let store = TimedStore::new(fake, Duration::from_secs(45));
        let started = tokio::time::Instant::now();
        let err = store
            .create_partition(Month::new(2026, 11).unwrap())
            .await
            .unwrap_err();
        assert_eq!(err.kind, ErrorKind::Transient);
        assert_eq!(started.elapsed(), Duration::from_secs(45));
        assert_eq!(
            store.load_twins().await.unwrap_err().kind,
            ErrorKind::Transient
        );
        // a healthy store passes through
        store.inner.state.lock().unwrap().hang = false;
        store
            .create_partition(Month::new(2026, 11).unwrap())
            .await
            .unwrap();
        store.close().await;
    }

    #[test]
    fn sqlstate_classes() {
        assert_eq!(classify_sqlstate(None), ErrorKind::Transient);
        assert_eq!(
            classify_sqlstate(Some("23514")),
            ErrorKind::MissingPartition
        );
        assert_eq!(classify_sqlstate(Some("23505")), ErrorKind::BadData);
        assert_eq!(classify_sqlstate(Some("22021")), ErrorKind::BadData);
        assert_eq!(classify_sqlstate(Some("22P02")), ErrorKind::BadData);
        for transient in ["08006", "40001", "53300", "57P01", "55P03", "58030"] {
            assert_eq!(
                classify_sqlstate(Some(transient)),
                ErrorKind::Transient,
                "{transient}"
            );
        }
        for permanent in ["42501", "42P01", "42601", "3F000"] {
            assert_eq!(
                classify_sqlstate(Some(permanent)),
                ErrorKind::Permanent,
                "{permanent}"
            );
        }
    }
}
