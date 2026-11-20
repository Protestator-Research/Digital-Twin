//! Twin logic of the server (`DigitalTwinServerInstanceManager::createDTTopicAndCallback` and
//! `DigitalTwinManager::generateMQTTInterface` in C++), written against the [`TwinBackend`] trait.
//!
//! - Subscribes to `connectToTwin` on the hub. Every valid payload starts a download on a bounded pool
//!   (default 4, like the C++ `DownloadPool`). Blocking backend calls run in `spawn_blocking`, never on
//!   the network tasks.
//! - At most one download per twin id at a time; a repeated request while one is queued or running is
//!   skipped and logged (like `DownloadsInProgress`). Errors are logged and never end the service.
//! - A download refused as unauthorized (expired session) logs in again and is retried once.
//! - After a successful load the MQTT interface is published through the hub: a valid initial value on
//!   `<TwinName>/<element>` for every variable (a variable whose initial value is invalid is skipped and counted)
//!   and the empty registration request on
//!   `<TwinName>/registerFunction`. Payloads of `<TwinName>/registerFunction` are forwarded to the backend.
//!
//! Login failures: the C++ server lets the exception end `runInstance`. Here the login happens in the
//! background right after startup; if it fails (or the backend is not available) the error is logged and
//! the server keeps running (the broker is useful without Structura). The login is retried before every
//! download until it succeeds. A binary without the `ffi` feature therefore logs the error and answers
//! every `connectToTwin` with a logged error.
//!
//! A twin whose name is empty, contains `+`, `#`, `/` or NUL, or starts with `$`, is rejected before anything is
//! subscribed or published (otherwise `<name>/registerFunction` could be a wildcard filter that receives the
//! registrations of every twin). Element paths that are no valid topic, and elements whose initial value is no valid
//! value payload, are skipped and counted once per load.
//!
//! The initial values are published with [`Origin::Server`], which the history recorder ignores, so the
//! placeholders are never stored as measurements.

pub mod backend;
#[cfg(feature = "ffi")]
pub mod ffi;

use std::collections::{HashMap, HashSet};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, Mutex};
use std::time::Duration;

use thiserror::Error;
use tokio::sync::{watch, Semaphore};
use tokio::task::{JoinHandle, JoinSet};
use tracing::{debug, error, info, warn};
use uuid::Uuid;

pub use backend::{BackendError, TwinBackend, TwinElement, TwinInfo, UnavailableBackend};

use crate::config::SysmlConfig;
use crate::hub::{Hub, HubError, HubSubscription, Message, Origin};
use crate::payload::twin::{ConnectToTwin, FunctionRegistrationRequest};
use crate::payload::values::{classify, Value, ValuePayload};
use crate::payload::{CONNECT_TO_TWIN, REGISTER_FUNCTION};

/// Number of parallel downloads (the C++ `DownloadPool` has 4 threads).
pub const DEFAULT_POOL_SIZE: usize = 4;

#[derive(Debug, Clone)]
pub struct TwinOptions {
    /// Maximum number of downloads running in parallel.
    pub pool_size: usize,
    /// Maximum number of distinct twins queued or running; further requests are skipped (bounds memory
    /// against a client that sends requests for endless random twin ids).
    pub max_pending: usize,
    /// Capacity of the hub subscriptions of the twin logic.
    pub subscription_capacity: usize,
}

impl Default for TwinOptions {
    fn default() -> Self {
        Self {
            pool_size: DEFAULT_POOL_SIZE,
            max_pending: 64,
            subscription_capacity: 256,
        }
    }
}

/// A twin whose model has been loaded.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LoadedTwin {
    pub twin_id: Uuid,
    pub project_id: Uuid,
    pub name: String,
}

/// Is told about every successfully loaded twin, before its interface topics are published. Must not block.
pub trait TwinObserver: Send + Sync {
    fn twin_loaded(&self, twin: &LoadedTwin);
}

