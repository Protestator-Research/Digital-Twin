//! Interface between the broker and a durable copy of its client sessions.
//!
//! The broker keeps all session state in memory and mirrors the part that has to survive a restart through a
//! [`SessionStore`]: every change is submitted as an [`Op`] while the broker holds the lock that protects the
//! state the op describes, so the order of the ops is the order of the state changes. Ops are applied
//! write-behind. Where the broker promises durability to a client (CONNACK, SUBACK, PUBACK, ...), it waits
//! for a [`SessionStore::barrier`] submitted after the ops, and sends the packet from its closure.
//!
//! Only sessions that outlive their connection produce ops (`expiry > 0`), see `ClientSession`.

use std::collections::HashMap;
use std::fmt;
use std::str::FromStr;
use std::sync::atomic::{AtomicBool, AtomicU64, AtomicUsize, Ordering};
use std::sync::{Arc, Mutex};
use std::time::Duration;

use bytes::Bytes;
use sqlx::postgres::{PgConnectOptions, PgConnection, PgPool, PgPoolOptions};
use sqlx::{Connection as _, Row as _};
use thiserror::Error;
use tokio::sync::{mpsc, oneshot, watch};
use tokio::task::JoinHandle;
use tracing::{error, info, warn};

use super::limits::BrokerLimits;
use super::protocol::{ProtocolVersion, QoS};
use super::subscriptions::SessionId;
use crate::hub::Origin;

/// One change of the durable copy.
#[derive(Debug, Clone, PartialEq, Eq)]
pub(crate) enum Op {
    /// Full state of a session: replaces whatever is stored for this id / client id.
    PutSession(SessionSnapshot),
    /// New expiry and (dis)connect time of a stored session.
    UpdateSession {
        id: SessionId,
        expiry_s: u32,
        /// When the connection ended (Unix milliseconds); `None` while a client is connected.
        disconnected_at_ms: Option<i64>,
    },
    /// The session ended (expired, replaced by a clean start, or its expiry became 0).
    DeleteSession {
        id: SessionId,
    },
    SetSubscription {
        id: SessionId,
        filter: String,
        qos: QoS,
        no_local: bool,
    },
    RemoveSubscription {
        id: SessionId,
        filter: String,
    },
    /// A message was queued for the session (QoS 1 or 2).
    Enqueue {
        id: SessionId,
        entry: StoredEntry,
    },
    /// The subscriber acknowledged the QoS 2 PUBLISH with PUBREC; the entry is now in the PUBREL state.
    OutboundPubrec {
        id: SessionId,
        seq: u64,
    },
    /// The subscriber finished the entry (PUBACK, PUBCOMP, or a PUBREC with an error).
    DeleteOutbound {
        id: SessionId,
        seq: u64,
    },
    /// The session received a QoS 2 PUBLISH with this packet identifier and waits for the PUBREL.
    AddInbound {
        id: SessionId,
        packet_id: u16,
    },
    RemoveInbound {
        id: SessionId,
        packet_id: u16,
    },
}

/// A message in the outbound queue of a session.
#[derive(Debug, Clone, PartialEq, Eq)]
pub(crate) struct StoredEntry {
    /// Position in the queue of the session, strictly increasing.
    pub(crate) seq: u64,
    pub(crate) packet_id: u16,
    /// QoS the message is delivered with (1 or 2).
    pub(crate) qos: QoS,
    /// QoS 2 only: PUBREC arrived, the PUBLISH must not be sent again.
    pub(crate) pubrec_received: bool,
    pub(crate) origin: Origin,
    pub(crate) topic: Arc<str>,
    pub(crate) payload: Bytes,
    pub(crate) received_at_ms: i64,
}

/// Everything the broker needs to bring a session back after a restart.
#[derive(Debug, Clone, PartialEq, Eq)]
pub(crate) struct SessionSnapshot {
    pub(crate) id: SessionId,
    pub(crate) client_id: String,
    /// User name that created the session (only meaningful with authentication).
    pub(crate) username: Option<String>,
    pub(crate) version: ProtocolVersion,
    pub(crate) expiry_s: u32,
    pub(crate) disconnected_at_ms: Option<i64>,
    /// (filter, granted QoS, No Local)
    pub(crate) subscriptions: Vec<(String, QoS, bool)>,
    /// The outbound queue in order.
    pub(crate) outbound: Vec<StoredEntry>,
    /// Packet identifiers of QoS 2 PUBLISH packets received and not yet released.
    pub(crate) inbound_qos2: Vec<u16>,
}

/// Durable mirror of the session state. Both methods never block. Operations and barriers are applied in the
/// order they were submitted; a barrier's closure runs once everything submitted before it is durable, and
/// the closures of several barriers run in the order of their submission (the broker relies on this to keep
/// acknowledgements in order). An implementation must run every barrier closure; it may only drop them unrun when the broker shuts
/// down (a closure dropped otherwise makes the broker refuse the CONNECT that waits for it, while an
/// acknowledgement waiting for it is lost). A closure must be short and must not call back into the broker's locks.
pub(crate) trait SessionStore: Send + Sync {
    fn submit(&self, op: Op);
    fn barrier(&self, done: Box<dyn FnOnce() + Send>);
}

// ---------------------------------------------------------------------------------------------
// PostgreSQL
// ---------------------------------------------------------------------------------------------

/// Where the sessions are stored (`run.mqtt.session_database_url`).
#[derive(Clone, PartialEq, Eq)]
pub struct SessionDatabase {
    /// PostgreSQL connection URL. May contain a password: never logged, never part of an error.
    pub url: String,
    /// Schema the tables live in, set as the `search_path` of every connection (the tests use one schema per
    /// test, like `api::connect_lazy`). `None`: the default of the database role.
    pub schema: Option<String>,
}

impl fmt::Debug for SessionDatabase {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("SessionDatabase")
            .field(
                "url",
                &if self.url.is_empty() {
                    ""
                } else {
                    "<redacted>"
                },
            )
            .field("schema", &self.schema)
            .finish()
    }
}

/// Why the session store could not be opened. The messages never contain the URL, the password or the
/// message of the server (which can name the user): only the kind of failure and the SQLSTATE.
#[derive(Debug, Error)]
pub(crate) enum SessionStoreError {
    #[error("the session database URL is not a valid PostgreSQL URL")]
    InvalidUrl,
    #[error("the session database did not answer within {} seconds", CONNECT_TIMEOUT.as_secs())]
    Timeout,
    #[error("the session database cannot be reached ({0})")]
    Unreachable(String),
    #[error("the session database refused the request ({0})")]
    Database(String),
    #[error(
        "the MQTT session tables are missing and the database role may not create them; apply \
         sql/0001_mqtt_session.sql"
    )]
    MissingTables,
    #[error("another broker uses this MQTT session database (schema)")]
    InUse,
}

impl SessionStoreError {
    fn from_sqlx(error: &sqlx::Error) -> Self {
        match error {
            sqlx::Error::Database(_) => Self::Database(describe(error, false)),
            _ => Self::Unreachable(describe(error, false)),
        }
    }
}

/// The statements, in one place. Table names are unqualified: the schema is whatever the `search_path` of the
/// connection selects. Every statement that touches the children of a session is safe when the session row is
/// gone (ops for a destroyed session can still arrive): inserts are guarded by `WHERE EXISTS`, updates and
/// deletes just match nothing, so no foreign key violation can abort a batch.
pub(crate) mod sql {
    /// The schema, applied at every start (`CREATE TABLE IF NOT EXISTS`).
    pub(crate) const SCHEMA: &str = include_str!("../../sql/0001_mqtt_session.sql");

    /// Do the four tables exist (in the schema of the `search_path`)? PostgreSQL checks the CREATE privilege
    /// before it looks at `IF NOT EXISTS`, so the DDL only runs when something is missing: a role that may only
    /// read and write the tables can then start the broker.
    pub(crate) const TABLES_EXIST: &str = "\
SELECT to_regclass('mqtt_session') IS NOT NULL AND to_regclass('mqtt_subscription') IS NOT NULL \
AND to_regclass('mqtt_outbound') IS NOT NULL AND to_regclass('mqtt_inbound_qos2') IS NOT NULL";

    /// One broker per schema: a session-level advisory lock on a key derived from a fixed seed and the schema,
    /// taken on a connection that lives as long as the store. `true`: taken.
    pub(crate) const BROKER_LOCK: &str =
        "SELECT pg_try_advisory_lock(hashtextextended(current_schema(), 7364502))";

