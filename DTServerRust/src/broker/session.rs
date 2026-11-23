//! One task per client connection plus a writer task. Port of `Session`.
//!
//! A connection is not a session: the state that outlives it (queue of QoS 1/2 messages, packet identifiers)
//! is a [`ClientSession`], attached to the connection by the CONNECT. Here live the connection itself: the
//! reader loop with keep alive, the writer task, the handling of every packet, and the ordered, possibly
//! deferred sending of acknowledgements.

use std::net::SocketAddr;
use std::sync::atomic::{AtomicU8, AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::Duration;

use bytes::{Bytes, BytesMut};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::tcp::OwnedWriteHalf;
use tokio::net::TcpStream;
use tokio::sync::{mpsc, oneshot, watch, Notify};
use tokio::time::{sleep_until, timeout, Instant};
use tracing::{debug, info, warn};

use super::client::{message_size, ClientSession, Inbound, PubrecAction};
use super::codec::{
    self, connack, disconnect, puback, suback, unsuback, ConnackAccepted, Connect, DecodeError,
    Packet, Publish, QoS,
};
use super::core::{AttachError, AttachRequest, Core};
use super::persist::{Op, SessionStore};
use super::protocol::ProtocolVersion;
use super::subscriptions::AddResult;
use super::topic;
use super::tracker::ConnectionPermit;
use crate::auth::{AuthOutcome, Denied};
use crate::hub::{now_unix_ms, Message, Origin};

pub(crate) enum Outgoing {
    /// A QoS 0 PUBLISH. Counted in the pending bytes of the connection.
    Publish(Message),
    /// A QoS 1/2 PUBLISH. Not counted in the pending bytes: it is counted in the queue of the session until
    /// the client acknowledges it.
    PublishQos {
        message: Message,
        qos: QoS,
        packet_id: u16,
        dup: bool,
    },
    /// A control packet (CONNACK, SUBACK, PINGRESP, ...). Counted like publishes in the pending bytes.
    Raw(Bytes),
    /// Writes the optional final packet (a CONNACK refusing the connection, or the server DISCONNECT of
    /// an MQTT 5 session), then closes the connection.
    Close(Option<Bytes>),
}

/// Result of offering a message to a session.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum Offer {
    Queued,
    Dropped,
    /// Dropped, and this was the Nth consecutive overflow: the session is told to disconnect.
    DroppedAndKicked,
}

/// Why another task asks a connection to end.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum KickReason {
    /// The client does not keep up with its messages.
    Slow,
    /// Another connection took over the session, or the session was destroyed.
    TakenOver,
}

/// Sending side of a connection: the queue towards the writer task with byte accounting.
pub(crate) struct SessionOut {
    tx: mpsc::UnboundedSender<Outgoing>,
    pending: AtomicUsize,
    overflows: AtomicUsize,
    max_pending: usize,
    max_overflows: usize,
    kick: Notify,
    /// 0 = not kicked, else the [`KickReason`]; the first reason stays.
    kick_reason: AtomicU8,
    writer_done: Notify,
    /// Protocol version of the connection, set when the CONNECT is accepted (0 = not yet).
    version: AtomicU8,
    /// Acknowledgements waiting for a barrier of the store; while there are some, later ones queue up behind
    /// them to keep their order.
    deferred: AtomicUsize,
    /// Woken when `deferred` has decreased.
    deferred_done: Notify,
}

impl SessionOut {
    pub(crate) fn new(
        max_pending: usize,
        max_overflows: usize,
    ) -> (Arc<Self>, mpsc::UnboundedReceiver<Outgoing>) {
        let (tx, rx) = mpsc::unbounded_channel();
        let out = Arc::new(Self {
            tx,
            pending: AtomicUsize::new(0),
            overflows: AtomicUsize::new(0),
            max_pending,
            max_overflows,
            kick: Notify::new(),
            kick_reason: AtomicU8::new(0),
            writer_done: Notify::new(),
            version: AtomicU8::new(0),
            deferred: AtomicUsize::new(0),
            deferred_done: Notify::new(),
        });
        (out, rx)
    }

    /// Records the version negotiated by CONNECT; the writer encodes PUBLISH accordingly.
    fn set_version(&self, version: ProtocolVersion) {
        let v = match version {
            ProtocolVersion::V311 => 1,
            ProtocolVersion::V5 => 2,
        };
        self.version.store(v, Ordering::Release);
    }

    /// The negotiated version, `None` until a CONNECT was accepted.
    fn version(&self) -> Option<ProtocolVersion> {
        match self.version.load(Ordering::Acquire) {
            1 => Some(ProtocolVersion::V311),
            2 => Some(ProtocolVersion::V5),
            _ => None,
        }
    }

    /// Asks the connection to end. The first reason counts. Never blocks.
    pub(crate) fn kick(&self, reason: KickReason) {
        let code = match reason {
            KickReason::Slow => 1,
            KickReason::TakenOver => 2,
        };
        let _ = self
            .kick_reason
            .compare_exchange(0, code, Ordering::AcqRel, Ordering::Acquire);
        self.kick.notify_one();
    }

    /// The reason the connection was asked to end, if it was.
    pub(crate) fn kicked(&self) -> Option<KickReason> {
        match self.kick_reason.load(Ordering::Acquire) {
            1 => Some(KickReason::Slow),
            2 => Some(KickReason::TakenOver),
            _ => None,
        }
    }

    /// Queues a QoS 0 PUBLISH unless the pending bytes (topic + payload) would exceed the limit. The
    /// version dependent bytes of the encoding (properties) are not counted, in any version.
    /// Never blocks.
    pub(crate) fn offer_publish(&self, message: &Message) -> Offer {
        let size = message_size(message);
        let before = self.pending.fetch_add(size, Ordering::AcqRel);
        if before + size > self.max_pending {
            self.pending.fetch_sub(size, Ordering::AcqRel);
            let n = self.overflows.fetch_add(1, Ordering::AcqRel) + 1;
            if n >= self.max_overflows {
                self.kick(KickReason::Slow);
                return Offer::DroppedAndKicked;
            }
            return Offer::Dropped;
        }
        self.overflows.store(0, Ordering::Release);
        if self.tx.send(Outgoing::Publish(message.clone())).is_err() {
            self.pending.fetch_sub(size, Ordering::AcqRel);
            return Offer::Dropped;
        }
        Offer::Queued
    }

    /// Queues a QoS 1/2 PUBLISH. The session queue bounds these, so they are not counted here. Returns false
    /// if the writer is gone.
    pub(crate) fn send_publish(
        &self,
        message: &Message,
        qos: QoS,
        packet_id: u16,
        dup: bool,
    ) -> bool {
        self.tx
            .send(Outgoing::PublishQos {
                message: message.clone(),
                qos,
                packet_id,
                dup,
            })
            .is_ok()
    }

    /// Queues a control packet. The bytes count against the pending limit, so the memory of a client that
    /// does not read is bounded. Returns false (and wakes the session to end it) if the limit is exceeded.
    pub(crate) fn send_control(&self, bytes: Bytes) -> bool {
        let size = bytes.len();
        let before = self.pending.fetch_add(size, Ordering::AcqRel);
        if before + size > self.max_pending || self.tx.send(Outgoing::Raw(bytes)).is_err() {
            self.pending.fetch_sub(size, Ordering::AcqRel);
            self.kick(KickReason::Slow);
            return false;
        }
        true
    }

