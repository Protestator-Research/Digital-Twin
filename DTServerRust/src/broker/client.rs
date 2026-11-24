//! A client session: the state of one client identifier that can outlive its connection (MQTT 3.1.1 4.1,
//! MQTT 5 4.1). It owns the queue of QoS 1/2 messages towards the client with the state of their handshakes,
//! and the packet identifiers of the QoS 2 PUBLISH packets received from the client that wait for their PUBREL.
//! Subscriptions live in the `SubscriptionStore`, keyed by the same [`SessionId`].
//!
//! All state is behind one mutex. Lock order of the broker: registry (`Core::clients`), subscription store
//! (`Core::store`), then the `inner` lock of one session; nobody takes a registry or store lock while holding
//! a session lock. Ops for the durable copy ([`Op`]) are submitted while the session lock is held, so their
//! order is the order of the state changes. Nothing here waits: packets are handed to the unbounded queue of
//! the writer task of the connection.

use std::collections::{HashMap, HashSet, VecDeque};
use std::sync::{Arc, Mutex};

use tracing::{debug, warn};

use super::codec::{self, puback};
use super::limits::BrokerLimits;
use super::persist::{Op, SessionSnapshot, SessionStore, StoredEntry};
use super::protocol::{ProtocolVersion, QoS};
use super::session::{KickReason, SessionOut};
use super::subscriptions::{SessionId, SubscriptionStore};
use crate::hub::{Message, Origin};

/// Every queue entry owns a packet identifier (1 to 65535), so the queue cannot be longer.
const MAX_QUEUED_HARD_LIMIT: usize = 65_535;

/// Bytes of a message that count against the queue limit: topic and payload, like the pending bytes of QoS 0.
pub(crate) fn message_size(message: &Message) -> usize {
    message.topic.len() + message.payload.len()
}

/// The limits one session works with.
#[derive(Debug, Clone, Copy)]
pub(crate) struct SessionLimits {
    max_bytes: usize,
    max_queued: usize,
}

impl SessionLimits {
    pub(crate) fn new(limits: &BrokerLimits) -> Self {
        Self {
            max_bytes: limits.max_pending_send_bytes,
            max_queued: limits.max_queued_messages.min(MAX_QUEUED_HARD_LIMIT),
        }
    }
}

/// Where a queued message is in its delivery.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum EntryState {
    /// Not sent on the current connection.
    Queued,
    /// PUBLISH sent, waiting for PUBACK (QoS 1) or PUBREC (QoS 2).
    Sent,
    /// QoS 2: PUBREC received, PUBREL sent, waiting for PUBCOMP.
    PubrecReceived,
}

struct Entry {
    seq: u64,
    packet_id: u16,
    qos: QoS,
    state: EntryState,
    /// The PUBLISH was sent on an earlier connection (or is being resent): it goes out with DUP = 1.
    dup: bool,
    message: Message,
}

impl Entry {
    fn stored(&self) -> StoredEntry {
        StoredEntry {
            seq: self.seq,
            packet_id: self.packet_id,
            qos: self.qos,
            pubrec_received: self.state == EntryState::PubrecReceived,
            origin: self.message.origin,
            topic: Arc::clone(&self.message.topic),
            payload: self.message.payload.clone(),
            received_at_ms: self.message.received_at_ms,
        }
    }
}

struct Conn {
    id: u64,
    out: Arc<SessionOut>,
}

struct Inner {
    conn: Option<Conn>,
    /// False from the attach until the CONNACK is queued: nothing is delivered before it, not even QoS 0.
    started: bool,
    /// The session was destroyed. A closed session accepts nothing and submits no ops.
    closed: bool,
    version: ProtocolVersion,
    /// Receive Maximum of the client: how many QoS 1/2 PUBLISH packets may be unacknowledged.
    receive_maximum: u16,
    /// Maximum Packet Size of the client (MQTT 5).
    max_packet_size: Option<u32>,
    expiry_s: u32,
    disconnected_at_ms: Option<i64>,
    /// The session exists in the durable store.
    in_db: bool,
    /// Messages for the client in delivery order. Entries keep their position. On a live connection the
    /// ones in flight (sent, not finished) come first; after a reconnect the sent ones are `Queued` again, so
    /// PUBREC received entries can sit behind them.
    queue: VecDeque<Entry>,
    /// Topic and payload bytes of all entries.
    bytes: usize,
    /// Entries that are `Sent` or `PubrecReceived`.
    in_flight: usize,
    next_seq: u64,
    next_packet_id: u16,
    /// Packet identifier to `seq` of the entry that owns it.
    ids: HashMap<u16, u64>,
    /// Packet identifiers of QoS 2 PUBLISH packets of the client that wait for their PUBREL.
    inbound_qos2: HashSet<u16>,
    /// Messages dropped because the queue was full.
    dropped: u64,
    /// A drop of the current episode was logged as a warning already; cleared when a message fits again.
    drop_warned: bool,
}

/// What a PUBREC from the client asks the connection to do.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum PubrecAction {
    /// Send PUBREL with this reason code (always 0 in MQTT 3.1.1). `durable`: the change must be durable
    /// before the packet leaves.
    Release { reason: u8, durable: bool },
    /// The client refused the message (MQTT 5): the flow is over, nothing is sent.
    Done,
}

/// Result of checking an incoming QoS 2 PUBLISH against the identifiers that wait for their PUBREL.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum Inbound {
    /// First time this identifier is seen: deliver the message.
    New,
    /// The client repeats a PUBLISH that was delivered already: only acknowledge it.
    Duplicate,
    /// More identifiers wait for their PUBREL than allowed.
    Exceeded,
    /// The connection is not the current one of the session any more.
    Stale,
}

/// What `detach` did.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(crate) enum Detached {
    /// Another connection owns the session, nothing changed.
    Stale,
    /// The session waits offline for the client.
    Kept,
    /// The session had expiry 0 and is closed; the caller removes it from the registry.
    Destroyed,
}

/// What the connection of a CONNECT announces.
pub(crate) struct AttachParams {
    pub(crate) conn_id: u64,
    pub(crate) out: Arc<SessionOut>,
    pub(crate) version: ProtocolVersion,
    pub(crate) expiry_s: u32,
    pub(crate) receive_maximum: u16,
    pub(crate) maximum_packet_size: Option<u32>,
}

pub(crate) struct ClientSession {
    pub(crate) id: SessionId,
    pub(crate) client_id: String,
    /// User name that created the session.
    pub(crate) username: Option<String>,
    limits: SessionLimits,
    persist: Option<Arc<dyn SessionStore>>,
    inner: Mutex<Inner>,
}

fn too_large(
    version: ProtocolVersion,
    max_packet_size: Option<u32>,
    message: &Message,
    qos: QoS,
) -> bool {
    max_packet_size.is_some_and(|max| {
        codec::publish_encoded_len(version, message.topic.len(), message.payload.len(), qos)
            .is_none_or(|len| len as u64 > u64::from(max))
    })
}

impl ClientSession {
    pub(crate) fn new(
        id: SessionId,
        client_id: String,
        username: Option<String>,
        limits: SessionLimits,
        persist: Option<Arc<dyn SessionStore>>,
    ) -> Self {
        Self {
            id,
            client_id,
            username,
            limits,
            persist,
            inner: Mutex::new(Inner {
                conn: None,
                started: false,
                closed: false,
                version: ProtocolVersion::V311,
                receive_maximum: u16::MAX,
                max_packet_size: None,
                expiry_s: 0,
                disconnected_at_ms: None,
                in_db: false,
                queue: VecDeque::new(),
                bytes: 0,
                in_flight: 0,
                next_seq: 1,
                next_packet_id: 0,
                ids: HashMap::new(),
                inbound_qos2: HashSet::new(),
                dropped: 0,
                drop_warned: false,
            }),
        }
    }