    /// Serializes the schema creation of brokers that start at the same moment (`CREATE TABLE IF NOT
    /// EXISTS` can fail with a duplicate key in the catalog when two sessions run it concurrently).
    pub(crate) const SCHEMA_LOCK: &str = "SELECT pg_advisory_xact_lock(7364501)";

    /// `$1` now, `$2` the longest expiry in seconds (the limit may have been lowered since the session was stored).
    pub(crate) const DELETE_EXPIRED: &str = "\
DELETE FROM mqtt_session \
WHERE disconnected_at_ms IS NOT NULL AND disconnected_at_ms <= $1::bigint - LEAST(expiry_s, $2::bigint) * 1000";

    /// Sessions that were connected when the broker stopped or crashed start their expiry now.
    pub(crate) const STAMP_DISCONNECTED: &str =
        "UPDATE mqtt_session SET disconnected_at_ms = $1 WHERE disconnected_at_ms IS NULL";

    pub(crate) const LOAD_SESSIONS: &str = "\
SELECT session_id, client_id, username, protocol, expiry_s, disconnected_at_ms FROM mqtt_session ORDER BY session_id";

    pub(crate) const LOAD_SUBSCRIPTIONS: &str = "\
SELECT session_id, filter, qos, no_local FROM mqtt_subscription ORDER BY session_id, filter";

    pub(crate) const LOAD_OUTBOUND: &str = "\
SELECT session_id, seq, packet_id, qos, pubrec_received, origin, topic, payload, received_at_ms \
FROM mqtt_outbound ORDER BY session_id, seq";

    pub(crate) const LOAD_INBOUND: &str =
        "SELECT session_id, packet_id FROM mqtt_inbound_qos2 ORDER BY session_id, packet_id";

    /// `PutSession`: whatever is stored for this session id or this client identifier goes (the children
    /// with it, by cascade). `$1` session id, `$2` client id.
    pub(crate) const DELETE_FOR_PUT: &str =
        "DELETE FROM mqtt_session WHERE session_id = $1 OR client_id = $2";

    /// `$1` id, `$2` client id, `$3` user name, `$4` protocol, `$5` expiry, `$6` disconnect time.
    pub(crate) const INSERT_SESSION: &str = "\
INSERT INTO mqtt_session (session_id, client_id, username, protocol, expiry_s, disconnected_at_ms) \
VALUES ($1, $2, $3, $4, $5, $6)";

    /// `$1` id, `$2` expiry, `$3` disconnect time.
    pub(crate) const UPDATE_SESSION: &str =
        "UPDATE mqtt_session SET expiry_s = $2, disconnected_at_ms = $3 WHERE session_id = $1";

    pub(crate) const DELETE_SESSION: &str = "DELETE FROM mqtt_session WHERE session_id = $1";

    /// `$1` id, `$2` filter, `$3` QoS, `$4` No Local.
    pub(crate) const SET_SUBSCRIPTION: &str = "\
INSERT INTO mqtt_subscription (session_id, filter, qos, no_local) \
SELECT $1::bigint, $2::text, $3::smallint, $4::boolean \
WHERE EXISTS (SELECT 1 FROM mqtt_session WHERE session_id = $1) \
ON CONFLICT (session_id, filter) DO UPDATE SET qos = EXCLUDED.qos, no_local = EXCLUDED.no_local";

    pub(crate) const REMOVE_SUBSCRIPTION: &str =
        "DELETE FROM mqtt_subscription WHERE session_id = $1 AND filter = $2";

    /// `$1` id, `$2` seq, `$3` packet id, `$4` QoS, `$5` PUBREC received, `$6` origin, `$7` topic,
    /// `$8` payload, `$9` receive time.
    pub(crate) const ENQUEUE: &str = "\
INSERT INTO mqtt_outbound (session_id, seq, packet_id, qos, pubrec_received, origin, topic, payload, received_at_ms) \
SELECT $1::bigint, $2::bigint, $3::integer, $4::smallint, $5::boolean, $6::smallint, $7::text, $8::bytea, $9::bigint \
WHERE EXISTS (SELECT 1 FROM mqtt_session WHERE session_id = $1) \
ON CONFLICT (session_id, seq) DO UPDATE SET packet_id = EXCLUDED.packet_id, qos = EXCLUDED.qos, \
pubrec_received = EXCLUDED.pubrec_received, origin = EXCLUDED.origin, topic = EXCLUDED.topic, \
payload = EXCLUDED.payload, received_at_ms = EXCLUDED.received_at_ms";

    pub(crate) const OUTBOUND_PUBREC: &str =
        "UPDATE mqtt_outbound SET pubrec_received = true WHERE session_id = $1 AND seq = $2";

    pub(crate) const DELETE_OUTBOUND: &str =
        "DELETE FROM mqtt_outbound WHERE session_id = $1 AND seq = $2";

    pub(crate) const ADD_INBOUND: &str = "\
INSERT INTO mqtt_inbound_qos2 (session_id, packet_id) \
SELECT $1::bigint, $2::integer \
WHERE EXISTS (SELECT 1 FROM mqtt_session WHERE session_id = $1) \
ON CONFLICT (session_id, packet_id) DO NOTHING";

    pub(crate) const REMOVE_INBOUND: &str =
        "DELETE FROM mqtt_inbound_qos2 WHERE session_id = $1 AND packet_id = $2";

    /// Every statement with the number of its parameters (for the tests).
    #[cfg(test)]
    pub(crate) const ALL: &[(&str, usize)] = &[
        (TABLES_EXIST, 0),
        (BROKER_LOCK, 0),
        (SCHEMA_LOCK, 0),
        (DELETE_EXPIRED, 2),
        (STAMP_DISCONNECTED, 1),
        (LOAD_SESSIONS, 0),
        (LOAD_SUBSCRIPTIONS, 0),
        (LOAD_OUTBOUND, 0),
        (LOAD_INBOUND, 0),
        (DELETE_FOR_PUT, 2),
        (INSERT_SESSION, 6),
        (UPDATE_SESSION, 3),
        (DELETE_SESSION, 1),
        (SET_SUBSCRIPTION, 4),
        (REMOVE_SUBSCRIPTION, 2),
        (ENQUEUE, 9),
        (OUTBOUND_PUBREC, 2),
        (DELETE_OUTBOUND, 2),
        (ADD_INBOUND, 2),
        (REMOVE_INBOUND, 2),
    ];
}

/// Overall time `PgSessionStore::connect` may take: an unreachable database is an error at the start, not a
/// retry loop (the operator sees it at once).
const CONNECT_TIMEOUT: Duration = Duration::from_secs(10);
/// Time the writer waits for a free connection.
const ACQUIRE_TIMEOUT: Duration = Duration::from_secs(5);
/// A statement that runs longer is cancelled by the server and retried like a lost connection, so a database
/// that hangs does not block the writer for ever.
const STATEMENT_TIMEOUT_MS: &str = "30000";
/// The writer needs one connection; the second one lets it reconnect while a broken one is being discarded.
const POOL_SIZE: u32 = 2;
/// Most items (ops and barriers) that share one transaction.
const BATCH_SIZE: usize = 500;
/// Wait before the first retry of a batch, doubled up to [`BACKOFF_MAX`].
const BACKOFF_START: Duration = Duration::from_millis(200);
const BACKOFF_MAX: Duration = Duration::from_secs(5);
/// Queued items from which the writer stops accepting [`Op::Enqueue`].
const MAX_BACKLOG_ITEMS: usize = 100_000;
/// Topic and payload bytes of the queued [`Op::Enqueue`] ops from which the writer stops accepting them: a
/// message is up to `max_packet_size` large, so the number of items alone does not bound the memory.
const MAX_BACKLOG_BYTES: usize = 64 * 1024 * 1024;
/// How often the connection that holds the broker lock is checked.
const LOCK_CHECK_INTERVAL: Duration = Duration::from_secs(30);
/// A stall of the writer is logged again after this long.
const STALL_LOG_INTERVAL: Duration = Duration::from_secs(60);
/// Time `shutdown` waits for the writer to end after the flush.
const STOP_GRACE: Duration = Duration::from_secs(2);

// --- encodings -------------------------------------------------------------------------------

/// `mqtt_session.protocol`: the protocol level of the CONNECT.
fn protocol_to_db(version: ProtocolVersion) -> i16 {
    match version {
        ProtocolVersion::V311 => 4,
        ProtocolVersion::V5 => 5,
    }
}

