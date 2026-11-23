//! The interface between the twin logic and the model backend (the C++ libraries behind the FFI bridge).
//!
//! All calls block (HTTP via libcurl, model parsing), so they are only made from `spawn_blocking`.

use std::fmt;
use std::time::Duration;

use thiserror::Error;
use uuid::Uuid;

use crate::config::{HistoryConfig, SysmlConfig};

/// How the server's own session at the SysMLv2 server is established.
#[derive(Clone, PartialEq, Eq)]
pub enum Credentials {
    /// User name and password: a login request is made and the returned token is used. A refused session (HTTP 401)
    /// means the token expired; logging in again helps.
    Password { username: String, password: String },
    /// An application key, used directly as the bearer token. No request is made, so a wrong key is only noticed by
    /// the first request. A refused session (HTTP 401) means the key is wrong, revoked or lacks rights; there is
    /// nothing to renew.
    ApplicationKey(String),
}

impl fmt::Debug for Credentials {
    /// Never shows the password or the key.
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Credentials::Password { username, .. } => f
                .debug_struct("Password")
                .field("username", username)
                .field("password", &"<redacted>")
                .finish(),
            Credentials::ApplicationKey(_) => f
                .debug_tuple("ApplicationKey")
                .field(&"<redacted>")
                .finish(),
        }
    }
}

impl Credentials {
    /// What the configuration says: the application key if there is one, else user name and password.
    pub fn from_config(sysml: &SysmlConfig) -> Self {
        if sysml.application_key.is_empty() {
            Credentials::Password {
                username: sysml.username.clone(),
                password: sysml.password.clone(),
            }
        } else {
            Credentials::ApplicationKey(sysml.application_key.clone())
        }
    }

    pub fn is_application_key(&self) -> bool {
        matches!(self, Credentials::ApplicationKey(_))
    }
}

/// A variable of a twin with the value it is published with after the load.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TwinElement {
    /// Topic path relative to the twin name (`Component/part/variable`).
    pub path: String,
    /// `ValueEntity::getJson()` of the variable's type: `{"time_point": ..., "value": ...}` (a valid payload of
    /// `payload::values`).
    pub initial_json: String,
}

/// Result of loading a twin: the topic base name and the variables below it.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TwinInfo {
    /// `DigitalTwinModel::digitalTwinName()`, the first topic level of everything the twin publishes.
    pub name: String,
    /// `DigitalTwinModel::getElementValueEntities()`. Functions are not part of it: their MQTT interface exists
    /// only after `registerFunction`.
    pub elements: Vec<TwinElement>,
}

/// Settings of the history store of the backend (`run.history.*`).
#[derive(Clone, PartialEq, Eq)]
pub struct StoreSettings {
    /// PostgreSQL URL, may contain a password: never logged.
    pub database_url: String,
    /// 0 keeps the data forever.
    pub retention_days: u32,
    /// Seconds between two checkpoints of the full state. 0 switches the interval checkpoints off (the
    /// checkpoints at load, in a new month, after data loss and at shutdown remain).
    pub checkpoint_interval_s: u32,
    /// Maximum number of rows (not change sets) waiting for the writer; further ones are dropped and counted.
    /// 0 selects the default of the store, like for `batch_size` and `flush_interval_ms`.
    pub queue_capacity: u32,
    /// Maximum number of rows per insert.
    pub batch_size: u32,
    /// Maximum time rows wait before they are written.
    pub flush_interval_ms: u32,
}

impl fmt::Debug for StoreSettings {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("StoreSettings")
            .field(
                "database_url",
                &if self.database_url.is_empty() {
                    ""
                } else {
                    "<redacted>"
                },
            )
            .field("retention_days", &self.retention_days)
            .field("checkpoint_interval_s", &self.checkpoint_interval_s)
            .field("queue_capacity", &self.queue_capacity)
            .field("batch_size", &self.batch_size)
            .field("flush_interval_ms", &self.flush_interval_ms)
            .finish()
    }
}

