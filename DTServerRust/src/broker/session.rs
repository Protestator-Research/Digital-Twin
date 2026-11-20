//! One task per client connection plus a writer task. Port of `Session`.

use std::net::SocketAddr;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::Duration;

use bytes::{Bytes, BytesMut};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::tcp::OwnedWriteHalf;
use tokio::net::TcpStream;
use tokio::sync::{mpsc, watch, Notify};
use tokio::time::{sleep_until, timeout, Instant};
use tracing::{debug, info, warn};

use super::codec::{self, connack, suback, DecodeError, Packet};
use super::core::Core;
use super::subscriptions::{AddResult, SessionId};
use super::topic;
use super::tracker::ConnectionPermit;
use crate::hub::{Message, Origin};

pub(crate) enum Outgoing {
    Publish(Message),
    /// A control packet (CONNACK, SUBACK, PINGRESP, ...). Counted like publishes in the pending bytes.
    Raw(Bytes),
    /// Writes the optional final packet (only a CONNACK refusing the connection, MQTT 3.1.1 has no
    /// server DISCONNECT), then closes the connection.
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
        });
        (out, rx)
    }

    /// Queues a QoS 0 PUBLISH unless the pending bytes (topic + payload) would exceed the limit.
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
                codec::encode_publish(&mut buf, &message.topic, &message.payload);
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

/// Why the session ended, with an optional last packet for the client. The connection is always just closed
/// (MQTT 3.1.1 has no server DISCONNECT); `why` is only for the log.
struct End {
    why: &'static str,
    goodbye: Option<Bytes>,
    /// Let the writer flush the queued messages before closing. False for slow or broken connections.
    flush: bool,
}

impl End {
    fn closed(why: &'static str) -> Self {
        Self {
            why,
            goodbye: None,
            flush: true,
        }
    }

    fn abandoned(why: &'static str) -> Self {
        Self {
            why,
            goodbye: None,
            flush: false,
        }
    }

    /// Refuses the CONNECT with a CONNACK carrying `return_code`, then closes.
    fn refused(why: &'static str, return_code: u8) -> Self {
        Self {
            why,
            goodbye: Some(codec::encode_connack(return_code)),
            flush: true,
        }
    }
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

    let mut client_id = String::new();
    let end = drive(&mut rd, &core, id, &out, shutdown, &mut client_id).await;