fn protocol_from_db(value: i16) -> Option<ProtocolVersion> {
    match value {
        4 => Some(ProtocolVersion::V311),
        5 => Some(ProtocolVersion::V5),
        _ => None,
    }
}

fn qos_to_db(qos: QoS) -> i16 {
    i16::from(qos.bits())
}

fn qos_from_db(value: i16) -> Option<QoS> {
    u8::try_from(value).ok().and_then(QoS::from_bits)
}

/// `mqtt_outbound.origin`: 0 = a client, 1 = the server.
fn origin_to_db(origin: Origin) -> i16 {
    match origin {
        Origin::Client => 0,
        Origin::Server => 1,
    }
}

fn origin_from_db(value: i16) -> Option<Origin> {
    match value {
        0 => Some(Origin::Client),
        1 => Some(Origin::Server),
        _ => None,
    }
}

/// Session ids start at 1 and count up, so they fit a `BIGINT` for any realistic lifetime.
fn id_to_db(id: SessionId) -> i64 {
    id as i64
}

fn id_from_db(value: i64) -> Option<SessionId> {
    SessionId::try_from(value).ok()
}

/// A packet identifier is 1 to 65535.
fn packet_id_from_db(value: i32) -> Option<u16> {
    u16::try_from(value).ok().filter(|id| *id != 0)
}

// --- rows to snapshots -----------------------------------------------------------------------

fn session_from_db(
    id: i64,
    client_id: String,
    username: Option<String>,
    protocol: i16,
    expiry_s: i64,
    disconnected_at_ms: Option<i64>,
) -> Option<SessionSnapshot> {
    Some(SessionSnapshot {
        id: id_from_db(id)?,
        client_id,
        username,
        version: protocol_from_db(protocol)?,
        expiry_s: u32::try_from(expiry_s).ok()?,
        disconnected_at_ms,
        subscriptions: Vec::new(),
        outbound: Vec::new(),
        inbound_qos2: Vec::new(),
    })
}

fn subscription_from_db(filter: String, qos: i16, no_local: bool) -> Option<(String, QoS, bool)> {
    Some((filter, qos_from_db(qos)?, no_local))
}

#[allow(clippy::too_many_arguments)]
fn entry_from_db(
    seq: i64,
    packet_id: i32,
    qos: i16,
    pubrec_received: bool,
    origin: i16,
    topic: String,
    payload: Vec<u8>,
    received_at_ms: i64,
) -> Option<StoredEntry> {
    // only QoS 1 and 2 are queued
    let qos = qos_from_db(qos).filter(|q| *q != QoS::AtMostOnce)?;
    Some(StoredEntry {
        seq: u64::try_from(seq).ok()?,
        packet_id: packet_id_from_db(packet_id)?,
        qos,
        pubrec_received,
        origin: origin_from_db(origin)?,
        topic: Arc::from(topic),
        payload: Bytes::from(payload),
        received_at_ms,
    })
}

// --- errors ----------------------------------------------------------------------------------

/// A description for the log that contains nothing secret. `detail` adds the message of the server, which is
/// only safe once the connection is authenticated (an authentication failure names the user).
fn describe(error: &sqlx::Error, detail: bool) -> String {
    match error {
        sqlx::Error::Io(e) => format!("I/O error: {}", e.kind()),
        sqlx::Error::Tls(_) => "TLS error".to_owned(),
        sqlx::Error::PoolTimedOut => "no connection became available in time".to_owned(),
        sqlx::Error::PoolClosed => "the connection pool is closed".to_owned(),
        sqlx::Error::WorkerCrashed => "the connection worker crashed".to_owned(),
        sqlx::Error::Protocol(_) => "protocol error".to_owned(),
        sqlx::Error::Configuration(_) => "invalid connection configuration".to_owned(),
        sqlx::Error::Database(db) => {
            let code = db.code().map_or_else(|| "?".to_owned(), |c| c.into_owned());
            // 08 (connection) and 28 (authorization) messages name the user or the host
            if detail && !code.starts_with("08") && !code.starts_with("28") {
                format!("SQLSTATE {code}: {}", db.message())
            } else {
                format!("SQLSTATE {code}")
            }
        }
        _ => "unexpected driver error".to_owned(),
    }
}

/// Errors that are about the data of one change and nothing else: SQLSTATE class 22 (data exception, e.g. a
/// NUL in a text), 23 (integrity constraint) and 54 (program limit exceeded), and a value the driver cannot
/// encode. They would fail again and again, so the change is skipped (and logged) once the batch was split into
/// single changes. EVERYTHING else (a lost connection, a missing table or privilege, a read-only server, a
/// failed BEGIN, an internal error of the server, ...) is retried with backoff: a visible stall is better than
/// acknowledging a state that is never written.
fn is_data_error(error: &sqlx::Error) -> bool {
    match error {
        sqlx::Error::Database(db) => db.code().is_some_and(|code| is_data_sqlstate(&code)),
        sqlx::Error::Encode(_) => true,
        _ => false,
    }
}

fn is_data_sqlstate(code: &str) -> bool {
    ["22", "23", "54"]
        .iter()
        .any(|class| code.starts_with(class))
}

// --- backlog ---------------------------------------------------------------------------------

/// What is queued for the writer, and which ops are given up when the database stays away for long.
///
/// The queue is unbounded because `submit` must never block, so memory is bounded here instead. Only
/// [`Op::Enqueue`] is ever dropped, because it is the only op that carries a payload (the others are a few
/// dozen bytes, and a session cannot produce more of them than the broker's limits allow) and because losing
/// it leaves a state that is at least consistent: the stored queue has a gap, and the ops that follow for the
/// lost message (`OutboundPubrec`, `DeleteOutbound`) find no row and do nothing.
///
/// It is NOT harmless, though. The publisher's acknowledgement waits for a barrier, and a barrier does not
/// know that an `Enqueue` before it was dropped. So while the database is behind, QoS 1 and 2 messages that were
/// acknowledged to their publisher can be missing for an offline or reconnecting session after a broker
/// restart. And because the client keeps its own session state, a QoS 2 flow that was already past PUBREC can
/// be left half-open on the client (it waits for a PUBLISH or PUBREL that the restarted broker does not know
/// about). This is the price of not blocking the broker; the drop is logged as a warning.
///
/// Every other op would leave something wrong behind:
/// - a lost `DeleteOutbound` brings an acknowledged message back after a restart, twice for QoS 2 (the
///   exactly-once promise breaks), and the broker may have reused its packet identifier for a new message;
/// - a lost `OutboundPubrec` makes the PUBLISH of a QoS 2 message go out again after a restart;
/// - a lost `UpdateSession` leaves the old expiry or, worse, "connected" for a session that has gone offline
///   or the other way round, so a session can expire early;
/// - a lost subscription op, or `PutSession` / `DeleteSession`, changes who receives what, or leaves a session
///   (and its messages) that the broker no longer has;
/// - a lost inbound op breaks the QoS 2 deduplication.
///
/// Barriers are never dropped: the acknowledgements wait for them.
#[derive(Debug)]
pub(crate) struct Backlog {
    items: AtomicUsize,
    enqueue_bytes: AtomicUsize,
    dropped_total: AtomicU64,
    dropped_episode: AtomicU64,
    /// A drop was reported and the backlog has not shrunk since.
    warned: AtomicBool,
    max_items: usize,
    max_bytes: usize,
}

impl Backlog {
    pub(crate) fn new(max_items: usize, max_bytes: usize) -> Self {
        Self {
            items: AtomicUsize::new(0),
            enqueue_bytes: AtomicUsize::new(0),
            dropped_total: AtomicU64::new(0),
            dropped_episode: AtomicU64::new(0),
            warned: AtomicBool::new(false),
            max_items,
            max_bytes,
        }
    }

    /// Topic and payload bytes of an `Enqueue`; 0 for every other op.
    fn weight(op: &Op) -> usize {
        match op {
            Op::Enqueue { entry, .. } => entry.topic.len() + entry.payload.len(),
            _ => 0,
        }
    }

    /// Counts the op as queued. False: the op has to be dropped (see the type's documentation).
    pub(crate) fn admit(&self, op: &Op) -> bool {
        let weight = Self::weight(op);
        if matches!(op, Op::Enqueue { .. })
            && (self.items.load(Ordering::Relaxed) >= self.max_items
                || self
                    .enqueue_bytes
                    .load(Ordering::Relaxed)
                    .saturating_add(weight)
                    > self.max_bytes)
        {
            self.dropped_total.fetch_add(1, Ordering::Relaxed);
            self.dropped_episode.fetch_add(1, Ordering::Relaxed);
            if !self.warned.swap(true, Ordering::Relaxed) {
                warn!(
                    queued = self.items.load(Ordering::Relaxed),
                    "the session database is too far behind: queued messages of offline sessions are no longer \
                     stored (they are lost if the broker restarts before the database is back)"
                );
            }
            return false;
        }
        self.items.fetch_add(1, Ordering::Relaxed);
        self.enqueue_bytes.fetch_add(weight, Ordering::Relaxed);
        true
    }

