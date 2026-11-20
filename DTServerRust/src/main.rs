use std::sync::Arc;
use std::time::Duration;

use dt_server::api::{ApiService, BasicAuth};
use dt_server::auth::{redact_url, AuthMode, Authenticator, SysmlAuthOptions, SysmlAuthenticator};
use dt_server::broker::{Broker, BrokerOptions};
use dt_server::config::Config;
use dt_server::twin::{StoreSettings, StoreStats, TwinBackend, TwinOptions, TwinService};
use tracing::{error, info, warn};

fn main() -> std::process::ExitCode {
    tracing_subscriber::fmt()
        .with_env_filter(
            tracing_subscriber::EnvFilter::try_from_default_env().unwrap_or_else(|_| "info".into()),
        )
        .init();

    let runtime = match tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
    {
        Ok(runtime) => runtime,
        Err(e) => {
            error!("cannot start the async runtime: {e}");
            return std::process::ExitCode::FAILURE;
        }
    };
    let code = runtime.block_on(run());
    // A backend call that hangs (HTTP download in the C++ library) cannot be cancelled; do not wait for it.
    runtime.shutdown_timeout(Duration::from_secs(5));
    code
}

/// The backend of this binary: the C++ libraries with `--features ffi`, otherwise one that fails every request.
fn backend() -> Arc<dyn TwinBackend> {
    #[cfg(feature = "ffi")]
    {
        Arc::new(dt_server::twin::ffi::FfiBackend::new())
    }
    #[cfg(not(feature = "ffi"))]
    {
        tracing::warn!(
            "built without the `ffi` feature: digital twins cannot be loaded (twin backend not available)"
        );
        Arc::new(dt_server::twin::UnavailableBackend)
    }
}

async fn run() -> std::process::ExitCode {
    let config = match Config::from_os_args(std::env::args_os()) {
        Ok(config) => config,
        Err(e) => {
            error!("{e}");
            return std::process::ExitCode::FAILURE;
        }
    };
    // Debug output of the configuration redacts passwords.
    info!(?config, "configuration loaded");

    let authenticator: Option<Arc<dyn Authenticator>> = match config.mqtt.authentication {
        AuthMode::None => {
            warn!("MQTT authentication is disabled (run.mqtt.authentication = \"none\"): every client is accepted");
            None
        }
        AuthMode::Sysml => {
            if config.sysml.url.trim().is_empty() {
                error!("MQTT authentication needs the SysML backend: sysml.url is empty (or set run.mqtt.authentication to \"none\")");
                return std::process::ExitCode::FAILURE;
            }
            let options = SysmlAuthOptions {
                cache_ttl: Duration::from_secs(config.mqtt.auth_cache_s),
                max_concurrent: config.mqtt.auth_max_concurrent,
                ..SysmlAuthOptions::default()
            };
            match SysmlAuthenticator::new(&config.sysml.url, config.sysml.port, options) {
                Ok(auth) => {
                    if config.sysml.url.contains("://") && !matches!(config.sysml.port, 80 | 443) {
                        warn!(
                            port = config.sysml.port,
                            "sysml.url contains a scheme, so sysml.port is ignored for the login URL"
                        );
                    }
                    info!(login_url = %redact_url(auth.url()), "MQTT clients are authenticated against the SysML backend");
                    Some(Arc::new(auth))
                }
                Err(e) => {
                    error!("cannot set up the MQTT authentication: {e}");
                    return std::process::ExitCode::FAILURE;
                }
            }
        }
    };
    let broker = match Broker::start_with(
        config.mqtt.port,
        config.mqtt.limits.clone(),
        BrokerOptions {
            protocol: config.mqtt.protocol,
            authenticator: authenticator.clone(),
            throttle: config.mqtt.auth_throttle.clone(),
        },
    )
    .await
    {
        Ok(broker) => broker,
        Err(e) => {
            error!("cannot start MQTT broker on port {}: {e}", config.mqtt.port);
            return std::process::ExitCode::FAILURE;
        }
    };

    let backend = backend();

    // The history store lives in the backend (the C++ core). It is configured before the twin logic starts: a
    // twin loaded earlier would not be recorded.
    let store_running = start_store(&backend, &config).await;
    let stats_logger = store_running.then(|| tokio::spawn(log_store_stats(Arc::clone(&backend))));

    let api = if config.api.listen.trim().is_empty() {
        info!("the REST API is disabled (no run.api.listen or DT_API_LISTEN)");
        None
    } else {
        let database_url = config.api.effective_database_url(&config.history);
        if database_url.is_empty() {
            error!(
                "the REST API needs a database: set run.api.database_url or run.history.database_url \
                 (or DT_API_DATABASE_URL / DT_HISTORY_DATABASE_URL), or clear run.api.listen"
            );
            broker.shutdown().await;
            if store_running {
                stop_store(&backend, stats_logger).await;
            }
            return std::process::ExitCode::FAILURE;
        }
        // The same credentials and the same failure counting as the MQTT clients, when MQTT authentication is on.
        let auth = authenticator.map(|authenticator| {
            BasicAuth::new(authenticator, broker.throttle(), config.api.behind_proxy)
        });
        let authenticated = auth.is_some();
        if config.api.behind_proxy {
            info!("REST API: the client address is taken from the last X-Forwarded-For entry (run.api.behind_proxy)");
        }
        match ApiService::start(&config.api.listen, database_url, auth).await {
            Ok(api) => {
                let addr = api.local_addr();
                if authenticated {
                    info!(%addr, "REST API listening (read-only, HTTP Basic authentication as for MQTT)");
                } else if addr.ip().is_loopback() {
                    info!(%addr, "REST API listening (read-only, no authentication, local access only)");
                } else {
                    warn!(
                        %addr,
                        "REST API listening WITHOUT authentication on a non-local address: everyone who can reach it \
                         can read the whole history; enable MQTT authentication, bind to localhost or use a reverse proxy"
                    );
                }
                Some(api)
            }
            Err(e) => {
                error!("cannot start the REST API: {e}");
                broker.shutdown().await;
                if store_running {
                    stop_store(&backend, stats_logger).await;
                }
                return std::process::ExitCode::FAILURE;
            }
        }
    };

    let twin = match TwinService::start(
        broker.hub(),
        Arc::clone(&backend),
        config.sysml.clone(),
        TwinOptions::default(),
    ) {
        Ok(twin) => twin,
        Err(e) => {
            error!("cannot start the twin logic: {e}");
            broker.shutdown().await;
            if let Some(api) = api {
                api.shutdown().await;
            }
            if store_running {
                stop_store(&backend, stats_logger).await;
            }
            return std::process::ExitCode::FAILURE;
        }
    };

    wait_for_signal().await;
    info!("shutting down");
    // Closing the broker closes the hub: the twin logic sees the end of its stream. The twin logic is stopped
    // before the store, so that no update arrives at a store that is shutting down.
    broker.shutdown().await;
    twin.shutdown().await;
    if let Some(api) = api {
        api.shutdown().await;
    }
    if store_running {
        stop_store(&backend, stats_logger).await;
    }
    std::process::ExitCode::SUCCESS
}

