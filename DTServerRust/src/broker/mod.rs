//! MQTT v5 broker (QoS 0 only). Port of `MqttBrokerService`.

pub mod codec;
pub(crate) mod core;
pub mod limits;
pub(crate) mod session;
pub mod subscriptions;
pub mod topic;
pub mod tracker;

use std::net::{Ipv4Addr, Ipv6Addr, SocketAddr};
use std::sync::Arc;
use std::time::Duration;

use tokio::net::{TcpListener, TcpStream};
use tokio::sync::{mpsc, watch};
use tokio::task::JoinHandle;
use tracing::{info, warn};

pub use limits::BrokerLimits;

use self::core::Core;
use crate::hub::Hub;

/// A running broker. Dropping it starts the same shutdown as [`Broker::shutdown`] (clients receive
/// DISCONNECT "server shutting down", the hub is closed) but does not wait for the sessions to end;
/// use [`Broker::shutdown`] to wait.
pub struct Broker {
    local_addr: SocketAddr,
    hub: Hub,
    core: Arc<Core>,
    shutdown_tx: watch::Sender<bool>,
    accept: JoinHandle<()>,
    sessions_done: mpsc::Receiver<()>,
}

impl Broker {
    /// Binds the listener (dual stack, IPv4 only as fallback) and starts accepting connections.
    /// Port 0 selects an ephemeral port, see [`Broker::local_addr`].
    pub async fn start(port: u16, limits: BrokerLimits) -> std::io::Result<Broker> {
        let listener = bind_dual_stack(port)?;
        let local_addr = listener.local_addr()?;
        info!(%local_addr, "MQTT broker listening");
        let core = Core::new(limits);
        let hub = Hub::new(Arc::clone(&core));
        let (shutdown_tx, shutdown_rx) = watch::channel(false);
        let (alive_tx, sessions_done) = mpsc::channel::<()>(1);
        let accept = tokio::spawn(accept_loop(
            listener,
            Arc::clone(&core),
            shutdown_rx,
            alive_tx,
        ));
        Ok(Broker {
            local_addr,
            hub,
            core,
            shutdown_tx,
            accept,
            sessions_done,
        })
    }

    pub fn local_addr(&self) -> SocketAddr {
        self.local_addr
    }

    /// Port the broker listens on (useful with port 0).
    pub fn port(&self) -> u16 {
        self.local_addr.port()
    }

    pub fn hub(&self) -> Hub {
        self.hub.clone()
    }

    /// Number of open client connections (including those that have not sent CONNECT yet).
    pub fn connection_count(&self) -> usize {
        self.core.tracker.total()
    }

    /// Number of subscriptions (session, filter) of all client sessions.
    pub fn subscription_count(&self) -> usize {
        self.core.store.lock().unwrap().subscription_count()
    }

    fn begin_shutdown(&self) {
        self.core.close();
        let _ = self.shutdown_tx.send(true);
    }

    /// Stops accepting, disconnects all clients (reason code "server shutting down") and waits until
    /// all sessions are gone (at most 5 seconds).
    ///
    /// Afterwards the hub is closed: internal subscriptions end (`recv()` returns `None`) and
    /// `Hub::publish` / `Hub::subscribe` return `HubError::Closed`.
    pub async fn shutdown(mut self) {
        self.begin_shutdown();
        let _ = (&mut self.accept).await;
        if tokio::time::timeout(Duration::from_secs(5), self.sessions_done.recv())
            .await
            .is_err()
        {
            warn!("not all sessions ended within 5 seconds");
        }
        info!("MQTT broker stopped");
    }
}

fn bind_dual_stack(port: u16) -> std::io::Result<TcpListener> {
    match bind_v6(port) {
        Ok(listener) => Ok(listener),
        Err(e) => {
            warn!(error = %e, "dual stack listener not available, falling back to IPv4");
            let std_listener =
                std::net::TcpListener::bind(SocketAddr::from((Ipv4Addr::UNSPECIFIED, port)))?;
            std_listener.set_nonblocking(true)?;
            TcpListener::from_std(std_listener)
        }
    }
}

fn bind_v6(port: u16) -> std::io::Result<TcpListener> {
    use socket2::{Domain, Socket, Type};
    let socket = Socket::new(Domain::IPV6, Type::STREAM, None)?;
    socket.set_only_v6(false)?;
    socket.set_reuse_address(true)?;
    socket.bind(&SocketAddr::from((Ipv6Addr::UNSPECIFIED, port)).into())?;
    socket.listen(1024)?;
    socket.set_nonblocking(true)?;
    TcpListener::from_std(socket.into())
}

async fn accept_loop(
    listener: TcpListener,
    core: Arc<Core>,
    mut shutdown: watch::Receiver<bool>,
    alive: mpsc::Sender<()>,
) {
    loop {
        let accepted = tokio::select! {
            biased;
            _ = shutdown.wait_for(|stop| *stop) => break,
            accepted = listener.accept() => accepted,
        };
        match accepted {
            Ok((stream, peer)) => accept_one(stream, peer, &core, &shutdown, &alive),
            Err(e) => {
                // Back off briefly to avoid a busy loop, e.g. on EMFILE.
                warn!(error = %e, "MQTT accept failed");
                tokio::time::sleep(Duration::from_millis(100)).await;
            }
        }
    }
}

fn accept_one(
    stream: TcpStream,
    peer: SocketAddr,
    core: &Arc<Core>,
    shutdown: &watch::Receiver<bool>,
    alive: &mpsc::Sender<()>,
) {
    let key = tracker::peer_key(peer);
    match core.tracker.acquire_permit(key) {
        Some(permit) => {
            tokio::spawn(session::run_session(
                stream,
                peer,
                Arc::clone(core),
                permit,
                shutdown.clone(),
                alive.clone(),
            ));
        }
        None => {
            warn!(ip = %key, "MQTT connection rejected (connection limit)");
            drop(stream);
        }
    }
}

impl Drop for Broker {
    fn drop(&mut self) {
        self.begin_shutdown();
    }
}
