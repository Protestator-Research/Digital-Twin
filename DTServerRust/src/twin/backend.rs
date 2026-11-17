//! The interface between the twin logic and the model backend (the C++ libraries behind the FFI bridge).
//!
//! All calls block (HTTP via libcurl, model parsing), so they are only made from `spawn_blocking`.

use thiserror::Error;
use uuid::Uuid;

/// Result of loading a twin: the topic base name and the element paths below it.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TwinInfo {
    /// `DigitalTwinModel::digitalTwinName()`, the first topic level of everything the twin publishes.
    pub name: String,
    /// `DigitalTwinModel::getElementStrings()`, paths relative to the name (`Component/part/variable`).
    pub elements: Vec<String>,
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
    /// Logs in to the Structura server. The password must never be logged.
    fn login(
        &self,
        url: &str,
        port: u16,
        username: &str,
        password: &str,
    ) -> Result<(), BackendError>;

    /// Downloads and parses the model of a twin and keeps it for [`TwinBackend::register_function`].
    /// Loading a twin that is already loaded replaces the old model.
    fn load_twin(&self, project_id: Uuid, twin_id: Uuid) -> Result<TwinInfo, BackendError>;

    /// Hands a `registerFunction` payload (valid JSON) to the model of the twin.
    fn register_function(&self, twin_id: Uuid, request_json: &str) -> Result<(), BackendError>;

    /// Frees the model of the twin. Unknown twins are not an error.
    fn release_twin(&self, twin_id: Uuid) -> Result<(), BackendError>;
}

/// Backend of a binary without the `ffi` feature: every request fails with "twin backend not available".
#[derive(Debug, Default, Clone, Copy)]
pub struct UnavailableBackend;

impl TwinBackend for UnavailableBackend {
    fn login(&self, _: &str, _: u16, _: &str, _: &str) -> Result<(), BackendError> {
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
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn unavailable_backend_fails_everything() {
        let b = UnavailableBackend;
        assert_eq!(b.login("h", 1, "u", "p"), Err(BackendError::NotAvailable));
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
            BackendError::NotAvailable.to_string(),
            "twin backend not available"
        );
    }
}