    /// Counts a barrier as queued.
    fn admit_barrier(&self) {
        self.items.fetch_add(1, Ordering::Relaxed);
    }

    /// The item was not queued after all (the writer is gone).
    fn withdraw(&self, items: usize, bytes: usize) {
        self.items.fetch_sub(items, Ordering::Relaxed);
        self.enqueue_bytes.fetch_sub(bytes, Ordering::Relaxed);
    }

    /// The writer took items out of the queue. Ends the episode of drops once the backlog is half empty.
    fn release(&self, items: usize, bytes: usize) {
        self.withdraw(items, bytes);
        if self.warned.load(Ordering::Relaxed)
            && self.items.load(Ordering::Relaxed) <= self.max_items / 2
            && self.enqueue_bytes.load(Ordering::Relaxed) <= self.max_bytes / 2
            && self.warned.swap(false, Ordering::Relaxed)
        {
            // not inside the macro: its arguments are not evaluated when the level is off
            let dropped = self.dropped_episode.swap(0, Ordering::Relaxed);
            info!(
                dropped,
                "the session database caught up: queued messages are stored again"
            );
        }
    }

    pub(crate) fn queued(&self) -> usize {
        self.items.load(Ordering::Relaxed)
    }

    #[cfg(test)]
    pub(crate) fn dropped(&self) -> u64 {
        self.dropped_total.load(Ordering::Relaxed)
    }
}

// --- the store -------------------------------------------------------------------------------

type Done = Box<dyn FnOnce() + Send>;

enum Item {
    Op(Op),
    Barrier(Done),
}

/// The sessions in PostgreSQL. `submit` and `barrier` only queue; a task applies the queue in transactions
/// (group commit).
pub(crate) struct PgSessionStore {
    tx: mpsc::UnboundedSender<Item>,
    backlog: Arc<Backlog>,
    /// After `shutdown`: nothing is accepted any more.
    closed: AtomicBool,
    abort: watch::Sender<bool>,
    task: Mutex<Option<JoinHandle<()>>>,
    /// Holds the connection with the advisory lock of the schema (see [`sql::BROKER_LOCK`]).
    lock_task: Mutex<Option<JoinHandle<()>>>,
    pool: PgPool,
}

fn connect_options(db: &SessionDatabase) -> Result<PgConnectOptions, SessionStoreError> {
    // the parse error can echo parts of the URL, so it is not kept
    let options = PgConnectOptions::from_str(&db.url).map_err(|_| SessionStoreError::InvalidUrl)?;
    let options = options
        .application_name("dt-server mqtt sessions")
        .options([("statement_timeout", STATEMENT_TIMEOUT_MS)]);
    Ok(match &db.schema {
        Some(schema) => options.options([("search_path", schema.as_str())]),
        None => options,
    })
}

impl PgSessionStore {
    /// Connects, prepares the database and returns the store with the sessions it holds.
    ///
    /// An unreachable database is an error (after at most 10 seconds), not a retry loop. The schema is
    /// created if it is missing. Sessions whose expiry has passed (`now_ms` against their disconnect time,
    /// with the expiry capped at `limits.session_expiry_max_s`, as the broker does) are deleted. Sessions that
    /// were still connected when the broker stopped or crashed start their expiry at `now_ms`.
    pub(crate) async fn connect(
        db: &SessionDatabase,
        now_ms: i64,
        limits: &BrokerLimits,
    ) -> Result<(Arc<PgSessionStore>, Vec<SessionSnapshot>), SessionStoreError> {
        let options = connect_options(db)?;
        let max_expiry_s = i64::from(limits.session_expiry_max_s);
        let (abort, abort_rx) = watch::channel(false);
        // Only connecting and preparing are bounded: loading a large backlog must not make the broker
        // unstartable.
        let setup = async {
            let pool = PgPoolOptions::new()
                .max_connections(POOL_SIZE)
                .acquire_timeout(ACQUIRE_TIMEOUT)
                .connect_with(options.clone())
                .await
                .map_err(|e| SessionStoreError::from_sqlx(&e))?;
            let lock = match take_broker_lock(&options).await {
                Ok(lock) => lock,
                Err(e) => {
                    pool.close().await;
                    return Err(e);
                }
            };
            match prepare(&pool, now_ms, max_expiry_s).await {
                Ok(()) => Ok((pool, lock)),
                Err(e) => {
                    pool.close().await;
                    lock.close().await.ok();
                    Err(e)
                }
            }
        };
        let (pool, lock) = tokio::time::timeout(CONNECT_TIMEOUT, setup)
            .await
            .map_err(|_| SessionStoreError::Timeout)??;
        let lock_task = tokio::spawn(hold_lock(lock, abort_rx.clone()));
        let snapshots = match load(&pool).await {
            Ok(snapshots) => snapshots,
            Err(e) => {
                let _ = abort.send(true);
                let _ = tokio::time::timeout(STOP_GRACE, lock_task).await;
                pool.close().await;
                return Err(SessionStoreError::from_sqlx(&e));
            }
        };

        let (tx, rx) = mpsc::unbounded_channel();
        let backlog = Arc::new(Backlog::new(MAX_BACKLOG_ITEMS, MAX_BACKLOG_BYTES));
        let writer = Writer {
            pool: pool.clone(),
            rx,
            backlog: Arc::clone(&backlog),
            abort: abort_rx,
            failing: false,
            last_warned: None,
        };
        let task = tokio::spawn(writer.run());
        let store = Arc::new(PgSessionStore {
            tx,
            backlog,
            closed: AtomicBool::new(false),
            abort,
            task: Mutex::new(Some(task)),
            lock_task: Mutex::new(Some(lock_task)),
            pool,
        });
        Ok((store, snapshots))
    }

    /// Items waiting for the writer.
    #[cfg(test)]
    pub(crate) fn queued(&self) -> usize {
        self.backlog.queued()
    }

    /// Everything submitted so far is stored (or `timeout` has passed), then the writer ends and the
    /// connections are closed. Afterwards `submit` and `barrier` do nothing; a barrier submitted later is
    /// dropped unrun.
    pub(crate) async fn shutdown(&self, timeout: Duration) {
        let (done_tx, done_rx) = oneshot::channel();
        self.barrier(Box::new(move || {
            let _ = done_tx.send(());
        }));
        match tokio::time::timeout(timeout, done_rx).await {
            Ok(_) => {}
            Err(_) => warn!(
                queued = self.backlog.queued(),
                "the session database did not take all changes within {} seconds: they are lost",
                timeout.as_secs()
            ),
        }
        self.closed.store(true, Ordering::SeqCst);
        let _ = self.abort.send(true);
        let task = self.task.lock().unwrap().take();
        if let Some(mut task) = task {
            if tokio::time::timeout(STOP_GRACE, &mut task).await.is_err() {
                task.abort();
            }
        }
        // closing the lock connection releases the lock
        let lock_task = self.lock_task.lock().unwrap().take();
        if let Some(mut task) = lock_task {
            if tokio::time::timeout(STOP_GRACE, &mut task).await.is_err() {
                task.abort();
            }
        }
        let _ = tokio::time::timeout(STOP_GRACE, self.pool.close()).await;
    }
}

impl SessionStore for PgSessionStore {
    fn submit(&self, op: Op) {
        if self.closed.load(Ordering::SeqCst) || !self.backlog.admit(&op) {
            return;
        }
        let weight = Backlog::weight(&op);
        if self.tx.send(Item::Op(op)).is_err() {
            self.backlog.withdraw(1, weight);
        }
    }

    fn barrier(&self, done: Box<dyn FnOnce() + Send>) {
        if self.closed.load(Ordering::SeqCst) {
            return;
        }
        self.backlog.admit_barrier();
        if self.tx.send(Item::Barrier(done)).is_err() {
            self.backlog.withdraw(1, 0);
        }
    }
}

