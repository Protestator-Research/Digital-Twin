//! Real [`TwinBackend`]: the cxx bridge to the C++ facade in `ffi/` (feature `ffi`).
//!
//! The facade catches every C++ exception and returns an error code and message. All calls block; the twin
//! logic calls them from `spawn_blocking`.

use uuid::Uuid;

use super::backend::{BackendError, TwinBackend, TwinInfo};

#[cxx::bridge(namespace = "dtrs")]
mod bridge {
    /// Result without payload. `code` 0 is success, see `ffi/dt_facade.h`.
    struct CallResult {
        code: i32,
        message: String,
    }

    struct TwinLoad {
        code: i32,
        message: String,
        name: String,
        elements: Vec<String>,
    }

    unsafe extern "C++" {
        include!("dt_facade.h");

        fn facade_login(url: &str, port: u16, username: &str, password: &str) -> CallResult;
        fn facade_load_twin(project_id: &str, twin_id: &str) -> TwinLoad;
        fn facade_register_function(twin_id: &str, request_json: &str) -> CallResult;
        fn facade_release_twin(twin_id: &str) -> CallResult;
    }
}

/// Backend that calls the C++ libraries (`DigitalTwinLib`, `BackendCommunication`).
#[derive(Debug, Default, Clone, Copy)]
pub struct FfiBackend;

impl FfiBackend {
    pub fn new() -> Self {
        Self
    }
}

fn check(code: i32, message: String) -> Result<(), BackendError> {
    if code == 0 {
        Ok(())
    } else {
        // 2 = not logged in, 6 = HTTP 401 (see ffi/dt_facade.h)
        let message = if message.is_empty() {
            format!("backend error (code {code})")
        } else {
            message
        };
        if code == 2 || code == 6 {
            Err(BackendError::Unauthorized(message))
        } else {
            Err(BackendError::Failed(message))
        }
    }
}

impl TwinBackend for FfiBackend {
    fn login(
        &self,
        url: &str,
        port: u16,
        username: &str,
        password: &str,
    ) -> Result<(), BackendError> {
        let r = bridge::facade_login(url, port, username, password);
        check(r.code, r.message)
    }

    fn load_twin(&self, project_id: Uuid, twin_id: Uuid) -> Result<TwinInfo, BackendError> {
        let r = bridge::facade_load_twin(&project_id.to_string(), &twin_id.to_string());
        check(r.code, r.message)?;
        Ok(TwinInfo {
            name: r.name,
            elements: r.elements,
        })
    }

    fn register_function(&self, twin_id: Uuid, request_json: &str) -> Result<(), BackendError> {
        let r = bridge::facade_register_function(&twin_id.to_string(), request_json);
        check(r.code, r.message)
    }

    fn release_twin(&self, twin_id: Uuid) -> Result<(), BackendError> {
        let r = bridge::facade_release_twin(&twin_id.to_string());
        check(r.code, r.message)
    }
}
