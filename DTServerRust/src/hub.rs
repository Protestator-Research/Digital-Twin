//! In-process hub. Every PUBLISH accepted by the broker is also offered to internal subscribers, and
//! internal code can publish into the broker. This replaces the loopback MQTT client of the C++ server.
//!
//! Internal subscribers get a bounded channel. If a subscriber is too slow, messages for it are dropped and
//! counted; the broker never waits for it.

use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, Mutex};

use bytes::Bytes;
use thiserror::Error;
use tokio::sync::mpsc::{self, error::TrySendError};

use crate::broker::codec::{self, ProtocolVersion, QoS};
use crate::broker::core::Core;
use crate::broker::topic;

/// Where a message entered the broker.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Origin {
    /// Published by an MQTT client.
    Client,
    /// Published by the server itself through [`Hub::publish`] (initial values, registration requests).
    /// These are not measurements, so the twin logic does not hand them to the backend.
    Server,
}

/// A message delivered to an internal subscriber.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Message {
    pub origin: Origin,
    pub topic: Arc<str>,
    pub payload: Bytes,
    /// When the broker accepted the message: Unix time in milliseconds (UTC). Taken when the PUBLISH has been
    /// decoded (or [`Hub::publish`] is called), before any queueing for subscribers, so a slow subscriber does
    /// not shift it. Not monotonic: it follows the system clock.
    pub received_at_ms: i64,
}

/// The current time as Unix milliseconds (UTC), the unit of [`Message::received_at_ms`].
pub fn now_unix_ms() -> i64 {
    chrono::Utc::now().timestamp_millis()
}

#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum HubError {
    #[error("invalid topic filter")]
    InvalidFilter,
    #[error("invalid topic name")]
    InvalidTopic,
    #[error("topic exceeds the length or level limit")]
    TopicTooLong,
    #[error("channel capacity must be greater than zero")]
    ZeroCapacity,
    #[error("payload does not fit into the maximum packet size")]
    PayloadTooLarge,
    #[error("the broker has been shut down")]
    Closed,
}

/// Receiving end of an internal subscription. Dropping it ends the subscription.
#[derive(Debug)]
pub struct HubSubscription {
    rx: mpsc::Receiver<Message>,
    dropped: Arc<AtomicU64>,
}

impl HubSubscription {
    /// Waits for the next message. `None` if the broker is gone.
    pub async fn recv(&mut self) -> Option<Message> {
        self.rx.recv().await
    }

    pub fn try_recv(&mut self) -> Result<Message, mpsc::error::TryRecvError> {
        self.rx.try_recv()
    }

    /// Number of messages dropped because this subscriber's channel was full.
    pub fn dropped(&self) -> u64 {
        self.dropped.load(Ordering::Relaxed)
    }
}

struct InternalSub {
    filter: String,
    tx: mpsc::Sender<Message>,
    dropped: Arc<AtomicU64>,
}

/// The list of internal subscribers, owned by the broker core.
#[derive(Default)]
pub(crate) struct InternalBus {
    subs: Mutex<Vec<InternalSub>>,
    dropped_total: AtomicU64,
}

impl InternalBus {
    /// Offers a message to every matching subscriber without blocking. Subscribers whose receiver is gone
    /// are removed.
    pub(crate) fn offer(&self, message: &Message) {
        let topic = &message.topic;
        let mut subs = self.subs.lock().unwrap();
        subs.retain(|sub| {
            if sub.tx.is_closed() {
                return false;
            }
            if !topic::match_filter(&sub.filter, topic) {
                return true;
            }
            match sub.tx.try_send(message.clone()) {
                Ok(()) => true,
                Err(TrySendError::Full(_)) => {
                    sub.dropped.fetch_add(1, Ordering::Relaxed);
                    self.dropped_total.fetch_add(1, Ordering::Relaxed);
                    true
                }
                Err(TrySendError::Closed(_)) => false,
            }
        });
    }
}

impl InternalBus {
    /// Drops all subscriber channels, so receivers see the end of the stream.
    pub(crate) fn close(&self) {
        self.subs.lock().unwrap().clear();
    }