/// Takes the lock that makes this broker the only one of the schema. The connection is not part of the pool
/// and is kept for the life of the store.
async fn take_broker_lock(options: &PgConnectOptions) -> Result<PgConnection, SessionStoreError> {
    let mut conn = PgConnection::connect_with(options)
        .await
        .map_err(|e| SessionStoreError::from_sqlx(&e))?;
    match sqlx::query_scalar::<_, bool>(sql::BROKER_LOCK)
        .fetch_one(&mut conn)
        .await
    {
        Ok(true) => Ok(conn),
        Ok(false) => {
            let _ = conn.close().await;
            Err(SessionStoreError::InUse)
        }
        Err(e) => Err(SessionStoreError::from_sqlx(&e)),
    }
}

/// Keeps the lock connection open until the store stops (or is dropped). A lost connection means the lock is
/// gone: that is said once, and the broker runs on.
async fn hold_lock(mut conn: PgConnection, mut stop: watch::Receiver<bool>) {
    loop {
        tokio::select! {
            // a signal or a dropped store: both end the task
            _ = stop.changed() => break,
            () = tokio::time::sleep(LOCK_CHECK_INTERVAL) => {
                if conn.ping().await.is_err() {
                    warn!(
                        "the connection that holds the exclusive lock of the MQTT session database was lost: \
                         the lock is gone, so a second broker could use the same sessions now"
                    );
                    return;
                }
            }
        }
    }
    let _ = conn.close().await;
}

async fn tables_exist(conn: &mut PgConnection) -> Result<bool, sqlx::Error> {
    sqlx::query_scalar::<_, bool>(sql::TABLES_EXIST)
        .fetch_one(conn)
        .await
}

/// Creates the tables if one is missing, expires and stamps the sessions.
async fn prepare(pool: &PgPool, now_ms: i64, max_expiry_s: i64) -> Result<(), SessionStoreError> {
    let db = |e: sqlx::Error| SessionStoreError::from_sqlx(&e);
    let mut tx = pool.begin().await.map_err(db)?;
    if !tables_exist(&mut tx).await.map_err(db)? {
        // serialized, and checked again: a tool that applies the schema by hand may have created them meanwhile
        sqlx::query(sql::SCHEMA_LOCK)
            .execute(&mut *tx)
            .await
            .map_err(db)?;
        if !tables_exist(&mut tx).await.map_err(db)? {
            if let Err(e) = sqlx::raw_sql(sql::SCHEMA).execute(&mut *tx).await {
                return Err(match &e {
                    sqlx::Error::Database(d) if d.code().as_deref() == Some("42501") => {
                        SessionStoreError::MissingTables
                    }
                    _ => db(e),
                });
            }
        }
    }
    let expired = sqlx::query(sql::DELETE_EXPIRED)
        .bind(now_ms)
        .bind(max_expiry_s)
        .execute(&mut *tx)
        .await
        .map_err(db)?
        .rows_affected();
    sqlx::query(sql::STAMP_DISCONNECTED)
        .bind(now_ms)
        .execute(&mut *tx)
        .await
        .map_err(db)?;
    tx.commit().await.map_err(db)?;
    if expired > 0 {
        info!(
            sessions = expired,
            "expired MQTT sessions removed from the database"
        );
    }
    Ok(())
}

/// Reads all sessions with their subscriptions, outbound entries and inbound identifiers. A row that cannot be
/// understood (an unknown protocol, QoS or origin, an identifier out of range) is skipped with a warning; it
/// must not keep the broker from starting.
async fn load(pool: &PgPool) -> Result<Vec<SessionSnapshot>, sqlx::Error> {
    let mut sessions: Vec<SessionSnapshot> = Vec::new();
    let mut index: HashMap<i64, usize> = HashMap::new();
    for row in sqlx::query(sql::LOAD_SESSIONS).fetch_all(pool).await? {
        let id: i64 = row.try_get("session_id")?;
        let snapshot = session_from_db(
            id,
            row.try_get("client_id")?,
            row.try_get("username")?,
            row.try_get("protocol")?,
            row.try_get("expiry_s")?,
            row.try_get("disconnected_at_ms")?,
        );
        match snapshot {
            Some(snapshot) => {
                index.insert(id, sessions.len());
                sessions.push(snapshot);
            }
            None => warn!(
                session = id,
                "stored MQTT session skipped: it has an unknown protocol or an invalid value"
            ),
        }
    }
    for row in sqlx::query(sql::LOAD_SUBSCRIPTIONS).fetch_all(pool).await? {
        let id: i64 = row.try_get("session_id")?;
        let Some(&at) = index.get(&id) else { continue };
        let filter: String = row.try_get("filter")?;
        match subscription_from_db(filter, row.try_get("qos")?, row.try_get("no_local")?) {
            Some(subscription) => sessions[at].subscriptions.push(subscription),
            None => warn!(session = id, "stored subscription skipped: invalid QoS"),
        }
    }
    for row in sqlx::query(sql::LOAD_OUTBOUND).fetch_all(pool).await? {
        let id: i64 = row.try_get("session_id")?;
        let Some(&at) = index.get(&id) else { continue };
        let seq: i64 = row.try_get("seq")?;
        let entry = entry_from_db(
            seq,
            row.try_get("packet_id")?,
            row.try_get("qos")?,
            row.try_get("pubrec_received")?,
            row.try_get("origin")?,
            row.try_get("topic")?,
            row.try_get("payload")?,
            row.try_get("received_at_ms")?,
        );
        match entry {
            Some(entry) => sessions[at].outbound.push(entry),
            None => warn!(session = id, seq, "stored message skipped: invalid value"),
        }
    }
    // The schema has a unique key on (session, packet id), but a table from before that key may still hold two
    // entries with one identifier. They cannot both be in flight: the one with the lower seq (the older) stays.
    for session in &mut sessions {
        let (entries, dropped) = dedupe_by_packet_id(std::mem::take(&mut session.outbound));
        session.outbound = entries;
        if dropped > 0 {
            warn!(
                session = session.id,
                dropped,
                "stored messages skipped: they share a packet identifier with an older one"
            );
        }
    }
    for row in sqlx::query(sql::LOAD_INBOUND).fetch_all(pool).await? {
        let id: i64 = row.try_get("session_id")?;
        let Some(&at) = index.get(&id) else { continue };
        match packet_id_from_db(row.try_get("packet_id")?) {
            Some(packet_id) => sessions[at].inbound_qos2.push(packet_id),
            None => warn!(
                session = id,
                "stored packet identifier skipped: out of range"
            ),
        }
    }
    Ok(sessions)
}

/// Keeps the first entry (the lowest seq, they are loaded in that order) of every packet identifier.
fn dedupe_by_packet_id(entries: Vec<StoredEntry>) -> (Vec<StoredEntry>, usize) {
    let mut seen = std::collections::HashSet::new();
    let total = entries.len();
    let kept: Vec<StoredEntry> = entries
        .into_iter()
        .filter(|entry| seen.insert(entry.packet_id))
        .collect();
    let dropped = total - kept.len();
    (kept, dropped)
}

// --- the writer ------------------------------------------------------------------------------

/// The writer was told to stop (or the store was dropped while the database is down).
struct Aborted;

/// Completes when `shutdown` asks the writer to stop. Never completes if the store was dropped instead: the
/// writer then ends when its queue is closed and empty.
async fn aborted(abort: &mut watch::Receiver<bool>) {
    if abort.wait_for(|stop| *stop).await.is_err() {
        std::future::pending::<()>().await;
    }
}

struct Writer {
    pool: PgPool,
    rx: mpsc::UnboundedReceiver<Item>,
    backlog: Arc<Backlog>,
    abort: watch::Receiver<bool>,
    /// The last attempt failed with a connection error (so that the log says it once, not per retry).
    failing: bool,
    /// When the stall was last logged (it is repeated every minute, so that it stays visible).
    last_warned: Option<std::time::Instant>,
}

impl Writer {
    /// Takes an item and everything that is waiting behind it (up to [`BATCH_SIZE`]), applies the ops of that
    /// batch in one transaction, and only then runs its barriers in the order they were submitted. This is the
    /// group commit: a barrier runs after everything submitted before it is committed.
    async fn run(mut self) {
        loop {
            let first = tokio::select! {
                biased;
                item = self.rx.recv() => item,
                () = aborted(&mut self.abort) => None,
            };
            let Some(first) = first else { break };
            let mut batch = vec![first];
            while batch.len() < BATCH_SIZE {
                match self.rx.try_recv() {
                    Ok(item) => batch.push(item),
                    Err(_) => break,
                }
            }
            let mut ops = Vec::with_capacity(batch.len());
            let mut barriers = Vec::new();
            let mut bytes = 0;
            let items = batch.len();
            for item in batch {
                match item {
                    Item::Op(op) => {
                        bytes += Backlog::weight(&op);
                        ops.push(op);
                    }
                    Item::Barrier(done) => barriers.push(done),
                }
            }
            self.backlog.release(items, bytes);
            if !ops.is_empty() && self.apply(&ops).await.is_err() {
                // stopping: barriers that did not run are dropped, which is allowed at shutdown
                break;
            }
            for done in barriers {
                // a closure that panics must not end the writer: the broker would wait for it for ever
                if std::panic::catch_unwind(std::panic::AssertUnwindSafe(done)).is_err() {
                    error!("a barrier of the session store panicked");
                }
            }
        }
    }