impl From<&HistoryConfig> for StoreSettings {
    fn from(config: &HistoryConfig) -> Self {
        Self {
            database_url: config.database_url.clone(),
            retention_days: config.retention_days,
            checkpoint_interval_s: config.checkpoint_interval_s,
            queue_capacity: config.queue_capacity,
            batch_size: config.batch_size,
            flush_interval_ms: config.flush_interval_ms,
        }
    }
}

/// Counters of the history store of the backend. All zero and `enabled == false` if it is not configured.
#[derive(Debug, Default, Clone, Copy, PartialEq, Eq)]
pub struct StoreStats {
    /// The store has been configured.
    pub enabled: bool,
    /// The writer currently has a database connection.
    pub connected: bool,
    /// Rows waiting for the writer (not change sets: one update can write several rows).
    pub pending: u64,
    /// Rows written.
    pub written: u64,
    /// Rows dropped because the queue was full.
    pub dropped: u64,
    /// Repeated write attempts after errors.
    pub retries: u64,
    /// Checkpoints written.
    pub checkpoints: u64,
}

#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum BackendError {
    /// No real backend is linked into this binary (built without the `ffi` feature).
    #[error("twin backend not available")]
    NotAvailable,
    /// The session at the backend is missing or expired (HTTP 401, not logged in). The twin logic logs in again
    /// and retries once.
    #[error("{0}")]
    Unauthorized(String),
    /// The backend reported an error (HTTP error, parse error, unknown twin, ...). Never contains secrets.
    #[error("{0}")]
    Failed(String),
}

/// What the twin logic needs from the model library. Implementations must be callable from several
/// threads (up to the size of the download pool at the same time).
pub trait TwinBackend: Send + Sync + 'static {
    /// Sets up the session at the Structura server and replaces the current one. With a password a login request is
    /// made; with an application key nothing is sent (see [`Credentials`]). The password and the key must never be
    /// logged or be part of an error.
    fn login(&self, url: &str, port: u16, credentials: &Credentials) -> Result<(), BackendError>;

    /// Downloads and parses the model of a twin and keeps it for [`TwinBackend::register_function`].
    /// Loading a twin that is already loaded replaces the old model.
    fn load_twin(&self, project_id: Uuid, twin_id: Uuid) -> Result<TwinInfo, BackendError>;

    /// Hands a `registerFunction` payload (valid JSON) to the model of the twin.
    fn register_function(&self, twin_id: Uuid, request_json: &str) -> Result<(), BackendError>;

    /// Frees the model of the twin. Unknown twins are not an error.
    fn release_twin(&self, twin_id: Uuid) -> Result<(), BackendError>;

    /// Hands a client update of the value of a variable (`path` relative to the twin name, valid value payload)
    /// to the model. `received_at_ms` is when the broker accepted the message (Unix time in milliseconds, UTC);
    /// the history is ordered by it.
    fn update_variable(
        &self,
        twin_id: Uuid,
        path: &str,
        payload_json: &str,
        received_at_ms: i64,
    ) -> Result<(), BackendError>;

    /// Creates the history store and its housekeeping thread. Can be done once: a second call is an error.
    /// Twins loaded before the call are not recorded. It does not touch the database and does not fail because
    /// it is down: the store connects (and applies the schema) in the background, so `Ok` says nothing about
    /// the database. `store_stats().connected` tells whether there is a connection. (A schema that is not the
    /// expected one is reported by the store itself, on its own log.)
    fn configure_store(&self, settings: &StoreSettings) -> Result<(), BackendError>;

    /// The counters of the history store; all zero if it is not configured.
    fn store_stats(&self) -> StoreStats;

    /// Writes the queued changes and a last checkpoint and stops the store, waiting at most `timeout`. Blocks.
    fn shutdown_store(&self, timeout: Duration) -> Result<(), BackendError>;
}

/// Backend of a binary without the `ffi` feature: every request fails with "twin backend not available".
#[derive(Debug, Default, Clone, Copy)]
pub struct UnavailableBackend;

impl TwinBackend for UnavailableBackend {
    fn login(&self, _: &str, _: u16, _: &Credentials) -> Result<(), BackendError> {
        Err(BackendError::NotAvailable)
    }

