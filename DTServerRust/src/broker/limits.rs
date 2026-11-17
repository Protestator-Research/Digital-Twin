//! Configurable limits of the MQTT broker. Port of `DTServer/src/BrokerLimits.h`.

/// Limits of the broker. Every value has a default, so missing configuration keys are not an error.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct BrokerLimits {
    /// Maximum size of a received packet in bytes (fixed header included). Announced in CONNACK.
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
    /// Keep alive announced to every client (server keep alive). 0 disables the enforcement.
    pub server_keep_alive_s: u16,
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
            server_keep_alive_s: 60,
        }
    }
}