/// Time the store gets to write its queue and the last checkpoint at shutdown.
const STORE_SHUTDOWN_TIMEOUT: Duration = Duration::from_secs(10);

/// Configures the history store of the backend if a database URL is configured. Returns whether it runs. A
/// failure is logged and the server continues without history. The URL is never logged.
async fn start_store(backend: &Arc<dyn TwinBackend>, config: &Config) -> bool {
    if config.history.database_url.is_empty() {
        info!("history is disabled: no run.history.database_url or DT_HISTORY_DATABASE_URL");
        return false;
    }
    if !cfg!(feature = "ffi") {
        // Said once, at startup: the history is written by the C++ core, so there is nothing to configure here.
        warn!(
            "history is NOT recorded: a database URL is configured, but this binary was built without the `ffi` \
             feature and the history is written by the C++ core"
        );
        return false;
    }
    let settings = StoreSettings::from(&config.history);
    let configure = {
        let backend = Arc::clone(backend);
        let settings = settings.clone();
        tokio::task::spawn_blocking(move || backend.configure_store(&settings))
    };
    match configure.await {
        Ok(Ok(())) => {
            // the Debug output of the settings redacts the URL
            info!(
                ?settings,
                "history store configured: the C++ core connects to PostgreSQL in the background (watch `connected` in the history log lines)"
            );
            true
        }
        Ok(Err(e)) => {
            error!("history is NOT recorded: cannot start the history store: {e}");
            false
        }
        Err(e) => {
            error!("history is NOT recorded: the history store setup ended abnormally: {e}");
            false
        }
    }
}

/// Stops the logger, then lets the store write what is queued, waiting at most [`STORE_SHUTDOWN_TIMEOUT`].
async fn stop_store(
    backend: &Arc<dyn TwinBackend>,
    stats_logger: Option<tokio::task::JoinHandle<()>>,
) {
    if let Some(logger) = stats_logger {
        logger.abort();
    }
    let shutdown = {
        let backend = Arc::clone(backend);
        tokio::task::spawn_blocking(move || backend.shutdown_store(STORE_SHUTDOWN_TIMEOUT))
    };
    // the backend keeps the limit itself; the extra seconds are for the call to return
    match tokio::time::timeout(STORE_SHUTDOWN_TIMEOUT + Duration::from_secs(5), shutdown).await {
        Ok(Ok(Ok(()))) => {
            let stats = backend.store_stats();
            info!(
                written_rows = stats.written,
                dropped_rows = stats.dropped,
                checkpoints = stats.checkpoints,
                "history store stopped"
            );
        }
        Ok(Ok(Err(e))) => warn!("history store did not stop cleanly: {e}"),
        Ok(Err(e)) => warn!("history store shutdown ended abnormally: {e}"),
        Err(_) => {
            warn!("history store did not stop within the time limit; queued changes may be lost")
        }
    }
}

/// How long the store has to connect before a missing connection is worth a warning.
const CONNECT_GRACE: Duration = Duration::from_secs(30);