    /// Sends PUBACK, PUBREC, PUBREL or PUBCOMP of this connection. They have to leave in the order of the
    /// packets that caused them (MQTT-4.6.0-2 to 4), and an acknowledgement that promises durable state
    /// (`durable`) may only leave after the store has made the state durable. So the packet is sent inline
    /// only if it needs no durability and no earlier acknowledgement is still waiting; otherwise it goes
    /// through a barrier of the store, which runs the closures in submission order. Returns false if the
    /// connection has to end because its queue is full (only known for packets sent inline).
    pub(crate) fn send_ordered(
        self: &Arc<Self>,
        persist: Option<&Arc<dyn SessionStore>>,
        durable: bool,
        bytes: Bytes,
    ) -> bool {
        let store = match persist {
            Some(store) if durable || self.deferred.load(Ordering::Acquire) > 0 => store,
            _ => return self.send_control(bytes),
        };
        self.deferred.fetch_add(1, Ordering::AcqRel);
        let out = Arc::clone(self);
        store.barrier(Box::new(move || {
            out.send_control(bytes);
            out.deferred.fetch_sub(1, Ordering::AcqRel);
            out.deferred_done.notify_waiters();
        }));
        true
    }

    fn close(&self, goodbye: Option<Bytes>) {
        let _ = self.tx.send(Outgoing::Close(goodbye));
    }

    #[cfg(test)]
    pub(crate) fn pending(&self) -> usize {
        self.pending.load(Ordering::Acquire)
    }

    #[cfg(test)]
    pub(crate) fn deferred(&self) -> usize {
        self.deferred.load(Ordering::Acquire)
    }
}

async fn writer_task(
    mut wr: OwnedWriteHalf,
    mut rx: mpsc::UnboundedReceiver<Outgoing>,
    out: Arc<SessionOut>,
) {
    let mut buf = BytesMut::with_capacity(1024);
    while let Some(msg) = rx.recv().await {
        let result = match msg {
            Outgoing::Publish(message) => {
                buf.clear();
                // deliveries only start after the CONNACK, so the CONNECT has set the version
                let version = out.version().unwrap_or_default();
                codec::encode_publish(
                    version,
                    &mut buf,
                    &message.topic,
                    &message.payload,
                    QoS::AtMostOnce,
                    0,
                    false,
                );
                let r = wr.write_all(&buf).await;
                out.pending
                    .fetch_sub(message_size(&message), Ordering::AcqRel);
                r
            }
            Outgoing::PublishQos {
                message,
                qos,
                packet_id,
                dup,
            } => {
                buf.clear();
                let version = out.version().unwrap_or_default();
                codec::encode_publish(
                    version,
                    &mut buf,
                    &message.topic,
                    &message.payload,
                    qos,
                    packet_id,
                    dup,
                );
                wr.write_all(&buf).await
            }
            Outgoing::Raw(bytes) => {
                let r = wr.write_all(&bytes).await;
                out.pending.fetch_sub(bytes.len(), Ordering::AcqRel);
                r
            }
            Outgoing::Close(goodbye) => {
                if let Some(bytes) = goodbye {
                    let _ = wr.write_all(&bytes).await;
                }
                break;
            }
        };
        if result.is_err() {
            break;
        }
    }
    let _ = wr.shutdown().await;
    out.writer_done.notify_one();
}

/// Why the session ended, with an optional last packet for the client. `why` is for the log.
struct End {
    why: &'static str,
    /// Last packet for the client: the CONNACK of a refused CONNECT.
    goodbye: Option<Bytes>,
    /// MQTT 5 DISCONNECT reason code. Sent as the last packet to MQTT 5 clients that are connected;
    /// MQTT 3.1.1 has no server DISCONNECT, those connections are just closed.
    reason: Option<u8>,
    /// Let the writer flush the queued messages before closing. False for slow or broken connections.
    flush: bool,
}

impl End {
    fn closed(why: &'static str) -> Self {
        Self {
            why,
            goodbye: None,
            reason: None,
            flush: true,
        }
    }

    /// Like [`End::closed`], with the reason code of the DISCONNECT an MQTT 5 client is told.
    fn closed_with(why: &'static str, reason: u8) -> Self {
        Self {
            reason: Some(reason),
            ..End::closed(why)
        }
    }

    fn abandoned(why: &'static str) -> Self {
        Self {
            why,
            goodbye: None,
            reason: None,
            flush: false,
        }
    }

    /// Refuses the CONNECT with the given CONNACK (already in the format of the client's version), then
    /// closes.
    fn refused(why: &'static str, connack: Bytes) -> Self {
        Self {
            why,
            goodbye: Some(connack),
            reason: None,
            flush: true,
        }
    }
}

/// What the connection knows about itself.
struct State {
    connected: bool,
    /// The session of the client, from the accepted CONNECT on.
    session: Option<Arc<ClientSession>>,
    client_id: String,
    /// Keep alive of the client from its CONNECT (3.1.2.10): the connection is closed after 1.5 times
    /// that interval without any packet; `None` (keep alive 0) disables the check.
    keep_alive: Option<Duration>,
    /// Version of the CONNECT; 3.1.1 until then, so that nothing is parsed as MQTT 5 before the client
    /// asked for it.
    version: ProtocolVersion,
    /// Address of the peer, for the throttling of logins.
    peer_ip: std::net::IpAddr,
}

/// Runs a connection until it ends. `_alive` is dropped at the end so the broker can wait for all sessions.
pub(crate) async fn run_session(
    stream: TcpStream,
    peer: SocketAddr,
    core: Arc<Core>,
    permit: ConnectionPermit,
    shutdown: watch::Receiver<bool>,
    _alive: mpsc::Sender<()>,
) {
    let _ = stream.set_nodelay(true);
    let conn_id = core.next_conn_id();
    let (mut rd, wr) = stream.into_split();
    let (out, rx) = SessionOut::new(
        core.limits.max_pending_send_bytes,
        core.limits.max_consecutive_send_overflows,
    );
    let writer = tokio::spawn(writer_task(wr, rx, Arc::clone(&out)));

    let mut state = State {
        connected: false,
        session: None,
        client_id: String::new(),
        keep_alive: None,
        version: ProtocolVersion::V311,
        peer_ip: super::tracker::peer_key(peer),
    };
    let end = drive(&mut rd, &core, conn_id, &out, shutdown, &mut state).await;

    // First stop deliveries (and destroy the session if it ends with the connection), then say goodbye and
    // close. A connection whose session was taken over leaves the session alone.
    if let Some(session) = &state.session {
        core.detach(session, conn_id);
    }
    info!(conn = conn_id, %peer, client_id = %state.client_id, reason = end.why, "connection closed");
    let mut writer = writer;
    if end.flush {
        let goodbye = end.goodbye.or_else(|| {
            // only a connected MQTT 5 client understands a server DISCONNECT
            (out.version() == Some(ProtocolVersion::V5))
                .then(|| end.reason.map(codec::encode_disconnect))
                .flatten()
        });
        out.close(goodbye);
        if timeout(Duration::from_secs(2), &mut writer).await.is_err() {
            debug!(
                conn = conn_id,
                "writer did not finish in time, connection dropped"
            );
            writer.abort();
        }
    } else {
        writer.abort();
    }
    drop(permit);
}

/// The way a kicked connection ends.
fn kick_end(out: &SessionOut, client_id: &str) -> End {
    match out.kicked() {
        Some(KickReason::TakenOver) => {
            End::closed_with("session taken over", disconnect::SESSION_TAKEN_OVER)
        }
        _ => {
            warn!(client_id = %client_id, "disconnecting slow client");
            End::abandoned("send queue overflow")
        }
    }
}

