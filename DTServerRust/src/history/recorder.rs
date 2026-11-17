//! The ingest side of the recorder: hub messages -> classified rows -> bounded queue.
//!
//! It never blocks: a full queue drops the row and counts it, and so does a full hub subscription (counted
//! by the hub, copied into the statistics).

use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, RwLock};

use chrono::{DateTime, Utc};
use tokio::sync::mpsc::error::TrySendError;
use tokio::sync::{mpsc, watch};
use tracing::debug;

use super::partition::in_window;
use super::registry::Registry;
use super::row::Row;
use crate::hub::{HubSubscription, Message, Origin};
use crate::payload::values::classify;
use crate::payload::{CONNECT_TO_TWIN, REGISTER_FUNCTION};

/// Longest text (String value, digits of a huge integer, complex JSON) that is recorded. Longer values are
/// counted as invalid; this bounds the size of a queued row (see the memory bound in the module docs).
pub const MAX_ROW_TEXT_BYTES: usize = 4096;

/// Counters of the history, readable at any time.
#[derive(Debug, Default)]
pub struct HistoryStats {
    /// Messages published by MQTT clients that the recorder looked at.
    pub received: AtomicU64,
    /// Control topics, server published messages and topics without an element path.
    pub ignored: AtomicU64,
    /// Topics whose first level is not the name of a registered twin.
    pub unknown_twin: AtomicU64,
    /// Payloads that are not a valid value.
    pub invalid: AtomicU64,
    /// Valid values whose `time_point` is outside the accepted window.
    pub out_of_window: AtomicU64,
    /// Rows put into the queue.
    pub queued: AtomicU64,
    /// Rows dropped because the queue was full.
    pub queue_full: AtomicU64,
    /// Messages dropped by the hub because the recorder's subscription was full.
    pub hub_dropped: AtomicU64,
    /// Rows inserted into the database.
    pub recorded: AtomicU64,
    /// Rows the database refused (bad data, no partition could be created).
    pub rejected: AtomicU64,
    /// Rows still unwritten when the shutdown time limit ended.
    pub lost_on_shutdown: AtomicU64,
    /// Failed insert attempts that were retried.
    pub flush_retries: AtomicU64,
    /// Successful batch inserts.
    pub batches: AtomicU64,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct StatsSnapshot {
    pub received: u64,
    pub ignored: u64,
    pub unknown_twin: u64,
    pub invalid: u64,
    pub out_of_window: u64,
    pub queued: u64,
    pub queue_full: u64,
    pub hub_dropped: u64,
    pub recorded: u64,
    pub rejected: u64,
    pub lost_on_shutdown: u64,
    pub flush_retries: u64,
    pub batches: u64,
}

impl HistoryStats {
    pub fn snapshot(&self) -> StatsSnapshot {
        let get = |c: &AtomicU64| c.load(Ordering::Relaxed);
        StatsSnapshot {
            received: get(&self.received),
            ignored: get(&self.ignored),
            unknown_twin: get(&self.unknown_twin),
            invalid: get(&self.invalid),
            out_of_window: get(&self.out_of_window),
            queued: get(&self.queued),
            queue_full: get(&self.queue_full),
            hub_dropped: get(&self.hub_dropped),
            recorded: get(&self.recorded),
            rejected: get(&self.rejected),
            lost_on_shutdown: get(&self.lost_on_shutdown),
            flush_retries: get(&self.flush_retries),
            batches: get(&self.batches),
        }
    }