#[derive(Debug, Error)]
pub enum TwinError {
    #[error("cannot subscribe to {CONNECT_TO_TWIN}: {0}")]
    Subscribe(#[from] HubError),
}

/// Counters of the twin logic.
#[derive(Debug, Default)]
pub struct TwinStats {
    /// Valid `connectToTwin` requests received.
    pub requests: AtomicU64,
    /// Payloads on `connectToTwin` that are not valid requests.
    pub malformed: AtomicU64,
    /// Requests skipped because the twin was already being downloaded, or too many were pending.
    pub skipped: AtomicU64,
    /// Requests that ended in an error (login or load).
    pub failed: AtomicU64,
    /// Twins loaded and published.
    pub loaded: AtomicU64,
    /// Logins repeated because a download was refused as unauthorized.
    pub relogins: AtomicU64,
    /// Messages seen on the `<TwinName>/registerFunction` topics (including the server's own request).
    pub registration_messages: AtomicU64,
    /// Registrations handed to the backend.
    pub registrations_forwarded: AtomicU64,
    /// Registrations ignored (server's own request, malformed, nameless).
    pub registrations_ignored: AtomicU64,
    /// Elements not published: the path is no valid topic (wildcards, empty levels, NUL) or the initial value is
    /// no valid value payload (including values the C++ side could not serialize).
    pub elements_skipped: AtomicU64,
}

impl TwinStats {
    fn bump(counter: &AtomicU64) {
        counter.fetch_add(1, Ordering::Relaxed);
    }
}

/// A twin name is the first topic level of everything the twin publishes and the filter
/// `<name>/registerFunction` is subscribed to, so it must not contain wildcards or separators.
pub fn check_twin_name(name: &str) -> Result<(), &'static str> {
    if name.is_empty() {
        Err("the name is empty")
    } else if name.contains(['+', '#']) {
        Err("the name contains an MQTT wildcard (+ or #)")
    } else if name.contains('/') {
        Err("the name contains a topic separator (/)")
    } else if name.contains('\0') {
        Err("the name contains a NUL character")
    } else if name.starts_with('$') {
        Err("the name starts with $ (reserved topics)")
    } else {
        Ok(())
    }
}

/// An element path below the twin name: non-empty levels, no wildcards, no NUL.
pub fn valid_element_path(path: &str) -> bool {
    !path.is_empty()
        && !path.contains(['+', '#', '\0'])
        && path.split('/').all(|level| !level.is_empty())
}

struct Shared {
    hub: Hub,
    backend: Arc<dyn TwinBackend>,
    sysml: SysmlConfig,
    observer: Option<Arc<dyn TwinObserver>>,
    options: TwinOptions,
    stats: Arc<TwinStats>,
    in_flight: Mutex<HashSet<Uuid>>,
    pool: Semaphore,
    /// `true` after a successful login. The async mutex serializes login attempts.
    logged_in: tokio::sync::Mutex<bool>,
    /// Forwarding task of `<TwinName>/registerFunction` per twin.
    registrations: Mutex<HashMap<Uuid, JoinHandle<()>>>,
    loaded: Mutex<HashSet<Uuid>>,
}

/// Removes the twin from the in-progress set when dropped (also on panic or abort).
struct InFlight {
    shared: Arc<Shared>,
    twin_id: Uuid,
}

impl Drop for InFlight {
    fn drop(&mut self) {
        self.shared.in_flight.lock().unwrap().remove(&self.twin_id);
    }
}

/// Handle to the running twin logic.
pub struct TwinService {
    shutdown_tx: watch::Sender<bool>,
    task: JoinHandle<()>,
    stats: Arc<TwinStats>,
}