    /// A session loaded from the durable store: offline, in the store, with everything that was in flight
    /// waiting to be sent again. `now_ms` stands in for a missing disconnect time (the broker was killed while
    /// the client was connected).
    pub(crate) fn restore(
        snapshot: SessionSnapshot,
        limits: SessionLimits,
        persist: Option<Arc<dyn SessionStore>>,
        now_ms: i64,
    ) -> Self {
        let session = Self::new(
            snapshot.id,
            snapshot.client_id,
            snapshot.username,
            limits,
            persist,
        );
        {
            let mut inner = session.inner.lock().unwrap();
            inner.version = snapshot.version;
            inner.expiry_s = snapshot.expiry_s;
            inner.disconnected_at_ms = Some(snapshot.disconnected_at_ms.unwrap_or(now_ms));
            inner.in_db = true;
            for stored in snapshot.outbound {
                inner.next_seq = inner.next_seq.max(stored.seq.saturating_add(1));
                inner.ids.insert(stored.packet_id, stored.seq);
                inner.bytes += stored.topic.len() + stored.payload.len();
                let pubrec = stored.pubrec_received && stored.qos == QoS::ExactlyOnce;
                if pubrec {
                    inner.in_flight += 1;
                }
                inner.queue.push_back(Entry {
                    seq: stored.seq,
                    packet_id: stored.packet_id,
                    qos: stored.qos,
                    state: if pubrec {
                        EntryState::PubrecReceived
                    } else {
                        EntryState::Queued
                    },
                    dup: true,
                    message: Message {
                        origin: stored.origin,
                        topic: stored.topic,
                        payload: stored.payload,
                        received_at_ms: stored.received_at_ms,
                    },
                });
            }
            inner.queue.make_contiguous().sort_by_key(|e| e.seq);
            inner.inbound_qos2 = snapshot.inbound_qos2.into_iter().collect();
        }
        session
    }

    // --- durable copy -------------------------------------------------------------------------

    /// Submits `op` if the session is mirrored in the store. Returns whether it was submitted.
    fn submit(&self, inner: &Inner, op: Op) -> bool {
        match &self.persist {
            Some(store) if !inner.closed && inner.expiry_s > 0 && inner.in_db => {
                store.submit(op);
                true
            }
            _ => false,
        }
    }

    /// Submits an op that does not touch the queue (a subscription change), under the caller's store lock.
    /// Returns whether it was submitted, which means the acknowledgement has to wait for a barrier.
    pub(crate) fn persist_op(&self, op: Op) -> bool {
        let inner = self.inner.lock().unwrap();
        self.submit(&inner, op)
    }

    fn snapshot(&self, inner: &Inner, subscriptions: &SubscriptionStore) -> SessionSnapshot {
        SessionSnapshot {
            id: self.id,
            client_id: self.client_id.clone(),
            username: self.username.clone(),
            version: inner.version,
            expiry_s: inner.expiry_s,
            disconnected_at_ms: inner.disconnected_at_ms,
            subscriptions: subscriptions.subscriptions_of(self.id),
            outbound: inner.queue.iter().map(Entry::stored).collect(),
            inbound_qos2: inner.inbound_qos2.iter().copied().collect(),
        }
    }

    // --- connection ---------------------------------------------------------------------------

    fn current(inner: &Inner, conn_id: u64) -> bool {
        !inner.closed && inner.conn.as_ref().is_some_and(|c| c.id == conn_id)
    }

    /// Makes the connection the one of the session. An earlier connection is told to close (session taken
    /// over). Everything that was sent and not acknowledged on it is queued again. Nothing is delivered
    /// until [`ClientSession::start`]. `subscriptions` is only read to describe the session to the store.
    /// Returns true if ops were submitted, so the CONNACK has to wait for a barrier.
    pub(crate) fn attach(&self, params: AttachParams, subscriptions: &SubscriptionStore) -> bool {
        let mut guard = self.inner.lock().unwrap();
        let inner = &mut *guard;
        if let Some(old) = inner.conn.take() {
            old.out.kick(KickReason::TakenOver);
        }
        inner.conn = Some(Conn {
            id: params.conn_id,
            out: params.out,
        });
        inner.started = false;
        inner.version = params.version;
        inner.receive_maximum = params.receive_maximum.max(1);
        inner.max_packet_size = params.maximum_packet_size;
        inner.expiry_s = params.expiry_s;
        inner.disconnected_at_ms = None;
        Self::requeue_sent(inner);

        if self.persist.is_none() {
            return false;
        }
        let store = self.persist.as_ref().expect("checked above");
        if inner.expiry_s > 0 && !inner.in_db {
            store.submit(Op::PutSession(self.snapshot(inner, subscriptions)));
            inner.in_db = true;
            true
        } else if inner.expiry_s > 0 {
            store.submit(Op::UpdateSession {
                id: self.id,
                expiry_s: inner.expiry_s,
                disconnected_at_ms: None,
            });
            true
        } else if inner.in_db {
            store.submit(Op::DeleteSession { id: self.id });
            inner.in_db = false;
            true
        } else {
            false
        }
    }

    /// The CONNACK is queued: deliveries may begin. First the PUBREL of every QoS 2 message whose PUBREC
    /// arrived on an earlier connection, then the messages that were not acknowledged (with DUP = 1), then
    /// the rest. Returns false if the connection is not the current one.
    pub(crate) fn start(&self, conn_id: u64) -> bool {
        let mut guard = self.inner.lock().unwrap();
        let inner = &mut *guard;
        if !Self::current(inner, conn_id) {
            return false;
        }
        inner.started = true;
        let out = Arc::clone(&inner.conn.as_ref().expect("current").out);
        for entry in &inner.queue {
            if entry.state == EntryState::PubrecReceived {
                // already durable, so no barrier; the packet goes out before any PUBLISH
                out.send_control(codec::encode_pubrel(inner.version, entry.packet_id, 0));
            }
        }
        self.pump(inner);
        true
    }

    /// The connection ended. Without a newer connection the unacknowledged messages go back to the queue,
    /// and the session either waits for the client or, with expiry 0, is destroyed.
    pub(crate) fn detach(&self, conn_id: u64, now_ms: i64) -> Detached {
        let mut guard = self.inner.lock().unwrap();
        let inner = &mut *guard;
        if !Self::current(inner, conn_id) {
            return Detached::Stale;
        }
        inner.conn = None;
        inner.started = false;
        inner.disconnected_at_ms = Some(now_ms);
        Self::requeue_sent(inner);
        if inner.expiry_s == 0 {
            self.close_locked(inner);
            return Detached::Destroyed;
        }
        self.submit(
            inner,
            Op::UpdateSession {
                id: self.id,
                expiry_s: inner.expiry_s,
                disconnected_at_ms: Some(now_ms),
            },
        );
        Detached::Kept
    }

    /// Ends the session for good: the connection (if any) is told that its session was taken over, the
    /// queue is dropped and the store forgets the session. The caller removes it from the registry.
    pub(crate) fn destroy(&self) {
        let mut guard = self.inner.lock().unwrap();
        self.close_locked(&mut guard);
    }

    fn close_locked(&self, inner: &mut Inner) {
        if inner.closed {
            return;
        }
        inner.closed = true;
        inner.started = false;
        if let Some(old) = inner.conn.take() {
            old.out.kick(KickReason::TakenOver);
        }
        if inner.in_db {
            if let Some(store) = &self.persist {
                store.submit(Op::DeleteSession { id: self.id });
            }
            inner.in_db = false;
        }
        inner.queue.clear();
        inner.ids.clear();
        inner.inbound_qos2.clear();
        inner.bytes = 0;
        inner.in_flight = 0;
    }