/// What the connection state of the history store means for the log.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum ConnectionEvent {
    /// Connected (the first time, or again after a loss).
    Connected,
    /// The connection was there and is gone.
    Lost,
    /// There was never a connection and the grace period is over.
    StillNotConnected,
}

/// Turns the observed `connected` flag into log events. At startup the store is not connected for a moment, which
/// is not worth a warning: that comes only if it stays that way for [`CONNECT_GRACE`], or when a connection is lost.
#[derive(Debug, Default)]
struct ConnectionWatch {
    up: bool,
    ever_connected: bool,
    warned: bool,
}

impl ConnectionWatch {
    /// `since_start`: time since the store was configured.
    fn observe(&mut self, connected: bool, since_start: Duration) -> Option<ConnectionEvent> {
        if connected {
            let event = (!self.up).then_some(ConnectionEvent::Connected);
            self.up = true;
            self.ever_connected = true;
            self.warned = false;
            return event;
        }
        if self.up {
            self.up = false;
            self.warned = true;
            return Some(ConnectionEvent::Lost);
        }
        if !self.ever_connected && !self.warned && since_start >= CONNECT_GRACE {
            self.warned = true;
            return Some(ConnectionEvent::StillNotConnected);
        }
        None
    }
}

/// Watches the history store. The configuration call does not touch the database, so whether the core has a
/// connection is only known from `connected`: the first connection and a later loss are logged at once (checked
/// every 2 seconds), a store that does not connect within [`CONNECT_GRACE`] is warned about once. The other
/// counters are logged every 10 seconds, but only when they changed.
async fn log_store_stats(backend: Arc<dyn TwinBackend>) {
    let mut interval = tokio::time::interval(Duration::from_secs(2));
    interval.set_missed_tick_behavior(tokio::time::MissedTickBehavior::Skip);
    let started = tokio::time::Instant::now();
    let mut last = StoreStats::default();
    let mut watch = ConnectionWatch::default();
    let mut ticks = 0u64;
    loop {
        interval.tick().await;
        ticks += 1;
        // only reads counters
        let stats = backend.store_stats();
        match watch.observe(stats.connected, started.elapsed()) {
            Some(ConnectionEvent::Connected) => info!("history database connected"),
            Some(ConnectionEvent::Lost) => warn!(
                "history database connection LOST: changes are queued (and dropped when the queue is full) until \
                 the C++ core connects again; see its [DigitalTwinStore] messages on stderr"
            ),
            Some(ConnectionEvent::StillNotConnected) => warn!(
                "history database NOT connected {} s after the start: changes are queued (and dropped when the \
                 queue is full); see the [DigitalTwinStore] messages of the C++ core on stderr",
                CONNECT_GRACE.as_secs()
            ),
            None => {}
        }
        if ticks.is_multiple_of(5) && stats != last {
            // `pending`, `written` and `dropped` count rows, not change sets
            info!(
                connected = stats.connected,
                pending_rows = stats.pending,
                written_rows = stats.written,
                dropped_rows = stats.dropped,
                retries = stats.retries,
                checkpoints = stats.checkpoints,
                "history"
            );
            last = stats;
        }
    }
}

#[cfg(unix)]
async fn wait_for_signal() {
    use tokio::signal::unix::{signal, SignalKind};
    let mut term = signal(SignalKind::terminate()).expect("SIGTERM handler");
    tokio::select! {
        _ = tokio::signal::ctrl_c() => {}
        _ = term.recv() => {}
    }
}

#[cfg(not(unix))]
async fn wait_for_signal() {
    let _ = tokio::signal::ctrl_c().await;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn a_slow_start_is_not_warned_about() {
        let mut watch = ConnectionWatch::default();
        let secs = Duration::from_secs;
        // disconnected in the first moments: nothing
        assert_eq!(watch.observe(false, secs(0)), None);
        assert_eq!(watch.observe(false, secs(2)), None);
        // the connection comes up: said once
        assert_eq!(
            watch.observe(true, secs(4)),
            Some(ConnectionEvent::Connected)
        );
        assert_eq!(watch.observe(true, secs(6)), None);
        // lost later: warned at once, once
        assert_eq!(watch.observe(false, secs(8)), Some(ConnectionEvent::Lost));
        assert_eq!(watch.observe(false, secs(10)), None);
        assert_eq!(
            watch.observe(true, secs(12)),
            Some(ConnectionEvent::Connected)
        );
    }

    #[test]
    fn a_store_that_never_connects_is_warned_about_once_after_the_grace_period() {
        let mut watch = ConnectionWatch::default();
        let secs = Duration::from_secs;
        assert_eq!(watch.observe(false, secs(29)), None);
        assert_eq!(
            watch.observe(false, CONNECT_GRACE),
            Some(ConnectionEvent::StillNotConnected)
        );
        assert_eq!(watch.observe(false, secs(40)), None);
        // and still says so when it connects in the end
        assert_eq!(
            watch.observe(true, secs(50)),
            Some(ConnectionEvent::Connected)
        );
    }
}