/// Holds back the next packet while many acknowledgements wait for the store, so a client that pipelines
/// PUBLISH packets cannot make them pile up in memory. The keep alive does not run meanwhile: the client is
/// idle because the broker does not read. Returns `Some` if the connection has to end.
async fn wait_for_acks(
    out: &SessionOut,
    limit: usize,
    client_id: &str,
    shutdown: &mut watch::Receiver<bool>,
) -> Option<End> {
    loop {
        if out.deferred.load(Ordering::Acquire) <= limit {
            return None;
        }
        let woken = out.deferred_done.notified();
        tokio::pin!(woken);
        // registered before the check below, so a decrease in between is not missed
        woken.as_mut().enable();
        if out.deferred.load(Ordering::Acquire) <= limit {
            return None;
        }
        tokio::select! {
            biased;
            _ = shutdown.wait_for(|stop| *stop) => {
                return Some(End::closed_with("server shutting down", disconnect::SERVER_SHUTTING_DOWN));
            }
            _ = out.kick.notified() => return Some(kick_end(out, client_id)),
            _ = out.writer_done.notified() => return Some(End::abandoned("write failed")),
            _ = &mut woken => {}
        }
    }
}

async fn drive(
    rd: &mut tokio::net::tcp::OwnedReadHalf,
    core: &Arc<Core>,
    conn_id: u64,
    out: &Arc<SessionOut>,
    mut shutdown: watch::Receiver<bool>,
    state: &mut State,
) -> End {
    let limits = &core.limits;
    let mut buf = BytesMut::with_capacity(4096);
    let connect_deadline =
        Instant::now() + Duration::from_secs(u64::from(limits.connect_timeout_s));
    let mut deadline = Some(connect_deadline);
    let deferred_limit = 2 * usize::from(limits.receive_maximum) + 16;

    loop {
        loop {
            if out.deferred.load(Ordering::Acquire) > deferred_limit {
                if let Some(end) =
                    wait_for_acks(out, deferred_limit, &state.client_id, &mut shutdown).await
                {
                    return end;
                }
                if state.connected {
                    deadline = state.keep_alive.map(|d| Instant::now() + d);
                }
            }
            match codec::decode(&mut buf, limits.max_packet_size, state.version) {
                Ok(Some(packet)) => {
                    let ended = match packet {
                        Packet::Connect(connect) if !state.connected => {
                            handle_connect(
                                connect,
                                core,
                                conn_id,
                                out,
                                state,
                                &mut shutdown,
                                connect_deadline,
                            )
                            .await
                        }
                        packet => handle_packet(packet, core, conn_id, out, state),
                    };
                    if let Some(end) = ended {
                        return end;
                    }
                    if state.connected {
                        // CONNECT was handled, from now on the keep alive applies
                        deadline = state.keep_alive.map(|d| Instant::now() + d);
                    }
                }
                Ok(None) => break,
                Err(e) => return decode_failure(e, state.connected),
            }
        }
        tokio::select! {
            biased;
            _ = shutdown.wait_for(|stop| *stop) => {
                return End::closed_with("server shutting down", disconnect::SERVER_SHUTTING_DOWN);
            }
            _ = out.kick.notified() => return kick_end(out, &state.client_id),
            _ = out.writer_done.notified() => return End::abandoned("write failed"),
            read = rd.read_buf(&mut buf) => match read {
                Ok(0) => return End::closed("closed by peer"),
                Ok(_) => {
                    if state.connected {
                        deadline = state.keep_alive.map(|d| Instant::now() + d);
                    }
                }
                Err(_) => return End::closed("read error"),
            },
            _ = sleep_deadline(deadline) => {
                return if state.connected {
                    End::closed_with("keep alive timeout", disconnect::KEEP_ALIVE_TIMEOUT)
                } else {
                    End::closed("CONNECT timeout")
                };
            }
        }
    }
}

async fn sleep_deadline(deadline: Option<Instant>) {
    match deadline {
        Some(d) => sleep_until(d).await,
        None => std::future::pending().await,
    }
}

fn decode_failure(e: DecodeError, connected: bool) -> End {
    debug!(error = %e, "closing connection after invalid packet");
    // In 3.1.1 a protocol violation is answered by closing the network connection, in MQTT 5 by a DISCONNECT
    // with a reason code first (added by the caller for connected MQTT 5 clients only).
    // The one exception before the connection is established is a CONNECT with an unsupported protocol level,
    // which gets CONNACK 0x01 ([MQTT-3.1.2-2]; the version of the client is not known, so in the 3.1.1
    // format); a wrong protocol name is just closed.
    if !connected {
        if e == DecodeError::UnacceptableProtocolLevel {
            return End::refused(
                "unacceptable protocol level",
                codec::encode_connack(
                    ProtocolVersion::V311,
                    connack::UNACCEPTABLE_PROTOCOL_VERSION,
                ),
            );
        }
        return End::closed(match e {
            DecodeError::UnsupportedProtocol => "unsupported protocol",
            _ => "invalid first packet",
        });
    }
    match e {
        DecodeError::PacketTooLarge { .. } => {
            End::closed_with("packet too large", disconnect::PACKET_TOO_LARGE)
        }
        DecodeError::Malformed(_) => {
            End::closed_with("malformed packet", disconnect::MALFORMED_PACKET)
        }
        DecodeError::UnsupportedProtocol | DecodeError::UnacceptableProtocolLevel => {
            End::closed_with("unsupported protocol", disconnect::PROTOCOL_ERROR)
        }
        DecodeError::ProtocolError(why) => End::closed_with(why, disconnect::PROTOCOL_ERROR),
    }
}

/// How long before the CONNECT deadline a running login check is given up, to leave time for the CONNACK.
const AUTH_MARGIN: Duration = Duration::from_millis(500);

const QUEUE_FULL: &str = "control packet queue overflow";

/// Generates a client identifier for clients that connect with an empty one.
fn generate_client_id(conn_id: u64) -> String {
    let nanos = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map_or(0, |d| d.as_nanos());
    format!("dt-{nanos:x}-{conn_id}")
}

/// Waits until the store has made everything submitted so far durable. Returns `Some` if the connection has
/// to end first (the broker shuts down, the connection is kicked, or the CONNECT timeout runs out).
async fn wait_for_store(
    store: &Arc<dyn SessionStore>,
    out: &SessionOut,
    client_id: &str,
    shutdown: &mut watch::Receiver<bool>,
    deadline: Instant,
    version: ProtocolVersion,
) -> Option<End> {
    let (tx, rx) = oneshot::channel();
    store.barrier(Box::new(move || {
        let _ = tx.send(());
    }));
    tokio::select! {
        biased;
        _ = shutdown.wait_for(|stop| *stop) => {
            Some(End::closed_with("server shutting down", disconnect::SERVER_SHUTTING_DOWN))
        }
        _ = out.kick.notified() => Some(kick_end(out, client_id)),
        _ = out.writer_done.notified() => Some(End::abandoned("write failed")),
        _ = sleep_until(deadline) => Some(End::closed("CONNECT timeout")),
        // a barrier that is dropped without running means the store cannot make the state durable
        done = rx => done.err().map(|_| {
            End::refused(
                "session store unavailable",
                codec::encode_connack(version, connack::server_unavailable(version)),
            )
        }),
    }
}

/// Handles the CONNECT of a connection that is not connected yet: checks version, client id and credentials,
/// attaches the session, then accepts it. Returns `Some` if the connection has to end.
/// Longest client identifier and user name in bytes; both are stored with the session.
pub(crate) const MAX_IDENTITY_LEN: usize = 1024;