    /// Entries that were sent on a connection that is gone wait for the next one and are marked as repeated.
    fn requeue_sent(inner: &mut Inner) {
        for entry in &mut inner.queue {
            if entry.state == EntryState::Sent {
                entry.state = EntryState::Queued;
                entry.dup = true;
                inner.in_flight -= 1;
            }
        }
    }

    /// MQTT 5 DISCONNECT with a Session Expiry Interval: replaces the expiry of the session (capped by
    /// `max_s`). Raising 0 is a protocol error (MQTT 3.14.2.2.2): returns false and leaves the session as it is
    /// (it ends with the connection).
    pub(crate) fn apply_disconnect_expiry(&self, conn_id: u64, requested: u32, max_s: u32) -> bool {
        let mut inner = self.inner.lock().unwrap();
        if !Self::current(&inner, conn_id) {
            return true;
        }
        if inner.expiry_s == 0 && requested != 0 {
            return false;
        }
        inner.expiry_s = requested.min(max_s);
        true
    }

    /// True if `conn_id` is the connection of the session (and the session is not destroyed).
    pub(crate) fn is_current(&self, conn_id: u64) -> bool {
        Self::current(&self.inner.lock().unwrap(), conn_id)
    }

    /// True if the session is offline and its expiry has passed.
    pub(crate) fn is_expired(&self, now_ms: i64) -> bool {
        let inner = self.inner.lock().unwrap();
        match inner.disconnected_at_ms {
            Some(at) if inner.conn.is_none() && !inner.closed => {
                now_ms >= at.saturating_add(i64::from(inner.expiry_s).saturating_mul(1000))
            }
            _ => false,
        }
    }

    /// True if the session outlives its connection (counts against `max_persistent_sessions`).
    pub(crate) fn is_persistent(&self) -> bool {
        let inner = self.inner.lock().unwrap();
        !inner.closed && inner.expiry_s > 0
    }

    #[cfg(test)]
    pub(crate) fn is_closed(&self) -> bool {
        self.inner.lock().unwrap().closed
    }

    // --- delivery to the client -----------------------------------------------------------------

    /// Delivers a message with the given QoS (already the minimum of the message and the subscription).
    /// QoS 0 goes to the connection if there is a started one; QoS 1/2 is queued, also for an offline session, and
    /// sent as far as the Receive Maximum of the client allows. Exception: a message of the server itself
    /// (`Origin::Server`) at QoS 1/2 is only queued while the session has a connection (attached, started or
    /// not: it is sent at `start`); for an offline session it is skipped (nothing is retained, so it would only
    /// replay outdated values). Returns true if the message was queued in a
    /// session mirrored in the store, so that a publisher's acknowledgement has to wait for a barrier.
    pub(crate) fn deliver(&self, message: &Message, qos: QoS) -> bool {
        let mut guard = self.inner.lock().unwrap();
        let inner = &mut *guard;
        if inner.closed {
            return false;
        }
        if qos == QoS::AtMostOnce {
            if let (true, Some(conn)) = (inner.started, &inner.conn) {
                // the client's Maximum Packet Size binds QoS 0 as well (MQTT 5, 3.1.2.11.4)
                if !too_large(inner.version, inner.max_packet_size, message, qos) {
                    conn.out.offer_publish(message);
                }
            }
            return false;
        }
        // Messages of the server itself (initial values, registration requests) are not kept for a client that is
        // not there: nothing is retained, so a session that stays away would replay outdated values. Skipped
        // silently: not queued, no store op, not a drop. What was queued while the client was connected stays.
        if message.origin == Origin::Server && inner.conn.is_none() {
            return false;
        }
        if inner.conn.is_some() && too_large(inner.version, inner.max_packet_size, message, qos) {
            debug!(session = self.id, topic = %message.topic, "message dropped: larger than the maximum packet size of the client");
            return false;
        }
        let size = message_size(message);
        if inner.bytes + size > self.limits.max_bytes || inner.queue.len() >= self.limits.max_queued
        {
            // A full queue says nothing about the socket (small Receive Maximum, a backlog from an earlier
            // connection), so the client is not kicked: the message is dropped and counted.
            inner.dropped += 1;
            if !inner.drop_warned {
                inner.drop_warned = true;
                warn!(
                    session = self.id,
                    client_id = %self.client_id,
                    connected = inner.conn.is_some(),
                    queued = inner.queue.len(),
                    queued_bytes = inner.bytes,
                    max_queued_messages = self.limits.max_queued,
                    max_pending_send_bytes = self.limits.max_bytes,
                    topic = %message.topic,
                    "queue of the session is full: messages for it are dropped until the client acknowledges some \
                     (further drops of this episode are logged at debug level)"
                );
            } else {
                debug!(session = self.id, topic = %message.topic, online = inner.conn.is_some(), "message dropped: queue of the session is full");
            }
            return false;
        }
        inner.drop_warned = false;
        let packet_id = Self::allocate_packet_id(inner);
        let seq = inner.next_seq;
        inner.next_seq += 1;
        inner.bytes += size;
        inner.ids.insert(packet_id, seq);
        let entry = Entry {
            seq,
            packet_id,
            qos,
            state: EntryState::Queued,
            dup: false,
            message: message.clone(),
        };
        let stored = entry.stored();
        inner.queue.push_back(entry);
        let submitted = self.submit(
            inner,
            Op::Enqueue {
                id: self.id,
                entry: stored,
            },
        );
        self.pump(inner);
        submitted
    }

    /// The next identifier that no entry of the queue uses. The queue is shorter than 65535 entries
    /// whenever this is called, so there is one.
    fn allocate_packet_id(inner: &mut Inner) -> u16 {
        loop {
            inner.next_packet_id = if inner.next_packet_id == u16::MAX {
                1
            } else {
                inner.next_packet_id + 1
            };
            if !inner.ids.contains_key(&inner.next_packet_id) {
                return inner.next_packet_id;
            }
        }
    }

    /// Sends queued entries in order while fewer than Receive Maximum are in flight.
    fn pump(&self, inner: &mut Inner) {
        if !inner.started {
            return;
        }
        let Some(out) = inner.conn.as_ref().map(|c| Arc::clone(&c.out)) else {
            return;
        };
        let window = usize::from(inner.receive_maximum);
        let mut i = 0;
        while inner.in_flight < window && i < inner.queue.len() {
            let entry = &inner.queue[i];
            if entry.state != EntryState::Queued {
                i += 1;
                continue;
            }
            if too_large(
                inner.version,
                inner.max_packet_size,
                &entry.message,
                entry.qos,
            ) {
                debug!(session = self.id, topic = %entry.message.topic, "message dropped: larger than the maximum packet size of the client");
                self.remove_at(inner, i);
                continue;
            }
            let entry = &mut inner.queue[i];
            if !out.send_publish(&entry.message, entry.qos, entry.packet_id, entry.dup) {
                return;
            }
            entry.state = EntryState::Sent;
            inner.in_flight += 1;
            i += 1;
        }
    }

    fn find(inner: &Inner, packet_id: u16) -> Option<usize> {
        let seq = *inner.ids.get(&packet_id)?;
        inner.queue.binary_search_by_key(&seq, |e| e.seq).ok()
    }

    /// Removes a finished entry: its identifier becomes free, the store forgets it (write-behind).
    fn remove_at(&self, inner: &mut Inner, index: usize) {
        let Some(entry) = inner.queue.remove(index) else {
            return;
        };
        inner.bytes -= message_size(&entry.message);
        inner.ids.remove(&entry.packet_id);
        if entry.state != EntryState::Queued {
            inner.in_flight -= 1;
        }
        self.submit(
            inner,
            Op::DeleteOutbound {
                id: self.id,
                seq: entry.seq,
            },
        );
    }

