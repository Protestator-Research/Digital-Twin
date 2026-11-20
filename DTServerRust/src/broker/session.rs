//! One task per client connection plus a writer task. Port of `Session`.

use std::net::SocketAddr;
use std::sync::atomic::{AtomicU8, AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::Duration;

use bytes::{Bytes, BytesMut};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::tcp::OwnedWriteHalf;
use tokio::net::TcpStream;
use tokio::sync::{mpsc, watch, Notify};
use tokio::time::{sleep_until, timeout, Instant};
use tracing::{debug, info, warn};

use super::codec::{self, connack, disconnect, suback, unsuback, Connect, DecodeError, Packet};
use super::core::Core;
use super::protocol::ProtocolVersion;
use super::subscriptions::{AddResult, SessionId};
use super::topic;
use super::tracker::ConnectionPermit;
use crate::auth::{AuthOutcome, Denied};
use crate::hub::{now_unix_ms, Message, Origin};

pub(crate) enum Outgoing {
    Publish(Message),
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

/// Sending side of a session: the queue towards the writer task with byte accounting.
pub(crate) struct SessionOut {
    tx: mpsc::UnboundedSender<Outgoing>,
    pending: AtomicUsize,
    overflows: AtomicUsize,
    max_pending: usize,
    max_overflows: usize,
    kick: Notify,
    writer_done: Notify,
    /// Protocol version of the connection, set when the CONNECT is accepted (0 = not yet).
    version: AtomicU8,
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
            writer_done: Notify::new(),
            version: AtomicU8::new(0),
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
                self.kick.notify_one();
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

    /// Queues a control packet. The bytes count against the pending limit, so the memory of a client that
    /// does not read is bounded. Returns false (and wakes the session to end it) if the limit is exceeded.
    fn send_control(&self, bytes: Bytes) -> bool {
        let size = bytes.len();
        let before = self.pending.fetch_add(size, Ordering::AcqRel);
        if before + size > self.max_pending || self.tx.send(Outgoing::Raw(bytes)).is_err() {
            self.pending.fetch_sub(size, Ordering::AcqRel);
            self.kick.notify_one();
            return false;
        }
        true
    }

    fn close(&self, goodbye: Option<Bytes>) {
        let _ = self.tx.send(Outgoing::Close(goodbye));
    }

    #[cfg(test)]
    pub(crate) fn pending(&self) -> usize {
        self.pending.load(Ordering::Acquire)
    }
}

fn message_size(message: &Message) -> usize {
    message.topic.len() + message.payload.len()
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
                // deliveries only start after a subscription, so the CONNECT has set the version
                let version = out.version().unwrap_or_default();
                codec::encode_publish(version, &mut buf, &message.topic, &message.payload);
                let r = wr.write_all(&buf).await;
                out.pending
                    .fetch_sub(message_size(&message), Ordering::AcqRel);
                r
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

/// What the session knows about its connection.
struct State {
    connected: bool,
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

/// Runs a session until it ends. `_alive` is dropped at the end so the broker can wait for all sessions.
pub(crate) async fn run_session(
    stream: TcpStream,
    peer: SocketAddr,
    core: Arc<Core>,
    permit: ConnectionPermit,
    shutdown: watch::Receiver<bool>,
    _alive: mpsc::Sender<()>,
) {
    let _ = stream.set_nodelay(true);
    let id = core.next_session_id();
    let (mut rd, wr) = stream.into_split();
    let (out, rx) = SessionOut::new(
        core.limits.max_pending_send_bytes,
        core.limits.max_consecutive_send_overflows,
    );
    let writer = tokio::spawn(writer_task(wr, rx, Arc::clone(&out)));
    core.sessions.lock().unwrap().insert(id, Arc::clone(&out));

    let mut state = State {
        connected: false,
        client_id: String::new(),
        keep_alive: None,
        version: ProtocolVersion::V311,
        peer_ip: super::tracker::peer_key(peer),
    };
    let end = drive(&mut rd, &core, id, &out, shutdown, &mut state).await;

    // First stop deliveries, then say goodbye and close.
    core.remove_session(id);
    info!(session = id, %peer, client_id = %state.client_id, reason = end.why, "connection closed");
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
                session = id,
                "writer did not finish in time, connection dropped"
            );
            writer.abort();
        }
    } else {
        writer.abort();
    }
    drop(permit);
}

async fn drive(
    rd: &mut tokio::net::tcp::OwnedReadHalf,
    core: &Arc<Core>,
    id: SessionId,
    out: &Arc<SessionOut>,
    mut shutdown: watch::Receiver<bool>,
    state: &mut State,
) -> End {
    let limits = &core.limits;
    let mut buf = BytesMut::with_capacity(4096);
    let connect_deadline =
        Instant::now() + Duration::from_secs(u64::from(limits.connect_timeout_s));
    let mut deadline = Some(connect_deadline);

    loop {
        loop {
            match codec::decode(&mut buf, limits.max_packet_size, state.version) {
                Ok(Some(packet)) => {
                    let ended = match packet {
                        Packet::Connect(connect) if !state.connected => {
                            handle_connect(
                                connect,
                                core,
                                id,
                                out,
                                state,
                                &mut shutdown,
                                connect_deadline,
                            )
                            .await
                        }
                        packet => handle_packet(packet, core, id, out, state),
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
            _ = out.kick.notified() => {
                warn!(session = id, client_id = %state.client_id, "disconnecting slow client");
                return End::abandoned("send queue overflow");
            }
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
        DecodeError::QosNotSupported => End::closed_with(
            "QoS above 0 is not supported",
            disconnect::QOS_NOT_SUPPORTED,
        ),
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
fn generate_client_id(session: SessionId) -> String {
    let nanos = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map_or(0, |d| d.as_nanos());
    format!("dt-{nanos:x}-{session}")
}

/// Handles the CONNECT of a session that is not connected yet: checks version, client id and credentials,
/// then accepts it. Returns `Some` if the session has to end.
async fn handle_connect(
    connect: Connect,
    core: &Arc<Core>,
    id: SessionId,
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
            state.client_id = generate_client_id(id);
            assigned = Some(state.client_id.clone());
        } else if connect.clean_session {
            // With clean session the broker makes up an id for its own bookkeeping; a 3.1.1
            // client has no way to be told about it.
            state.client_id = generate_client_id(id);
        } else {
            // 3.1.1, 3.1.3.1: an empty client id is only allowed with clean session = 1,
            // otherwise the server answers CONNACK 0x02 (identifier rejected) and closes.
            return Some(End::refused(
                "empty client id without clean session",
                codec::encode_connack(version, connack::identifier_rejected(version)),
            ));
        }
    } else {
        state.client_id = connect.client_id;
    }
    if let Some(authenticator) = &core.authenticator {
        // After the version and client id checks, before anything is accepted. Pipelined packets stay in the
        // read buffer until the verdict is in.
        let (Some(username), Some(password)) = (&connect.username, &connect.password) else {
            warn!(session = id, client_id = %state.client_id, "CONNECT refused: no user name and password");
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
                warn!(session = id, username = ?username, "login refused: too many failed logins");
                return refuse_credentials("too many failed logins");
            }
            Err(Denied::TooManyPending) => {
                warn!(session = id, ip = %state.peer_ip, "login refused: too many pending checks");
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
                info!(session = id, username = ?username, "login accepted");
            }
            AuthOutcome::Rejected => {
                core.throttle.record_failure(username, state.peer_ip);
                warn!(session = id, username = ?username, "login rejected");
                return refuse_credentials("bad user name or password");
            }
            AuthOutcome::Unavailable(why) => {
                warn!(session = id, username = ?username, reason = %why, "login check unavailable");
                return refuse_unavailable("authentication unavailable");
            }
        }
    }
    info!(
        session = id,
        client_id = %state.client_id,
        keep_alive = connect.keep_alive,
        protocol = ?version,
        "CONNECT"
    );
    state.connected = true;
    state.version = version;
    state.keep_alive = (connect.keep_alive > 0)
        .then(|| Duration::from_millis(u64::from(connect.keep_alive) * 1500));
    out.set_version(version);
    let connack =
        codec::encode_connack_accepted(version, limits.max_packet_size, assigned.as_deref());
    if !out.send_control(connack) {
        return Some(End::abandoned(QUEUE_FULL));
    }
    None
}

/// Returns `Some` if the session has to end.
fn handle_packet(
    packet: Packet,
    core: &Arc<Core>,
    id: SessionId,
    out: &Arc<SessionOut>,
    state: &mut State,
) -> Option<End> {
    let limits = &core.limits;
    if !state.connected {
        // CONNECT is handled by `handle_connect`; whatever else comes first is a protocol violation.
        return Some(End::closed("first packet is not CONNECT"));
    }
    let version = state.version;
    let v5 = version == ProtocolVersion::V5;
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
            let mut store = core.store.lock().unwrap();
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
                match store.add_with_options(
                    id,
                    &entry.filter,
                    limits.max_subscriptions_per_session,
                    entry.no_local,
                ) {
                    AddResult::QuotaExceeded => reasons.push(failure(suback::V5_QUOTA_EXCEEDED)),
                    // QoS 0 is the only level the broker grants, whatever the client asked for
                    AddResult::Added | AddResult::Updated => reasons.push(suback::GRANTED_QOS_0),
                }
            }
            drop(store);
            if !out.send_control(codec::encode_suback(version, packet_id, &reasons)) {
                return Some(End::abandoned(QUEUE_FULL));
            }
            None
        }
        Packet::Unsubscribe { packet_id, filters } => {
            let mut reasons = Vec::with_capacity(filters.len());
            let mut store = core.store.lock().unwrap();
            for filter in &filters {
                reasons.push(if store.remove(id, filter) {
                    unsuback::SUCCESS
                } else {
                    unsuback::NO_SUBSCRIPTION_EXISTED
                });
            }
            drop(store);
            if !out.send_control(codec::encode_unsuback(version, packet_id, &reasons)) {
                return Some(End::abandoned(QUEUE_FULL));
            }
            None
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
            if !topic::within_limits(&publish.topic, limits) {
                debug!(session = id, "PUBLISH dropped: topic exceeds limits");
                return None;
            }
            debug!(session = id, topic = %publish.topic, payload_bytes = publish.payload.len(), "PUBLISH");
            core.dispatch(
                Message {
                    origin: Origin::Client,
                    topic: Arc::from(publish.topic),
                    payload: publish.payload,
                    received_at_ms,
                },
                Some(id),
            );
            None
        }
        Packet::Disconnect => Some(End::closed("client disconnect")),
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
}