async fn handle_connect(
    connect: Connect,
    core: &Arc<Core>,
    conn_id: u64,
    out: &Arc<SessionOut>,
    state: &mut State,
    shutdown: &mut watch::Receiver<bool>,
    connect_deadline: Instant,
) -> Option<End> {
    let limits = &core.limits;
    let version = connect.protocol;
    if !core.protocol.allows(version) {
        // refused in the format of the version the client speaks: 0x01 (3.1.1) or 0x84 (MQTT 5)
        return Some(End::refused(
            "protocol version not enabled",
            codec::encode_connack(version, connack::unsupported_protocol(version)),
        ));
    }
    let mut assigned = None;
    if connect.client_id.is_empty() {
        if version == ProtocolVersion::V5 {
            // MQTT 5, 3.1.3.2: the server assigns an identifier and tells the client in CONNACK
            state.client_id = generate_client_id(conn_id);
            assigned = Some(state.client_id.clone());
        } else if connect.clean_session {
            // With clean session the broker makes up an id for its own bookkeeping; a 3.1.1
            // client has no way to be told about it.
            state.client_id = generate_client_id(conn_id);
        } else {
            // 3.1.1, 3.1.3.1: an empty client id is only allowed with clean session = 1,
            // otherwise the server answers CONNACK 0x02 (identifier rejected) and closes.
            return Some(End::refused(
                "empty client id without clean session",
                codec::encode_connack(version, connack::identifier_rejected(version)),
            ));
        }
    } else if connect.client_id.len() > MAX_IDENTITY_LEN {
        // the client id is the key of the stored session: unbounded identifiers would bloat the database
        return Some(End::refused(
            "client id too long",
            codec::encode_connack(version, connack::identifier_rejected(version)),
        ));
    } else {
        state.client_id = connect.client_id.clone();
    }
    if connect
        .username
        .as_ref()
        .is_some_and(|name| name.len() > MAX_IDENTITY_LEN)
    {
        return Some(End::refused(
            "user name too long",
            codec::encode_connack(version, connack::bad_credentials(version)),
        ));
    }
    if let Some(authenticator) = &core.authenticator {
        // After the version and client id checks, before anything is accepted. Pipelined packets stay in the
        // read buffer until the verdict is in.
        let (Some(username), Some(password)) = (&connect.username, &connect.password) else {
            warn!(conn = conn_id, client_id = %state.client_id, "CONNECT refused: no user name and password");
            return Some(End::refused(
                "missing credentials",
                codec::encode_connack(version, connack::bad_credentials(version)),
            ));
        };
        let refuse_credentials = |why: &'static str| {
            Some(End::refused(
                why,
                codec::encode_connack(version, connack::bad_credentials(version)),
            ))
        };
        let refuse_unavailable = |why: &'static str| {
            Some(End::refused(
                why,
                codec::encode_connack(version, connack::server_unavailable(version)),
            ))
        };
        // Throttling: too many failures of this user name / address, or too many checks running for the address
        let _pending = match core.throttle.begin(username, state.peer_ip) {
            Ok(guard) => guard,
            Err(Denied::Blocked) => {
                warn!(conn = conn_id, username = ?username, "login refused: too many failed logins");
                return refuse_credentials("too many failed logins");
            }
            Err(Denied::TooManyPending) => {
                warn!(conn = conn_id, ip = %state.peer_ip, "login refused: too many pending checks");
                return refuse_unavailable("too many pending login checks");
            }
        };
        // The check, including the wait for a free slot at the backend, is bounded by the CONNECT deadline
        // minus a margin, so that the client gets a CONNACK before the connection would be closed for timing out.
        let limit = connect_deadline
            .checked_sub(AUTH_MARGIN)
            .unwrap_or(connect_deadline);
        let verdict = tokio::select! {
            biased;
            _ = shutdown.wait_for(|stop| *stop) => {
                return Some(End::closed_with(
                    "server shutting down",
                    disconnect::SERVER_SHUTTING_DOWN,
                ));
            }
            verdict = tokio::time::timeout_at(limit, authenticator.verify(username, password)) => {
                verdict.unwrap_or_else(|_| AuthOutcome::Unavailable("login check timed out".into()))
            }
        };
        // the user name comes from the network: escaped by the debug format
        match verdict {
            AuthOutcome::Accepted => {
                core.throttle.record_success(username);
                info!(conn = conn_id, username = ?username, "login accepted");
            }
            AuthOutcome::Rejected => {
                core.throttle.record_failure(username, state.peer_ip);
                warn!(conn = conn_id, username = ?username, "login rejected");
                return refuse_credentials("bad user name or password");
            }
            AuthOutcome::Unavailable(why) => {
                warn!(conn = conn_id, username = ?username, reason = %why, "login check unavailable");
                return refuse_unavailable("authentication unavailable");
            }
        }
    }
    // Effective Session Expiry Interval: MQTT 5 asks for it, 3.1.1 only distinguishes clean session (ends
    // with the connection) from the rest (as long as the broker allows).
    let expiry_s = match version {
        ProtocolVersion::V311 if connect.clean_session => 0,
        ProtocolVersion::V311 => limits.session_expiry_max_s,
        ProtocolVersion::V5 => connect.session_expiry.min(limits.session_expiry_max_s),
    };
    let announced_expiry =
        (version == ProtocolVersion::V5 && expiry_s != connect.session_expiry).then_some(expiry_s);
    let attached = match core.attach(AttachRequest {
        client_id: &state.client_id,
        username: connect.username.as_deref(),
        clean: connect.clean_session,
        version,
        expiry_s,
        receive_maximum: connect.receive_maximum,
        maximum_packet_size: connect.maximum_packet_size,
        conn_id,
        out,
    }) {
        Ok(attached) => attached,
        Err(AttachError::IdentifierRejected) => {
            warn!(conn = conn_id, client_id = %state.client_id, "CONNECT refused: the client identifier belongs to another user");
            return Some(End::refused(
                "client identifier belongs to another user",
                codec::encode_connack(version, connack::identifier_rejected(version)),
            ));
        }
        Err(AttachError::QuotaExceeded) => {
            warn!(conn = conn_id, client_id = %state.client_id, "CONNECT refused: too many persistent sessions");
            return Some(End::refused(
                "too many persistent sessions",
                codec::encode_connack(version, connack::quota_exceeded(version)),
            ));
        }
    };
    let session = attached.session;
    state.session = Some(Arc::clone(&session));
    info!(
        conn = conn_id,
        session = session.id,
        client_id = %state.client_id,
        keep_alive = connect.keep_alive,
        protocol = ?version,
        session_present = attached.session_present,
        expiry_s,
        "CONNECT"
    );
    if let (true, Some(store)) = (attached.submitted, &core.persist) {
        // the CONNACK promises that the session exists durably
        if let Some(end) = wait_for_store(
            store,
            out,
            &state.client_id,
            shutdown,
            connect_deadline,
            version,
        )
        .await
        {
            return Some(end);
        }
    }
    state.connected = true;
    state.version = version;
    state.keep_alive = (connect.keep_alive > 0)
        .then(|| Duration::from_millis(u64::from(connect.keep_alive) * 1500));
    out.set_version(version);
    let connack = codec::encode_connack_accepted(
        version,
        &ConnackAccepted {
            session_present: attached.session_present,
            max_packet_size: limits.max_packet_size,
            receive_maximum: limits.receive_maximum,
            session_expiry: announced_expiry,
            assigned_client_id: assigned.as_deref(),
        },
    );
    if !out.send_control(connack) {
        return Some(End::abandoned(QUEUE_FULL));
    }
    // the CONNACK is queued before the first PUBLISH can be
    if !session.start(conn_id) {
        return Some(End::closed_with(
            "session taken over",
            disconnect::SESSION_TAKEN_OVER,
        ));
    }
    None
}

/// Sends an acknowledgement through [`SessionOut::send_ordered`]; `Some` if the connection has to end.
fn ack(core: &Core, out: &Arc<SessionOut>, durable: bool, bytes: Bytes) -> Option<End> {
    if out.send_ordered(core.persist.as_ref(), durable, bytes) {
        None
    } else {
        Some(End::abandoned(QUEUE_FULL))
    }
}