impl TwinService {
    /// Subscribes to `connectToTwin` (so no request after this call is missed) and starts the service.
    /// The login to the backend is started in the background.
    pub fn start(
        hub: Hub,
        backend: Arc<dyn TwinBackend>,
        sysml: SysmlConfig,
        options: TwinOptions,
        observer: Option<Arc<dyn TwinObserver>>,
    ) -> Result<TwinService, TwinError> {
        let subscription = hub.subscribe(CONNECT_TO_TWIN, options.subscription_capacity.max(1))?;
        let stats = Arc::new(TwinStats::default());
        let shared = Arc::new(Shared {
            hub,
            backend,
            sysml,
            observer,
            pool: Semaphore::new(options.pool_size.max(1)),
            options,
            stats: Arc::clone(&stats),
            in_flight: Mutex::new(HashSet::new()),
            logged_in: tokio::sync::Mutex::new(false),
            registrations: Mutex::new(HashMap::new()),
            loaded: Mutex::new(HashSet::new()),
        });
        let (shutdown_tx, shutdown_rx) = watch::channel(false);
        let task = tokio::spawn(run(shared, subscription, shutdown_rx));
        Ok(TwinService {
            shutdown_tx,
            task,
            stats,
        })
    }

    pub fn stats(&self) -> &TwinStats {
        &self.stats
    }

    /// Stops the service: pending downloads are abandoned (a running backend call cannot be interrupted),
    /// the loaded twins are released (at most 5 seconds). Waits at most 10 seconds.
    pub async fn shutdown(self) {
        let _ = self.shutdown_tx.send(true);
        if tokio::time::timeout(Duration::from_secs(10), self.task)
            .await
            .is_err()
        {
            warn!("twin logic did not stop within 10 seconds");
        }
    }

    /// Waits until the service has ended on its own (the hub was closed).
    pub async fn join(self) {
        let _ = self.task.await;
    }
}

async fn run(
    shared: Arc<Shared>,
    mut subscription: HubSubscription,
    mut shutdown: watch::Receiver<bool>,
) {
    let mut tasks: JoinSet<()> = JoinSet::new();
    {
        let shared = Arc::clone(&shared);
        tasks.spawn(async move {
            match shared.ensure_login().await {
                Ok(()) => info!("logged in to the Structura server"),
                Err(e) => error!(
                    "login to the Structura server failed: {e}; connectToTwin requests will \
                     retry the login"
                ),
            }
        });
    }
    loop {
        tokio::select! {
            biased;
            _ = shutdown.wait_for(|stop| *stop) => break,
            message = subscription.recv() => match message {
                Some(message) => shared.on_connect_request(&message, &mut tasks),
                None => break,
            },
            Some(finished) = tasks.join_next(), if !tasks.is_empty() => {
                if let Err(e) = finished {
                    if e.is_panic() {
                        error!("a twin task panicked: {e}");
                    }
                }
            }
        }
    }
    debug!(dropped = subscription.dropped(), "twin logic is stopping");
    tasks.abort_all();
    while tasks.join_next().await.is_some() {}
    let registrations: Vec<_> = shared
        .registrations
        .lock()
        .unwrap()
        .drain()
        .map(|(_, handle)| handle)
        .collect();
    for handle in registrations {
        handle.abort();
    }
    shared.release_all().await;
}

impl Shared {
    fn on_connect_request(self: &Arc<Self>, message: &Message, tasks: &mut JoinSet<()>) {
        let request = match ConnectToTwin::from_json(&message.payload) {
            Ok(request) => request,
            Err(e) => {
                TwinStats::bump(&self.stats.malformed);
                warn!("ignoring malformed {CONNECT_TO_TWIN} payload ({e})");
                return;
            }
        };
        TwinStats::bump(&self.stats.requests);
        {
            let mut in_flight = self.in_flight.lock().unwrap();
            if in_flight.contains(&request.twin_id) {
                TwinStats::bump(&self.stats.skipped);
                warn!(
                    twin = %request.twin_id,
                    "download of digital twin already in progress, request skipped"
                );
                return;
            }
            if in_flight.len() >= self.options.max_pending {
                TwinStats::bump(&self.stats.skipped);
                warn!(
                    twin = %request.twin_id,
                    "too many digital twin downloads pending, request skipped"
                );
                return;
            }
            in_flight.insert(request.twin_id);
        }
        let guard = InFlight {
            shared: Arc::clone(self),
            twin_id: request.twin_id,
        };
        let shared = Arc::clone(self);
        tasks.spawn(async move {
            shared.handle_request(request).await;
            drop(guard);
        });
    }

