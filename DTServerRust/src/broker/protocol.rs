//! MQTT protocol versions the broker can speak and the configurable mode that selects them.

use std::fmt;
use std::str::FromStr;

/// The protocol version of one connection, taken from the protocol level of its CONNECT.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum ProtocolVersion {
    /// MQTT 3.1.1, protocol level 4.
    #[default]
    V311,
    /// MQTT 5.0, protocol level 5.
    V5,
}

/// Quality of service level of a PUBLISH or a subscription (MQTT 3.1.1 4.3, MQTT 5 4.3).
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Default)]
pub enum QoS {
    /// At most once: no acknowledgement, no retransmission.
    #[default]
    AtMostOnce = 0,
    /// At least once: acknowledged with PUBACK.
    AtLeastOnce = 1,
    /// Exactly once: PUBREC / PUBREL / PUBCOMP handshake.
    ExactlyOnce = 2,
}

impl QoS {
    /// The QoS of the two wire bits; 3 is not a valid level and gives `None`.
    pub fn from_bits(bits: u8) -> Option<QoS> {
        match bits {
            0 => Some(QoS::AtMostOnce),
            1 => Some(QoS::AtLeastOnce),
            2 => Some(QoS::ExactlyOnce),
            _ => None,
        }
    }

    /// The value of the two wire bits.
    pub fn bits(self) -> u8 {
        self as u8
    }
}

/// Which protocol versions the broker accepts (`run.mqtt.protocol_version`).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum ProtocolMode {
    /// Only MQTT 3.1.1 (default, the behaviour before MQTT 5 was added).
    #[default]
    V311,
    /// Only MQTT 5.
    V5,
    /// MQTT 3.1.1 and MQTT 5, decided per connection.
    Both,
}

impl ProtocolMode {
    /// True if a client speaking `version` may connect.
    pub fn allows(self, version: ProtocolVersion) -> bool {
        matches!(
            (self, version),
            (ProtocolMode::Both, _)
                | (ProtocolMode::V311, ProtocolVersion::V311)
                | (ProtocolMode::V5, ProtocolVersion::V5)
        )
    }

    /// The configuration value of this mode.
    pub fn as_str(self) -> &'static str {
        match self {
            ProtocolMode::V311 => "3.1.1",
            ProtocolMode::V5 => "5",
            ProtocolMode::Both => "both",
        }
    }
}

impl fmt::Display for ProtocolMode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.as_str())
    }
}

/// The text is not a valid protocol mode.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ParseProtocolModeError(String);

impl fmt::Display for ParseProtocolModeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{:?} is not a valid MQTT protocol version (expected \"3.1.1\", \"5\" or \"both\")",
            self.0
        )
    }
}

impl std::error::Error for ParseProtocolModeError {}

impl FromStr for ProtocolMode {
    type Err = ParseProtocolModeError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s.trim().to_ascii_lowercase().as_str() {
            "3.1.1" => Ok(ProtocolMode::V311),
            "5" => Ok(ProtocolMode::V5),
            "both" => Ok(ProtocolMode::Both),
            _ => Err(ParseProtocolModeError(s.to_owned())),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn qos_bits_round_trip() {
        for bits in 0..=2u8 {
            assert_eq!(QoS::from_bits(bits).unwrap().bits(), bits);
        }
        assert_eq!(QoS::from_bits(3), None);
        assert_eq!(QoS::from_bits(255), None);
        assert!(QoS::AtMostOnce < QoS::AtLeastOnce && QoS::AtLeastOnce < QoS::ExactlyOnce);
        assert_eq!(QoS::default(), QoS::AtMostOnce);
    }

    #[test]
    fn parses_the_three_modes() {
        assert_eq!("3.1.1".parse(), Ok(ProtocolMode::V311));
        assert_eq!("5".parse(), Ok(ProtocolMode::V5));
        assert_eq!("both".parse(), Ok(ProtocolMode::Both));
        assert_eq!(" BOTH ".parse(), Ok(ProtocolMode::Both));
        for bad in ["", "3.1", "4", "5.0", "3.1.1,5"] {
            assert!(bad.parse::<ProtocolMode>().is_err(), "{bad:?}");
        }
    }

    #[test]
    fn display_round_trips() {
        for mode in [ProtocolMode::V311, ProtocolMode::V5, ProtocolMode::Both] {
            assert_eq!(mode.to_string().parse(), Ok(mode));
        }
    }

    #[test]
    fn allows_matrix() {
        use ProtocolVersion::{V311, V5};
        assert!(ProtocolMode::V311.allows(V311) && !ProtocolMode::V311.allows(V5));
        assert!(!ProtocolMode::V5.allows(V311) && ProtocolMode::V5.allows(V5));
        assert!(ProtocolMode::Both.allows(V311) && ProtocolMode::Both.allows(V5));
        assert_eq!(ProtocolMode::default(), ProtocolMode::V311);
    }
}