    /// PUBACK of the client. Unknown identifiers (and identifiers of QoS 2 messages) are ignored.
    pub(crate) fn on_puback(&self, conn_id: u64, packet_id: u16) {
        let mut guard = self.inner.lock().unwrap();
        let inner = &mut *guard;
        if !Self::current(inner, conn_id) {
            return;
        }
        if let Some(i) = Self::find(inner, packet_id) {
            let entry = &inner.queue[i];
            if entry.qos == QoS::AtLeastOnce && entry.state == EntryState::Sent {
                self.remove_at(inner, i);
                self.pump(inner);
            }
        }
    }

    /// PUBREC of the client for a QoS 2 message.
    pub(crate) fn on_pubrec(&self, conn_id: u64, packet_id: u16, reason: u8) -> PubrecAction {
        let mut guard = self.inner.lock().unwrap();
        let inner = &mut *guard;
        let unknown = PubrecAction::Release {
            reason: if inner.version == ProtocolVersion::V5 {
                puback::PACKET_IDENTIFIER_NOT_FOUND
            } else {
                0
            },
            durable: false,
        };
        if !Self::current(inner, conn_id) {
            return PubrecAction::Done;
        }
        let Some(i) = Self::find(inner, packet_id) else {
            return unknown;
        };
        let entry = &mut inner.queue[i];
        if entry.qos != QoS::ExactlyOnce || entry.state == EntryState::Queued {
            return unknown;
        }
        if reason >= 0x80 {
            // MQTT 5: the client refuses the message, the handshake ends here
            self.remove_at(inner, i);
            self.pump(inner);
            return PubrecAction::Done;
        }
        if entry.state == EntryState::PubrecReceived {
            // PUBREC repeated: the PUBREL may have been lost, send it again
            return PubrecAction::Release {
                reason: 0,
                durable: false,
            };
        }
        entry.state = EntryState::PubrecReceived;
        let seq = entry.seq;
        let durable = self.submit(inner, Op::OutboundPubrec { id: self.id, seq });
        PubrecAction::Release { reason: 0, durable }
    }

    /// PUBCOMP of the client: the QoS 2 handshake is over.
    pub(crate) fn on_pubcomp(&self, conn_id: u64, packet_id: u16) {
        let mut guard = self.inner.lock().unwrap();
        let inner = &mut *guard;
        if !Self::current(inner, conn_id) {
            return;
        }
        if let Some(i) = Self::find(inner, packet_id) {
            if inner.queue[i].state == EntryState::PubrecReceived {
                self.remove_at(inner, i);
                self.pump(inner);
            }
        }
    }

    // --- QoS 2 from the client ------------------------------------------------------------------

    /// Looks at the identifier of a QoS 2 PUBLISH of the client and, if it is new, remembers it right away,
    /// atomically with the check, so that a repeated PUBLISH can never be delivered twice, whatever happens
    /// to the connection while the message is dispatched. `max_awaiting` is the number of identifiers that
    /// may wait for their PUBREL (MQTT 5 only).
    pub(crate) fn inbound_begin(
        &self,
        conn_id: u64,
        packet_id: u16,
        max_awaiting: Option<usize>,
    ) -> Inbound {
        let mut inner = self.inner.lock().unwrap();
        if !Self::current(&inner, conn_id) {
            return Inbound::Stale;
        }
        if inner.inbound_qos2.contains(&packet_id) {
            return Inbound::Duplicate;
        }
        if max_awaiting.is_some_and(|max| inner.inbound_qos2.len() >= max) {
            return Inbound::Exceeded;
        }
        inner.inbound_qos2.insert(packet_id);
        Inbound::New
    }

    /// Submits the op for an identifier remembered by [`ClientSession::inbound_begin`]. Called after the
    /// message was delivered (so a stored identifier implies a stored message) and before the PUBREC is
    /// sent. Returns true if an op was submitted (the PUBREC waits for a barrier).
    pub(crate) fn inbound_persist(&self, packet_id: u16) -> bool {
        let inner = self.inner.lock().unwrap();
        inner.inbound_qos2.contains(&packet_id)
            && self.submit(
                &inner,
                Op::AddInbound {
                    id: self.id,
                    packet_id,
                },
            )
    }

    /// PUBREL of the client. Returns whether the identifier was known and whether an op was submitted (the
    /// PUBCOMP waits for a barrier).
    pub(crate) fn inbound_release(&self, conn_id: u64, packet_id: u16) -> (bool, bool) {
        let mut guard = self.inner.lock().unwrap();
        let inner = &mut *guard;
        if !Self::current(inner, conn_id) || !inner.inbound_qos2.remove(&packet_id) {
            return (false, false);
        }
        let submitted = self.submit(
            inner,
            Op::RemoveInbound {
                id: self.id,
                packet_id,
            },
        );
        (true, submitted)
    }

    // --- introspection (tests and logs) ---------------------------------------------------------

    #[cfg(test)]
    pub(crate) fn dropped(&self) -> u64 {
        self.inner.lock().unwrap().dropped
    }

    #[cfg(test)]
    pub(crate) fn queue_len(&self) -> usize {
        self.inner.lock().unwrap().queue.len()
    }

    #[cfg(test)]
    pub(crate) fn queued_bytes(&self) -> usize {
        self.inner.lock().unwrap().bytes
    }

    #[cfg(test)]
    pub(crate) fn in_flight(&self) -> usize {
        self.inner.lock().unwrap().in_flight
    }

    /// (packet id, state, dup) of every entry in queue order.
    #[cfg(test)]
    pub(crate) fn entries(&self) -> Vec<(u16, EntryState, bool)> {
        let inner = self.inner.lock().unwrap();
        inner
            .queue
            .iter()
            .map(|e| (e.packet_id, e.state, e.dup))
            .collect()
    }

    #[cfg(test)]
    pub(crate) fn inbound_ids(&self) -> Vec<u16> {
        let mut v: Vec<u16> = self
            .inner
            .lock()
            .unwrap()
            .inbound_qos2
            .iter()
            .copied()
            .collect();
        v.sort_unstable();
        v
    }