    /// Applies the ops of a batch. If the database refuses the batch itself, every op gets its own
    /// transaction and the ones that fail are skipped and logged: the broker must not stall on one bad op.
    async fn apply(&mut self, ops: &[Op]) -> Result<(), Aborted> {
        let Err(error) = self.run_with_retry(ops).await? else {
            return Ok(());
        };
        error!(
            error = %describe(&error, true),
            ops = ops.len(),
            "the session database refused a batch of changes: applying them one by one"
        );
        if ops.len() == 1 {
            error!(op = op_name(&ops[0]), "change skipped");
            return Ok(());
        }
        for op in ops {
            if let Err(error) = self.run_with_retry(std::slice::from_ref(op)).await? {
                error!(op = op_name(op), error = %describe(&error, true), "change skipped");
            }
        }
        Ok(())
    }

    /// Runs the ops in one transaction. Every error except a data error (see [`is_data_error`]) is retried
    /// (after 200 ms, doubling up to 5 s) for as long as it lasts; the outer `Err` is only returned when the
    /// writer has to stop. The inner `Err` is a data error.
    async fn run_with_retry(&mut self, ops: &[Op]) -> Result<Result<(), sqlx::Error>, Aborted> {
        let mut delay = BACKOFF_START;
        loop {
            let result = tokio::select! {
                biased;
                () = aborted(&mut self.abort) => return Err(Aborted),
                result = run_batch(&self.pool, ops) => result,
            };
            match result {
                Err(error) if !is_data_error(&error) => {
                    let repeat = self
                        .last_warned
                        .is_none_or(|at| at.elapsed() >= STALL_LOG_INTERVAL);
                    if repeat {
                        self.failing = true;
                        self.last_warned = Some(std::time::Instant::now());
                        warn!(
                            error = %describe(&error, true),
                            queued = self.backlog.queued(),
                            "the session database does not take the changes: they are queued and tried again"
                        );
                    }
                    if self.rx.is_closed() {
                        // the broker is gone and nobody can wait for these changes any more
                        warn!("the session store was dropped while the database is unavailable: queued changes are lost");
                        return Err(Aborted);
                    }
                    tokio::select! {
                        biased;
                        () = aborted(&mut self.abort) => return Err(Aborted),
                        () = tokio::time::sleep(delay) => {}
                    }
                    delay = (delay * 2).min(BACKOFF_MAX);
                }
                other => {
                    if self.failing {
                        self.failing = false;
                        self.last_warned = None;
                        info!("the session database is available again");
                    }
                    return Ok(other);
                }
            }
        }
    }
}

fn op_name(op: &Op) -> &'static str {
    match op {
        Op::PutSession(_) => "PutSession",
        Op::UpdateSession { .. } => "UpdateSession",
        Op::DeleteSession { .. } => "DeleteSession",
        Op::SetSubscription { .. } => "SetSubscription",
        Op::RemoveSubscription { .. } => "RemoveSubscription",
        Op::Enqueue { .. } => "Enqueue",
        Op::OutboundPubrec { .. } => "OutboundPubrec",
        Op::DeleteOutbound { .. } => "DeleteOutbound",
        Op::AddInbound { .. } => "AddInbound",
        Op::RemoveInbound { .. } => "RemoveInbound",
    }
}

async fn run_batch(pool: &PgPool, ops: &[Op]) -> Result<(), sqlx::Error> {
    let mut tx = pool.begin().await?;
    for op in ops {
        apply_op(&mut tx, op).await?;
    }
    tx.commit().await
}

async fn set_subscription(
    conn: &mut PgConnection,
    id: i64,
    filter: &str,
    qos: QoS,
    no_local: bool,
) -> Result<(), sqlx::Error> {
    sqlx::query(sql::SET_SUBSCRIPTION)
        .bind(id)
        .bind(filter)
        .bind(qos_to_db(qos))
        .bind(no_local)
        .execute(conn)
        .await?;
    Ok(())
}

async fn enqueue(conn: &mut PgConnection, id: i64, entry: &StoredEntry) -> Result<(), sqlx::Error> {
    sqlx::query(sql::ENQUEUE)
        .bind(id)
        .bind(entry.seq as i64)
        .bind(i32::from(entry.packet_id))
        .bind(qos_to_db(entry.qos))
        .bind(entry.pubrec_received)
        .bind(origin_to_db(entry.origin))
        .bind(&*entry.topic)
        .bind(&entry.payload[..])
        .bind(entry.received_at_ms)
        .execute(conn)
        .await?;
    Ok(())
}

async fn add_inbound(conn: &mut PgConnection, id: i64, packet_id: u16) -> Result<(), sqlx::Error> {
    sqlx::query(sql::ADD_INBOUND)
        .bind(id)
        .bind(i32::from(packet_id))
        .execute(conn)
        .await?;
    Ok(())
}

async fn apply_op(conn: &mut PgConnection, op: &Op) -> Result<(), sqlx::Error> {
    match op {
        Op::PutSession(session) => {
            let id = id_to_db(session.id);
            sqlx::query(sql::DELETE_FOR_PUT)
                .bind(id)
                .bind(session.client_id.as_str())
                .execute(&mut *conn)
                .await?;
            sqlx::query(sql::INSERT_SESSION)
                .bind(id)
                .bind(session.client_id.as_str())
                .bind(session.username.as_deref())
                .bind(protocol_to_db(session.version))
                .bind(i64::from(session.expiry_s))
                .bind(session.disconnected_at_ms)
                .execute(&mut *conn)
                .await?;
            for (filter, qos, no_local) in &session.subscriptions {
                set_subscription(&mut *conn, id, filter, *qos, *no_local).await?;
            }
            for entry in &session.outbound {
                enqueue(&mut *conn, id, entry).await?;
            }
            for packet_id in &session.inbound_qos2 {
                add_inbound(&mut *conn, id, *packet_id).await?;
            }
        }
        Op::UpdateSession {
            id,
            expiry_s,
            disconnected_at_ms,
        } => {
            sqlx::query(sql::UPDATE_SESSION)
                .bind(id_to_db(*id))
                .bind(i64::from(*expiry_s))
                .bind(*disconnected_at_ms)
                .execute(conn)
                .await?;
        }
        Op::DeleteSession { id } => {
            sqlx::query(sql::DELETE_SESSION)
                .bind(id_to_db(*id))
                .execute(conn)
                .await?;
        }
        Op::SetSubscription {
            id,
            filter,
            qos,
            no_local,
        } => set_subscription(conn, id_to_db(*id), filter, *qos, *no_local).await?,
        Op::RemoveSubscription { id, filter } => {
            sqlx::query(sql::REMOVE_SUBSCRIPTION)
                .bind(id_to_db(*id))
                .bind(filter.as_str())
                .execute(conn)
                .await?;
        }
        Op::Enqueue { id, entry } => enqueue(conn, id_to_db(*id), entry).await?,
        Op::OutboundPubrec { id, seq } => {
            sqlx::query(sql::OUTBOUND_PUBREC)
                .bind(id_to_db(*id))
                .bind(*seq as i64)
                .execute(conn)
                .await?;
        }
        Op::DeleteOutbound { id, seq } => {
            sqlx::query(sql::DELETE_OUTBOUND)
                .bind(id_to_db(*id))
                .bind(*seq as i64)
                .execute(conn)
                .await?;
        }
        Op::AddInbound { id, packet_id } => add_inbound(conn, id_to_db(*id), *packet_id).await?,
        Op::RemoveInbound { id, packet_id } => {
            sqlx::query(sql::REMOVE_INBOUND)
                .bind(id_to_db(*id))
                .bind(i32::from(*packet_id))
                .execute(conn)
                .await?;
        }
    }
    Ok(())
}

#[cfg(test)]
mod db_tests;

#[cfg(test)]
mod unit_tests {
    use super::*;