    fn load_twin(&self, _: Uuid, _: Uuid) -> Result<TwinInfo, BackendError> {
        Err(BackendError::NotAvailable)
    }

    fn register_function(&self, _: Uuid, _: &str) -> Result<(), BackendError> {
        Err(BackendError::NotAvailable)
    }

    fn release_twin(&self, _: Uuid) -> Result<(), BackendError> {
        Err(BackendError::NotAvailable)
    }

    fn update_variable(&self, _: Uuid, _: &str, _: &str, _: i64) -> Result<(), BackendError> {
        Err(BackendError::NotAvailable)
    }

    fn configure_store(&self, _: &StoreSettings) -> Result<(), BackendError> {
        Err(BackendError::NotAvailable)
    }

    fn store_stats(&self) -> StoreStats {
        StoreStats::default()
    }

    fn shutdown_store(&self, _: Duration) -> Result<(), BackendError> {
        Err(BackendError::NotAvailable)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn unavailable_backend_fails_everything() {
        let b = UnavailableBackend;
        let password = Credentials::Password {
            username: "u".into(),
            password: "p".into(),
        };
        assert_eq!(b.login("h", 1, &password), Err(BackendError::NotAvailable));
        let key = Credentials::ApplicationKey("k".into());
        assert_eq!(b.login("h", 1, &key), Err(BackendError::NotAvailable));
        assert_eq!(
            b.load_twin(Uuid::nil(), Uuid::nil()),
            Err(BackendError::NotAvailable)
        );
        assert_eq!(
            b.register_function(Uuid::nil(), "{}"),
            Err(BackendError::NotAvailable)
        );
        assert_eq!(b.release_twin(Uuid::nil()), Err(BackendError::NotAvailable));
        assert_eq!(
            b.update_variable(Uuid::nil(), "a", "{}", 0),
            Err(BackendError::NotAvailable)
        );
        let settings = StoreSettings::from(
            &crate::config::Config::from_args(Vec::<String>::new())
                .unwrap()
                .history,
        );
        assert_eq!(
            b.configure_store(&settings),
            Err(BackendError::NotAvailable)
        );
        assert_eq!(b.store_stats(), StoreStats::default());
        assert!(!b.store_stats().enabled);
        assert_eq!(
            b.shutdown_store(Duration::from_secs(1)),
            Err(BackendError::NotAvailable)
        );
        assert_eq!(
            BackendError::NotAvailable.to_string(),
            "twin backend not available"
        );
    }

    #[test]
    fn store_settings_never_show_the_url() {
        let mut config = crate::config::Config::from_args(Vec::<String>::new())
            .unwrap()
            .history;
        config.database_url = "postgres://user:hunter2@db/history".into();
        let shown = format!("{:?}", StoreSettings::from(&config));
        assert!(!shown.contains("hunter2") && shown.contains("<redacted>"));
    }

    #[test]
    fn credentials_follow_the_configuration_and_never_show_secrets() {
        let mut sysml = crate::config::Config::from_args(Vec::<String>::new())
            .unwrap()
            .sysml;
        sysml.username = "bob".into();
        sysml.password = "hunter2".into();
        sysml.application_key = String::new();
        let c = Credentials::from_config(&sysml);
        assert_eq!(
            c,
            Credentials::Password {
                username: "bob".into(),
                password: "hunter2".into()
            }
        );
        assert!(!c.is_application_key());
        // the key replaces user name and password
        sysml.application_key = "sysml_ak_topsecret".into();
        let c = Credentials::from_config(&sysml);
        assert_eq!(c, Credentials::ApplicationKey("sysml_ak_topsecret".into()));
        assert!(c.is_application_key());
        for shown in [
            format!("{c:?}"),
            format!(
                "{:?}",
                Credentials::from_config(
                    &crate::config::Config::from_args(Vec::<String>::new())
                        .unwrap()
                        .sysml
                )
            ),
        ] {
            assert!(
                !shown.contains("topsecret") && !shown.contains("hunter2"),
                "{shown}"
            );
        }
        let password = Credentials::Password {
            username: "bob".into(),
            password: "hunter2".into(),
        };
        assert!(!format!("{password:?}").contains("hunter2"));
    }
}