    #[cfg(test)]
    pub(crate) fn expiry_s(&self) -> u32 {
        self.inner.lock().unwrap().expiry_s
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::broker::persist::testing::RecordingStore;
    use crate::broker::session::Outgoing;
    use crate::hub::Origin;
    use bytes::Bytes;
    use tokio::sync::mpsc::UnboundedReceiver;

    /// What the writer task would be asked to write, in a form tests can compare.
    #[derive(Debug, PartialEq, Eq)]
    enum Sent {
        Publish {
            topic: String,
            qos: u8,
            id: u16,
            dup: bool,
        },
        Qos0(String),
        Raw(Vec<u8>),
    }

    fn publish(topic: &str, qos: QoS, id: u16, dup: bool) -> Sent {
        Sent::Publish {
            topic: topic.to_owned(),
            qos: qos.bits(),
            id,
            dup,
        }
    }

    fn drain(rx: &mut UnboundedReceiver<Outgoing>) -> Vec<Sent> {
        let mut v = Vec::new();
        while let Ok(o) = rx.try_recv() {
            v.push(match o {
                Outgoing::PublishQos {
                    message,
                    qos,
                    packet_id,
                    dup,
                } => Sent::Publish {
                    topic: message.topic.to_string(),
                    qos: qos.bits(),
                    id: packet_id,
                    dup,
                },
                Outgoing::Publish(m) => Sent::Qos0(m.topic.to_string()),
                Outgoing::Raw(b) => Sent::Raw(b.to_vec()),
                Outgoing::Close(_) => unreachable!("tests never close"),
            });
        }
        v
    }

    fn msg(topic: &str, payload: &[u8]) -> Message {
        Message {
            origin: Origin::Client,
            topic: Arc::from(topic),
            payload: Bytes::copy_from_slice(payload),
            received_at_ms: 7,
        }
    }

    fn limits() -> SessionLimits {
        SessionLimits::new(&BrokerLimits::default())
    }

    fn session(limits: SessionLimits) -> ClientSession {
        ClientSession::new(1, "c".into(), None, limits, None)
    }

    struct Conn5 {
        out: Arc<SessionOut>,
        rx: UnboundedReceiver<Outgoing>,
    }

    /// Attaches a new connection and starts it.
    fn connect(
        s: &ClientSession,
        conn_id: u64,
        version: ProtocolVersion,
        expiry_s: u32,
        receive_maximum: u16,
    ) -> Conn5 {
        let c = attach(s, conn_id, version, expiry_s, receive_maximum, None);
        assert!(s.start(conn_id));
        c
    }

    fn attach(
        s: &ClientSession,
        conn_id: u64,
        version: ProtocolVersion,
        expiry_s: u32,
        receive_maximum: u16,
        max_packet_size: Option<u32>,
    ) -> Conn5 {
        let (out, rx) = SessionOut::new(1 << 20, 3);
        s.attach(
            AttachParams {
                conn_id,
                out: Arc::clone(&out),
                version,
                expiry_s,
                receive_maximum,
                maximum_packet_size: max_packet_size,
            },
            &SubscriptionStore::new(),
        );
        Conn5 { out, rx }
    }

    const V311: ProtocolVersion = ProtocolVersion::V311;
    const V5: ProtocolVersion = ProtocolVersion::V5;
    const Q1: QoS = QoS::AtLeastOnce;
    const Q2: QoS = QoS::ExactlyOnce;

    #[test]
    fn at_most_receive_maximum_messages_are_in_flight_and_the_rest_waits_in_order() {
        let s = session(limits());
        let mut c = connect(&s, 1, V311, 0, 2);
        for i in 0..5 {
            s.deliver(&msg(&format!("t{i}"), b"x"), Q1);
        }
        assert_eq!(
            drain(&mut c.rx),
            vec![publish("t0", Q1, 1, false), publish("t1", Q1, 2, false)]
        );
        assert_eq!((s.in_flight(), s.queue_len()), (2, 5));
        // an acknowledgement opens the window by one, the next message in order goes out
        s.on_puback(1, 1);
        assert_eq!(drain(&mut c.rx), vec![publish("t2", Q1, 3, false)]);
        // an unknown id, and an id that is not in flight yet, change nothing
        s.on_puback(1, 99);
        s.on_puback(1, 5);
        assert!(drain(&mut c.rx).is_empty());
        s.on_puback(1, 2);
        s.on_puback(1, 3);
        assert_eq!(
            drain(&mut c.rx),
            vec![publish("t3", Q1, 4, false), publish("t4", Q1, 5, false)]
        );
        assert_eq!(s.in_flight(), 2);
    }

    #[test]
    fn packet_ids_are_unique_never_zero_and_reused_after_the_wrap() {
        let s = session(limits());
        let mut c = connect(&s, 1, V311, 0, 10);
        for i in 0..3 {
            s.deliver(&msg(&format!("t{i}"), b""), Q1);
        }
        let ids: Vec<u16> = s.entries().iter().map(|e| e.0).collect();
        assert_eq!(ids, [1, 2, 3]);
        s.on_puback(1, 2);
        // ids go on counting up, the freed id 2 is not handed out again right away
        s.deliver(&msg("t3", b""), Q1);
        assert_eq!(s.entries().last().unwrap().0, 4);
        drain(&mut c.rx);
        // one at a time through the whole id space: never 0, wraps to 1, skips ids still in use
        for id in [1u16, 3, 4] {
            s.on_puback(1, id);
        }
        s.deliver(&msg("keep", b""), Q1); // stays unacknowledged
        let keep = s.entries()[0].0;
        let mut last = keep;
        for _ in 0..70_000 {
            s.deliver(&msg("t", b""), Q1);
            let id = s.entries().last().unwrap().0;
            assert_ne!(id, 0);
            assert_ne!(id, keep);
            last = id;
            s.on_puback(1, id);
        }
        assert_ne!(last, 0);
        assert_eq!(s.queue_len(), 1);
    }

    #[test]
    fn packet_ids_wrap_past_65535_without_colliding_with_ids_still_in_flight() {
        let s = session(limits());
        let mut c = connect(&s, 1, V311, 0, u16::MAX);
        // 50 messages stay unacknowledged for two full rounds through the identifier space, spread over it;
        // all the others are acknowledged at once
        let mut held = HashSet::new();
        let mut wraps = 0;
        let mut previous = 0;
        for n in 1..=140_000u32 {
            s.deliver(&msg("t", b""), Q1);
            let id = s.entries().last().unwrap().0;
            assert_ne!(id, 0, "delivery {n}");
            assert!(
                !held.contains(&id),
                "delivery {n} got id {id}, which is still in flight"
            );
            if id < previous {
                wraps += 1;
            }
            previous = id;
            if n <= 65_535 && n % 1_300 == 0 {
                held.insert(id);
            } else {
                s.on_puback(1, id);
            }
            if n % 10_000 == 0 {
                drain(&mut c.rx);
            }
        }
        assert_eq!(wraps, 2, "the counter wrapped twice");
        assert_eq!(held.len(), 50);
        assert_eq!(s.queue_len(), 50);
        let mut ids: Vec<u16> = s.entries().iter().map(|e| e.0).collect();
        ids.sort_unstable();
        ids.dedup();
        assert_eq!(ids.len(), 50, "no identifier is used twice");
    }

    #[test]
    fn unacknowledged_messages_are_sent_again_with_dup_after_a_reconnect() {
        let s = session(limits());
        let mut c = connect(&s, 1, V311, 60, 2);
        for i in 0..3 {
            s.deliver(&msg(&format!("t{i}"), b"x"), Q1);
        }
        drain(&mut c.rx); // t0 and t1 sent
        s.on_puback(1, 1);
        drain(&mut c.rx); // t2 sent
        assert_eq!(s.detach(1, 1000), Detached::Kept);
        assert_eq!(
            s.entries(),
            vec![(2, EntryState::Queued, true), (3, EntryState::Queued, true)]
        );
        s.deliver(&msg("t3", b"x"), Q1);
        let mut c = connect(&s, 2, V311, 60, 2);
        // the two that were sent before come first with DUP, the new one waits for window space
        assert_eq!(
            drain(&mut c.rx),
            vec![publish("t1", Q1, 2, true), publish("t2", Q1, 3, true)]
        );
        s.on_puback(2, 2);
        assert_eq!(drain(&mut c.rx), vec![publish("t3", Q1, 4, false)]);
    }

    #[test]
    fn qos2_goes_through_publish_pubrec_pubrel_pubcomp() {
        let s = session(limits());
        let mut c = connect(&s, 1, V311, 0, 10);
        s.deliver(&msg("t", b"x"), Q2);
        assert_eq!(drain(&mut c.rx), vec![publish("t", Q2, 1, false)]);
        assert_eq!(
            s.on_pubrec(1, 1, 0),
            PubrecAction::Release {
                reason: 0,
                durable: false
            }
        );
        assert_eq!(s.entries(), vec![(1, EntryState::PubrecReceived, false)]);
        // still in flight until PUBCOMP, and a PUBACK does not finish a QoS 2 message
        s.on_puback(1, 1);
        assert_eq!(s.in_flight(), 1);
        // a repeated PUBREC asks for the PUBREL again
        assert_eq!(
            s.on_pubrec(1, 1, 0),
            PubrecAction::Release {
                reason: 0,
                durable: false
            }
        );
        s.on_pubcomp(1, 1);
        assert_eq!((s.queue_len(), s.in_flight(), s.queued_bytes()), (0, 0, 0));
    }

    #[test]
    fn pubrec_for_an_unknown_id_is_answered_with_pubrel_and_mqtt5_says_not_found() {
        for (version, reason) in [(V311, 0), (V5, 0x92)] {
            let s = session(limits());
            let _c = connect(&s, 1, version, 0, 10);
            assert_eq!(
                s.on_pubrec(1, 42, 0),
                PubrecAction::Release {
                    reason,
                    durable: false
                }
            );
            // unknown PUBCOMP and PUBACK are ignored
            s.on_pubcomp(1, 42);
            s.on_puback(1, 42);
        }
    }

    #[test]
    fn mqtt5_pubrec_with_an_error_ends_the_flow() {
        let s = session(limits());
        let mut c = connect(&s, 1, V5, 0, 1);
        s.deliver(&msg("a", b"x"), Q2);
        s.deliver(&msg("b", b"x"), Q1);
        assert_eq!(drain(&mut c.rx).len(), 1);
        assert_eq!(s.on_pubrec(1, 1, 0x80), PubrecAction::Done);
        // the entry is gone without a PUBREL, the window is free for the next message
        assert_eq!(drain(&mut c.rx), vec![publish("b", Q1, 2, false)]);
        assert_eq!(s.queue_len(), 1);
    }

    #[test]
    fn reconnect_sends_pubrel_first_then_the_unacknowledged_publishes() {
        let s = session(limits());
        let mut c = connect(&s, 1, V311, 60, 10);
        s.deliver(&msg("a", b"x"), Q2); // id 1: PUBREC received
        s.deliver(&msg("b", b"x"), Q1); // id 2: unacknowledged
        s.deliver(&msg("c", b"x"), Q2); // id 3: unacknowledged
        drain(&mut c.rx);
        s.on_pubrec(1, 1, 0);
        s.detach(1, 5);
        s.deliver(&msg("d", b"x"), Q1); // id 4, queued while offline
        let mut c = connect(&s, 2, V311, 60, 10);
        assert_eq!(
            drain(&mut c.rx),
            vec![
                Sent::Raw(vec![0x62, 2, 0, 1]),
                publish("b", Q1, 2, true),
                publish("c", Q2, 3, true),
                publish("d", Q1, 4, false),
            ]
        );
        s.on_pubcomp(2, 1);
        assert_eq!(s.queue_len(), 3);
    }

    #[test]
    fn a_pubrec_received_entry_counts_as_in_flight_after_a_reconnect() {
        let s = session(limits());
        let mut c = connect(&s, 1, V311, 60, 1);
        s.deliver(&msg("a", b"x"), Q2);
        s.deliver(&msg("b", b"x"), Q1);
        s.on_pubrec(1, 1, 0);
        drain(&mut c.rx);
        s.detach(1, 5);
        let mut c = connect(&s, 2, V311, 60, 1);
        // window 1: only the PUBREL, "b" waits for the PUBCOMP
        assert_eq!(drain(&mut c.rx), vec![Sent::Raw(vec![0x62, 2, 0, 1])]);
        s.on_pubcomp(2, 1);
        // "b" was never sent before, so it is no repetition
        assert_eq!(drain(&mut c.rx), vec![publish("b", Q1, 2, false)]);
    }

    #[test]
    fn nothing_is_delivered_before_start_and_qos0_is_not_kept_offline() {
        let s = session(limits());
        let mut c = attach(&s, 1, V311, 60, 10, None);
        s.deliver(&msg("q0", b"x"), QoS::AtMostOnce);
        s.deliver(&msg("q1", b"x"), Q1);
        assert!(drain(&mut c.rx).is_empty());
        assert!(s.start(1));
        // the QoS 0 message was dropped, the QoS 1 message was waiting
        assert_eq!(drain(&mut c.rx), vec![publish("q1", Q1, 1, false)]);
        s.deliver(&msg("q0", b"x"), QoS::AtMostOnce);
        assert_eq!(drain(&mut c.rx), vec![Sent::Qos0("q0".into())]);
        s.detach(1, 5);
        s.deliver(&msg("q0", b"x"), QoS::AtMostOnce);
        assert_eq!(s.queue_len(), 1);
    }

    #[test]
    fn inbound_qos2_is_delivered_once_until_the_pubrel() {
        let s = session(limits());
        let _c = connect(&s, 1, V311, 0, 10);
        // the identifier is remembered by the check itself, before the message is dispatched
        assert_eq!(s.inbound_begin(1, 7, None), Inbound::New);
        assert_eq!(s.inbound_ids(), [7]);
        assert_eq!(s.inbound_begin(1, 7, None), Inbound::Duplicate);
        assert!(!s.inbound_persist(7)); // no store, so no op
        assert_eq!(s.inbound_release(1, 7), (true, false));
        assert_eq!(s.inbound_begin(1, 7, None), Inbound::New);
        // PUBREL of an unknown id
        assert_eq!(s.inbound_release(1, 8), (false, false));
        // another connection owns the session: the old one is ignored
        assert_eq!(s.inbound_begin(9, 8, None), Inbound::Stale);
        assert_eq!(s.inbound_ids(), [7]);
    }

    #[test]
    fn a_repeat_after_a_take_over_during_the_dispatch_is_a_duplicate() {
        let s = session(limits());
        let _old = connect(&s, 1, V311, 60, 10);
        assert_eq!(s.inbound_begin(1, 7, None), Inbound::New);
        // take-over between the check and the end of the dispatch; the client repeats the PUBLISH
        let _new = connect(&s, 2, V311, 60, 10);
        assert_eq!(s.inbound_begin(2, 7, None), Inbound::Duplicate);
    }

    #[test]
    fn inbound_qos2_limit_counts_waiting_identifiers() {
        let s = session(limits());
        let _c = connect(&s, 1, V5, 0, 10);
        for id in 1..=2 {
            assert_eq!(s.inbound_begin(1, id, Some(2)), Inbound::New);
        }
        assert_eq!(s.inbound_begin(1, 3, Some(2)), Inbound::Exceeded);
        assert_eq!(s.inbound_ids(), [1, 2], "a refused id leaves no state");
        // a repeated id is no new one
        assert_eq!(s.inbound_begin(1, 2, Some(2)), Inbound::Duplicate);
        s.inbound_release(1, 1);
        assert_eq!(s.inbound_begin(1, 3, Some(2)), Inbound::New);
    }

    #[test]
    fn inbound_ids_survive_the_connection() {
        let s = session(limits());
        let _c = connect(&s, 1, V311, 60, 10);
        s.inbound_begin(1, 5, None);
        s.detach(1, 1);
        let _c = connect(&s, 2, V311, 60, 10);
        assert_eq!(s.inbound_begin(2, 5, None), Inbound::Duplicate);
    }

    #[test]
    fn a_full_queue_drops_and_counts_without_kicking_the_client() {
        let l = BrokerLimits {
            max_pending_send_bytes: 10,
            max_consecutive_send_overflows: 3,
            ..BrokerLimits::default()
        };
        let s = session(SessionLimits::new(&l));
        let mut c = connect(&s, 1, V311, 0, 1);
        s.deliver(&msg("t", b"12345678"), Q1); // 9 bytes
        for _ in 0..10 {
            s.deliver(&msg("t", b"12345678"), Q1); // would be 18: dropped
        }
        // a full queue says nothing about the socket
        assert_eq!(c.out.kicked(), None);
        assert_eq!((s.queue_len(), s.queued_bytes()), (1, 9));
        assert_eq!(drain(&mut c.rx).len(), 1);
        assert_eq!(s.dropped(), 10);
        // and the session works on
        s.on_puback(1, 1);
        s.deliver(&msg("t", b"1"), Q1);
        assert_eq!(drain(&mut c.rx).len(), 1);
    }

    #[test]
    fn count_limit_applies_online_and_offline_and_drops_are_counted() {
        let l = BrokerLimits {
            max_queued_messages: 2,
            ..BrokerLimits::default()
        };
        let s = session(SessionLimits::new(&l));
        let c = connect(&s, 1, V311, 60, 10);
        for _ in 0..3 {
            s.deliver(&msg("t", b"x"), Q1);
        }
        assert_eq!((s.queue_len(), s.dropped()), (2, 1));
        assert!(s.inner.lock().unwrap().drop_warned, "the first drop warns");
        s.deliver(&msg("t", b"x"), Q1);
        assert_eq!(
            s.dropped(),
            2,
            "every drop is counted, the warning is once per episode"
        );
        assert_eq!(c.out.kicked(), None);
        s.detach(1, 1);
        for _ in 0..4 {
            s.deliver(&msg("t", b"x"), Q2);
        }
        assert_eq!((s.queue_len(), s.dropped()), (2, 6));
        // freeing an entry makes room
        let _c = connect(&s, 2, V311, 60, 10);
        s.on_puback(2, 1);
        assert!(!s.deliver(&msg("t", b"x"), Q1));
        assert_eq!((s.queue_len(), s.dropped()), (2, 6));
        assert!(
            !s.inner.lock().unwrap().drop_warned,
            "a message fit again: the episode is over"
        );
    }

    #[test]
    fn the_queue_never_exceeds_the_identifier_space() {
        let l = BrokerLimits {
            max_queued_messages: 1_000_000,
            ..BrokerLimits::default()
        };
        assert_eq!(SessionLimits::new(&l).max_queued, 65_535);
    }

    #[test]
    fn messages_larger_than_the_client_accepts_are_not_sent() {
        let s = session(limits());
        // 3.1.1 has no limit; MQTT 5 client with maximum packet size 20
        let mut c = connect(&s, 1, V311, 60, 10);
        s.deliver(&msg("t", &[0; 100]), Q1);
        assert_eq!(drain(&mut c.rx).len(), 1);
        s.detach(1, 1);
        let mut c = attach(&s, 2, V5, 60, 10, Some(20));
        s.start(2);
        // the entry waiting from before is dropped for this client, a small one passes
        assert!(drain(&mut c.rx).is_empty());
        assert_eq!(s.queue_len(), 0);
        s.deliver(&msg("t", &[0; 100]), Q1);
        s.deliver(&msg("t", b"ok"), Q1);
        assert_eq!(drain(&mut c.rx), vec![publish("t", Q1, 2, false)]);
        assert_eq!(s.queue_len(), 1);
    }

    #[test]
    fn a_closed_session_accepts_nothing() {
        let s = session(limits());
        let mut c = connect(&s, 1, V311, 60, 10);
        s.deliver(&msg("t", b"x"), Q1);
        drain(&mut c.rx);
        s.destroy();
        assert!(s.is_closed());
        // the connection is told that the session is gone
        assert_eq!(c.out.kicked(), Some(KickReason::TakenOver));
        assert_eq!(s.queue_len(), 0);
        let (dropped, in_flight) = (s.dropped(), s.in_flight());
        s.deliver(&msg("t", b"x"), Q1);
        s.deliver(&msg("t", b"x"), QoS::AtMostOnce);
        // nothing was queued, counted as dropped or sent
        assert_eq!(s.queue_len(), 0);
        assert_eq!((s.dropped(), s.in_flight()), (dropped, in_flight));
        assert!(drain(&mut c.rx).is_empty());
        assert_eq!(s.inbound_begin(1, 1, None), Inbound::Stale);
        assert!(!s.start(1));
        assert_eq!(s.detach(1, 5), Detached::Stale);
        assert!(!s.is_persistent());
    }

    #[test]
    fn a_second_connection_takes_the_session_over() {
        let s = session(limits());
        let mut old = connect(&s, 1, V311, 60, 10);
        s.deliver(&msg("t", b"x"), Q1);
        drain(&mut old.rx);
        let mut new = connect(&s, 2, V311, 60, 10);
        assert_eq!(old.out.kicked(), Some(KickReason::TakenOver));
        // what the old connection had in flight goes to the new one, with DUP
        assert_eq!(drain(&mut new.rx), vec![publish("t", Q1, 1, true)]);
        // the old connection can neither acknowledge nor detach
        s.on_puback(1, 1);
        assert_eq!(s.queue_len(), 1);
        assert_eq!(s.detach(1, 9), Detached::Stale);
        assert_eq!(new.out.kicked(), None);
        s.on_puback(2, 1);
        assert_eq!(s.queue_len(), 0);
    }

    #[test]
    fn expiry_zero_destroys_the_session_at_detach_and_otherwise_it_expires_offline() {
        let s = session(limits());
        let _c = connect(&s, 1, V311, 0, 10);
        assert_eq!(s.detach(1, 5), Detached::Destroyed);
        assert!(s.is_closed());

        let s = session(limits());
        let _c = connect(&s, 1, V311, 10, 10);
        assert!(!s.is_expired(i64::MAX), "connected sessions do not expire");
        assert_eq!(s.detach(1, 1_000), Detached::Kept);
        assert!(!s.is_expired(10_999));
        assert!(s.is_expired(11_000));
    }

    #[test]
    fn disconnect_expiry_is_capped_and_raising_zero_is_refused() {
        let s = session(limits());
        let _c = connect(&s, 1, V5, 60, 10);
        assert!(s.apply_disconnect_expiry(1, 5_000, 1_000));
        assert_eq!(s.expiry_s(), 1_000);
        assert!(s.apply_disconnect_expiry(1, 7, 1_000));
        assert_eq!(s.expiry_s(), 7);
        // a stale connection does not change it
        s.apply_disconnect_expiry(2, 1, 1_000);
        assert_eq!(s.expiry_s(), 7);
        assert!(s.apply_disconnect_expiry(1, 0, 1_000));
        assert_eq!(s.detach(1, 1), Detached::Destroyed);

        // protocol error: the caller disconnects with 0x82, the session ends with the connection
        let s = session(limits());
        let _c = connect(&s, 1, V5, 0, 10);
        assert!(!s.apply_disconnect_expiry(1, 100, 1_000));
        assert_eq!(s.expiry_s(), 0);
        assert_eq!(s.detach(1, 1), Detached::Destroyed);
    }

    // --- ops for the durable copy ---

    fn stored_session(store: &Arc<RecordingStore>) -> ClientSession {
        ClientSession::new(
            1,
            "c".into(),
            Some("u".into()),
            limits(),
            Some(Arc::clone(store) as Arc<dyn SessionStore>),
        )
    }

    fn enqueue_op(seq: u64, packet_id: u16, qos: QoS, topic: &str) -> Op {
        Op::Enqueue {
            id: 1,
            entry: StoredEntry {
                seq,
                packet_id,
                qos,
                pubrec_received: false,
                origin: Origin::Client,
                topic: Arc::from(topic),
                payload: Bytes::from_static(b"x"),
                received_at_ms: 7,
            },
        }
    }

    #[test]
    fn a_durable_session_mirrors_its_state_changes_in_order() {
        let store = RecordingStore::new();
        let s = stored_session(&store);
        let (out, _rx) = SessionOut::new(1 << 20, 3);
        assert!(s.attach(
            AttachParams {
                conn_id: 1,
                out,
                version: V311,
                expiry_s: 60,
                receive_maximum: 10,
                maximum_packet_size: None,
            },
            &SubscriptionStore::new()
        ));
        assert!(s.start(1));
        let ops = store.take_ops();
        assert_eq!(ops.len(), 1);
        let Op::PutSession(snapshot) = &ops[0] else {
            panic!("expected PutSession, got {ops:?}");
        };
        assert_eq!(
            (
                snapshot.id,
                snapshot.client_id.as_str(),
                snapshot.username.as_deref()
            ),
            (1, "c", Some("u"))
        );
        assert_eq!((snapshot.expiry_s, snapshot.disconnected_at_ms), (60, None));

        assert!(s.deliver(&msg("a", b"x"), Q1));
        assert!(s.deliver(&msg("b", b"x"), Q2));
        s.on_pubrec(1, 2, 0);
        s.on_puback(1, 1);
        assert_eq!(s.inbound_begin(1, 9, None), Inbound::New);
        assert!(s.inbound_persist(9));
        assert_eq!(s.inbound_release(1, 9), (true, true));
        s.on_pubcomp(1, 2);
        assert_eq!(
            store.take_ops(),
            vec![
                enqueue_op(1, 1, Q1, "a"),
                enqueue_op(2, 2, Q2, "b"),
                Op::OutboundPubrec { id: 1, seq: 2 },
                Op::DeleteOutbound { id: 1, seq: 1 },
                Op::AddInbound {
                    id: 1,
                    packet_id: 9
                },
                Op::RemoveInbound {
                    id: 1,
                    packet_id: 9
                },
                Op::DeleteOutbound { id: 1, seq: 2 },
            ]
        );

        s.detach(1, 5_000);
        s.deliver(&msg("c", b"x"), Q1);
        assert_eq!(
            store.take_ops(),
            vec![
                Op::UpdateSession {
                    id: 1,
                    expiry_s: 60,
                    disconnected_at_ms: Some(5_000)
                },
                enqueue_op(3, 3, Q1, "c"),
            ]
        );

        // reconnecting a stored session only updates it, and the snapshot of a put holds the queue
        let _c = connect(&s, 2, V311, 120, 10);
        assert_eq!(
            store.take_ops(),
            vec![Op::UpdateSession {
                id: 1,
                expiry_s: 120,
                disconnected_at_ms: None
            }]
        );
        s.destroy();
        assert_eq!(store.take_ops(), vec![Op::DeleteSession { id: 1 }]);
        // a closed session submits nothing any more
        s.deliver(&msg("d", b"x"), Q1);
        assert!(store.take_ops().is_empty());
    }

    #[test]
    fn a_session_that_ends_with_its_connection_never_touches_the_store() {
        let store = RecordingStore::new();
        let s = stored_session(&store);
        let _c = connect(&s, 1, V311, 0, 10);
        assert!(!s.deliver(&msg("a", b"x"), Q1));
        s.on_puback(1, 1);
        s.inbound_begin(1, 3, None);
        assert!(!s.inbound_persist(3));
        s.inbound_release(1, 3);
        assert!(!s.persist_op(Op::RemoveSubscription {
            id: 1,
            filter: "f".into()
        }));
        s.detach(1, 5);
        assert!(store.ops().is_empty());
    }

    #[test]
    fn a_live_session_that_becomes_durable_is_put_with_its_state_and_one_that_stops_is_deleted() {
        let store = RecordingStore::new();
        let s = stored_session(&store);
        let _c = connect(&s, 1, V311, 0, 10);
        s.deliver(&msg("a", b"x"), Q1);
        s.inbound_begin(1, 4, None);
        assert!(store.ops().is_empty());
        let mut subs = SubscriptionStore::new();
        subs.add_subscription(1, "f/#", 10, Q2, true);
        // takeover by a connection with expiry
        let (out, _rx) = SessionOut::new(1 << 20, 3);
        assert!(s.attach(
            AttachParams {
                conn_id: 2,
                out,
                version: V311,
                expiry_s: 30,
                receive_maximum: 10,
                maximum_packet_size: None,
            },
            &subs
        ));
        let ops = store.take_ops();
        let [Op::PutSession(snapshot)] = &ops[..] else {
            panic!("{ops:?}");
        };
        assert_eq!(snapshot.subscriptions, vec![("f/#".to_owned(), Q2, true)]);
        assert_eq!(snapshot.outbound.len(), 1);
        assert_eq!(snapshot.outbound[0].topic.as_ref(), "a");
        assert_eq!(snapshot.inbound_qos2, vec![4]);
        // and back to expiry 0: the stored copy goes away
        let (out, _rx) = SessionOut::new(1 << 20, 3);
        assert!(s.attach(
            AttachParams {
                conn_id: 3,
                out,
                version: V311,
                expiry_s: 0,
                receive_maximum: 10,
                maximum_packet_size: None,
            },
            &subs
        ));
        assert_eq!(store.take_ops(), vec![Op::DeleteSession { id: 1 }]);
        assert!(s.start(3));
        s.deliver(&msg("b", b"x"), Q1);
        assert!(store.take_ops().is_empty());
    }

    #[test]
    fn restored_sessions_are_offline_and_resend_everything_with_dup() {
        let store = RecordingStore::new();
        let stored = |seq, id, qos, pubrec| StoredEntry {
            seq,
            packet_id: id,
            qos,
            pubrec_received: pubrec,
            origin: Origin::Server,
            topic: Arc::from("t"),
            payload: Bytes::from_static(b"xy"),
            received_at_ms: 3,
        };
        let snapshot = SessionSnapshot {
            id: 5,
            client_id: "c".into(),
            username: None,
            version: V311,
            expiry_s: 100,
            disconnected_at_ms: None,
            subscriptions: vec![],
            outbound: vec![
                stored(8, 2, Q2, true),
                stored(4, 1, Q1, false),
                stored(9, 3, Q1, true),
            ],
            inbound_qos2: vec![11],
        };
        let s = ClientSession::restore(
            snapshot,
            limits(),
            Some(Arc::clone(&store) as Arc<dyn SessionStore>),
            50_000,
        );
        // no disconnect time was stored: the expiry runs from the restore
        assert!(!s.is_expired(50_000 + 99_999));
        assert!(s.is_expired(50_000 + 100_000));
        // a PUBREC flag on a QoS 1 entry is meaningless and ignored
        assert_eq!(
            s.entries(),
            vec![
                (1, EntryState::Queued, true),
                (2, EntryState::PubrecReceived, true),
                (3, EntryState::Queued, true),
            ]
        );
        assert_eq!(s.inbound_ids(), [11]);
        let mut c = connect(&s, 1, V311, 100, 10);
        assert_eq!(
            drain(&mut c.rx),
            vec![
                Sent::Raw(vec![0x62, 2, 0, 2]),
                publish("t", Q1, 1, true),
                publish("t", Q1, 3, true),
            ]
        );
        // loaded sessions are in the store already: an update, not a put
        assert!(matches!(store.take_ops()[..], [Op::UpdateSession { .. }]));
        // new messages continue behind the loaded ones
        s.deliver(&msg("n", b"x"), Q1);
        assert_eq!(s.entries().last().unwrap().0, 4);
        assert!(matches!(&store.take_ops()[..], [Op::Enqueue { entry, .. }] if entry.seq == 10));
    }
}
