//! Configurable limits of the MQTT broker. Port of `DTServer/src/BrokerLimits.h`.

/// Limits of the broker. Every value has a default, so missing configuration keys are not an error.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct BrokerLimits {
    /// Maximum size of a received packet in bytes (fixed header included). A larger packet closes the
    /// connection (MQTT 5 clients get DISCONNECT 0x95 and are told the limit in CONNACK; MQTT 3.1.1 cannot
    /// announce it).
    pub max_packet_size: u32,
    /// Maximum number of open connections in total.
    pub max_connections: usize,
    /// Maximum number of open connections per remote IP address.
    pub max_connections_per_ip: usize,
    /// Maximum number of distinct subscription filters per session.
    pub max_subscriptions_per_session: usize,
    /// Maximum number of bytes (topic + payload) queued for one session.
    pub max_pending_send_bytes: usize,
    /// Maximum length of a topic name or filter in bytes.
    pub max_topic_length: usize,
    /// Maximum number of levels of a topic name or filter.
    pub max_topic_levels: usize,
    /// A session is disconnected after this many consecutive dropped messages.
    pub max_consecutive_send_overflows: usize,
    /// Seconds a new connection may take until it sends CONNECT.
    pub connect_timeout_s: u32,
    /// Receive Maximum the broker announces to clients: how many QoS 2 PUBLISH packets of one client may
    /// await their PUBREL at the same time. MQTT 5 clients that send more are disconnected (0x93); MQTT 3.1.1
    /// has no such limit. What the broker sends to a client is bounded by the client's own Receive Maximum.
    pub receive_maximum: u16,
    /// Maximum number of QoS 1/2 messages queued or unacknowledged for one session. Every entry owns a packet
    /// identifier, so values above 65535 are treated as 65535.
    pub max_queued_messages: usize,
    /// Longest time in seconds an offline session is kept. A client that asks for more (MQTT 5 Session Expiry
    /// Interval) is told the shorter value; MQTT 3.1.1 sessions without clean session use exactly this value.
    pub session_expiry_max_s: u32,
    /// Maximum number of sessions that outlive their connection (expiry above 0), online or offline. A
    /// CONNECT that would create one more is refused.
    pub max_persistent_sessions: usize,
}

impl Default for BrokerLimits {
    fn default() -> Self {
        Self {
            max_packet_size: 262_144,
            max_connections: 1000,
            max_connections_per_ip: 20,
            max_subscriptions_per_session: 100,
            max_pending_send_bytes: 4 * 1024 * 1024,
            max_topic_length: 512,
            max_topic_levels: 16,
            max_consecutive_send_overflows: 3,
            connect_timeout_s: 10,
            receive_maximum: 100,
            max_queued_messages: 1000,
            session_expiry_max_s: 86_400,
            max_persistent_sessions: 1000,
        }
    }
}