    /// Logs in unless that already happened. Concurrent callers wait for the same attempt.
    async fn ensure_login(&self) -> Result<(), BackendError> {
        let mut logged_in = self.logged_in.lock().await;
        if *logged_in {
            return Ok(());
        }
        let backend = Arc::clone(&self.backend);
        let sysml = self.sysml.clone();
        let result = tokio::task::spawn_blocking(move || {
            backend.login(&sysml.url, sysml.port, &sysml.username, &sysml.password)
        })
        .await
        .unwrap_or_else(|e| Err(BackendError::Failed(format!("login task failed: {e}"))));
        if result.is_ok() {
            *logged_in = true;
        }
        result
    }

    async fn load_once(&self, request: ConnectToTwin) -> Result<TwinInfo, BackendError> {
        let backend = Arc::clone(&self.backend);
        tokio::task::spawn_blocking(move || backend.load_twin(request.project_id, request.twin_id))
            .await
            .unwrap_or_else(|e| {
                Err(BackendError::Failed(format!(
                    "download ended abnormally: {e}"
                )))
            })
    }

    /// Loads the twin; if the backend refuses the session (expired token) logs in again and tries once more.
    async fn load_with_relogin(&self, request: ConnectToTwin) -> Result<TwinInfo, BackendError> {
        match self.load_once(request).await {
            Err(BackendError::Unauthorized(reason)) => {
                warn!(twin = %request.twin_id, "the session at the backend was refused ({reason}), logging in again");
                TwinStats::bump(&self.stats.relogins);
                *self.logged_in.lock().await = false;
                self.ensure_login().await?;
                self.load_once(request).await
            }
            other => other,
        }
    }

    async fn handle_request(self: &Arc<Self>, request: ConnectToTwin) {
        let twin = request.twin_id;
        // At most `pool_size` downloads run at the same time.
        let Ok(_permit) = self.pool.acquire().await else {
            return;
        };
        if let Err(e) = self.ensure_login().await {
            TwinStats::bump(&self.stats.failed);
            error!(%twin, "cannot load digital twin: login failed: {e}");
            return;
        }
        let info = match self.load_with_relogin(request).await {
            Ok(info) => info,
            Err(e) => {
                TwinStats::bump(&self.stats.failed);
                error!(%twin, "error while downloading digital twin: {e}");
                return;
            }
        };
        if info.name.is_empty() {
            TwinStats::bump(&self.stats.failed);
            error!(%twin, "the backend returned a digital twin without a name");
            return;
        }
        self.publish_interface(&request, &info);
    }

