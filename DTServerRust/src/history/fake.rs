//! In-memory [`Store`] for the tests of the writer, the maintenance and the service.

use std::collections::{BTreeSet, HashMap, HashSet, VecDeque};
use std::sync::Mutex;

use super::partition::Month;
use super::registry::TwinRecord;
use super::row::BatchColumns;
use super::store::{ErrorKind, Store, StoreError};

#[derive(Default)]
pub struct FakeState {
    /// Successful inserts, one entry per statement.
    pub inserts: Vec<BatchColumns>,
    pub insert_calls: usize,
    /// Errors for the next insert calls, in order.
    pub fail_next_inserts: VecDeque<ErrorKind>,
    /// Every insert fails with this kind while set.
    pub fail_all_inserts: Option<ErrorKind>,
    /// An insert containing one of these element paths fails with `BadData`.
    pub bad_paths: HashSet<String>,
    pub create_calls: Vec<Month>,
    pub partitions: BTreeSet<Month>,
    pub create_failures: HashMap<Month, ErrorKind>,
    pub dropped: Vec<Month>,
    pub twins: Vec<TwinRecord>,
    pub upserts: Vec<TwinRecord>,
    pub fail_next_upserts: VecDeque<ErrorKind>,
    pub load_failures: usize,
    pub closed: bool,
    /// Every call hangs forever while set.
    pub hang: bool,
}

#[derive(Default)]
pub struct FakeStore {
    pub state: Mutex<FakeState>,
}

impl FakeStore {
    fn hung(&self) -> bool {
        self.state.lock().unwrap().hang
    }

    pub fn recorded_rows(&self) -> usize {
        self.state
            .lock()
            .unwrap()
            .inserts
            .iter()
            .map(|c| c.len())
            .sum()
    }

    pub fn recorded_paths(&self) -> Vec<String> {
        self.state
            .lock()
            .unwrap()
            .inserts
            .iter()
            .flat_map(|c| c.element_path.clone())
            .collect()
    }
}

fn err(kind: ErrorKind, what: &str) -> StoreError {
    StoreError::new(kind, format!("fake store: {what}"))
}

impl Store for FakeStore {
    async fn insert_batch(&self, rows: &BatchColumns) -> Result<(), StoreError> {
        if self.hung() {
            std::future::pending::<()>().await;
        }
        let mut s = self.state.lock().unwrap();
        s.insert_calls += 1;
        if let Some(kind) = s.fail_next_inserts.pop_front() {
            return Err(err(kind, "scripted failure"));
        }
        if let Some(kind) = s.fail_all_inserts {
            return Err(err(kind, "database down"));
        }
        if rows.element_path.iter().any(|p| s.bad_paths.contains(p)) {
            return Err(err(ErrorKind::BadData, "bad row"));
        }
        s.inserts.push(rows.clone());
        Ok(())
    }

    async fn create_partition(&self, month: Month) -> Result<(), StoreError> {
        if self.hung() {
            std::future::pending::<()>().await;
        }
        let mut s = self.state.lock().unwrap();
        s.create_calls.push(month);
        if let Some(kind) = s.create_failures.get(&month) {
            return Err(err(*kind, "cannot create partition"));
        }
        s.partitions.insert(month);
        Ok(())
    }

    async fn list_partitions(&self) -> Result<Vec<Month>, StoreError> {
        if self.hung() {
            std::future::pending::<()>().await;
        }
        Ok(self
            .state
            .lock()
            .unwrap()
            .partitions
            .iter()
            .copied()
            .collect())
    }

    async fn drop_partition(&self, month: Month) -> Result<(), StoreError> {
        if self.hung() {
            std::future::pending::<()>().await;
        }
        let mut s = self.state.lock().unwrap();
        s.partitions.remove(&month);
        s.dropped.push(month);
        Ok(())
    }

    async fn upsert_twin(&self, twin: &TwinRecord) -> Result<(), StoreError> {
        if self.hung() {
            std::future::pending::<()>().await;
        }
        let mut s = self.state.lock().unwrap();
        if let Some(kind) = s.fail_next_upserts.pop_front() {
            return Err(err(kind, "scripted upsert failure"));
        }
        s.upserts.push(twin.clone());
        Ok(())
    }

    async fn load_twins(&self) -> Result<Vec<TwinRecord>, StoreError> {
        if self.hung() {
            std::future::pending::<()>().await;
        }
        let mut s = self.state.lock().unwrap();
        if s.load_failures > 0 {
            s.load_failures -= 1;
            return Err(err(ErrorKind::Transient, "cannot load twins"));
        }
        Ok(s.twins.clone())
    }

    async fn close(&self) {
        self.state.lock().unwrap().closed = true;
    }
}