    #[test]
    fn statements_use_contiguous_parameter_numbers() {
        let highest = |sql: &str| {
            (1..=12)
                .rev()
                .find(|n| sql.contains(&format!("${n}")))
                .unwrap_or(0)
        };
        for (sql, n) in sql::ALL {
            assert_eq!(highest(sql), *n, "{sql}");
            for i in 1..=*n {
                assert!(sql.contains(&format!("${i}")), "${i} missing in {sql}");
            }
        }
    }

    #[test]
    fn inserts_below_a_session_cannot_violate_the_foreign_key() {
        for sql in [sql::SET_SUBSCRIPTION, sql::ENQUEUE, sql::ADD_INBOUND] {
            assert!(
                sql.contains("WHERE EXISTS (SELECT 1 FROM mqtt_session WHERE session_id = $1)"),
                "{sql}"
            );
            assert!(sql.contains("ON CONFLICT"), "{sql}");
        }
    }

    #[test]
    fn the_schema_is_idempotent_and_unqualified() {
        let creates = sql::SCHEMA.matches("CREATE TABLE").count();
        assert_eq!(creates, 4);
        assert_eq!(
            sql::SCHEMA
                .matches("CREATE TABLE IF NOT EXISTS mqtt_")
                .count(),
            creates
        );
        assert!(!sql::SCHEMA.contains("public."));
    }

    /// The real PostgreSQL parser accepts the schema and every statement (syntax only; the statements are
    /// executed by the database tests, `DT_TEST_DATABASE_URL`).
    #[cfg(feature = "sql-syntax-check")]
    #[test]
    fn schema_and_statements_are_valid_postgresql() {
        let parsed = pg_query::parse(sql::SCHEMA).expect("the schema parses");
        assert_eq!(parsed.protobuf.stmts.len(), 4, "four tables");
        for (statement, _) in sql::ALL {
            pg_query::parse(statement).unwrap_or_else(|e| panic!("{e}: {statement}"));
        }
    }

    #[test]
    fn encodings_round_trip() {
        for version in [ProtocolVersion::V311, ProtocolVersion::V5] {
            assert_eq!(protocol_from_db(protocol_to_db(version)), Some(version));
        }
        assert_eq!(protocol_to_db(ProtocolVersion::V311), 4);
        assert_eq!(protocol_to_db(ProtocolVersion::V5), 5);
        for bits in 0..=2u8 {
            let qos = QoS::from_bits(bits).unwrap();
            assert_eq!(qos_to_db(qos), i16::from(bits));
            assert_eq!(qos_from_db(qos_to_db(qos)), Some(qos));
        }
        for origin in [Origin::Client, Origin::Server] {
            assert_eq!(origin_from_db(origin_to_db(origin)), Some(origin));
        }
        assert_eq!(origin_to_db(Origin::Client), 0);
        assert_eq!(origin_to_db(Origin::Server), 1);
        for bad in [-1, 3, 6, 255, i16::MAX, i16::MIN] {
            assert_eq!(qos_from_db(bad), None);
        }
        assert_eq!(protocol_from_db(3), None);
        assert_eq!(protocol_from_db(0), None);
        assert_eq!(origin_from_db(2), None);
        assert_eq!(id_from_db(id_to_db(42)), Some(42));
        assert_eq!(id_from_db(-1), None);
        assert_eq!(packet_id_from_db(1), Some(1));
        assert_eq!(packet_id_from_db(65535), Some(65535));
        for bad in [0, -1, 65536] {
            assert_eq!(packet_id_from_db(bad), None);
        }
    }

    #[test]
    fn rows_that_cannot_be_understood_are_rejected() {
        let session =
            |protocol, expiry| session_from_db(1, "c".into(), None, protocol, expiry, None);
        assert!(session(4, 10).is_some());
        assert!(session(5, i64::from(u32::MAX)).is_some());
        assert!(session(6, 10).is_none());
        assert!(session(4, -1).is_none());
        assert!(session(4, i64::from(u32::MAX) + 1).is_none());
        assert!(session_from_db(-1, "c".into(), None, 4, 1, None).is_none());
        assert_eq!(
            subscription_from_db("a/#".into(), 2, true),
            Some(("a/#".into(), QoS::ExactlyOnce, true))
        );
        assert_eq!(subscription_from_db("a".into(), 3, false), None);
        let entry = |seq, packet_id, qos, origin| {
            entry_from_db(seq, packet_id, qos, true, origin, "t".into(), vec![1, 2], 7)
        };
        let ok = entry(5, 9, 2, 1).unwrap();
        assert_eq!(
            (
                ok.seq,
                ok.packet_id,
                ok.qos,
                ok.pubrec_received,
                ok.origin,
                &*ok.topic,
                &ok.payload[..],
                ok.received_at_ms
            ),
            (
                5,
                9,
                QoS::ExactlyOnce,
                true,
                Origin::Server,
                "t",
                &[1u8, 2][..],
                7
            )
        );
        assert!(entry(-1, 9, 1, 0).is_none());
        assert!(entry(1, 0, 1, 0).is_none(), "packet identifier 0");
        assert!(entry(1, 70000, 1, 0).is_none());
        assert!(entry(1, 9, 0, 0).is_none(), "QoS 0 is never queued");
        assert!(entry(1, 9, 3, 0).is_none());
        assert!(entry(1, 9, 1, 2).is_none());
    }

    fn enqueue_of(total_bytes: usize) -> Op {
        Op::Enqueue {
            id: 1,
            entry: StoredEntry {
                seq: 1,
                packet_id: 1,
                qos: QoS::AtLeastOnce,
                pubrec_received: false,
                origin: Origin::Client,
                topic: Arc::from("t"),
                payload: Bytes::from(vec![0u8; total_bytes - 1]),
                received_at_ms: 0,
            },
        }
    }

    fn snapshot() -> SessionSnapshot {
        SessionSnapshot {
            id: 1,
            client_id: "c".into(),
            username: None,
            version: ProtocolVersion::V311,
            expiry_s: 10,
            disconnected_at_ms: None,
            subscriptions: vec![],
            outbound: vec![],
            inbound_qos2: vec![],
        }
    }

    /// Everything but `Enqueue`.
    fn other_ops() -> Vec<Op> {
        vec![
            Op::PutSession(snapshot()),
            Op::UpdateSession {
                id: 1,
                expiry_s: 1,
                disconnected_at_ms: None,
            },
            Op::DeleteSession { id: 1 },
            Op::SetSubscription {
                id: 1,
                filter: "f".into(),
                qos: QoS::AtMostOnce,
                no_local: false,
            },
            Op::RemoveSubscription {
                id: 1,
                filter: "f".into(),
            },
            Op::OutboundPubrec { id: 1, seq: 1 },
            Op::DeleteOutbound { id: 1, seq: 1 },
            Op::AddInbound {
                id: 1,
                packet_id: 1,
            },
            Op::RemoveInbound {
                id: 1,
                packet_id: 1,
            },
        ]
    }

    #[test]
    fn a_full_backlog_drops_only_queued_messages() {
        let backlog = Backlog::new(3, 1000);
        assert!(backlog.admit(&enqueue_of(10)));
        assert!(backlog.admit(&enqueue_of(10)));
        assert!(backlog.admit(&Op::DeleteSession { id: 1 }));
        assert_eq!(backlog.queued(), 3);
        // full: no more messages ...
        assert!(!backlog.admit(&enqueue_of(10)));
        assert!(!backlog.admit(&enqueue_of(10)));
        assert_eq!(backlog.dropped(), 2);
        assert_eq!(backlog.queued(), 3, "a dropped op is not counted as queued");
        // ... but everything else still goes in, and so do barriers
        for op in other_ops() {
            assert!(backlog.admit(&op), "{op:?}");
        }
        backlog.admit_barrier();
        assert_eq!(backlog.queued(), 3 + other_ops().len() + 1);
        assert_eq!(backlog.dropped(), 2);
    }

    #[test]
    fn the_bytes_of_queued_messages_are_limited_too() {
        let backlog = Backlog::new(100, 50);
        assert!(backlog.admit(&enqueue_of(30)));
        assert!(!backlog.admit(&enqueue_of(30)), "30 + 30 > 50");
        assert!(backlog.admit(&enqueue_of(20)));
        assert!(!backlog.admit(&enqueue_of(1)));
        // other ops weigh nothing
        assert!(backlog.admit(&Op::OutboundPubrec { id: 1, seq: 1 }));
        // the writer takes the two messages: there is room again
        backlog.release(2, 50);
        assert!(backlog.admit(&enqueue_of(50)));
    }