    /// Port of `generateMQTTInterface`, with a valid initial value.
    fn publish_interface(self: &Arc<Self>, request: &ConnectToTwin, info: &TwinInfo) {
        let twin = request.twin_id;
        if let Err(reason) = check_twin_name(&info.name) {
            TwinStats::bump(&self.stats.failed);
            error!(%twin, name = %info.name, "twin not published: {reason}");
            return;
        }
        let register_topic = format!("{}/{REGISTER_FUNCTION}", info.name);

        // Subscribe before publishing, so a registration cannot be missed. The server's own empty
        // request is told apart by its origin.
        match self
            .hub
            .subscribe(&register_topic, self.options.subscription_capacity.max(1))
        {
            Ok(subscription) => {
                let handle = tokio::spawn(forward_registrations(
                    Arc::clone(self),
                    twin,
                    register_topic.clone(),
                    subscription,
                ));
                if let Some(old) = self.registrations.lock().unwrap().insert(twin, handle) {
                    old.abort();
                }
            }
            Err(e) => {
                warn!(%twin, topic = %register_topic, "cannot subscribe to function registrations: {e}")
            }
        }

        // Function topics (payload with `parameters`) are outputs of the twin: a client write there must never
        // reach backend.update_variable, so they are not forwardable paths. Decided by the shape, not by a successful
        // classify: a function whose initial payload is invalid (e.g. a NaN parameter) must not become forwardable.
        let paths: HashSet<String> = info
            .elements
            .iter()
            .filter(|e| !is_function_payload(&e.initial_json))
            .map(|e| e.path.clone())
            .collect();
        match self.hub.subscribe(
            &format!("{}/#", info.name),
            self.options.subscription_capacity.max(1),
        ) {
            Ok(sub) => {
                let handle = tokio::spawn(forward_variable_updates(
                    Arc::clone(self),
                    twin,
                    info.name.clone(),
                    paths,
                    sub,
                ));
                if let Some(old) = self.registrations.lock().unwrap().insert(twin, handle) {
                    old.abort();
                }
            }
            Err(e) => warn!(%twin, "cannot subscribe to variable updates: {e}"),
        }

        if let Some(observer) = &self.observer {
            observer.twin_loaded(&LoadedTwin {
                twin_id: twin,
                project_id: request.project_id,
                name: info.name.clone(),
            });
        }
        self.loaded.lock().unwrap().insert(twin);

        let mut published = 0usize;
        let mut skipped = 0usize;
        for element in &info.elements {
            if !valid_element_path(&element.path) {
                skipped += 1;
                continue;
            }
            let topic = format!("{}/{}", info.name, element.path);
            // the recorder and the clients parse every value, so never publish something they would reject
            if let Err(e) = classify(element.initial_json.as_bytes()) {
                skipped += 1;
                debug!(%twin, %topic, "initial value from the backend is not a valid value payload: {e}");
                continue;
            }
            match self.hub.publish(&topic, element.initial_json.clone()) {
                Ok(()) => published += 1,
                Err(HubError::Closed) => return,
                Err(e) => warn!(%twin, %topic, "cannot publish initial value: {e}"),
            }
        }
        if skipped > 0 {
            self.stats
                .elements_skipped
                .fetch_add(skipped as u64, Ordering::Relaxed);
            warn!(%twin, skipped, "elements with an invalid topic path or initial value were not published");
        }
        let empty = FunctionRegistrationRequest {
            linked_variable_return: String::new(),
            linked_variables: Vec::new(),
            qualified_name: String::new(),
        };
        if let Err(e) = self.hub.publish(&register_topic, empty.to_json()) {
            warn!(%twin, topic = %register_topic, "cannot publish the registration request: {e}");
        }
        TwinStats::bump(&self.stats.loaded);
        info!(
            %twin,
            name = %info.name,
            elements = info.elements.len(),
            published,
            "digital twin loaded, MQTT interface published"
        );
    }

    /// Releases all loaded twins in the backend (shutdown), waiting at most 5 seconds.
    async fn release_all(&self) {
        let twins: Vec<Uuid> = self.loaded.lock().unwrap().drain().collect();
        if twins.is_empty() {
            return;
        }
        let backend = Arc::clone(&self.backend);
        let release = tokio::task::spawn_blocking(move || {
            for twin in twins {
                if let Err(e) = backend.release_twin(twin) {
                    warn!(%twin, "cannot release digital twin: {e}");
                }
            }
        });
        if tokio::time::timeout(Duration::from_secs(5), release)
            .await
            .is_err()
        {
            warn!("releasing the digital twins took longer than 5 seconds");
        }
    }
}

/// Forwards client payloads on `<TwinName>/registerFunction` to the backend.
async fn forward_registrations(
    shared: Arc<Shared>,
    twin: Uuid,
    topic: String,
    mut subscription: HubSubscription,
) {
    while let Some(message) = subscription.recv().await {
        TwinStats::bump(&shared.stats.registration_messages);
        if message.origin == Origin::Server {
            TwinStats::bump(&shared.stats.registrations_ignored);
            continue;
        }
        let request = match FunctionRegistrationRequest::from_json(&message.payload) {
            Ok(request) => request,
            Err(e) => {
                TwinStats::bump(&shared.stats.registrations_ignored);
                warn!(%twin, %topic, "ignoring malformed function registration ({e})");
                continue;
            }
        };
        if request.qualified_name.trim().is_empty() {
            TwinStats::bump(&shared.stats.registrations_ignored);
            debug!(%twin, %topic, "ignoring function registration without a name");
            continue;
        }
        let Ok(json) = String::from_utf8(message.payload.to_vec()) else {
            continue;
        };
        let backend = Arc::clone(&shared.backend);
        TwinStats::bump(&shared.stats.registrations_forwarded);
        match tokio::task::spawn_blocking(move || backend.register_function(twin, &json)).await {
            Ok(Ok(())) => debug!(%twin, function = %request.qualified_name, "function registered"),
            Ok(Err(e)) => {
                warn!(%twin, function = %request.qualified_name, "cannot register function: {e}")
            }
            Err(e) => error!(%twin, "function registration ended abnormally: {e}"),
        }
    }
}