/// Returns `Some` if the connection has to end.
fn handle_packet(
    packet: Packet,
    core: &Arc<Core>,
    conn_id: u64,
    out: &Arc<SessionOut>,
    state: &mut State,
) -> Option<End> {
    let limits = &core.limits;
    let Some(session) = state.session.clone().filter(|_| state.connected) else {
        // CONNECT is handled by `handle_connect`; whatever else comes first is a protocol violation.
        return Some(End::closed("first packet is not CONNECT"));
    };
    if out.kicked().is_some() {
        // taken over (or too slow): nothing the client sends may touch the session any more
        return Some(kick_end(out, &state.client_id));
    }
    let version = state.version;
    let v5 = version == ProtocolVersion::V5;
    let taken_over = || End::closed_with("session taken over", disconnect::SESSION_TAKEN_OVER);
    match packet {
        Packet::Connect(_) => Some(End::closed_with(
            "second CONNECT",
            disconnect::PROTOCOL_ERROR,
        )),
        Packet::PingReq => {
            if !out.send_control(codec::encode_pingresp()) {
                return Some(End::abandoned(QUEUE_FULL));
            }
            None
        }
        Packet::Subscribe { packet_id, entries } => {
            let mut reasons = Vec::with_capacity(entries.len());
            let mut durable = false;
            let mut store = core.store.lock().unwrap();
            // checked under the store lock, which is also held while a connection takes the session over or the
            // session is destroyed: an old connection must not change the subscriptions of the new one
            if !session.is_current(conn_id) {
                return Some(taken_over());
            }
            for entry in &entries {
                // MQTT 5 has specific failure codes, 3.1.1 only the generic one
                let failure = |v5_code: u8| if v5 { v5_code } else { suback::FAILURE };
                if v5 && entry.filter.starts_with("$share/") {
                    reasons.push(suback::V5_SHARED_NOT_SUPPORTED);
                    continue;
                }
                if !topic::within_limits(&entry.filter, limits)
                    || !topic::is_valid_filter(&entry.filter)
                {
                    reasons.push(failure(suback::V5_TOPIC_FILTER_INVALID));
                    continue;
                }
                match store.add_subscription(
                    session.id,
                    &entry.filter,
                    limits.max_subscriptions_per_session,
                    entry.qos,
                    entry.no_local,
                ) {
                    AddResult::QuotaExceeded => reasons.push(failure(suback::V5_QUOTA_EXCEEDED)),
                    // the requested QoS is granted: the SUBACK code of a level is the level
                    AddResult::Added | AddResult::Updated => {
                        durable |= session.persist_op(Op::SetSubscription {
                            id: session.id,
                            filter: entry.filter.clone(),
                            qos: entry.qos,
                            no_local: entry.no_local,
                        });
                        reasons.push(entry.qos.bits());
                    }
                }
            }
            drop(store);
            ack(
                core,
                out,
                durable,
                codec::encode_suback(version, packet_id, &reasons),
            )
        }
        Packet::Unsubscribe { packet_id, filters } => {
            let mut reasons = Vec::with_capacity(filters.len());
            let mut durable = false;
            let mut store = core.store.lock().unwrap();
            if !session.is_current(conn_id) {
                return Some(taken_over());
            }
            for filter in &filters {
                reasons.push(if store.remove(session.id, filter) {
                    durable |= session.persist_op(Op::RemoveSubscription {
                        id: session.id,
                        filter: filter.clone(),
                    });
                    unsuback::SUCCESS
                } else {
                    unsuback::NO_SUBSCRIPTION_EXISTED
                });
            }
            drop(store);
            ack(
                core,
                out,
                durable,
                codec::encode_unsuback(version, packet_id, &reasons),
            )
        }
        Packet::Publish(publish) => {
            // the arrival time of the message: stamped before validation and before any queueing
            let received_at_ms = now_unix_ms();
            if !topic::is_valid_topic_name(&publish.topic) {
                return Some(End::closed_with(
                    "invalid topic name",
                    disconnect::TOPIC_NAME_INVALID,
                ));
            }
            let within = topic::within_limits(&publish.topic, limits);
            debug!(conn = conn_id, topic = %publish.topic, payload_bytes = publish.payload.len(), qos = publish.qos.bits(), "PUBLISH");
            let message = |publish: Publish| Message {
                origin: Origin::Client,
                topic: Arc::from(publish.topic),
                payload: publish.payload,
                received_at_ms,
            };
            let qos = publish.qos;
            match (qos, publish.packet_id) {
                (QoS::AtMostOnce, _) => {
                    if !within {
                        debug!(conn = conn_id, "PUBLISH dropped: topic exceeds limits");
                        return None;
                    }
                    core.dispatch(message(publish), Some(session.id), qos);
                    None
                }
                (_, None) => Some(End::closed_with(
                    "QoS above 0 without packet identifier",
                    disconnect::PROTOCOL_ERROR,
                )),
                (QoS::AtLeastOnce, Some(packet_id)) => {
                    if !within {
                        // MQTT 5 can tell the client; 3.1.1 acknowledges the message it will never see
                        debug!(
                            conn = conn_id,
                            "PUBLISH not delivered: topic exceeds limits"
                        );
                        let reason = if v5 { puback::QUOTA_EXCEEDED } else { 0 };
                        return ack(
                            core,
                            out,
                            false,
                            codec::encode_puback(version, packet_id, reason),
                        );
                    }
                    let queued = core.dispatch(message(publish), Some(session.id), qos);
                    ack(
                        core,
                        out,
                        queued,
                        codec::encode_puback(version, packet_id, 0),
                    )
                }
                (QoS::ExactlyOnce, Some(packet_id)) => {
                    // MQTT 5: the client may not have more of these in flight than announced
                    let max_awaiting = v5.then_some(usize::from(limits.receive_maximum));
                    if v5 && !within {
                        // no state: the client may forget the identifier
                        debug!(
                            conn = conn_id,
                            "PUBLISH not delivered: topic exceeds limits"
                        );
                        return ack(
                            core,
                            out,
                            false,
                            codec::encode_pubrec(version, packet_id, puback::QUOTA_EXCEEDED),
                        );
                    }
                    // the identifier is remembered by the check itself, before the dispatch
                    match session.inbound_begin(conn_id, packet_id, max_awaiting) {
                        Inbound::Stale => Some(taken_over()),
                        Inbound::Exceeded => Some(End::closed_with(
                            "receive maximum exceeded",
                            disconnect::RECEIVE_MAXIMUM_EXCEEDED,
                        )),
                        Inbound::Duplicate => ack(
                            core,
                            out,
                            false,
                            codec::encode_pubrec(version, packet_id, 0),
                        ),
                        Inbound::New => {
                            // 3.1.1 acknowledges a message it will never deliver and remembers its id
                            let queued =
                                within && core.dispatch(message(publish), Some(session.id), qos);
                            if !within {
                                debug!(
                                    conn = conn_id,
                                    "PUBLISH not delivered: topic exceeds limits"
                                );
                            }
                            // after the delivery, so a stored identifier implies a stored message
                            let recorded = session.inbound_persist(packet_id);
                            ack(
                                core,
                                out,
                                queued || recorded,
                                codec::encode_pubrec(version, packet_id, 0),
                            )
                        }
                    }
                }
            }
        }
        Packet::PubAck { packet_id, .. } => {
            session.on_puback(conn_id, packet_id);
            None
        }
        Packet::PubRec { packet_id, reason } => match session.on_pubrec(conn_id, packet_id, reason)
        {
            PubrecAction::Release { reason, durable } => ack(
                core,
                out,
                durable,
                codec::encode_pubrel(version, packet_id, reason),
            ),
            PubrecAction::Done => None,
        },
        Packet::PubRel { packet_id, .. } => {
            let (known, durable) = session.inbound_release(conn_id, packet_id);
            let reason = if v5 && !known {
                puback::PACKET_IDENTIFIER_NOT_FOUND
            } else {
                0
            };
            ack(
                core,
                out,
                durable,
                codec::encode_pubcomp(version, packet_id, reason),
            )
        }
        Packet::PubComp { packet_id, .. } => {
            session.on_pubcomp(conn_id, packet_id);
            None
        }
        Packet::Disconnect { session_expiry } => {
            if let Some(requested) = session_expiry.filter(|_| v5) {
                if !session.apply_disconnect_expiry(conn_id, requested, limits.session_expiry_max_s)
                {
                    // MQTT 3.14.2.2.2: a session that ends with its connection cannot get an expiry
                    return Some(End::closed_with(
                        "session expiry raised from 0",
                        disconnect::PROTOCOL_ERROR,
                    ));
                }
            }
            Some(End::closed("client disconnect"))
        }
        Packet::Ignored(_) => None,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn topic(s: &str) -> Message {
        Message {
            origin: Origin::Client,
            topic: Arc::from(s),
            payload: Bytes::new(),
            received_at_ms: 0,
        }
    }

    fn msg(payload: Bytes) -> Message {
        Message {
            payload,
            ..topic("t")
        }
    }

    #[test]
    fn pending_bytes_limit_and_consecutive_overflows() {
        let (out, mut rx) = SessionOut::new(100, 3);
        let big = Bytes::from(vec![0u8; 60]);
        assert_eq!(out.offer_publish(&msg(big.clone())), Offer::Queued); // 61 bytes
        assert_eq!(out.pending(), 61);
        // 61 + 61 > 100: dropped, nothing is added
        assert_eq!(out.offer_publish(&msg(big.clone())), Offer::Dropped);
        assert_eq!(out.offer_publish(&msg(big.clone())), Offer::Dropped);
        assert_eq!(out.pending(), 61);
        // third consecutive overflow kicks
        assert_eq!(
            out.offer_publish(&msg(big.clone())),
            Offer::DroppedAndKicked
        );
        assert!(matches!(rx.try_recv(), Ok(Outgoing::Publish { .. })));
        assert!(rx.try_recv().is_err());
    }

    #[test]
    fn successful_offer_resets_the_overflow_counter() {
        let (out, _rx) = SessionOut::new(100, 3);
        let big = Bytes::from(vec![0u8; 60]);
        let small = Bytes::from(vec![0u8; 5]);
        assert_eq!(out.offer_publish(&msg(big.clone())), Offer::Queued);
        assert_eq!(out.offer_publish(&msg(big.clone())), Offer::Dropped);
        assert_eq!(out.offer_publish(&msg(big.clone())), Offer::Dropped);
        // not consecutive any more
        assert_eq!(out.offer_publish(&msg(small.clone())), Offer::Queued);
        assert_eq!(out.offer_publish(&msg(big.clone())), Offer::Dropped);
        assert_eq!(out.offer_publish(&msg(big.clone())), Offer::Dropped);
    }

    #[test]
    fn closed_queue_does_not_leak_pending_bytes() {
        let (out, rx) = SessionOut::new(100, 3);
        drop(rx);
        assert_eq!(
            out.offer_publish(&msg(Bytes::from_static(b"x"))),
            Offer::Dropped
        );
        assert_eq!(out.pending(), 0);
    }

    #[test]
    fn control_packets_count_against_the_pending_limit() {
        let (out, mut rx) = SessionOut::new(10, 3);
        let ping = codec::encode_pingresp();
        for _ in 0..5 {
            assert!(out.send_control(ping.clone()));
        }
        assert_eq!(out.pending(), 10);
        // the sixth would exceed the limit: refused, nothing queued, session is woken
        assert!(!out.send_control(ping.clone()));
        assert_eq!(out.pending(), 10);
        let mut queued = 0;
        while rx.try_recv().is_ok() {
            queued += 1;
        }
        assert_eq!(queued, 5);
    }

    // --- acknowledgements and the store ---

    use crate::broker::persist::testing::RecordingStore;

    fn raw_packets(rx: &mut mpsc::UnboundedReceiver<Outgoing>) -> Vec<Vec<u8>> {
        let mut v = Vec::new();
        while let Ok(o) = rx.try_recv() {
            match o {
                Outgoing::Raw(b) => v.push(b.to_vec()),
                _ => panic!("unexpected packet"),
            }
        }
        v
    }

    #[test]
    fn acknowledgements_without_durability_are_sent_inline() {
        let (out, mut rx) = SessionOut::new(1000, 3);
        let store = RecordingStore::new();
        let dynstore: Arc<dyn SessionStore> = store.clone();
        // no store at all
        assert!(out.send_ordered(
            None,
            true,
            codec::encode_puback(ProtocolVersion::V311, 1, 0)
        ));
        // a store, but nothing durable involved
        assert!(out.send_ordered(
            Some(&dynstore),
            false,
            codec::encode_pubcomp(ProtocolVersion::V311, 2, 0)
        ));
        assert_eq!(out.deferred(), 0);
        assert_eq!(
            raw_packets(&mut rx),
            vec![vec![0x40, 2, 0, 1], vec![0x70, 2, 0, 2]]
        );
        assert_eq!(store.held(), 0);
    }

    #[test]
    fn durable_acknowledgements_wait_for_the_barrier_and_later_ones_queue_behind_them() {
        let (out, mut rx) = SessionOut::new(1000, 3);
        let store = RecordingStore::new();
        let dynstore: Arc<dyn SessionStore> = store.clone();
        store.hold();
        let v = ProtocolVersion::V311;
        assert!(out.send_ordered(Some(&dynstore), true, codec::encode_puback(v, 1, 0)));
        // does not need durability itself, but must not overtake PUBACK 1 [MQTT-4.6.0-2..4]
        assert!(out.send_ordered(Some(&dynstore), false, codec::encode_pubcomp(v, 2, 0)));
        assert!(out.send_ordered(Some(&dynstore), true, codec::encode_pubrec(v, 3, 0)));
        assert_eq!(out.deferred(), 3);
        assert!(raw_packets(&mut rx).is_empty());
        store.release();
        assert_eq!(out.deferred(), 0);
        assert_eq!(
            raw_packets(&mut rx),
            vec![
                vec![0x40, 2, 0, 1],
                vec![0x70, 2, 0, 2],
                vec![0x50, 2, 0, 3]
            ]
        );
        // with nothing waiting any more the next one is inline again
        assert!(out.send_ordered(Some(&dynstore), false, codec::encode_pubrel(v, 4, 0)));
        assert_eq!(raw_packets(&mut rx), vec![vec![0x62, 2, 0, 4]]);
    }

    /// A store that drops every barrier closure unrun.
    struct DroppingStore;

    impl SessionStore for DroppingStore {
        fn submit(&self, _op: Op) {}
        fn barrier(&self, done: Box<dyn FnOnce() + Send>) {
            drop(done);
        }
    }

    #[tokio::test]
    async fn a_dropped_barrier_refuses_the_connect_instead_of_continuing() {
        let (out, _rx) = SessionOut::new(1000, 3);
        let (_tx, mut shutdown) = watch::channel(false);
        let store: Arc<dyn SessionStore> = Arc::new(DroppingStore);
        let deadline = Instant::now() + Duration::from_secs(5);
        for (version, code) in [(ProtocolVersion::V311, 0x03), (ProtocolVersion::V5, 0x88)] {
            let end = wait_for_store(&store, &out, "c", &mut shutdown, deadline, version)
                .await
                .expect("the CONNECT must fail");
            let connack = end.goodbye.expect("a CONNACK refusal");
            assert_eq!(connack[3], code);
        }
        // a barrier that runs lets the CONNECT go on
        let store: Arc<dyn SessionStore> = RecordingStore::new();
        assert!(wait_for_store(
            &store,
            &out,
            "c",
            &mut shutdown,
            deadline,
            ProtocolVersion::V5
        )
        .await
        .is_none());
    }

    // --- whole connections against a store that holds its barriers ---
    //
    // The tests below run `run_session` over real sockets with the `RecordingStore` in hold mode: a barrier
    // (and with it every acknowledgement that has to wait for durable state) stays pending until the test
    // releases it, which makes the interleavings of the store and the connection deterministic.

    use crate::auth::ThrottleConfig;
    use crate::broker::limits::BrokerLimits;
    use crate::broker::protocol::ProtocolMode;
    use tokio::net::TcpListener;

    const SHORT: Duration = Duration::from_millis(400);
    const LONG: Duration = Duration::from_secs(5);

    struct Server {
        core: Arc<Core>,
        store: Arc<RecordingStore>,
        addr: SocketAddr,
        stop: watch::Sender<bool>,
    }

    async fn server(limits: BrokerLimits) -> Server {
        let store = RecordingStore::new();
        let core = Core::with_authenticator(
            limits,
            ProtocolMode::Both,
            None,
            ThrottleConfig::default(),
            Some(store.clone()),
            Vec::new(),
        );
        let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
        let addr = listener.local_addr().unwrap();
        let (stop, stop_rx) = watch::channel(false);
        let (alive, _unused) = mpsc::channel(1);
        let accept_core = Arc::clone(&core);
        tokio::spawn(async move {
            while let Ok((stream, peer)) = listener.accept().await {
                let permit = accept_core
                    .tracker
                    .acquire_permit(crate::broker::tracker::peer_key(peer))
                    .expect("a connection permit");
                tokio::spawn(run_session(
                    stream,
                    peer,
                    Arc::clone(&accept_core),
                    permit,
                    stop_rx.clone(),
                    alive.clone(),
                ));
            }
        });
        Server {
            core,
            store,
            addr,
            stop,
        }
    }

    #[derive(Debug, PartialEq, Eq)]
    enum Got {
        Packet(u8, Vec<u8>),
        Closed,
        Silent,
    }

    /// A minimal raw MQTT 3.1.1 client.
    struct Peer {
        stream: TcpStream,
        buf: Vec<u8>,
    }

    fn string(s: &str) -> Vec<u8> {
        let mut out = (s.len() as u16).to_be_bytes().to_vec();
        out.extend_from_slice(s.as_bytes());
        out
    }

    fn packet(first: u8, body: &[u8]) -> Vec<u8> {
        assert!(body.len() < 128, "the helpers only build short packets");
        let mut out = vec![first, body.len() as u8];
        out.extend_from_slice(body);
        out
    }

    fn connect_packet(client_id: &str, clean: bool) -> Vec<u8> {
        let mut body = string("MQTT");
        body.extend([4, if clean { 0b10 } else { 0 }, 0, 30]);
        body.extend(string(client_id));
        packet(0x10, &body)
    }

    fn subscribe_packet(packet_id: u16, filters: &[(&str, u8)]) -> Vec<u8> {
        let mut body = packet_id.to_be_bytes().to_vec();
        for (filter, qos) in filters {
            body.extend(string(filter));
            body.push(*qos);
        }
        packet(0x82, &body)
    }

    fn unsubscribe_packet(packet_id: u16, filters: &[&str]) -> Vec<u8> {
        let mut body = packet_id.to_be_bytes().to_vec();
        for filter in filters {
            body.extend(string(filter));
        }
        packet(0xA2, &body)
    }

    fn publish_packet(topic: &str, payload: &[u8], qos: u8, packet_id: u16) -> Vec<u8> {
        let mut body = string(topic);
        body.extend(packet_id.to_be_bytes());
        body.extend_from_slice(payload);
        packet(0x30 | qos << 1, &body)
    }

    fn ack_packet(first: u8, packet_id: u16) -> Vec<u8> {
        packet(first, &packet_id.to_be_bytes())
    }

    /// QoS and packet identifier of a PUBLISH packet (first byte and body).
    fn publish_qos_and_id(first: u8, body: &[u8]) -> (u8, u16) {
        assert_eq!(first & 0xF0, 0x30, "expected a PUBLISH, got {first:#x}");
        let qos = (first >> 1) & 3;
        let topic_len = usize::from(u16::from_be_bytes([body[0], body[1]]));
        let at = 2 + topic_len;
        (qos, u16::from_be_bytes([body[at], body[at + 1]]))
    }

    impl Peer {
        async fn open(server: &Server) -> Peer {
            Peer {
                stream: TcpStream::connect(server.addr).await.unwrap(),
                buf: Vec::new(),
            }
        }

        /// Opens a connection and sends the CONNECT; the CONNACK is not read.
        async fn start(server: &Server, client_id: &str, clean: bool) -> Peer {
            let mut peer = Peer::open(server).await;
            peer.send(&connect_packet(client_id, clean)).await;
            peer
        }

        /// A connection whose CONNACK has arrived (the store must not hold barriers); `present` is its flag.
        async fn connected(server: &Server, client_id: &str, clean: bool) -> (Peer, bool) {
            let mut peer = Peer::start(server, client_id, clean).await;
            let Got::Packet(0x20, body) = peer.next(LONG).await else {
                panic!("expected a CONNACK");
            };
            assert_eq!(body[1], 0, "CONNACK accepted");
            (peer, body[0] == 1)
        }

        async fn send(&mut self, bytes: &[u8]) {
            self.stream.write_all(bytes).await.unwrap();
        }

        /// The next packet; `Silent` after `within`, `Closed` if the broker ended the connection.
        async fn next(&mut self, within: Duration) -> Got {
            let deadline = Instant::now() + within;
            loop {
                if self.buf.len() >= 2 {
                    // the remaining length of the packets of these tests is below 128: one byte
                    let end = 2 + usize::from(self.buf[1]);
                    if self.buf.len() >= end {
                        let body = self.buf[2..end].to_vec();
                        let first = self.buf[0];
                        self.buf.drain(..end);
                        return Got::Packet(first, body);
                    }
                }
                match tokio::time::timeout_at(deadline, self.stream.read_buf(&mut self.buf)).await {
                    Err(_) => return Got::Silent,
                    Ok(Ok(0)) | Ok(Err(_)) => return Got::Closed,
                    Ok(Ok(_)) => {}
                }
            }
        }
    }

    /// Polls until `cond` holds.
    async fn eventually(what: &str, mut cond: impl FnMut() -> bool) {
        for _ in 0..250 {
            if cond() {
                return;
            }
            tokio::time::sleep(Duration::from_millis(20)).await;
        }
        panic!("timed out waiting for: {what}");
    }

    #[tokio::test]
    async fn connack_suback_and_unsuback_wait_for_the_barrier_and_acknowledgements_keep_their_order(
    ) {
        let srv = server(BrokerLimits::default()).await;
        let store = &srv.store;

        // CONNACK: the session is stored first
        store.hold();
        let mut c = Peer::start(&srv, "c", false).await;
        assert_eq!(
            c.next(SHORT).await,
            Got::Silent,
            "no CONNACK before the store"
        );
        assert_eq!(store.held(), 1);
        store.release();
        assert_eq!(c.next(LONG).await, Got::Packet(0x20, vec![0, 0]));

        // SUBACK
        store.hold();
        c.send(&subscribe_packet(1, &[("t", 2), ("u", 0)])).await;
        assert_eq!(
            c.next(SHORT).await,
            Got::Silent,
            "no SUBACK before the store"
        );
        assert_eq!(store.held(), 1);
        store.release();
        assert_eq!(c.next(LONG).await, Got::Packet(0x90, vec![0, 1, 2, 0]));

        // UNSUBACK of a stored subscription waits; one for a filter that was never there needs no
        // durability and nothing waits before it, so it leaves at once, even though the store holds
        store.hold();
        c.send(&unsubscribe_packet(2, &["u"])).await;
        assert_eq!(
            c.next(SHORT).await,
            Got::Silent,
            "no UNSUBACK before the store"
        );
        assert_eq!(store.held(), 1);
        store.release();
        assert_eq!(c.next(LONG).await, Got::Packet(0xB0, vec![0, 2]));
        store.hold();
        c.send(&unsubscribe_packet(3, &["never"])).await;
        assert_eq!(c.next(LONG).await, Got::Packet(0xB0, vec![0, 3]));
        assert_eq!(store.held(), 0);

        // The client subscribed to "t" itself, so what it publishes is queued in its own durable session.
        // Its acknowledgements wait, and one that needs no durability (PUBCOMP of an unknown id) must not
        // overtake them [MQTT-4.6.0-2..4].
        c.send(&publish_packet("t", b"a", 1, 1)).await; // PUBACK 1 waits
        let Got::Packet(first, body) = c.next(LONG).await else {
            panic!("the message for the own subscription");
        };
        let (qos, id1) = publish_qos_and_id(first, &body);
        assert_eq!(qos, 1);
        c.send(&ack_packet(0x62, 9)).await; // PUBREL 9 (unknown): PUBCOMP needs nothing, but waits behind
        c.send(&publish_packet("t", b"b", 2, 2)).await; // PUBREC 2 waits
        let Got::Packet(first, body) = c.next(LONG).await else {
            panic!("the QoS 2 message for the own subscription");
        };
        let (qos, id2) = publish_qos_and_id(first, &body);
        assert_eq!(qos, 2);
        c.send(&ack_packet(0x50, id2)).await; // PUBREC of the client: the PUBREL waits
        c.send(&ack_packet(0x62, 2)).await; // PUBREL 2: PUBCOMP waits
        c.send(&ack_packet(0x40, id1)).await; // PUBACK of the client: nothing to answer
        assert_eq!(
            c.next(SHORT).await,
            Got::Silent,
            "all five acknowledgements are held"
        );
        assert_eq!(store.held(), 5);
        store.release();
        let expected = [
            Got::Packet(0x40, vec![0, 1]),
            Got::Packet(0x70, vec![0, 9]),
            Got::Packet(0x50, vec![0, 2]),
            Got::Packet(0x62, id2.to_be_bytes().to_vec()),
            Got::Packet(0x70, vec![0, 2]),
        ];
        for want in expected {
            assert_eq!(c.next(LONG).await, want);
        }
        // nothing waits any more: the next acknowledgement is inline again, whatever the store does
        store.hold();
        c.send(&ack_packet(0x62, 10)).await;
        assert_eq!(c.next(LONG).await, Got::Packet(0x70, vec![0, 10]));
        assert_eq!(store.held(), 0);
        store.release();
    }

    /// Sends 18 acknowledgements that wait for the store (the limit for Receive Maximum 1 is 18), a PING
    /// that is still answered, and one more PUBLISH. Returns the publisher and the subscriber that makes the
    /// acknowledgements durable (a persistent session subscribed to "t"); the PING behind the 19th
    /// PUBLISH is not read yet.
    async fn backed_up_publisher(srv: &Server, client_id: &str, clean: bool) -> (Peer, Peer) {
        let (mut sleeper, _) = Peer::connected(srv, "sleeper", false).await;
        sleeper.send(&subscribe_packet(1, &[("t", 1)])).await;
        assert_eq!(sleeper.next(LONG).await, Got::Packet(0x90, vec![0, 1, 1]));
        let (mut publisher, _) = Peer::connected(srv, client_id, clean).await;
        srv.store.hold();
        let mut bytes = Vec::new();
        for id in 1..=18u16 {
            bytes.extend(publish_packet("t", b"x", 1, id));
        }
        bytes.extend([0xC0, 0]);
        publisher.send(&bytes).await;
        // 18 held acknowledgements are not more than the limit: the PING is read and answered
        assert_eq!(publisher.next(LONG).await, Got::Packet(0xD0, vec![]));
        assert_eq!(srv.store.held(), 18);
        // the 19th is processed (18 is not above the limit) and the one after it waits
        let mut bytes = publish_packet("t", b"x", 1, 19);
        bytes.extend([0xC0, 0]);
        publisher.send(&bytes).await;
        assert_eq!(
            publisher.next(SHORT).await,
            Got::Silent,
            "the reader stopped"
        );
        assert_eq!(srv.store.held(), 19);
        (publisher, sleeper)
    }

    fn one_at_a_time() -> BrokerLimits {
        BrokerLimits {
            receive_maximum: 1,
            ..BrokerLimits::default()
        }
    }

    #[tokio::test]
    async fn more_held_acknowledgements_than_the_limit_stop_the_reader_until_the_store_catches_up()
    {
        let srv = server(one_at_a_time()).await;
        let (mut publisher, _sleeper) = backed_up_publisher(&srv, "pub", true).await;
        srv.store.release();
        for id in 1..=19u16 {
            assert_eq!(
                publisher.next(LONG).await,
                Got::Packet(0x40, vec![0, id as u8])
            );
        }
        // now the PING behind them is read
        assert_eq!(publisher.next(LONG).await, Got::Packet(0xD0, vec![]));
        assert_eq!(srv.store.held(), 0);
    }

    #[tokio::test]
    async fn a_shutdown_ends_a_connection_that_waits_for_the_store() {
        let srv = server(one_at_a_time()).await;
        let (mut publisher, _sleeper) = backed_up_publisher(&srv, "pub", true).await;
        srv.stop.send(true).unwrap();
        // 3.1.1 has no server DISCONNECT: the connection is closed, with the store still holding
        assert_eq!(publisher.next(LONG).await, Got::Closed);
        assert_eq!(srv.store.held(), 19, "the store was never released");
        eventually("all connections to end", || srv.core.tracker.total() == 0).await;
    }

    #[tokio::test]
    async fn a_take_over_ends_a_connection_that_waits_for_the_store() {
        let srv = server(one_at_a_time()).await;
        let (mut old, _sleeper) = backed_up_publisher(&srv, "pub", false).await;
        // the same client identifier connects: its CONNACK waits for the store too
        let mut new = Peer::start(&srv, "pub", false).await;
        assert_eq!(
            old.next(LONG).await,
            Got::Closed,
            "the old connection ends while it waits"
        );
        assert_eq!(new.next(SHORT).await, Got::Silent);
        srv.store.release();
        assert_eq!(new.next(LONG).await, Got::Packet(0x20, vec![1, 0]));
        assert_eq!(srv.core.clients.lock().unwrap().len(), 2, "pub and sleeper");
    }

    #[tokio::test]
    async fn a_take_over_that_arrives_while_the_first_connect_waits_for_the_store() {
        let srv = server(BrokerLimits::default()).await;
        srv.store.hold();
        let mut first = Peer::start(&srv, "same", false).await;
        eventually("the first CONNECT to wait for its barrier", || {
            srv.store.held() == 1
        })
        .await;
        let mut second = Peer::start(&srv, "same", false).await;
        // the first connection was attached already, so the second one takes the session over
        assert_eq!(
            first.next(LONG).await,
            Got::Closed,
            "closed without a CONNACK"
        );
        assert_eq!(
            second.next(SHORT).await,
            Got::Silent,
            "its CONNACK waits for the store"
        );
        // the end of the first connection did not destroy what the second one owns
        assert_eq!(srv.core.clients.lock().unwrap().len(), 1);
        srv.store.release();
        assert_eq!(
            second.next(LONG).await,
            Got::Packet(0x20, vec![1, 0]),
            "the session of the first connection is resumed"
        );
        second.send(&subscribe_packet(1, &[("t", 1)])).await;
        assert_eq!(second.next(LONG).await, Got::Packet(0x90, vec![0, 1, 1]));
        assert_eq!(srv.core.store.lock().unwrap().subscription_count(), 1);
        assert_eq!(srv.core.clients.lock().unwrap().len(), 1);
    }
}
