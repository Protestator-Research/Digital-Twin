use std::sync::Arc;
use std::time::Duration;

use dt_server::broker::Broker;
use dt_server::config::Config;
use dt_server::history::HistoryService;
use dt_server::twin::{TwinBackend, TwinObserver, TwinOptions, TwinService};
use tracing::{error, info};

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

    let broker = match Broker::start(config.mqtt.port, config.mqtt.limits.clone()).await {
        Ok(broker) => broker,
        Err(e) => {
            error!("cannot start MQTT broker on port {}: {e}", config.mqtt.port);
            return std::process::ExitCode::FAILURE;
        }
    };

    // History first, so that the twin logic can register twins with it from the start.
    let history = if config.history.database_url.is_empty() {
        info!(
            "history is disabled: no run.history.database_url or DT_HISTORY_DATABASE_URL, measurements are not recorded"
        );
        None
    } else {
        match HistoryService::start(broker.hub(), &config.history, &config.history.database_url) {
            Ok(history) => {
                info!("history enabled (PostgreSQL)");
                Some(history)
            }
            Err(e) => {
                error!("cannot start the history: {e}");
                broker.shutdown().await;
                return std::process::ExitCode::FAILURE;
            }
        }
    };

    let observer: Option<Arc<dyn TwinObserver>> = history.as_ref().map(HistoryService::observer);
    let twin = match TwinService::start(
        broker.hub(),
        backend(),
        config.sysml.clone(),
        TwinOptions::default(),
        observer,
    ) {
        Ok(twin) => twin,
        Err(e) => {
            error!("cannot start the twin logic: {e}");
            broker.shutdown().await;
            if let Some(history) = history {
                history.shutdown().await;
            }
            return std::process::ExitCode::FAILURE;
        }
    };

    wait_for_signal().await;
    info!("shutting down");
    // Closing the broker closes the hub: the twin logic and the recorder see the end of their streams.
    broker.shutdown().await;
    twin.shutdown().await;
    if let Some(history) = history {
        history.shutdown().await;
    }
    std::process::ExitCode::SUCCESS
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