/// True if the payload has the shape of a function payload (a JSON object with `parameters`), valid or not.
fn is_function_payload(json: &str) -> bool {
    serde_json::from_str::<serde_json::Value>(json)
        .ok()
        .and_then(|value| {
            value
                .as_object()
                .map(|object| object.contains_key("parameters"))
        })
        .unwrap_or(false)
}

/// Updates variable of DT in the Backend.
async fn forward_variable_updates(
    shared: Arc<Shared>,
    twin: Uuid,
    name: String,
    paths: HashSet<String>,
    mut sub: HubSubscription,
) {
    let prefix = format!("{name}/");
    while let Some(message) = sub.recv().await {
        if message.origin == Origin::Server {
            continue;
        } // eigene Initialwerte ignorieren
        let Some(path) = message.topic.strip_prefix(&prefix) else {
            continue;
        };
        if !paths.contains(path) {
            continue;
        } // registerFunction, Funktions-Topics usw.
          // ungültige Werte und Funktions-Payloads nicht an C++ geben (eine Variable nimmt nur einen einzelnen Wert an)
        match classify(&message.payload) {
            Ok(ValuePayload {
                value: Value::Function(_),
                ..
            })
            | Err(_) => continue,
            Ok(_) => {}
        }
        let Ok(json) = String::from_utf8(message.payload.to_vec()) else {
            continue;
        };
        let (backend, path) = (Arc::clone(&shared.backend), path.to_owned());
        match tokio::task::spawn_blocking(move || backend.update_variable(twin, &path, &json)).await
        {
            Ok(Ok(())) => {}
            Ok(Err(e)) => warn!(%twin, "cannot update variable: {e}"),
            Err(e) => error!(%twin, "variable update ended abnormally: {e}"),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn function_payloads_are_recognized_by_shape() {
        // valid and invalid (NaN dumped as null) function payloads are both never forwardable
        assert!(is_function_payload(
            r#"{"time_point":"2026-11-19T10:00:00Z","parameters":[{"name":"p","value":1.0}]}"#
        ));
        assert!(is_function_payload(
            r#"{"time_point":"2026-11-19T10:00:00Z","parameters":[{"name":"p","value":null}]}"#
        ));
        assert!(!is_function_payload(
            r#"{"time_point":"2026-11-19T10:00:00Z","value":1.0}"#
        ));
        assert!(!is_function_payload(""));
        assert!(!is_function_payload("[]"));
    }

    #[test]
    fn twin_names() {
        assert!(check_twin_name("Pump").is_ok());
        assert!(check_twin_name("My Pump 2").is_ok());
        assert!(check_twin_name("Pumpe-Ä").is_ok());
        assert!(check_twin_name("a$b").is_ok());
        for bad in ["", "+", "#", "a+b", "a#", "a/b", "/a", "a\0b", "$SYS", "$x"] {
            assert!(check_twin_name(bad).is_err(), "{bad:?}");
        }
    }

    #[test]
    fn element_paths() {
        assert!(valid_element_path("speed"));
        assert!(valid_element_path("Motor/Rotor/speed"));
        for bad in ["", "/", "a//b", "/a", "a/", "a/+", "+", "a/#", "#", "a\0"] {
            assert!(!valid_element_path(bad), "{bad:?}");
        }
    }
}