    #[cfg(test)]
    fn is_empty(&self) -> bool {
        self.subs.lock().unwrap().is_empty()
    }
}

/// The QoS the server's own publishes ([`Hub::publish`]) are distributed at. A subscriber gets the lower of
/// this and the QoS granted to its subscription, so QoS 0 subscribers see no difference. At QoS 1 the message is
/// only queued for sessions that have a connection (one still waiting for its CONNACK included): an offline session does not get it (see
/// `ClientSession::deliver`).
pub const SERVER_PUBLISH_QOS: QoS = QoS::AtLeastOnce;

/// Handle to the hub of a running broker. Cheap to clone.
#[derive(Clone)]
pub struct Hub {
    core: Arc<Core>,
}

impl Hub {
    pub(crate) fn new(core: Arc<Core>) -> Self {
        Self { core }
    }

    /// Subscribes to all messages matching `filter` (same wildcard rules as the broker). `capacity` is the
    /// size of the bounded channel.
    pub fn subscribe(&self, filter: &str, capacity: usize) -> Result<HubSubscription, HubError> {
        if capacity == 0 {
            return Err(HubError::ZeroCapacity);
        }
        if !topic::is_valid_filter(filter) {
            return Err(HubError::InvalidFilter);
        }
        let (tx, rx) = mpsc::channel(capacity);
        let dropped = Arc::new(AtomicU64::new(0));
        let mut subs = self.core.internal.subs.lock().unwrap();
        // checked under the lock: `Core::close` sets the flag first and clears the list afterwards
        if self.core.is_closed() {
            return Err(HubError::Closed);
        }
        subs.push(InternalSub {
            filter: filter.to_owned(),
            tx,
            dropped: Arc::clone(&dropped),
        });
        Ok(HubSubscription { rx, dropped })
    }

    /// Publishes into the broker as the server itself ([`Origin::Server`]): delivered to all matching client
    /// sessions and to all matching internal subscribers (including the caller's own, there is no no-local
    /// for internal code).
    ///
    /// Client sessions get the message at [`SERVER_PUBLISH_QOS`], lowered to the QoS of their subscription;
    /// internal subscribers see no QoS. A session whose client is not connected does **not** get it (nothing is
    /// retained, and queued server messages would replay outdated values); one that has a connection (even one that has not been accepted yet; the message goes out at its start) gets it queued,
    /// and a message already queued stays for a resume. Never blocks and does not wait for the session store: the
    /// `Enqueue` operations this causes for connected durable sessions are write-behind, without a barrier, and the
    /// result of the dispatch is deliberately ignored. After a broker crash a server message of the last commit
    /// window may therefore be missing.
    pub fn publish(&self, topic: &str, payload: impl Into<Bytes>) -> Result<(), HubError> {
        if !topic::is_valid_topic_name(topic) {
            return Err(HubError::InvalidTopic);
        }
        if !topic::within_limits(topic, &self.core.limits) {
            return Err(HubError::TopicTooLong);
        }
        if self.core.is_closed() {
            return Err(HubError::Closed);
        }
        let payload = payload.into();
        // sized for the largest encoding any allowed client gets (MQTT 5 adds a property length byte), with
        // the packet id of the QoS 1 delivery
        let version = if self.core.protocol.allows(ProtocolVersion::V5) {
            ProtocolVersion::V5
        } else {
            ProtocolVersion::V311
        };
        if codec::publish_encoded_len(version, topic.len(), payload.len(), SERVER_PUBLISH_QOS)
            .is_none_or(|len| len > self.core.limits.max_packet_size as usize)
        {
            return Err(HubError::PayloadTooLarge);
        }
        let _ = self.core.dispatch(
            Message {
                origin: Origin::Server,
                topic: Arc::from(topic),
                payload,
                received_at_ms: now_unix_ms(),
            },
            None,
            SERVER_PUBLISH_QOS,
        );
        Ok(())
    }