    // First stop deliveries, then say goodbye and close.
    core.remove_session(id);
    info!(session = id, %peer, client_id = %client_id, reason = end.why, "connection closed");
    let mut writer = writer;
    if end.flush {
        out.close(end.goodbye);
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
    client_id: &mut String,
) -> End {
    let limits = &core.limits;
    let mut buf = BytesMut::with_capacity(4096);
    let mut connected = false;
    let connect_deadline =
        Instant::now() + Duration::from_secs(u64::from(limits.connect_timeout_s));
    // Keep alive of the client from its CONNECT (3.1.1, 3.1.2.10): the connection is closed after 1.5 times
    // that interval without any packet; 0 disables the check.
    let mut keep_alive: Option<Duration> = None;
    let mut deadline = Some(connect_deadline);

    loop {
        loop {
            match codec::decode(&mut buf, limits.max_packet_size) {
                Ok(Some(packet)) => {
                    if let Some(end) = handle_packet(
                        packet,
                        core,
                        id,
                        out,
                        &mut connected,
                        client_id,
                        &mut keep_alive,
                    ) {
                        return end;
                    }
                    if connected {
                        // CONNECT was handled, from now on the keep alive applies
                        deadline = keep_alive.map(|d| Instant::now() + d);
                    }
                }
                Ok(None) => break,
                Err(e) => return decode_failure(e, connected),
            }
        }
        tokio::select! {
            biased;
            _ = shutdown.wait_for(|stop| *stop) => {
                return End::closed("server shutting down");
            }
            _ = out.kick.notified() => {
                warn!(session = id, client_id = %client_id, "disconnecting slow client");
                return End::abandoned("send queue overflow");
            }
            _ = out.writer_done.notified() => return End::abandoned("write failed"),
            read = rd.read_buf(&mut buf) => match read {
                Ok(0) => return End::closed("closed by peer"),
                Ok(_) => {
                    if connected {
                        deadline = keep_alive.map(|d| Instant::now() + d);
                    }
                }
                Err(_) => return End::closed("read error"),
            },
            _ = sleep_deadline(deadline) => {
                return if connected {
                    End::closed("keep alive timeout")
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
    // Nothing is sent: in 3.1.1 a protocol violation is answered by closing the network connection.
    // The one exception is a CONNECT with an unsupported protocol level, which gets CONNACK 0x01
    // ([MQTT-3.1.2-2]); a wrong protocol name is just closed.
    if !connected {
        if e == DecodeError::UnacceptableProtocolLevel {
            return End::refused(
                "unacceptable protocol level",
                connack::UNACCEPTABLE_PROTOCOL_VERSION,
            );
        }
        return End::closed(match e {
            DecodeError::UnsupportedProtocol => "unsupported protocol",
            _ => "invalid first packet",
        });
    }
    End::closed(match e {
        DecodeError::PacketTooLarge { .. } => "packet too large",
        DecodeError::Malformed(_) => "malformed packet",
        DecodeError::QosNotSupported => "QoS above 0 is not supported",
        DecodeError::UnsupportedProtocol | DecodeError::UnacceptableProtocolLevel => {
            "unsupported protocol"
        }
    })
}

const QUEUE_FULL: &str = "control packet queue overflow";

/// Generates a client identifier for clients that connect with an empty one.
fn generate_client_id(session: SessionId) -> String {
    let nanos = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map_or(0, |d| d.as_nanos());
    format!("dt-{nanos:x}-{session}")
}

/// Returns `Some` if the session has to end.
fn handle_packet(
    packet: Packet,
    core: &Arc<Core>,
    id: SessionId,
    out: &Arc<SessionOut>,
    connected: &mut bool,
    client_id: &mut String,
    keep_alive: &mut Option<Duration>,
) -> Option<End> {
    let limits = &core.limits;
    if !*connected {
        return match packet {
            Packet::Connect(connect) => {
                if connect.client_id.is_empty() {
                    // 3.1.1, 3.1.3.1: an empty client id is only allowed with clean session = 1, otherwise
                    // the server answers CONNACK 0x02 (identifier rejected) and closes. With clean session
                    // the broker makes up an id for its own bookkeeping; the client is not told about it.
                    if !connect.clean_session {
                        return Some(End::refused(
                            "empty client id without clean session",
                            connack::IDENTIFIER_REJECTED,
                        ));
                    }
                    *client_id = generate_client_id(id);
                } else {
                    *client_id = connect.client_id;
                }
                info!(session = id, client_id = %client_id, keep_alive = connect.keep_alive, "CONNECT");
                *connected = true;
                *keep_alive = (connect.keep_alive > 0)
                    .then(|| Duration::from_millis(u64::from(connect.keep_alive) * 1500));
                if !out.send_control(codec::encode_connack(connack::ACCEPTED)) {
                    return Some(End::abandoned(QUEUE_FULL));
                }
                None
            }
            // The first packet has to be CONNECT.
            _ => Some(End::closed("first packet is not CONNECT")),
        };
    }
    match packet {
        Packet::Connect(_) => Some(End::closed("second CONNECT")),
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
                if !topic::within_limits(&entry.filter, limits)
                    || !topic::is_valid_filter(&entry.filter)
                {
                    reasons.push(suback::FAILURE);
                    continue;
                }
                match store.add(id, &entry.filter, limits.max_subscriptions_per_session) {
                    AddResult::QuotaExceeded => reasons.push(suback::FAILURE),
                    // QoS 0 is the only level the broker grants, whatever the client asked for
                    AddResult::Added | AddResult::Updated => reasons.push(suback::GRANTED_QOS_0),
                }
            }
            drop(store);
            if !out.send_control(codec::encode_suback(packet_id, &reasons)) {
                return Some(End::abandoned(QUEUE_FULL));
            }
            None
        }
        Packet::Unsubscribe { packet_id, filters } => {
            let mut store = core.store.lock().unwrap();
            for filter in &filters {
                store.remove(id, filter);
            }
            drop(store);
            if !out.send_control(codec::encode_unsuback(packet_id)) {
                return Some(End::abandoned(QUEUE_FULL));
            }
            None
        }
        Packet::Publish(publish) => {
            if !topic::is_valid_topic_name(&publish.topic) {
                return Some(End::closed("invalid topic name"));
            }
            if !topic::within_limits(&publish.topic, limits) {
                debug!(session = id, "PUBLISH dropped: topic exceeds limits");
                return None;
            }
            debug!(session = id, topic = %publish.topic, payload_bytes = publish.payload.len(), "PUBLISH");
            core.dispatch(Message {
                origin: Origin::Client,
                topic: Arc::from(publish.topic),
                payload: publish.payload,
            });
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
