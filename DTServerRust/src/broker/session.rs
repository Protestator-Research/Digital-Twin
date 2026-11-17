//! One task per client connection plus a writer task. Port of `Session`.

use std::net::SocketAddr;
use std::sync::atomic::{AtomicU32, AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::Duration;

use bytes::{Bytes, BytesMut};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::tcp::OwnedWriteHalf;
use tokio::net::TcpStream;
use tokio::sync::{mpsc, watch, Notify};
use tokio::time::{sleep_until, timeout, Instant};
use tracing::{debug, info, warn};

use super::codec::{self, reason, DecodeError, Packet};
use super::core::Core;
use super::subscriptions::{AddResult, SessionId};
use super::topic;
use super::tracker::ConnectionPermit;
use crate::hub::{Message, Origin};

pub(crate) enum Outgoing {
    Publish(Message),
    /// A control packet (CONNACK, SUBACK, PINGRESP, ...). Counted like publishes in the pending bytes.
    Raw(Bytes),
    /// Writes the optional final packet, then closes the connection.
    Close(Option<Bytes>),
}

/// Result of offering a message to a session.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum Offer {
    Queued,
    Dropped,
    /// Dropped, and this was the Nth consecutive overflow: the session is told to disconnect.
    DroppedAndKicked,
    /// Not sent because it exceeds the Maximum Packet Size of the client. Not an overflow.
    TooLarge,
}

/// Sending side of a session: the queue towards the writer task with byte accounting.
pub(crate) struct SessionOut {
    tx: mpsc::UnboundedSender<Outgoing>,
    pending: AtomicUsize,
    overflows: AtomicUsize,
    max_pending: usize,
    max_overflows: usize,
    /// Maximum Packet Size announced by the client in CONNECT, 0 if none.
    client_max_packet: AtomicU32,
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
            client_max_packet: AtomicU32::new(0),
            kick: Notify::new(),
            writer_done: Notify::new(),
        });
        (out, rx)
    }

    pub(crate) fn set_client_max_packet(&self, size: u32) {
        self.client_max_packet.store(size, Ordering::Release);
    }

    /// Queues a QoS 0 PUBLISH unless the pending bytes (topic + properties + payload) would exceed the limit
    /// or the packet is larger than the client accepts. Never blocks.
    pub(crate) fn offer_publish(&self, message: &Message) -> Offer {
        let client_max = self.client_max_packet.load(Ordering::Acquire);
        if client_max != 0 {
            let encoded = codec::publish_encoded_len(
                message.topic.len(),
                message.properties.len(),
                message.payload.len(),
            );
            if encoded.is_none_or(|len| len > client_max as usize) {
                return Offer::TooLarge;
            }
        }
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
    message.topic.len() + message.properties.len() + message.payload.len()
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
                codec::encode_publish(
                    &mut buf,
                    &message.topic,
                    &message.properties,
                    &message.payload,
                );
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

/// Why the session ended, with an optional last packet for the client.
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

    fn disconnect(why: &'static str, reason_code: u8) -> Self {
        Self {
            why,
            goodbye: Some(codec::encode_disconnect(reason_code)),
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
    // 1.5 x the server keep alive, which is the value announced to the client.
    let keep_alive = (limits.server_keep_alive_s > 0)
        .then(|| Duration::from_millis(u64::from(limits.server_keep_alive_s) * 1500));
    let mut deadline = Some(connect_deadline);

    loop {
        loop {
            match codec::decode(&mut buf, limits.max_packet_size) {
                Ok(Some(packet)) => {
                    if let Some(end) =
                        handle_packet(packet, core, id, out, &mut connected, client_id)
                    {
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
                return End::disconnect("server shutting down", reason::SERVER_SHUTTING_DOWN);
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
                    End::disconnect("keep alive timeout", reason::KEEP_ALIVE_TIMEOUT)
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
    if !connected {
        // A server must not send anything but CONNACK before the connection is established.
        return End::closed("invalid first packet");
    }
    match e.disconnect_reason() {
        Some(code) => End::disconnect("invalid packet", code),
        None => End::closed("unsupported protocol"),
    }
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
) -> Option<End> {
    let limits = &core.limits;
    if !*connected {
        return match packet {
            Packet::Connect(connect) => {
                let assigned = connect.client_id.is_empty().then(|| generate_client_id(id));
                *client_id = assigned.clone().unwrap_or(connect.client_id);
                info!(session = id, client_id = %client_id, "CONNECT");
                *connected = true;
                out.set_client_max_packet(connect.max_packet_size.unwrap_or(0));
                let connack = codec::encode_connack(
                    limits.max_packet_size,
                    limits.server_keep_alive_s,
                    assigned.as_deref(),
                );
                if !out.send_control(connack) {
                    return Some(End::abandoned(QUEUE_FULL));
                }
                None
            }
            // The first packet has to be CONNECT.
            _ => Some(End::closed("first packet is not CONNECT")),
        };
    }
    match packet {
        Packet::Connect(_) => Some(End::disconnect("second CONNECT", reason::PROTOCOL_ERROR)),
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
                    reasons.push(reason::TOPIC_FILTER_INVALID);
                    continue;
                }
                match store.add(
                    id,
                    &entry.filter,
                    entry.no_local,
                    limits.max_subscriptions_per_session,
                ) {
                    AddResult::QuotaExceeded => reasons.push(reason::QUOTA_EXCEEDED),
                    AddResult::Added | AddResult::Updated => reasons.push(reason::GRANTED_QOS_0),
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
            let ack = codec::encode_unsuback(packet_id, &vec![reason::SUCCESS; filters.len()]);
            if !out.send_control(ack) {
                return Some(End::abandoned(QUEUE_FULL));
            }
            None
        }
        Packet::Publish(publish) => {
            if !topic::is_valid_topic_name(&publish.topic) {
                return Some(End::disconnect(
                    "invalid topic name",
                    reason::TOPIC_NAME_INVALID,
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
                    properties: publish.properties,
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
            properties: Bytes::new(),
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

    #[test]
    fn publish_larger_than_the_client_maximum_is_not_an_overflow() {
        let (out, mut rx) = SessionOut::new(1_000_000, 1);
        out.set_client_max_packet(20);
        // 1 + 1 + (2 + 1 + 1 + 10) = 16 bytes: fits
        assert_eq!(
            out.offer_publish(&msg(Bytes::from(vec![0; 10]))),
            Offer::Queued
        );
        // 1 + 1 + (2 + 1 + 1 + 20) = 26 bytes: too large, and no overflow even with max_overflows = 1
        assert_eq!(
            out.offer_publish(&msg(Bytes::from(vec![0; 20]))),
            Offer::TooLarge
        );
        assert_eq!(
            out.offer_publish(&msg(Bytes::from(vec![0; 10]))),
            Offer::Queued
        );
        assert!(rx.try_recv().is_ok() && rx.try_recv().is_ok() && rx.try_recv().is_err());
    }
}