    /// Total number of messages dropped for slow internal subscribers.
    pub fn dropped_total(&self) -> u64 {
        self.core.internal.dropped_total.load(Ordering::Relaxed)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::broker::limits::BrokerLimits;
    use crate::broker::protocol::ProtocolMode;

    fn hub() -> Hub {
        Hub::new(Core::new(BrokerLimits::default(), Default::default()))
    }

    #[test]
    fn publish_reaches_matching_subscribers_only() {
        let hub = hub();
        let mut all = hub.subscribe("#", 8).unwrap();
        let mut one = hub.subscribe("a/+", 8).unwrap();
        let mut other = hub.subscribe("b", 8).unwrap();
        hub.publish("a/x", &b"1"[..]).unwrap();
        assert_eq!(all.try_recv().unwrap().payload, Bytes::from_static(b"1"));
        assert_eq!(&*one.try_recv().unwrap().topic, "a/x");
        assert!(other.try_recv().is_err());
    }

    #[test]
    fn slow_subscriber_drops_and_counts_without_blocking() {
        let hub = hub();
        let mut slow = hub.subscribe("#", 2).unwrap();
        for i in 0..10u8 {
            hub.publish("t", vec![i]).unwrap();
        }
        assert_eq!(slow.dropped(), 8);
        assert_eq!(hub.dropped_total(), 8);
        assert_eq!(slow.try_recv().unwrap().payload[0], 0);
        assert_eq!(slow.try_recv().unwrap().payload[0], 1);
        assert!(slow.try_recv().is_err());
    }

    #[test]
    fn closed_subscribers_are_removed() {
        let hub = hub();
        let sub = hub.subscribe("#", 1).unwrap();
        drop(sub);
        hub.publish("t", &b"x"[..]).unwrap();
        assert!(hub.core.internal.is_empty());
    }

    /// Payload sizes (below, at, above the limit) for a publish of `topic` at QoS 1 in `mode`.
    fn boundary(mode: ProtocolMode, version: ProtocolVersion, topic: &str) -> [(usize, bool); 3] {
        let base = codec::publish_encoded_len(version, topic.len(), 0, QoS::AtLeastOnce).unwrap();
        // the limit is chosen so that a payload of 100 bytes fills the packet exactly
        let max = base + 100;
        let hub = Hub::new(Core::new(
            BrokerLimits {
                max_packet_size: max as u32,
                ..BrokerLimits::default()
            },
            mode,
        ));
        [99, 100, 101].map(|n| (n, hub.publish(topic, vec![0u8; n]).is_ok()))
    }

    #[test]
    fn size_check_counts_the_packet_id_of_qos1_at_the_exact_boundary() {
        let expected = [(99, true), (100, true), (101, false)];
        assert_eq!(
            boundary(ProtocolMode::V311, ProtocolVersion::V311, "a/b"),
            expected
        );
        assert_eq!(
            boundary(ProtocolMode::Both, ProtocolVersion::V5, "a/b"),
            expected
        );
        // a QoS 0 sizing would have let one more byte pass
        let qos0 = codec::publish_encoded_len(ProtocolVersion::V311, 3, 0, QoS::AtMostOnce);
        let qos1 = codec::publish_encoded_len(ProtocolVersion::V311, 3, 0, QoS::AtLeastOnce);
        assert_eq!(qos1.unwrap(), qos0.unwrap() + 2);
    }

    #[test]
    fn input_validation() {
        let hub = hub();
        assert_eq!(
            hub.subscribe("a/#/b", 1).unwrap_err(),
            HubError::InvalidFilter
        );
        assert_eq!(hub.subscribe("a", 0).unwrap_err(), HubError::ZeroCapacity);
        assert_eq!(
            hub.publish("a/+", &b""[..]).unwrap_err(),
            HubError::InvalidTopic
        );
        assert_eq!(
            hub.publish("", &b""[..]).unwrap_err(),
            HubError::InvalidTopic
        );
        let long = "a".repeat(513);
        assert_eq!(
            hub.publish(&long, &b""[..]).unwrap_err(),
            HubError::TopicTooLong
        );
    }
}