    pub(super) fn add(counter: &AtomicU64, n: u64) {
        counter.fetch_add(n, Ordering::Relaxed);
    }
}

/// What to do with one hub message.
#[derive(Debug, Clone, PartialEq)]
pub enum Verdict {
    Row(Row),
    Ignored,
    UnknownTwin,
    Invalid,
    OutOfWindow,
}

/// True for `connectToTwin` and `<anything>/registerFunction`.
pub fn is_control_topic(topic: &str) -> bool {
    topic == CONNECT_TO_TWIN
        || topic
            .rsplit_once('/')
            .is_some_and(|(_, last)| last == REGISTER_FUNCTION)
}

/// Classifies one message: maps the topic to a twin and an element, validates the payload and the time.
pub fn judge(
    registry: &Registry,
    message: &Message,
    now: DateTime<Utc>,
    retention_days: u32,
) -> Verdict {
    // The placeholders and registration requests the server publishes itself are not measurements.
    if message.origin == Origin::Server || is_control_topic(&message.topic) {
        return Verdict::Ignored;
    }
    // `<TwinName>/<elementPath>`: a topic without an element path cannot be a measurement
    match message.topic.split_once('/') {
        Some((_, element)) if !element.is_empty() => {}
        _ => return Verdict::Ignored,
    }
    let Some((twin, element)) = registry.lookup(&message.topic) else {
        return Verdict::UnknownTwin;
    };
    if element.contains('\0') {
        return Verdict::Invalid;
    }
    let payload = match classify(&message.payload) {
        Ok(payload) => payload,
        Err(e) => {
            debug!(topic = %message.topic, "payload not recorded: {e}");
            return Verdict::Invalid;
        }
    };
    if !in_window(payload.time_point, now, retention_days) {
        return Verdict::OutOfWindow;
    }
    let row = Row::new(twin.twin_id, twin.project_id, element, &payload, now);
    // PostgreSQL text cannot hold NUL; and the size of a queued row is bounded
    let too_big = |text: &Option<String>| {
        text.as_ref()
            .is_some_and(|t| t.len() > MAX_ROW_TEXT_BYTES || t.contains('\0'))
    };
    if too_big(&row.value_text) || too_big(&row.value_json) {
        return Verdict::Invalid;
    }
    Verdict::Row(row)
}

/// Judges the message, updates the counters and queues the row without waiting.
pub fn handle_message(
    registry: &RwLock<Registry>,
    stats: &HistoryStats,
    queue: &mpsc::Sender<Row>,
    message: &Message,
    now: DateTime<Utc>,
    retention_days: u32,
) {
    if message.origin == Origin::Client {
        HistoryStats::add(&stats.received, 1);
    }
    let verdict = {
        let registry = registry.read().unwrap();
        judge(&registry, message, now, retention_days)
    };
    match verdict {
        Verdict::Ignored => HistoryStats::add(&stats.ignored, 1),
        Verdict::UnknownTwin => HistoryStats::add(&stats.unknown_twin, 1),
        Verdict::Invalid => HistoryStats::add(&stats.invalid, 1),
        Verdict::OutOfWindow => HistoryStats::add(&stats.out_of_window, 1),
        Verdict::Row(row) => match queue.try_send(row) {
            Ok(()) => HistoryStats::add(&stats.queued, 1),
            Err(TrySendError::Full(_)) => HistoryStats::add(&stats.queue_full, 1),
            // the writer is gone (shutdown)
            Err(TrySendError::Closed(_)) => {}
        },
    }
}

/// Receives hub messages until the hub closes or the shutdown flag is set. Dropping the queue sender at the
/// end tells the writer to flush and stop.
pub async fn run_ingest(
    mut subscription: HubSubscription,
    registry: Arc<RwLock<Registry>>,
    stats: Arc<HistoryStats>,
    queue: mpsc::Sender<Row>,
    retention_days: u32,
    mut shutdown: watch::Receiver<bool>,
) {
    loop {
        tokio::select! {
            biased;
            _ = shutdown.wait_for(|stop| *stop) => break,
            message = subscription.recv() => match message {
                Some(message) => {
                    handle_message(&registry, &stats, &queue, &message, Utc::now(), retention_days);
                    stats.hub_dropped.store(subscription.dropped(), Ordering::Relaxed);
                }
                None => break,
            }
        }
    }
    stats
        .hub_dropped
        .store(subscription.dropped(), Ordering::Relaxed);
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::history::registry::TwinRecord;
    use bytes::Bytes;
    use chrono::TimeZone;
    use uuid::Uuid;

    fn now() -> DateTime<Utc> {
        Utc.with_ymd_and_hms(2026, 11, 17, 9, 40, 20).unwrap()
    }

    fn registry() -> Registry {
        let mut r = Registry::new();
        r.register(&TwinRecord {
            twin_id: Uuid::from_u128(1),
            project_id: Uuid::from_u128(2),
            name: "Pump".into(),
        });
        r
    }

    fn msg(origin: Origin, topic: &str, payload: &str) -> Message {
        Message {
            origin,
            topic: Arc::from(topic),
            payload: Bytes::from(payload.to_owned()),
            properties: Bytes::new(),
        }
    }

    const GOOD: &str = r#"{"time_point":"2026-11-17T09:40:12.345Z","value":21.5}"#;

    #[test]
    fn control_topics() {
        assert!(is_control_topic("connectToTwin"));
        assert!(is_control_topic("Pump/registerFunction"));
        assert!(is_control_topic("a/b/registerFunction"));
        assert!(!is_control_topic("registerFunction"));
        assert!(!is_control_topic("Pump/registerFunctions"));
        assert!(!is_control_topic("Pump/x"));
        assert!(!is_control_topic("connectToTwin/x"));
    }

    #[test]
    fn valid_value_becomes_a_row() {
        let Verdict::Row(row) = judge(
            &registry(),
            &msg(Origin::Client, "Pump/Motor/temp", GOOD),
            now(),
            0,
        ) else {
            panic!("expected a row");
        };
        assert_eq!(row.twin_id, Uuid::from_u128(1));
        assert_eq!(row.project_id, Uuid::from_u128(2));
        assert_eq!(row.element_path, "Motor/temp");
        assert_eq!(row.received_at, now());
        assert_eq!(row.value_num, Some(21.5));
        assert_eq!(
            row.ts,
            Utc.with_ymd_and_hms(2026, 11, 17, 9, 40, 12).unwrap()
                + chrono::Duration::milliseconds(345)
        );
    }

    #[test]
    fn verdicts() {
        let r = registry();
        let j =
            |origin, topic: &str, payload: &str| judge(&r, &msg(origin, topic, payload), now(), 0);
        // server published (initial values, registration request) and control topics
        assert_eq!(j(Origin::Server, "Pump/x", GOOD), Verdict::Ignored);
        assert_eq!(j(Origin::Client, "connectToTwin", "{}"), Verdict::Ignored);
        assert_eq!(
            j(Origin::Client, "Pump/registerFunction", "{}"),
            Verdict::Ignored
        );
        // no element path
        assert_eq!(j(Origin::Client, "Pump", GOOD), Verdict::Ignored);
        assert_eq!(j(Origin::Client, "Pump/", GOOD), Verdict::Ignored);
        // unknown twin
        assert_eq!(j(Origin::Client, "Other/x", GOOD), Verdict::UnknownTwin);
        // invalid payloads
        assert_eq!(j(Origin::Client, "Pump/x", "not json"), Verdict::Invalid);
        assert_eq!(
            j(Origin::Client, "Pump/x", "{value=0,\r\n timepoint=\"\"}"),
            Verdict::Invalid
        );
        assert_eq!(
            j(Origin::Client, "Pump/x", r#"{"value":1}"#),
            Verdict::Invalid
        );
        // a string that is not a fraction is a String value now
        assert!(matches!(
            j(Origin::Client, "Pump/x", r#"{"time_point":"2026-11-17T09:40:12Z","value":"abc"}"#),
            Verdict::Row(row) if row.value_type == crate::history::row::value_type::STRING
                && row.value_text.as_deref() == Some("abc")
        ));
        assert_eq!(
            j(
                Origin::Client,
                "Pump/x",
                r#"{"time_point":"2026-11-17T09:40:12Z","value":"1/0"}"#
            ),
            Verdict::Invalid
        );
        // NUL in a string, and strings beyond the row size limit
        assert_eq!(
            j(
                Origin::Client,
                "Pump/x",
                r#"{"time_point":"2026-11-17T09:40:12Z","value":"a\u0000b"}"#
            ),
            Verdict::Invalid
        );
        let long = format!(
            r#"{{"time_point":"2026-11-17T09:40:12Z","value":"{}"}}"#,
            "x".repeat(MAX_ROW_TEXT_BYTES + 1)
        );
        assert_eq!(j(Origin::Client, "Pump/x", &long), Verdict::Invalid);
        let max = format!(
            r#"{{"time_point":"2026-11-17T09:40:12Z","value":"{}"}}"#,
            "x".repeat(MAX_ROW_TEXT_BYTES)
        );
        assert!(matches!(j(Origin::Client, "Pump/x", &max), Verdict::Row(_)));
        assert_eq!(j(Origin::Client, "Pump/x\0y", GOOD), Verdict::Invalid);
        // out of the accepted window: far past and far future
        assert_eq!(
            j(
                Origin::Client,
                "Pump/x",
                r#"{"time_point":"1999-12-31T23:59:59Z","value":1}"#
            ),
            Verdict::OutOfWindow
        );
        assert_eq!(
            j(
                Origin::Client,
                "Pump/x",
                r#"{"time_point":"2027-01-01T00:00:00Z","value":1}"#
            ),
            Verdict::OutOfWindow
        );
        // retention: older than the cutoff is not stored
        assert_eq!(
            judge(
                &r,
                &msg(
                    Origin::Client,
                    "Pump/x",
                    r#"{"time_point":"2026-01-01T00:00:00Z","value":1}"#
                ),
                now(),
                30
            ),
            Verdict::OutOfWindow
        );
    }

    #[test]
    fn queue_overflow_is_dropped_and_counted_without_blocking() {
        let registry = RwLock::new(registry());
        let stats = HistoryStats::default();
        let (tx, mut rx) = mpsc::channel(3);
        let m = msg(Origin::Client, "Pump/x", GOOD);
        for _ in 0..10 {
            handle_message(&registry, &stats, &tx, &m, now(), 0);
        }
        let s = stats.snapshot();
        assert_eq!((s.received, s.queued, s.queue_full), (10, 3, 7));
        assert_eq!(rx.try_recv().unwrap().element_path, "x");
        // free space again: queueing continues
        handle_message(&registry, &stats, &tx, &m, now(), 0);
        assert_eq!(stats.snapshot().queued, 4);
        drop(rx);
        // a closed queue (shutdown) is not an error
        handle_message(&registry, &stats, &tx, &m, now(), 0);
    }

    #[test]
    fn counters_per_verdict() {
        let registry = RwLock::new(registry());
        let stats = HistoryStats::default();
        let (tx, _rx) = mpsc::channel(10);
        for (origin, topic, payload) in [
            (Origin::Client, "Pump/a", GOOD),
            (Origin::Client, "Other/a", GOOD),
            (Origin::Client, "Pump/a", "garbage"),
            (Origin::Client, "connectToTwin", "{}"),
            (Origin::Server, "Pump/a", GOOD),
            (
                Origin::Client,
                "Pump/a",
                r#"{"time_point":"1990-01-01T00:00:00Z","value":1}"#,
            ),
        ] {
            handle_message(
                &registry,
                &stats,
                &tx,
                &msg(origin, topic, payload),
                now(),
                0,
            );
        }
        let s = stats.snapshot();
        assert_eq!(
            (
                s.received,
                s.queued,
                s.unknown_twin,
                s.invalid,
                s.ignored,
                s.out_of_window
            ),
            (5, 1, 1, 1, 2, 1)
        );
    }
}