    #[test]
    fn dropping_starts_an_episode_that_ends_when_the_backlog_is_half_empty() {
        let backlog = Backlog::new(4, 1000);
        for _ in 0..4 {
            assert!(backlog.admit(&enqueue_of(1)));
        }
        assert!(!backlog.admit(&enqueue_of(1)));
        assert!(backlog.warned.load(Ordering::Relaxed));
        backlog.release(1, 1); // 3 left: above half
        assert!(backlog.warned.load(Ordering::Relaxed));
        assert!(backlog.admit(&enqueue_of(1)), "below the limit again");
        backlog.release(2, 2); // 2 left: half
        assert!(!backlog.warned.load(Ordering::Relaxed));
        assert_eq!(backlog.dropped_episode.load(Ordering::Relaxed), 0);
        assert_eq!(backlog.dropped(), 1);
    }

    #[test]
    fn only_the_payload_bearing_op_is_ever_droppable() {
        let full = Backlog::new(0, 0);
        for op in other_ops() {
            assert!(full.admit(&op), "{op:?}");
        }
        assert!(!full.admit(&enqueue_of(1)));
    }

    #[test]
    fn debug_output_never_shows_the_url() {
        let db = SessionDatabase {
            url: "postgres://user:secret-pw@host/db".into(),
            schema: Some("s".into()),
        };
        let shown = format!("{db:?}");
        assert!(
            !shown.contains("secret-pw") && !shown.contains("host"),
            "{shown}"
        );
        assert!(shown.contains("<redacted>") && shown.contains("\"s\""));
        let empty = SessionDatabase {
            url: String::new(),
            schema: None,
        };
        assert!(!format!("{empty:?}").contains("redacted"));
    }

    #[test]
    fn errors_never_contain_the_url_or_the_password() {
        let db = SessionDatabase {
            url: "not a url with secret-pw".into(),
            schema: None,
        };
        let error = connect_options(&db).unwrap_err();
        assert!(matches!(error, SessionStoreError::InvalidUrl));
        assert!(!format!("{error} {error:?}").contains("secret-pw"));
        let io = sqlx::Error::Io(std::io::Error::new(
            std::io::ErrorKind::ConnectionRefused,
            "secret-pw at 10.0.0.1",
        ));
        let error = SessionStoreError::from_sqlx(&io);
        let shown = format!("{error} {error:?} {}", describe(&io, true));
        assert!(
            !shown.contains("secret-pw") && !shown.contains("10.0.0.1"),
            "{shown}"
        );
        assert!(shown.contains("connection refused"), "{shown}");
        assert_eq!(
            SessionStoreError::Timeout.to_string(),
            "the session database did not answer within 10 seconds"
        );
    }

    #[test]
    fn only_data_errors_are_skipped_everything_else_is_retried() {
        // driver and connection errors are never skipped
        let io = || sqlx::Error::Io(std::io::Error::other("x"));
        assert!(!is_data_error(&io()));
        assert!(!is_data_error(&sqlx::Error::PoolTimedOut));
        assert!(!is_data_error(&sqlx::Error::PoolClosed));
        assert!(!is_data_error(&sqlx::Error::WorkerCrashed));
        assert!(!is_data_error(&sqlx::Error::Protocol("x".into())));
        assert!(!is_data_error(&sqlx::Error::BeginFailed));
        assert!(!is_data_error(&sqlx::Error::RowNotFound));
        assert!(!is_data_error(&sqlx::Error::ColumnNotFound("c".into())));
        // a value the driver cannot encode is about the change itself
        assert!(is_data_error(&sqlx::Error::Encode("x".into())));
        // by SQLSTATE: data exception, integrity constraint, program limit
        for code in [
            "22021", "22P02", "22001", "23503", "23505", "23502", "54000", "54001",
        ] {
            assert!(is_data_sqlstate(code), "{code}");
        }
        // missing privilege or table, schema or catalog, class 58, internal error, locks, connection and
        // authorization, serialization, resources, operator intervention, read-only: all retried
        for code in [
            "42501", "42P01", "3F000", "3D000", "58000", "58P01", "XX000", "55P03", "55006",
            "08006", "28P01", "40001", "40P01", "53300", "57P01", "57014", "25006", "25P02",
            "42601",
        ] {
            assert!(!is_data_sqlstate(code), "{code}");
        }
    }

    #[test]
    fn a_packet_identifier_is_kept_once_with_the_lowest_seq() {
        let entry = |seq, packet_id| StoredEntry {
            seq,
            packet_id,
            qos: QoS::AtLeastOnce,
            pubrec_received: false,
            origin: Origin::Client,
            topic: Arc::from("t"),
            payload: Bytes::new(),
            received_at_ms: 0,
        };
        let (kept, dropped) = dedupe_by_packet_id(vec![
            entry(1, 5),
            entry(2, 6),
            entry(3, 5),
            entry(4, 5),
            entry(9, 7),
        ]);
        let seqs: Vec<u64> = kept.iter().map(|e| e.seq).collect();
        assert_eq!((seqs, dropped), (vec![1, 2, 9], 2));
    }

    #[test]
    fn the_expiry_comparison_has_no_sum_that_can_overflow() {
        assert!(sql::DELETE_EXPIRED.contains("disconnected_at_ms <= $1::bigint - LEAST("));
        assert!(!sql::DELETE_EXPIRED.contains("disconnected_at_ms +"));
    }

    #[test]
    fn missing_tables_and_a_second_broker_have_clear_errors() {
        assert!(SessionStoreError::MissingTables
            .to_string()
            .contains("apply sql/0001_mqtt_session.sql"));
        assert!(SessionStoreError::InUse
            .to_string()
            .contains("another broker uses this MQTT session database"));
    }

    #[test]
    fn a_database_that_cannot_be_reached_is_an_error_without_secrets() {
        let runtime = tokio::runtime::Builder::new_current_thread()
            .enable_all()
            .build()
            .unwrap();
        let db = SessionDatabase {
            url: "postgres://dt:secret-pw@127.0.0.1:1/db".into(),
            schema: None,
        };
        let started = std::time::Instant::now();
        let result = runtime.block_on(PgSessionStore::connect(&db, 0, &BrokerLimits::default()));
        let Err(error) = result else {
            panic!("nothing listens on port 1");
        };
        assert!(started.elapsed() < Duration::from_secs(12));
        let shown = format!("{error} {error:?}");
        assert!(
            !shown.contains("secret-pw") && !shown.contains("127.0.0.1"),
            "{shown}"
        );
    }
}

#[cfg(test)]
pub(crate) mod testing {
    use std::sync::Mutex;

    use super::*;

    #[derive(Default)]
    struct Barriers {
        hold: bool,
        held: Vec<Box<dyn FnOnce() + Send>>,
    }

    /// Store for unit tests: records the ops and, once [`RecordingStore::hold`] was called, keeps the
    /// barriers until [`RecordingStore::release`]. A barrier never overtakes an earlier one.
    #[derive(Default)]
    pub(crate) struct RecordingStore {
        ops: Mutex<Vec<Op>>,
        barriers: Mutex<Barriers>,
    }

    impl RecordingStore {
        pub(crate) fn new() -> Arc<Self> {
            Arc::new(Self::default())
        }

        /// From now on barriers wait for [`RecordingStore::release`].
        pub(crate) fn hold(&self) {
            self.barriers.lock().unwrap().hold = true;
        }

        /// Runs the held barriers in order and stops holding.
        pub(crate) fn release(&self) {
            let held = {
                let mut b = self.barriers.lock().unwrap();
                b.hold = false;
                std::mem::take(&mut b.held)
            };
            for done in held {
                done();
            }
        }

        pub(crate) fn held(&self) -> usize {
            self.barriers.lock().unwrap().held.len()
        }

        pub(crate) fn ops(&self) -> Vec<Op> {
            self.ops.lock().unwrap().clone()
        }

        /// The recorded ops, which are forgotten.
        pub(crate) fn take_ops(&self) -> Vec<Op> {
            std::mem::take(&mut *self.ops.lock().unwrap())
        }
    }

    impl SessionStore for RecordingStore {
        fn submit(&self, op: Op) {
            self.ops.lock().unwrap().push(op);
        }

        fn barrier(&self, done: Box<dyn FnOnce() + Send>) {
            let mut b = self.barriers.lock().unwrap();
            if b.hold || !b.held.is_empty() {
                b.held.push(done);
                return;
            }
            drop(b);
            done();
        }
    }
}
