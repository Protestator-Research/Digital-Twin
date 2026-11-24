//! Minimal MQTT codec for exactly the packets the broker needs, for MQTT 3.1.1 (protocol level 4) and
//! MQTT 5 (level 5), with QoS 0, 1 and 2.
//!
//! Decoded: CONNECT, PUBLISH (any QoS, with packet identifier and DUP flag), PUBACK, PUBREC, PUBREL,
//! PUBCOMP, SUBSCRIBE (with the requested QoS of every filter), UNSUBSCRIBE, PINGREQ, DISCONNECT. All other
//! packet types are reported as [`Packet::Ignored`]. Encoded: CONNACK, PUBLISH (any QoS), PUBACK, PUBREC,
//! PUBREL, PUBCOMP, SUBACK, UNSUBACK, PINGRESP and, in MQTT 5 only, the server-sent DISCONNECT.
//! The version of a connection is known after its CONNECT ([`Connect::protocol`]); every later `decode` and
//! `encode_*` call takes it. In MQTT 5 the properties of CONNECT (Session Expiry Interval, Receive Maximum)
//! and DISCONNECT (Session Expiry Interval) are walked with the wire type of every property and the numeric
//! ones the broker acts on are reported; the properties of all other incoming packets are validated for
//! length and skipped. Outgoing packets carry no properties except in CONNACK. Of a CONNECT the will is
//! validated and skipped; user name and password are decoded ([`Connect::username`], [`Connect::password`])
//! for the authentication of the session.
//!
//! The size check happens on the fixed header, before the body is buffered.

use bytes::{Buf, BufMut, Bytes, BytesMut};
use thiserror::Error;

pub use super::protocol::{ProtocolVersion, QoS};

/// CONNACK return codes (MQTT 3.1.1, 3.2.2.3) and reason codes (MQTT 5, 3.2.2.2) used by the broker.
pub mod connack {
    use super::ProtocolVersion;

    pub const ACCEPTED: u8 = 0x00;
    /// Client identifier is correct UTF-8 but not allowed (here: empty id without clean session).
    pub const IDENTIFIER_REJECTED: u8 = 0x02;
    /// The server does not support the protocol level requested by the client.
    pub const UNACCEPTABLE_PROTOCOL_VERSION: u8 = 0x01;
    /// MQTT 5: Unsupported Protocol Version.
    pub const V5_UNSUPPORTED_PROTOCOL_VERSION: u8 = 0x84;
    /// MQTT 5: Client Identifier not valid.
    pub const V5_CLIENT_IDENTIFIER_NOT_VALID: u8 = 0x85;
    /// MQTT 3.1.1: Server unavailable (here: the authentication backend cannot be reached).
    pub const SERVER_UNAVAILABLE: u8 = 0x03;
    /// MQTT 3.1.1: Bad user name or password.
    pub const BAD_USERNAME_OR_PASSWORD: u8 = 0x04;
    /// MQTT 5: Bad User Name or Password.
    pub const V5_BAD_USERNAME_OR_PASSWORD: u8 = 0x86;
    /// MQTT 5: Server unavailable.
    pub const V5_SERVER_UNAVAILABLE: u8 = 0x88;
    /// MQTT 5: Quota exceeded (here: too many sessions that outlive their connection).
    pub const V5_QUOTA_EXCEEDED: u8 = 0x97;

    /// Code that refuses a CONNECT because no further persistent session may be created. MQTT 3.1.1 has no
    /// better code than "server unavailable".
    pub fn quota_exceeded(version: ProtocolVersion) -> u8 {
        match version {
            ProtocolVersion::V311 => SERVER_UNAVAILABLE,
            ProtocolVersion::V5 => V5_QUOTA_EXCEEDED,
        }
    }

    /// Code that refuses a CONNECT because of missing or wrong credentials.
    pub fn bad_credentials(version: ProtocolVersion) -> u8 {
        match version {
            ProtocolVersion::V311 => BAD_USERNAME_OR_PASSWORD,
            ProtocolVersion::V5 => V5_BAD_USERNAME_OR_PASSWORD,
        }
    }

    /// Code that refuses a CONNECT because the authentication backend is not available.
    pub fn server_unavailable(version: ProtocolVersion) -> u8 {
        match version {
            ProtocolVersion::V311 => SERVER_UNAVAILABLE,
            ProtocolVersion::V5 => V5_SERVER_UNAVAILABLE,
        }
    }

    /// Code that refuses a CONNECT because its protocol version is not served, in the format of that version.
    pub fn unsupported_protocol(version: ProtocolVersion) -> u8 {
        match version {
            ProtocolVersion::V311 => UNACCEPTABLE_PROTOCOL_VERSION,
            ProtocolVersion::V5 => V5_UNSUPPORTED_PROTOCOL_VERSION,
        }
    }

    /// Code that refuses a CONNECT because of its client identifier.
    pub fn identifier_rejected(version: ProtocolVersion) -> u8 {
        match version {
            ProtocolVersion::V311 => IDENTIFIER_REJECTED,
            ProtocolVersion::V5 => V5_CLIENT_IDENTIFIER_NOT_VALID,
        }
    }
}

/// SUBACK return codes (MQTT 3.1.1, 3.9.3) and reason codes (MQTT 5, 3.9.3).
pub mod suback {
    pub const GRANTED_QOS_0: u8 = 0x00;
    pub const GRANTED_QOS_1: u8 = 0x01;
    pub const GRANTED_QOS_2: u8 = 0x02;
    /// MQTT 3.1.1 failure; MQTT 5 uses the more specific codes below.
    pub const FAILURE: u8 = 0x80;
    /// MQTT 5: Topic Filter invalid.
    pub const V5_TOPIC_FILTER_INVALID: u8 = 0x8F;
    /// MQTT 5: Quota exceeded.
    pub const V5_QUOTA_EXCEEDED: u8 = 0x97;
    /// MQTT 5: Shared Subscriptions not supported.
    pub const V5_SHARED_NOT_SUPPORTED: u8 = 0x9E;
}

/// UNSUBACK reason codes (MQTT 5, 3.11.3). MQTT 3.1.1 has none.
pub mod unsuback {
    pub const SUCCESS: u8 = 0x00;
    pub const NO_SUBSCRIPTION_EXISTED: u8 = 0x11;
}

/// DISCONNECT reason codes (MQTT 5, 3.14.2.1) the broker sends.
pub mod disconnect {
    pub const MALFORMED_PACKET: u8 = 0x81;
    pub const PROTOCOL_ERROR: u8 = 0x82;
    pub const SERVER_SHUTTING_DOWN: u8 = 0x8B;
    pub const KEEP_ALIVE_TIMEOUT: u8 = 0x8D;
    /// Another connection with the same client identifier took over the session.
    pub const SESSION_TAKEN_OVER: u8 = 0x8E;
    pub const TOPIC_NAME_INVALID: u8 = 0x90;
    pub const PACKET_TOO_LARGE: u8 = 0x95;
    /// The client has more unacknowledged QoS 1/2 PUBLISH packets in flight than the broker allows.
    pub const RECEIVE_MAXIMUM_EXCEEDED: u8 = 0x93;
}

/// Reason codes (MQTT 5, 3.4.2.1, 3.5.2.1, 3.6.2.1, 3.7.2.1) shared by PUBACK, PUBREC, PUBREL and PUBCOMP.
/// MQTT 3.1.1 has none.
pub mod puback {
    pub const SUCCESS: u8 = 0x00;
    pub const PACKET_IDENTIFIER_IN_USE: u8 = 0x91;
    pub const PACKET_IDENTIFIER_NOT_FOUND: u8 = 0x92;
    pub const QUOTA_EXCEEDED: u8 = 0x97;
}

#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum DecodeError {
    #[error("packet of {size} bytes exceeds the maximum packet size of {max} bytes")]
    PacketTooLarge { size: usize, max: usize },
    #[error("malformed packet: {0}")]
    Malformed(&'static str),
    /// Protocol name is not `MQTT` (e.g. MQTT 3.1 `MQIsdp`): the connection is just closed.
    #[error("unsupported protocol (only MQTT 3.1.1 and 5 are supported)")]
    UnsupportedProtocol,
    /// Protocol name `MQTT` with a level other than 4 and 5 (e.g. 3.1 style level 3): CONNACK 0x01, then
    /// close ([MQTT-3.1.2-2]).
    #[error(
        "unsupported protocol level (only MQTT 3.1.1, level 4, and MQTT 5, level 5, are supported)"
    )]
    UnacceptableProtocolLevel,
    /// A packet that is not allowed here (MQTT 5 AUTH: there is no enhanced authentication): DISCONNECT 0x82.
    #[error("protocol error: {0}")]
    ProtocolError(&'static str),
}

#[derive(Clone, PartialEq, Eq)]
pub struct Connect {
    pub client_id: String,
    pub keep_alive: u16,
    /// Clean Session (3.1.1) or Clean Start (MQTT 5).
    pub clean_session: bool,
    pub protocol: ProtocolVersion,
    /// User name of the CONNECT, if the flag was set.
    pub username: Option<String>,
    /// Password of the CONNECT (binary data), if the flag was set. Never log it.
    pub password: Option<Bytes>,
    /// The raw MQTT 5 Session Expiry Interval in seconds (property 0x11): 0 when absent and always 0 in
    /// 3.1.1. The session layer derives the effective expiry of a 3.1.1 client from `clean_session`.
    pub session_expiry: u32,
    /// MQTT 5 Receive Maximum (property 0x21): how many unacknowledged QoS 1/2 PUBLISH packets the client
    /// accepts from the broker; 65535 if absent and always in 3.1.1.
    pub receive_maximum: u16,
    /// MQTT 5 Maximum Packet Size the client accepts (property 0x27); `None` when absent and always in
    /// 3.1.1. The value 0 is a protocol error.
    pub maximum_packet_size: Option<u32>,
}

impl std::fmt::Debug for Connect {
    /// Without the password (only whether there is one).
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Connect")
            .field("client_id", &self.client_id)
            .field("keep_alive", &self.keep_alive)
            .field("clean_session", &self.clean_session)
            .field("protocol", &self.protocol)
            .field("username", &self.username)
            .field("password", &self.password.as_ref().map(|_| "<redacted>"))
            .field("session_expiry", &self.session_expiry)
            .field("receive_maximum", &self.receive_maximum)
            .field("maximum_packet_size", &self.maximum_packet_size)
            .finish()
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Publish {
    pub topic: String,
    pub payload: Bytes,
    pub qos: QoS,
    /// Packet identifier, `Some` exactly for QoS 1 and 2 (never 0).
    pub packet_id: Option<u16>,
    /// DUP flag: the sender repeats an earlier attempt to send this packet (only possible above QoS 0).
    pub dup: bool,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SubscribeEntry {
    pub filter: String,
    /// MQTT 5 No Local option: do not deliver the client's own publishes back to it. Always false in 3.1.1.
    pub no_local: bool,
    /// QoS the client asks for (bits 0-1 of the subscription options, both versions).
    pub qos: QoS,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Packet {
    Connect(Connect),
    Publish(Publish),
    Subscribe {
        packet_id: u16,
        entries: Vec<SubscribeEntry>,
    },
    Unsubscribe {
        packet_id: u16,
        filters: Vec<String>,
    },
    PingReq,
    Disconnect {
        /// MQTT 5 Session Expiry Interval (property 0x11) of the DISCONNECT, `None` if absent or in 3.1.1.
        session_expiry: Option<u32>,
    },
    /// PUBACK: acknowledges a QoS 1 PUBLISH. `reason` is 0 in 3.1.1 and when MQTT 5 omits it.
    PubAck {
        packet_id: u16,
        reason: u8,
    },
    /// PUBREC: first answer to a QoS 2 PUBLISH.
    PubRec {
        packet_id: u16,
        reason: u8,
    },
    /// PUBREL: release of a QoS 2 message, answer to PUBREC.
    PubRel {
        packet_id: u16,
        reason: u8,
    },
    /// PUBCOMP: end of the QoS 2 handshake, answer to PUBREL.
    PubComp {
        packet_id: u16,
        reason: u8,
    },
    /// A well formed packet of a type the broker does not act on (SUBACK, ...).
    Ignored(u8),
}

const CONNECT: u8 = 1;
const PUBLISH: u8 = 3;
const PUBACK: u8 = 4;
const PUBREC: u8 = 5;
const PUBREL: u8 = 6;
const PUBCOMP: u8 = 7;
const SUBSCRIBE: u8 = 8;
const UNSUBSCRIBE: u8 = 10;
const PINGREQ: u8 = 12;
const DISCONNECT: u8 = 14;

/// Tries to decode one packet from the front of `buf`. Returns `Ok(None)` if more bytes are needed;
/// nothing is consumed in that case. `max_packet_size` is checked as soon as the length is known.
/// `version` is the version negotiated by the CONNECT of this connection ([`ProtocolVersion::V311`] before
/// it); a CONNECT itself carries its own level and is decoded independently of `version`.
pub fn decode(
    buf: &mut BytesMut,
    max_packet_size: u32,
    version: ProtocolVersion,
) -> Result<Option<Packet>, DecodeError> {
    if buf.is_empty() {
        return Ok(None);
    }
    let first = buf[0];
    let mut remaining = 0usize;
    let mut header_len = 0usize;
    for i in 0..4 {
        let Some(&b) = buf.get(1 + i) else {
            return Ok(None);
        };
        remaining |= usize::from(b & 0x7F) << (7 * i);
        if b & 0x80 == 0 {
            header_len = 2 + i;
            break;
        }
    }
    if header_len == 0 {
        return Err(DecodeError::Malformed("remaining length"));
    }
    let total = header_len + remaining;
    let max = max_packet_size as usize;
    if total > max {
        return Err(DecodeError::PacketTooLarge { size: total, max });
    }
    if buf.len() < total {
        buf.reserve(total - buf.len());
        return Ok(None);
    }
    let mut frame = buf.split_to(total).freeze();
    frame.advance(header_len);
    decode_body(first, frame, version).map(Some)
}

fn decode_body(first: u8, body: Bytes, version: ProtocolVersion) -> Result<Packet, DecodeError> {
    let v5 = version == ProtocolVersion::V5;
    let kind = first >> 4;
    let flags = first & 0x0F;
    let mut r = Reader(body);
    let required_flags = match kind {
        CONNECT | PINGREQ | DISCONNECT | PUBACK | PUBREC | PUBCOMP => Some(0),
        SUBSCRIBE | UNSUBSCRIBE | PUBREL => Some(0b0010),
        _ => None,
    };
    if required_flags.is_some_and(|f| f != flags) {
        return Err(DecodeError::Malformed("reserved fixed header flags"));
    }
    match kind {
        CONNECT => decode_connect(&mut r),
        PUBLISH => {
            // [MQTT-3.3.1-2]: DUP must be 0 for QoS 0
            if flags & 0b1000 != 0 && (flags >> 1) & 0b11 == 0 {
                return Err(DecodeError::Malformed("DUP flag on a QoS 0 PUBLISH"));
            }
            let qos = QoS::from_bits((flags >> 1) & 0b11).ok_or(DecodeError::Malformed("QoS 3"))?;
            let topic = r.string()?;
            // the packet identifier follows the topic, before the MQTT 5 properties (MQTT 3.3.2)
            let packet_id = if qos == QoS::AtMostOnce {
                None
            } else {
                match r.u16()? {
                    0 => return Err(DecodeError::Malformed("packet identifier 0")),
                    id => Some(id),
                }
            };
            if v5 {
                r.skip_properties()?;
            }
            Ok(Packet::Publish(Publish {
                topic,
                payload: r.0,
                qos,
                packet_id,
                dup: flags & 0b1000 != 0,
            }))
        }
        PUBACK | PUBREC | PUBREL | PUBCOMP => {
            let (packet_id, reason) = decode_ack(&mut r, v5)?;
            Ok(match kind {
                PUBACK => Packet::PubAck { packet_id, reason },
                PUBREC => Packet::PubRec { packet_id, reason },
                PUBREL => Packet::PubRel { packet_id, reason },
                _ => Packet::PubComp { packet_id, reason },
            })
        }
        SUBSCRIBE => {
            let packet_id = r.u16()?;
            if v5 {
                r.skip_properties()?;
            }
            let mut entries = Vec::new();
            while r.0.has_remaining() {
                let filter = r.string()?;
                let options = r.u8()?;
                // both versions: bits 0-1 are the requested QoS; 3 is rejected below
                let qos = QoS::from_bits(options & 0b11)
                    .ok_or(DecodeError::Malformed("subscription QoS 3"))?;
                let no_local = if v5 {
                    // MQTT 5 subscription options: bits 0-1 QoS (3 is invalid), 2 No Local, 3 Retain As
                    // Published, 4-5 Retain Handling (3 is invalid), 6-7 reserved
                    if options & 0xC0 != 0
                        || options & 0b11 == 0b11
                        || (options >> 4) & 0b11 == 0b11
                    {
                        return Err(DecodeError::Malformed("subscription options"));
                    }
                    options & 0b100 != 0
                } else {
                    // 3.1.1: only bits 0-1 (requested QoS) are used, QoS 3 and any of bits 2-7 are malformed
                    if options & 0xFC != 0 || options & 0b11 == 0b11 {
                        return Err(DecodeError::Malformed("requested QoS byte"));
                    }
                    false
                };
                entries.push(SubscribeEntry {
                    filter,
                    no_local,
                    qos,
                });
            }
            if entries.is_empty() {
                return Err(DecodeError::Malformed("SUBSCRIBE without filters"));
            }
            Ok(Packet::Subscribe { packet_id, entries })
        }
        UNSUBSCRIBE => {
            let packet_id = r.u16()?;
            if v5 {
                r.skip_properties()?;
            }
            let mut filters = Vec::new();
            while r.0.has_remaining() {
                filters.push(r.string()?);
            }
            if filters.is_empty() {
                return Err(DecodeError::Malformed("UNSUBSCRIBE without filters"));
            }
            Ok(Packet::Unsubscribe { packet_id, filters })
        }
        PINGREQ => Ok(Packet::PingReq),
        DISCONNECT => {
            let mut session_expiry = None;
            if v5 && r.0.has_remaining() {
                // optional reason code (the broker does not act on it), then optional properties
                r.u8()?;
                if r.0.has_remaining() {
                    r.read_properties(|id, value| {
                        if id == PROP_SESSION_EXPIRY_INTERVAL
                            && session_expiry.replace(value).is_some()
                        {
                            return Err(DecodeError::ProtocolError(
                                "duplicate Session Expiry Interval",
                            ));
                        }
                        Ok(())
                    })?;
                }
                if r.0.has_remaining() {
                    return Err(DecodeError::Malformed(
                        "data after the DISCONNECT properties",
                    ));
                }
            }
            Ok(Packet::Disconnect { session_expiry })
        }
        // 15 is AUTH in MQTT 5; the broker has no enhanced authentication, so it is a protocol error there
        15 if v5 => Err(DecodeError::ProtocolError(
            "AUTH without enhanced authentication",
        )),
        0 | 15 => Err(DecodeError::Malformed("reserved packet type")),
        other => Ok(Packet::Ignored(other)),
    }
}

/// Body of PUBACK, PUBREC, PUBREL and PUBCOMP: packet identifier, and in MQTT 5 an optional reason code
/// (absent means success) followed by optional properties (MQTT 3.4.2, 3.5.2, 3.6.2, 3.7.2).
fn decode_ack(r: &mut Reader, v5: bool) -> Result<(u16, u8), DecodeError> {
    let packet_id = match r.u16()? {
        0 => return Err(DecodeError::Malformed("packet identifier 0")),
        id => id,
    };
    let mut reason = 0;
    if v5 && r.0.has_remaining() {
        reason = r.u8()?;
        if r.0.has_remaining() {
            r.skip_properties()?;
        }
    }
    if r.0.has_remaining() {
        return Err(DecodeError::Malformed("data after the acknowledgement"));
    }
    Ok((packet_id, reason))
}

fn decode_connect(r: &mut Reader) -> Result<Packet, DecodeError> {
    let name = r.string().map_err(|_| DecodeError::UnsupportedProtocol)?;
    if name != "MQTT" {
        return Err(DecodeError::UnsupportedProtocol);
    }
    let protocol = match r.u8()? {
        4 => ProtocolVersion::V311,
        5 => ProtocolVersion::V5,
        _ => return Err(DecodeError::UnacceptableProtocolLevel),
    };
    let flags = r.u8()?;
    if flags & 1 != 0 {
        return Err(DecodeError::Malformed("reserved connect flag"));
    }
    let will_flag = flags & 0b100 != 0;
    let will_qos = (flags >> 3) & 0b11;
    let will_retain = flags & 0b10_0000 != 0;
    // [MQTT-3.1.2-14]: will QoS 3 is invalid; [MQTT-3.1.2-13/-15]: will QoS and will retain must be 0
    // without will flag; [MQTT-3.1.2-22]: a password needs a user name.
    if will_qos == 3 {
        return Err(DecodeError::Malformed("will QoS 3"));
    }
    if !will_flag && (will_qos != 0 || will_retain) {
        return Err(DecodeError::Malformed(
            "will QoS or will retain without will flag",
        ));
    }
    // [MQTT-3.1.2-22] is 3.1.1 only; MQTT 5 allows a password without user name
    if protocol == ProtocolVersion::V311 && flags & 0b1000_0000 == 0 && flags & 0b0100_0000 != 0 {
        return Err(DecodeError::Malformed(
            "password flag without user name flag",
        ));
    }
    let keep_alive = r.u16()?;
    let mut session_expiry = 0u32;
    let mut receive_maximum = u16::MAX;
    let mut maximum_packet_size = None;
    if protocol == ProtocolVersion::V5 {
        // CONNECT properties: Session Expiry Interval and Receive Maximum are used, each at most once
        // (MQTT 3.1.2.11.2, 3.1.2.11.3); all others are validated and ignored.
        let (mut seen_expiry, mut seen_receive) = (false, false);
        r.read_properties(|id, value| {
            match id {
                PROP_SESSION_EXPIRY_INTERVAL => {
                    if std::mem::replace(&mut seen_expiry, true) {
                        return Err(DecodeError::ProtocolError(
                            "duplicate Session Expiry Interval",
                        ));
                    }
                    session_expiry = value;
                }
                PROP_RECEIVE_MAXIMUM => {
                    if std::mem::replace(&mut seen_receive, true) {
                        return Err(DecodeError::ProtocolError("duplicate Receive Maximum"));
                    }
                    // the value is a u16 on the wire; 0 is a protocol error
                    receive_maximum = u16::try_from(value)
                        .ok()
                        .filter(|&v| v != 0)
                        .ok_or(DecodeError::ProtocolError("Receive Maximum 0"))?;
                }
                PROP_MAXIMUM_PACKET_SIZE => {
                    if value == 0 {
                        return Err(DecodeError::ProtocolError("Maximum Packet Size 0"));
                    }
                    if maximum_packet_size.replace(value).is_some() {
                        return Err(DecodeError::ProtocolError("duplicate Maximum Packet Size"));
                    }
                }
                _ => {}
            }
            Ok(())
        })?;
    }
    let client_id = r.string()?;
    if will_flag {
        if protocol == ProtocolVersion::V5 {
            r.skip_properties()?;
        }
        r.string()?; // will topic
        r.binary()?; // will payload
    }
    let username = if flags & 0b1000_0000 != 0 {
        Some(r.string()?)
    } else {
        None
    };
    let password = if flags & 0b0100_0000 != 0 {
        Some(r.binary()?)
    } else {
        None
    };
    if r.0.has_remaining() {
        return Err(DecodeError::Malformed("data after the CONNECT payload"));
    }
    Ok(Packet::Connect(Connect {
        client_id,
        keep_alive,
        clean_session: flags & 0b10 != 0,
        protocol,
        username,
        password,
        session_expiry,
        receive_maximum,
        maximum_packet_size,
    }))
}

struct Reader(Bytes);

impl Reader {
    fn u8(&mut self) -> Result<u8, DecodeError> {
        if self.0.remaining() < 1 {
            return Err(DecodeError::Malformed("truncated"));
        }
        Ok(self.0.get_u8())
    }

    fn u16(&mut self) -> Result<u16, DecodeError> {
        if self.0.remaining() < 2 {
            return Err(DecodeError::Malformed("truncated"));
        }
        Ok(self.0.get_u16())
    }

    /// Variable byte integer (at most 4 bytes).
    fn varint(&mut self) -> Result<usize, DecodeError> {
        let mut value = 0usize;
        for i in 0..4 {
            let b = self.u8()?;
            value |= usize::from(b & 0x7F) << (7 * i);
            if b & 0x80 == 0 {
                return Ok(value);
            }
        }
        Err(DecodeError::Malformed("variable byte integer"))
    }

    /// Skips an MQTT 5 property block (length prefix and content). Only the length is validated.
    fn skip_properties(&mut self) -> Result<(), DecodeError> {
        let len = self.varint()?;
        if self.0.remaining() < len {
            return Err(DecodeError::Malformed("truncated properties"));
        }
        self.0.advance(len);
        Ok(())
    }

    /// Walks an MQTT 5 property block (length prefix and content). Every property id has a fixed wire type
    /// (MQTT 2.2.2.2); numeric properties (byte, two byte, four byte integers and variable byte integers) are
    /// reported to `on` as `(id, value)`, strings, binary data and user properties are only skipped. An
    /// unknown id or a value running over the end of the block is malformed.
    fn read_properties(
        &mut self,
        mut on: impl FnMut(u8, u32) -> Result<(), DecodeError>,
    ) -> Result<(), DecodeError> {
        let len = self.varint()?;
        if self.0.remaining() < len {
            return Err(DecodeError::Malformed("truncated properties"));
        }
        let mut block = Reader(self.0.split_to(len));
        while block.0.has_remaining() {
            let id = block.u8()?;
            match id {
                // payload format, request problem information, request response information, maximum QoS,
                // retain available, wildcard / subscription identifier / shared subscription available
                0x01 | 0x17 | 0x19 | 0x24 | 0x25 | 0x28 | 0x29 | 0x2A => {
                    on(id, u32::from(block.u8()?))?
                }
                // server keep alive, receive maximum, topic alias maximum, topic alias
                0x13 | 0x21 | 0x22 | 0x23 => on(id, u32::from(block.u16()?))?,
                // message expiry, session expiry, will delay, maximum packet size
                0x02 | 0x11 | 0x18 | 0x27 => {
                    if block.0.remaining() < 4 {
                        return Err(DecodeError::Malformed("truncated property"));
                    }
                    on(id, block.0.get_u32())?
                }
                // subscription identifier
                0x0B => {
                    let value = block.varint()?;
                    on(id, value as u32)?
                }
                // UTF-8 strings: content type, response topic, authentication method, assigned client id,
                // response information, server reference, reason string
                0x03 | 0x08 | 0x12 | 0x15 | 0x1A | 0x1C | 0x1F => {
                    block.string()?;
                }
                // correlation data, authentication data: binary
                0x09 | 0x16 => {
                    block.binary()?;
                }
                // user property: a pair of UTF-8 strings
                0x26 => {
                    block.string()?;
                    block.string()?;
                }
                _ => return Err(DecodeError::Malformed("unknown property")),
            }
        }
        Ok(())
    }

    /// Binary data: two byte length, then the bytes.
    fn binary(&mut self) -> Result<Bytes, DecodeError> {
        let len = usize::from(self.u16()?);
        if self.0.remaining() < len {
            return Err(DecodeError::Malformed("truncated binary data"));
        }
        Ok(self.0.split_to(len))
    }

    fn string(&mut self) -> Result<String, DecodeError> {
        let len = usize::from(self.u16()?);
        if self.0.remaining() < len {
            return Err(DecodeError::Malformed("truncated string"));
        }
        let raw = self.0.split_to(len);
        let text =
            String::from_utf8(raw.to_vec()).map_err(|_| DecodeError::Malformed("invalid UTF-8"))?;
        // MQTT 3.1.1 1.5.3 / MQTT 5 1.5.4 [MQTT-1.5.4-2]: U+0000 must not be in a UTF-8 string. PostgreSQL TEXT
        // cannot hold it either, so the session store would refuse the row.
        if text.contains('\0') {
            return Err(DecodeError::Malformed("U+0000 in a UTF-8 string"));
        }
        Ok(text)
    }
}

fn put_varint(out: &mut BytesMut, mut value: usize) {
    loop {
        let mut b = (value % 128) as u8;
        value /= 128;
        if value > 0 {
            b |= 0x80;
        }
        out.put_u8(b);
        if value == 0 {
            break;
        }
    }
}

fn put_fixed_header(out: &mut BytesMut, first: u8, remaining: usize) {
    out.put_u8(first);
    put_varint(out, remaining);
}

/// CONNACK that refuses the connection: session_present = false, the given return code (3.1.1) or reason
/// code (MQTT 5) and, in MQTT 5, no properties.
pub fn encode_connack(version: ProtocolVersion, code: u8) -> Bytes {
    match version {
        ProtocolVersion::V311 => Bytes::copy_from_slice(&[0x20, 0x02, 0x00, code]),
        ProtocolVersion::V5 => Bytes::copy_from_slice(&[0x20, 0x03, 0x00, code, 0x00]),
    }
}

const PROP_SESSION_EXPIRY_INTERVAL: u8 = 0x11;
const PROP_ASSIGNED_CLIENT_IDENTIFIER: u8 = 0x12;
const PROP_RECEIVE_MAXIMUM: u8 = 0x21;
const PROP_RETAIN_AVAILABLE: u8 = 0x25;
const PROP_MAXIMUM_PACKET_SIZE: u8 = 0x27;
const PROP_SUBSCRIPTION_IDENTIFIERS_AVAILABLE: u8 = 0x29;
const PROP_SHARED_SUBSCRIPTION_AVAILABLE: u8 = 0x2A;

/// What a CONNACK that accepts the connection announces.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ConnackAccepted<'a> {
    /// An earlier session of the client is continued.
    pub session_present: bool,
    pub max_packet_size: u32,
    /// Receive Maximum the broker announces (MQTT 5 property 0x21).
    pub receive_maximum: u16,
    /// Session Expiry Interval the broker uses, announced (0x11) only when `Some` (it differs from what the
    /// client asked for).
    pub session_expiry: Option<u32>,
    /// The identifier the broker made up if the client sent an empty one.
    pub assigned_client_id: Option<&'a str>,
}

/// CONNACK that accepts the connection. In MQTT 5 it announces what the broker does and does not do:
/// Receive Maximum, Retain Available 0, the maximum packet size, Subscription Identifiers and Shared
/// Subscriptions not available, the Session Expiry Interval if the broker changed it, and the assigned
/// client identifier. Maximum QoS is not sent: absent means QoS 2 (MQTT 3.2.2.3.4).
pub fn encode_connack_accepted(version: ProtocolVersion, info: &ConnackAccepted<'_>) -> Bytes {
    if version == ProtocolVersion::V311 {
        return Bytes::copy_from_slice(&[
            0x20,
            0x02,
            u8::from(info.session_present),
            connack::ACCEPTED,
        ]);
    }
    let mut props = BytesMut::with_capacity(32);
    props.put_u8(PROP_RECEIVE_MAXIMUM);
    props.put_u16(info.receive_maximum);
    props.put_slice(&[PROP_RETAIN_AVAILABLE, 0]);
    props.put_u8(PROP_MAXIMUM_PACKET_SIZE);
    props.put_u32(info.max_packet_size);
    props.put_slice(&[
        PROP_SUBSCRIPTION_IDENTIFIERS_AVAILABLE,
        0,
        PROP_SHARED_SUBSCRIPTION_AVAILABLE,
        0,
    ]);
    if let Some(expiry) = info.session_expiry {
        props.put_u8(PROP_SESSION_EXPIRY_INTERVAL);
        props.put_u32(expiry);
    }
    if let Some(id) = info.assigned_client_id {
        // the id is generated by the broker and short; a longer one is simply not announced
        if let Ok(len) = u16::try_from(id.len()) {
            props.put_u8(PROP_ASSIGNED_CLIENT_IDENTIFIER);
            props.put_u16(len);
            props.put_slice(id.as_bytes());
        }
    }
    let remaining = 2 + varint_len(props.len()) + props.len();
    let mut out = BytesMut::with_capacity(1 + 4 + remaining);
    put_fixed_header(&mut out, 0x20, remaining);
    out.put_slice(&[u8::from(info.session_present), connack::ACCEPTED]);
    put_varint(&mut out, props.len());
    out.put_slice(&props);
    out.freeze()
}

fn varint_len(value: usize) -> usize {
    match value {
        0..=127 => 1,
        128..=16_383 => 2,
        16_384..=2_097_151 => 3,
        _ => 4,
    }
}

/// Largest value of the remaining length field.
const MAX_REMAINING_LENGTH: usize = 268_435_455;

/// Bytes between the topic and packet id and the payload (or reason codes) of PUBLISH and SUBACK:
/// nothing in 3.1.1, the empty property block in MQTT 5.
fn properties_len(version: ProtocolVersion) -> usize {
    match version {
        ProtocolVersion::V311 => 0,
        ProtocolVersion::V5 => 1,
    }
}

/// Size in bytes of an encoded PUBLISH, `None` if it cannot be encoded at all. Above QoS 0 the packet
/// identifier adds two bytes.
pub fn publish_encoded_len(
    version: ProtocolVersion,
    topic_len: usize,
    payload_len: usize,
    qos: QoS,
) -> Option<usize> {
    // the topic length is a 16 bit field
    if topic_len > usize::from(u16::MAX) {
        return None;
    }
    let remaining = 2usize
        .checked_add(topic_len)?
        .checked_add(if qos == QoS::AtMostOnce { 0 } else { 2 })?
        .checked_add(properties_len(version))?
        .checked_add(payload_len)?;
    (remaining <= MAX_REMAINING_LENGTH).then(|| 1 + varint_len(remaining) + remaining)
}

/// Appends a PUBLISH to `out`. The caller makes sure the packet fits (see [`publish_encoded_len`]).
/// `packet_id` is written after the topic for QoS 1 and 2 and ignored for QoS 0; `dup` marks a
/// retransmission. The retain flag is never set.
pub fn encode_publish(
    version: ProtocolVersion,
    out: &mut BytesMut,
    topic: &str,
    payload: &[u8],
    qos: QoS,
    packet_id: u16,
    dup: bool,
) {
    let props = properties_len(version);
    let id_len = if qos == QoS::AtMostOnce { 0 } else { 2 };
    let remaining = 2 + topic.len() + id_len + props + payload.len();
    out.reserve(5 + remaining);
    // DUP is only valid above QoS 0 [MQTT-3.3.1-2]
    let dup = dup && qos != QoS::AtMostOnce;
    debug_assert!(
        qos == QoS::AtMostOnce || packet_id != 0,
        "packet identifier 0 is invalid"
    );
    put_fixed_header(out, 0x30 | u8::from(dup) << 3 | qos.bits() << 1, remaining);
    out.put_u16(topic.len() as u16);
    out.put_slice(topic.as_bytes());
    if id_len > 0 {
        out.put_u16(packet_id);
    }
    if props > 0 {
        out.put_u8(0); // property length 0
    }
    out.put_slice(payload);
}

/// PUBACK, PUBREC, PUBREL or PUBCOMP with the given first byte. 3.1.1 only carries the packet identifier;
/// MQTT 5 adds the reason code unless it is 0x00 (success), and then needs no property length (it may be
/// left out when the remaining length is below 4).
fn encode_ack(version: ProtocolVersion, first: u8, packet_id: u16, reason: u8) -> Bytes {
    let id = packet_id.to_be_bytes();
    if version == ProtocolVersion::V5 && reason != 0 {
        Bytes::copy_from_slice(&[first, 3, id[0], id[1], reason])
    } else {
        Bytes::copy_from_slice(&[first, 2, id[0], id[1]])
    }
}

/// PUBACK ([`puback`] reason codes; `reason` is ignored in 3.1.1).
pub fn encode_puback(version: ProtocolVersion, packet_id: u16, reason: u8) -> Bytes {
    encode_ack(version, 0x40, packet_id, reason)
}

/// PUBREC, see [`encode_puback`].
pub fn encode_pubrec(version: ProtocolVersion, packet_id: u16, reason: u8) -> Bytes {
    encode_ack(version, 0x50, packet_id, reason)
}

/// PUBREL, see [`encode_puback`]. The fixed header flags are 0b0010 (MQTT 3.6.1).
pub fn encode_pubrel(version: ProtocolVersion, packet_id: u16, reason: u8) -> Bytes {
    encode_ack(version, 0x62, packet_id, reason)
}

/// PUBCOMP, see [`encode_puback`].
pub fn encode_pubcomp(version: ProtocolVersion, packet_id: u16, reason: u8) -> Bytes {
    encode_ack(version, 0x70, packet_id, reason)
}

/// SUBACK with one return code ([`suback`]) per requested filter.
pub fn encode_suback(version: ProtocolVersion, packet_id: u16, return_codes: &[u8]) -> Bytes {
    let props = properties_len(version);
    let mut out = BytesMut::with_capacity(9 + return_codes.len());
    put_fixed_header(&mut out, 0x90, 2 + props + return_codes.len());
    out.put_u16(packet_id);
    if props > 0 {
        out.put_u8(0);
    }
    out.put_slice(return_codes);
    out.freeze()
}

/// UNSUBACK. MQTT 5 carries one reason code ([`unsuback`]) per filter, 3.1.1 only the packet id.
pub fn encode_unsuback(version: ProtocolVersion, packet_id: u16, reason_codes: &[u8]) -> Bytes {
    let id = packet_id.to_be_bytes();
    match version {
        ProtocolVersion::V311 => Bytes::copy_from_slice(&[0xB0, 0x02, id[0], id[1]]),
        ProtocolVersion::V5 => {
            let mut out = BytesMut::with_capacity(9 + reason_codes.len());
            put_fixed_header(&mut out, 0xB0, 3 + reason_codes.len());
            out.put_slice(&id);
            out.put_u8(0); // property length 0
            out.put_slice(reason_codes);
            out.freeze()
        }
    }
}

/// Server-sent DISCONNECT with a reason code ([`disconnect`]). Only exists in MQTT 5.
pub fn encode_disconnect(reason: u8) -> Bytes {
    Bytes::copy_from_slice(&[0xE0, 0x02, reason, 0x00])
}

pub fn encode_pingresp() -> Bytes {
    Bytes::from_static(&[0xD0, 0x00])
}

#[cfg(test)]
mod tests {
    use super::*;

    const V5: ProtocolVersion = ProtocolVersion::V5;
    const V311: ProtocolVersion = ProtocolVersion::V311;
    const ALL_QOS: [QoS; 3] = [QoS::AtMostOnce, QoS::AtLeastOnce, QoS::ExactlyOnce];

    /// MQTT 3.1.1 decode, as before the connection has negotiated anything else.
    fn decode(buf: &mut BytesMut, max: u32) -> Result<Option<Packet>, DecodeError> {
        super::decode(buf, max, V311)
    }

    fn decode5(buf: &mut BytesMut, max: u32) -> Result<Option<Packet>, DecodeError> {
        super::decode(buf, max, V5)
    }

    fn buf(bytes: &[u8]) -> BytesMut {
        BytesMut::from(bytes)
    }

    fn connect_bytes(client_id: &str, keep_alive: u16) -> Vec<u8> {
        let mut body = vec![0, 4, b'M', b'Q', b'T', b'T', 4, 0b10];
        body.extend(keep_alive.to_be_bytes());
        body.extend((client_id.len() as u16).to_be_bytes());
        body.extend(client_id.as_bytes());
        let mut out = vec![0x10, body.len() as u8];
        out.extend(body);
        out
    }

    #[test]
    fn decode_connect_packet() {
        let mut b = buf(&connect_bytes("abc", 30));
        let p = decode(&mut b, 1024).unwrap().unwrap();
        assert_eq!(
            p,
            Packet::Connect(Connect {
                client_id: "abc".into(),
                keep_alive: 30,
                clean_session: true,
                protocol: V311,
                username: None,
                password: None,
                session_expiry: 0,
                receive_maximum: 65535,
                maximum_packet_size: None,
            })
        );
        assert!(b.is_empty());
    }

    #[test]
    fn connect_without_clean_session_and_empty_client_id() {
        // flags 0 = no clean session; the codec reports it, the session decides what to do with it
        let raw = [0x10, 12, 0, 4, b'M', b'Q', b'T', b'T', 4, 0, 0, 10, 0, 0];
        let p = decode(&mut buf(&raw), 1024).unwrap().unwrap();
        assert_eq!(
            p,
            Packet::Connect(Connect {
                client_id: String::new(),
                keep_alive: 10,
                clean_session: false,
                protocol: V311,
                username: None,
                password: None,
                session_expiry: 0,
                receive_maximum: 65535,
                maximum_packet_size: None,
            })
        );
    }

    /// CONNECT with arbitrary flags and payload (after the client id "a"), for the given protocol level.
    fn connect_with(level: u8, flags: u8, props: &[u8], payload: &[u8]) -> Vec<u8> {
        let mut body = vec![0, 4, b'M', b'Q', b'T', b'T', level, flags, 0, 5];
        if level == 5 {
            body.push(props.len() as u8);
            body.extend(props);
        }
        body.extend([0, 1, b'a']);
        body.extend(payload);
        let mut raw = vec![0x10, body.len() as u8];
        raw.extend(body);
        raw
    }

    #[test]
    fn connect_decodes_will_user_name_and_password() {
        // flags: user name, password, will flag, clean session; payload: will topic/message, user, pass
        let payload = [0, 1, b'w', 0, 1, b'm', 0, 1, b'u', 0, 2, b'p', 0xFF];
        let raw = connect_with(4, 0b1100_0110, &[], &payload);
        let p = decode(&mut buf(&raw), 1024).unwrap().unwrap();
        assert_eq!(
            p,
            Packet::Connect(Connect {
                client_id: "a".into(),
                keep_alive: 5,
                clean_session: true,
                protocol: V311,
                username: Some("u".into()),
                password: Some(Bytes::from_static(&[b'p', 0xFF])),
                session_expiry: 0,
                receive_maximum: 65535,
                maximum_packet_size: None,
            })
        );
    }

    #[test]
    fn connect_v5_decodes_will_properties_user_name_and_password() {
        // will properties: will delay interval (0x18, 4 bytes)
        let payload = [
            5, 0x18, 0, 0, 0, 1, 0, 1, b'w', 0, 1, b'm', 0, 3, b'a', b'b', b'c', 0, 1, b'p',
        ];
        let raw = connect_with(5, 0b1100_0110, &[0x11, 0, 0, 0, 60], &payload);
        let Packet::Connect(c) = decode5(&mut buf(&raw), 1024).unwrap().unwrap() else {
            panic!("not a CONNECT");
        };
        assert_eq!(c.protocol, V5);
        assert_eq!(c.session_expiry, 60);
        assert_eq!(c.username.as_deref(), Some("abc"));
        assert_eq!(c.password.as_deref(), Some(&b"p"[..]));
    }

    #[test]
    fn connect_user_name_without_password_and_empty_password() {
        let raw = connect_with(4, 0b1000_0010, &[], &[0, 1, b'u']);
        let Packet::Connect(c) = decode(&mut buf(&raw), 1024).unwrap().unwrap() else {
            panic!("not a CONNECT");
        };
        assert_eq!((c.username.as_deref(), c.password), (Some("u"), None));
        let raw = connect_with(4, 0b1100_0010, &[], &[0, 1, b'u', 0, 0]);
        let Packet::Connect(c) = decode(&mut buf(&raw), 1024).unwrap().unwrap() else {
            panic!("not a CONNECT");
        };
        assert_eq!(c.password, Some(Bytes::new()));
    }

    #[test]
    fn connect_with_truncated_or_extra_payload_fields_is_malformed() {
        for version in [4u8, 5] {
            let decode_v = |raw: Vec<u8>| super::decode(&mut buf(&raw), 1024, V311);
            // will flag, but no will topic; will topic without payload; user name flag without a user name
            for (flags, payload) in [
                (0b0000_0110u8, &[][..]),
                (0b0000_0110, &[0, 1, b'w'][..]),
                (0b0000_0110, &[0, 1, b'w', 0, 5, b'm'][..]),
                (0b1000_0010, &[][..]),
                (0b1000_0010, &[0, 3, b'u'][..]),
                (0b1100_0010, &[0, 1, b'u'][..]),
                (0b1100_0010, &[0, 1, b'u', 0, 4, b'p'][..]),
                // data behind the password
                (0b1100_0010, &[0, 1, b'u', 0, 1, b'p', 0][..]),
                // data although no user name / password flag
                (0b0000_0010, &[0, 1, b'u'][..]),
                // user name that is not UTF-8
                (0b1000_0010, &[0, 1, 0xFF][..]),
            ] {
                let raw = connect_with(version, flags, &[], payload);
                assert!(
                    matches!(decode_v(raw), Err(DecodeError::Malformed(_))),
                    "level {version} flags {flags:#010b} payload {payload:?}"
                );
            }
        }
    }

    #[test]
    fn connect_v5_allows_a_password_without_user_name() {
        let raw = connect_with(5, 0b0100_0010, &[], &[0, 1, b'p']);
        let Packet::Connect(c) = decode5(&mut buf(&raw), 1024).unwrap().unwrap() else {
            panic!("not a CONNECT");
        };
        assert_eq!(
            (c.username, c.password),
            (None, Some(Bytes::from_static(b"p")))
        );
        // 3.1.1 still refuses it
        let raw = connect_with(4, 0b0100_0010, &[], &[0, 1, b'p']);
        assert!(matches!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn connect_debug_does_not_show_the_password() {
        let raw = connect_with(
            4,
            0b1100_0010,
            &[],
            &[0, 1, b'u', 0, 6, b's', b'e', b'c', b'r', b'e', b't'],
        );
        let p = decode(&mut buf(&raw), 1024).unwrap().unwrap();
        let text = format!("{p:?}");
        assert!(!text.contains("secret") && !text.contains("115"), "{text}");
        assert!(text.contains("redacted") && text.contains("\"u\""));
    }

    #[test]
    fn connack_codes_for_credentials() {
        assert_eq!(connack::bad_credentials(V311), 0x04);
        assert_eq!(connack::bad_credentials(V5), 0x86);
        assert_eq!(connack::server_unavailable(V311), 0x03);
        assert_eq!(connack::server_unavailable(V5), 0x88);
    }

    #[test]
    fn connect_rejects_other_protocols() {
        // level 3 and level 6 with name "MQTT"
        for level in [3u8, 6] {
            let raw = [
                0x10, 12, 0, 4, b'M', b'Q', b'T', b'T', level, 2, 0, 10, 0, 0,
            ];
            assert_eq!(
                decode(&mut buf(&raw), 1024),
                Err(DecodeError::UnacceptableProtocolLevel),
                "level {level}"
            );
        }
        // MQTT 3.1 (level 3, name "MQIsdp"): wrong name
        let raw = [
            0x10, 14, 0, 6, b'M', b'Q', b'I', b's', b'd', b'p', 3, 2, 0, 10, 0, 0,
        ];
        assert_eq!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::UnsupportedProtocol)
        );
        // wrong protocol name
        let raw = [0x10, 12, 0, 4, b'X', b'Q', b'T', b'T', 4, 2, 0, 10, 0, 0];
        assert_eq!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::UnsupportedProtocol)
        );
    }

    #[test]
    fn connect_invalid_flag_combinations_are_malformed() {
        let with_flags = |flags: u8| {
            vec![
                0x10, 12, 0, 4, b'M', b'Q', b'T', b'T', 4, flags, 0, 10, 0, 0,
            ]
        };
        // will QoS 3 (with will flag), will QoS 1 / will retain without will flag, password without user name
        for flags in [0b0001_1110u8, 0b0000_1010, 0b0010_0010, 0b0100_0010] {
            assert!(
                matches!(
                    decode(&mut buf(&with_flags(flags)), 1024),
                    Err(DecodeError::Malformed(_))
                ),
                "flags {flags:#010b}"
            );
        }
        // valid: will flag with QoS 1 and retain; user name alone; user name and password
        let with_payload = |flags: u8, payload: &[u8]| {
            let mut raw = with_flags(flags);
            raw[1] += payload.len() as u8;
            raw.extend(payload);
            raw
        };
        let will = [0, 1, b'w', 0, 1, b'm'];
        for (flags, payload) in [
            (0b0010_1110u8, &will[..]),
            (0b1000_0010, &[0, 1, b'u'][..]),
            (0b1100_0010, &[0, 1, b'u', 0, 1, b'p'][..]),
        ] {
            assert!(
                matches!(
                    decode(&mut buf(&with_payload(flags, payload)), 1024),
                    Ok(Some(Packet::Connect(_)))
                ),
                "flags {flags:#010b}"
            );
        }
    }

    #[test]
    fn publish_with_dup_flag_at_qos0_is_malformed() {
        assert!(matches!(
            decode(&mut buf(&[0x38, 3, 0, 1, b'a']), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn connect_reserved_flag_is_malformed() {
        let raw = [0x10, 12, 0, 4, b'M', b'Q', b'T', b'T', 4, 3, 0, 10, 0, 0];
        assert!(matches!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn incomplete_input_consumes_nothing() {
        let full = connect_bytes("abc", 30);
        for cut in 0..full.len() {
            let mut b = buf(&full[..cut]);
            assert_eq!(decode(&mut b, 1024), Ok(None), "cut {cut}");
            assert_eq!(b.len(), cut);
        }
    }

    #[test]
    fn two_packets_in_one_buffer() {
        let mut raw = vec![0xC0, 0x00];
        raw.extend([0xE0, 0x00]);
        let mut b = buf(&raw);
        assert_eq!(decode(&mut b, 16), Ok(Some(Packet::PingReq)));
        assert_eq!(
            decode(&mut b, 16),
            Ok(Some(Packet::Disconnect {
                session_expiry: None
            }))
        );
        assert_eq!(decode(&mut b, 16), Ok(None));
    }

    #[test]
    fn publish_qos0() {
        // topic "a/b", payload "hi"
        let raw = [0x30, 7, 0, 3, b'a', b'/', b'b', b'h', b'i'];
        assert_eq!(
            decode(&mut buf(&raw), 1024),
            Ok(Some(Packet::Publish(Publish {
                topic: "a/b".into(),
                payload: Bytes::from_static(b"hi"),
                qos: QoS::AtMostOnce,
                packet_id: None,
                dup: false,
            })))
        );
    }

    #[test]
    fn publish_with_retain_flag() {
        // retain bit set; the payload starts right after the topic (there are no properties in 3.1.1)
        let raw = [0x31, 6, 0, 1, b'a', 2, 0x01, 1];
        let p = decode(&mut buf(&raw), 1024).unwrap().unwrap();
        assert_eq!(
            p,
            Packet::Publish(Publish {
                topic: "a".into(),
                payload: Bytes::from_static(&[2, 0x01, 1]),
                qos: QoS::AtMostOnce,
                packet_id: None,
                dup: false,
            })
        );
    }

    #[test]
    fn publish_qos1_and_qos2_carry_a_packet_id() {
        // 3.1.1: topic "a", packet id 0x0102, payload [0, 1]
        for (first, qos) in [(0x32u8, QoS::AtLeastOnce), (0x34, QoS::ExactlyOnce)] {
            let raw = [first, 7, 0, 1, b'a', 1, 2, 0, 1];
            assert_eq!(
                decode(&mut buf(&raw), 1024),
                Ok(Some(Packet::Publish(Publish {
                    topic: "a".into(),
                    payload: Bytes::from_static(&[0, 1]),
                    qos,
                    packet_id: Some(0x0102),
                    dup: false,
                }))),
                "flags {first:#x}"
            );
        }
        // the retain flag is still ignored
        assert!(matches!(
            decode(&mut buf(&[0x33, 5, 0, 1, b'a', 0, 9]), 64),
            Ok(Some(Packet::Publish(Publish {
                qos: QoS::AtLeastOnce,
                packet_id: Some(9),
                ..
            })))
        ));
    }

    #[test]
    fn publish_dup_is_allowed_above_qos0_and_packet_id_0_is_malformed() {
        for first in [0x3Au8, 0x3C] {
            let Ok(Some(Packet::Publish(p))) = decode(&mut buf(&[first, 5, 0, 1, b'a', 0, 4]), 64)
            else {
                panic!("not a PUBLISH");
            };
            assert!(p.dup && p.packet_id == Some(4), "flags {first:#x}");
        }
        for first in [0x32u8, 0x34] {
            assert!(
                matches!(
                    decode(&mut buf(&[first, 5, 0, 1, b'a', 0, 0]), 64),
                    Err(DecodeError::Malformed(_))
                ),
                "flags {first:#x}"
            );
            // packet id missing or cut in half
            for raw in [&[first, 3, 0, 1, b'a'][..], &[first, 4, 0, 1, b'a', 0]] {
                assert!(matches!(
                    decode(&mut buf(raw), 64),
                    Err(DecodeError::Malformed(_))
                ));
            }
        }
        // QoS 3, with and without DUP
        for first in [0x36u8, 0x3E] {
            assert!(matches!(
                decode(&mut buf(&[first, 5, 0, 1, b'a', 0, 1]), 64),
                Err(DecodeError::Malformed(_))
            ));
        }
    }

    #[test]
    fn subscribe_requested_qos() {
        // id 7, "a/#" qos 0, "b" qos 1, "c" qos 2
        let raw = [
            0x82, 16, 0, 7, 0, 3, b'a', b'/', b'#', 0, 0, 1, b'b', 1, 0, 1, b'c', 2,
        ];
        let p = decode(&mut buf(&raw), 1024).unwrap().unwrap();
        assert_eq!(
            p,
            Packet::Subscribe {
                packet_id: 7,
                entries: vec![
                    SubscribeEntry {
                        filter: "a/#".into(),
                        no_local: false,
                        qos: QoS::AtMostOnce
                    },
                    SubscribeEntry {
                        filter: "b".into(),
                        no_local: false,
                        qos: QoS::AtLeastOnce
                    },
                    SubscribeEntry {
                        filter: "c".into(),
                        no_local: false,
                        qos: QoS::ExactlyOnce
                    },
                ]
            }
        );
    }

    #[test]
    fn subscribe_without_filters_or_with_bad_options_is_malformed() {
        assert!(matches!(
            decode(&mut buf(&[0x82, 2, 0, 1]), 64),
            Err(DecodeError::Malformed(_))
        ));
        // QoS 3, and every reserved bit 2-7 set
        for options in [0x03u8, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0xFC] {
            assert!(
                matches!(
                    decode(&mut buf(&[0x82, 6, 0, 1, 0, 1, b'a', options]), 64),
                    Err(DecodeError::Malformed(_))
                ),
                "options {options:#x}"
            );
        }
    }

    #[test]
    fn unsubscribe() {
        let raw = [0xA2, 7, 0, 9, 0, 3, b'a', b'/', b'b'];
        assert_eq!(
            decode(&mut buf(&raw), 64),
            Ok(Some(Packet::Unsubscribe {
                packet_id: 9,
                filters: vec!["a/b".into()]
            }))
        );
        assert!(matches!(
            decode(&mut buf(&[0xA2, 2, 0, 9]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn oversize_is_detected_from_the_header() {
        // announces 300 bytes remaining, only the header is present
        let mut b = buf(&[0x30, 0xAC, 0x02]);
        assert_eq!(
            decode(&mut b, 256),
            Err(DecodeError::PacketTooLarge {
                size: 303,
                max: 256
            })
        );
        // exactly the limit is fine
        let mut b = buf(&[0x30, 0xAC, 0x02]);
        assert_eq!(decode(&mut b, 303), Ok(None));
    }

    #[test]
    fn malformed_remaining_length() {
        assert!(matches!(
            decode(&mut buf(&[0x30, 0x80, 0x80, 0x80, 0x80, 0x01]), u32::MAX),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn truncated_body_is_malformed() {
        // PUBLISH whose topic length points behind the packet
        assert!(matches!(
            decode(&mut buf(&[0x30, 3, 0, 9, b'a']), 64),
            Err(DecodeError::Malformed(_))
        ));
        // invalid UTF-8 topic
        assert!(matches!(
            decode(&mut buf(&[0x30, 3, 0, 1, 0xFF]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn a_nul_character_in_a_string_is_malformed() {
        // topic of a PUBLISH
        assert!(matches!(
            decode(&mut buf(&[0x30, 4, 0, 2, b'a', 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
        // filter of a SUBSCRIBE
        assert!(matches!(
            decode(&mut buf(&[0x82, 8, 0, 1, 0, 3, b'a', 0, b'b', 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
        // client identifier of a CONNECT
        let mut raw = connect_bytes("abc", 30);
        let last = raw.len() - 2;
        raw[last] = 0;
        assert!(matches!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::Malformed(_))
        ));
        // the same strings without it are fine
        assert!(decode(&mut buf(&[0x30, 4, 0, 2, b'a', b'b']), 64).is_ok());
    }

    #[test]
    fn reserved_and_unknown_types() {
        assert!(matches!(
            decode(&mut buf(&[0x00, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
        // type 15 is reserved in 3.1.1 (AUTH only exists in MQTT 5)
        assert!(matches!(
            decode(&mut buf(&[0xF0, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
        // PUBACK is decoded now; SUBACK (9) and the like are only reported
        assert_eq!(
            decode(&mut buf(&[0x90, 3, 0, 1, 0]), 64),
            Ok(Some(Packet::Ignored(9)))
        );
    }

    #[test]
    fn connack_bytes() {
        assert_eq!(
            &encode_connack(V311, connack::ACCEPTED)[..],
            &[0x20, 0x02, 0x00, 0x00]
        );
        assert_eq!(
            &encode_connack(V311, connack::IDENTIFIER_REJECTED)[..],
            &[0x20, 0x02, 0x00, 0x02]
        );
        let info = ConnackAccepted {
            session_present: false,
            max_packet_size: 1000,
            receive_maximum: 7,
            session_expiry: Some(3),
            assigned_client_id: Some("ignored"),
        };
        assert_eq!(
            &encode_connack_accepted(V311, &info)[..],
            &[0x20, 0x02, 0x00, 0x00]
        );
        let present = ConnackAccepted {
            session_present: true,
            ..info
        };
        assert_eq!(
            &encode_connack_accepted(V311, &present)[..],
            &[0x20, 0x02, 0x01, 0x00]
        );
    }

    #[test]
    fn publish_bytes_and_long_remaining_length() {
        let mut out = BytesMut::new();
        encode_publish(V311, &mut out, "a/b", b"hi", QoS::AtMostOnce, 99, false);
        assert_eq!(&out[..], &[0x30, 7, 0, 3, b'a', b'/', b'b', b'h', b'i']);

        let payload = vec![7u8; 300];
        let mut out = BytesMut::new();
        encode_publish(V311, &mut out, "t", &payload, QoS::AtMostOnce, 0, false);
        // remaining = 2 + 1 + 300 = 303 = 0xAF 0x02
        assert_eq!(&out[..3], &[0x30, 0xAF, 0x02]);
        let decoded = decode(&mut out, 1024).unwrap().unwrap();
        assert_eq!(
            decoded,
            Packet::Publish(Publish {
                topic: "t".into(),
                payload: Bytes::from(payload),
                qos: QoS::AtMostOnce,
                packet_id: None,
                dup: false,
            })
        );
    }

    #[test]
    fn publish_encoded_len_matches_encoder() {
        for (t, l) in [(1usize, 0usize), (3, 300), (10, 20_000)] {
            for version in [V311, V5] {
                for qos in ALL_QOS {
                    let mut out = BytesMut::new();
                    encode_publish(
                        version,
                        &mut out,
                        &"t".repeat(t),
                        &vec![2; l],
                        qos,
                        5,
                        false,
                    );
                    assert_eq!(publish_encoded_len(version, t, l, qos), Some(out.len()));
                }
            }
        }
        assert_eq!(
            publish_encoded_len(V311, 1, 268_435_455, QoS::AtMostOnce),
            None
        );
        // topic length does not fit into the 16 bit length field
        assert!(publish_encoded_len(V311, 65_535, 0, QoS::AtMostOnce).is_some());
        assert_eq!(publish_encoded_len(V311, 65_536, 0, QoS::AtMostOnce), None);
        // MQTT 5 needs one byte more, QoS 1 and 2 two bytes more
        let base = publish_encoded_len(V311, 3, 10, QoS::AtMostOnce).unwrap();
        assert_eq!(
            publish_encoded_len(V5, 3, 10, QoS::AtMostOnce),
            Some(base + 1)
        );
        assert_eq!(
            publish_encoded_len(V311, 3, 10, QoS::AtLeastOnce),
            Some(base + 2)
        );
        assert_eq!(
            publish_encoded_len(V5, 3, 10, QoS::ExactlyOnce),
            Some(base + 3)
        );
        // the packet identifier can push the packet over the largest remaining length
        assert!(publish_encoded_len(V311, 1, 268_435_455 - 3, QoS::AtMostOnce).is_some());
        assert_eq!(
            publish_encoded_len(V311, 1, 268_435_455 - 3, QoS::AtLeastOnce),
            None
        );
    }

    #[test]
    fn small_packets() {
        assert_eq!(
            &encode_suback(V311, 7, &[suback::GRANTED_QOS_0, suback::FAILURE])[..],
            &[0x90, 4, 0, 7, 0x00, 0x80]
        );
        assert_eq!(
            &encode_unsuback(V311, 9, &[unsuback::SUCCESS])[..],
            &[0xB0, 2, 0, 9]
        );
        assert_eq!(&encode_pingresp()[..], &[0xD0, 0]);
    }

    #[test]
    fn varint_boundaries() {
        for (value, expected) in [
            (0usize, vec![0u8]),
            (127, vec![0x7F]),
            (128, vec![0x80, 0x01]),
            (16_383, vec![0xFF, 0x7F]),
            (16_384, vec![0x80, 0x80, 0x01]),
            (268_435_455, vec![0xFF, 0xFF, 0xFF, 0x7F]),
        ] {
            let mut out = BytesMut::new();
            put_varint(&mut out, value);
            assert_eq!(&out[..], &expected[..], "value {value}");
        }
    }

    // ---- MQTT 5 ----

    /// MQTT 5 CONNECT with the given client id, clean start and the given property bytes.
    fn connect5_bytes(client_id: &str, keep_alive: u16, props: &[u8]) -> Vec<u8> {
        let mut body = vec![0, 4, b'M', b'Q', b'T', b'T', 5, 0b10];
        body.extend(keep_alive.to_be_bytes());
        body.push(props.len() as u8);
        body.extend(props);
        body.extend((client_id.len() as u16).to_be_bytes());
        body.extend(client_id.as_bytes());
        let mut out = vec![0x10, body.len() as u8];
        out.extend(body);
        out
    }

    #[test]
    fn connect_v5_without_and_with_properties() {
        let expect = |id: &str, session_expiry: u32| {
            Packet::Connect(Connect {
                client_id: id.into(),
                keep_alive: 30,
                clean_session: true,
                protocol: V5,
                username: None,
                password: None,
                session_expiry,
                receive_maximum: 65535,
                maximum_packet_size: None,
            })
        };
        // the CONNECT is decoded by its own level, whatever version the connection had before
        for version in [V311, V5] {
            let mut b = buf(&connect5_bytes("abc", 30, &[]));
            assert_eq!(
                super::decode(&mut b, 1024, version),
                Ok(Some(expect("abc", 0)))
            );
            assert!(b.is_empty());
        }
        // session expiry interval (0x11, 4 bytes), user property (0x26, two strings)
        let props = [0x11, 0, 0, 0, 60, 0x26, 0, 1, b'k', 0, 2, b'v', b'v'];
        assert_eq!(
            decode(&mut buf(&connect5_bytes("", 30, &props)), 1024),
            Ok(Some(expect("", 60)))
        );
    }

    #[test]
    fn connect_v5_with_truncated_properties_is_malformed() {
        // property length 5, but only the client id follows
        let mut raw = connect5_bytes("a", 30, &[]);
        raw[12] = 5; // property length byte (fixed header 2 + name 6 + level + flags + keep alive 2 = 12)
        assert!(matches!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn incomplete_v5_connect_consumes_nothing() {
        let full = connect5_bytes("abc", 30, &[0x11, 0, 0, 0, 60]);
        for cut in 0..full.len() {
            let mut b = buf(&full[..cut]);
            assert_eq!(decode(&mut b, 1024), Ok(None), "cut {cut}");
            assert_eq!(b.len(), cut);
        }
    }

    #[test]
    fn publish_v5_skips_properties() {
        // topic "a", properties: message expiry (0x02, 4 bytes) + content type (0x03, "x"), payload "hi"
        let raw = [
            0x30, 15, 0, 1, b'a', 9, 0x02, 0, 0, 0, 5, 0x03, 0, 1, b'x', b'h', b'i',
        ];
        let expected = Packet::Publish(Publish {
            topic: "a".into(),
            payload: Bytes::from_static(b"hi"),
            qos: QoS::AtMostOnce,
            packet_id: None,
            dup: false,
        });
        assert_eq!(decode5(&mut buf(&raw), 1024), Ok(Some(expected)));
        // empty properties, empty payload
        assert_eq!(
            decode5(&mut buf(&[0x30, 4, 0, 1, b'a', 0]), 1024),
            Ok(Some(Packet::Publish(Publish {
                topic: "a".into(),
                payload: Bytes::new(),
                qos: QoS::AtMostOnce,
                packet_id: None,
                dup: false,
            })))
        );
        // property length behind the packet
        assert!(matches!(
            decode5(&mut buf(&[0x30, 4, 0, 1, b'a', 9]), 1024),
            Err(DecodeError::Malformed(_))
        ));
        // missing property length
        assert!(matches!(
            decode5(&mut buf(&[0x30, 3, 0, 1, b'a']), 1024),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn publish_v5_packet_id_comes_before_the_properties() {
        // topic "a", packet id 7, properties: message expiry (0x02, 4 bytes), payload "hi"
        let raw = [0x3A, 13, 0, 1, b'a', 0, 7, 5, 0x02, 0, 0, 0, 5, b'h', b'i'];
        assert_eq!(
            decode5(&mut buf(&raw), 64),
            Ok(Some(Packet::Publish(Publish {
                topic: "a".into(),
                payload: Bytes::from_static(b"hi"),
                qos: QoS::AtLeastOnce,
                packet_id: Some(7),
                dup: true,
            })))
        );
        // empty properties, QoS 2, id 0 is malformed
        assert!(matches!(
            decode5(&mut buf(&[0x34, 6, 0, 1, b'a', 0, 1, 0]), 64),
            Ok(Some(Packet::Publish(Publish {
                qos: QoS::ExactlyOnce,
                packet_id: Some(1),
                ..
            })))
        ));
        assert!(matches!(
            decode5(&mut buf(&[0x34, 6, 0, 1, b'a', 0, 0, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
        // DUP at QoS 0 and QoS 3 stay malformed
        for first in [0x38u8, 0x36] {
            assert!(matches!(
                decode5(&mut buf(&[first, 6, 0, 1, b'a', 0, 1, 0]), 64),
                Err(DecodeError::Malformed(_))
            ));
        }
    }

    #[test]
    fn subscribe_v5_options() {
        // id 7, no properties, "a" with QoS 1 + No Local (0b101), "b" with retain handling 2 + RAP (0b10_1_0_00)
        let raw = [0x82, 11, 0, 7, 0, 0, 1, b'a', 0b101, 0, 1, b'b', 0b10_1000];
        assert_eq!(
            decode5(&mut buf(&raw), 64),
            Ok(Some(Packet::Subscribe {
                packet_id: 7,
                entries: vec![
                    SubscribeEntry {
                        filter: "a".into(),
                        no_local: true,
                        qos: QoS::AtLeastOnce
                    },
                    SubscribeEntry {
                        filter: "b".into(),
                        no_local: false,
                        qos: QoS::AtMostOnce
                    },
                ]
            }))
        );
        // with a subscription identifier property (0x0B, varint 5)
        let raw = [0x82, 9, 0, 7, 2, 0x0B, 5, 0, 1, b'a', 0];
        // (remaining length: id 2 + properties 3 + filter 3 + options 1 = 9)
        assert!(matches!(
            decode5(&mut buf(&raw), 64),
            Ok(Some(Packet::Subscribe { packet_id: 7, .. }))
        ));
    }

    #[test]
    fn subscribe_v5_invalid_options_and_missing_filters_are_malformed() {
        // QoS 3, retain handling 3, reserved bits 6 and 7
        for options in [0x03u8, 0x30, 0x40, 0x80, 0xC0] {
            assert!(
                matches!(
                    decode5(&mut buf(&[0x82, 7, 0, 1, 0, 0, 1, b'a', options][..]), 64),
                    Err(DecodeError::Malformed(_))
                ),
                "options {options:#x}"
            );
        }
        // bits 2, 3 and retain handling 0-2 are fine
        for options in [0x04u8, 0x08, 0x10, 0x20, 0x2C] {
            assert!(
                matches!(
                    decode5(&mut buf(&[0x82, 7, 0, 1, 0, 0, 1, b'a', options][..]), 64),
                    Ok(Some(Packet::Subscribe { .. }))
                ),
                "options {options:#x}"
            );
        }
        // only packet id and properties
        assert!(matches!(
            decode5(&mut buf(&[0x82, 3, 0, 1, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn subscribe_v311_has_no_properties_or_no_local() {
        // in 3.1.1 bit 2 is reserved: the same bytes that MQTT 5 accepts are malformed
        let raw = [0x82, 6, 0, 1, 0, 1, b'a', 0b100];
        assert!(matches!(
            decode(&mut buf(&raw), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn unsubscribe_v5() {
        let raw = [0xA2, 8, 0, 9, 0, 0, 3, b'a', b'/', b'b'];
        assert_eq!(
            decode5(&mut buf(&raw), 64),
            Ok(Some(Packet::Unsubscribe {
                packet_id: 9,
                filters: vec!["a/b".into()]
            }))
        );
        assert!(matches!(
            decode5(&mut buf(&[0xA2, 3, 0, 9, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn disconnect_v5_variants() {
        // no reason code, reason code only, reason code with properties (reason string 0x1F "x")
        for raw in [
            &[0xE0u8, 0][..],
            &[0xE0, 1, 0x04],
            &[0xE0, 2, 0x04, 0],
            &[0xE0, 6, 0x04, 4, 0x1F, 0, 1, b'x'],
        ] {
            assert_eq!(
                decode5(&mut buf(raw), 64),
                Ok(Some(Packet::Disconnect {
                    session_expiry: None
                })),
                "{raw:?}"
            );
        }
        // property length behind the packet
        assert!(matches!(
            decode5(&mut buf(&[0xE0, 3, 0x04, 5, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn connack_v5_bytes() {
        assert_eq!(
            &encode_connack(V5, connack::V5_UNSUPPORTED_PROTOCOL_VERSION)[..],
            &[0x20, 3, 0, 0x84, 0]
        );
        assert_eq!(
            connack::unsupported_protocol(V311),
            connack::UNACCEPTABLE_PROTOCOL_VERSION
        );
        assert_eq!(connack::identifier_rejected(V5), 0x85);
        assert_eq!(connack::identifier_rejected(V311), 0x02);

        // accepted: receive maximum 100, retain not available, max packet size 0x00040000, no subscription
        // identifiers, no shared subscriptions; no Maximum QoS property
        let info = ConnackAccepted {
            session_present: false,
            max_packet_size: 262_144,
            receive_maximum: 100,
            session_expiry: None,
            assigned_client_id: None,
        };
        let base_props = [0x21, 0, 100, 0x25, 0, 0x27, 0, 4, 0, 0, 0x29, 0, 0x2A, 0];
        let frame = |flag: u8, props: &[u8]| {
            let mut v = vec![
                0x20,
                (2 + 1 + props.len()) as u8,
                flag,
                0,
                props.len() as u8,
            ];
            v.extend(props);
            v
        };
        assert_eq!(
            &encode_connack_accepted(V5, &info)[..],
            &frame(0, &base_props)[..]
        );

        // session present sets the acknowledge flag
        let present = ConnackAccepted {
            session_present: true,
            ..info
        };
        assert_eq!(
            &encode_connack_accepted(V5, &present)[..],
            &frame(1, &base_props)[..]
        );

        // session expiry (0x11) comes after the standard properties, the assigned identifier (0x12) last
        let mut with_expiry = base_props.to_vec();
        with_expiry.extend([0x11, 0, 0, 0x0E, 0x10]);
        let expiry = ConnackAccepted {
            session_expiry: Some(3600),
            ..info
        };
        assert_eq!(
            &encode_connack_accepted(V5, &expiry)[..],
            &frame(0, &with_expiry)[..]
        );
        let mut with_both = with_expiry.clone();
        with_both.extend([0x12, 0, 3, b'd', b't', b'1']);
        let both = ConnackAccepted {
            session_present: true,
            assigned_client_id: Some("dt1"),
            ..expiry
        };
        assert_eq!(
            &encode_connack_accepted(V5, &both)[..],
            &frame(1, &with_both)[..]
        );

        // only the assigned identifier
        let mut with_id = base_props.to_vec();
        with_id.extend([0x12, 0, 3, b'd', b't', b'1']);
        let id_only = ConnackAccepted {
            assigned_client_id: Some("dt1"),
            ..info
        };
        assert_eq!(
            &encode_connack_accepted(V5, &id_only)[..],
            &frame(0, &with_id)[..]
        );
    }

    #[test]
    fn connack_v5_can_be_decoded_by_the_property_layout() {
        // a long assigned id pushes the property length over one byte: the varint must be right
        let id = "x".repeat(200);
        let ack = encode_connack_accepted(
            V5,
            &ConnackAccepted {
                session_present: false,
                max_packet_size: 1,
                receive_maximum: 100,
                session_expiry: None,
                assigned_client_id: Some(&id),
            },
        );
        // fixed header: type, remaining length (2 bytes); body: flags, reason, property length (2 bytes)
        let mut b = BytesMut::from(&ack[..]);
        let first = b.split_to(1);
        assert_eq!(first[0], 0x20);
        let mut r = Reader(b.freeze());
        let remaining = r.varint().unwrap();
        assert_eq!(remaining, r.0.remaining());
        assert_eq!(r.u8().unwrap(), 0);
        assert_eq!(r.u8().unwrap(), 0);
        let props = r.varint().unwrap();
        assert_eq!(props, r.0.remaining());
        assert_eq!(props, 3 + 2 + 5 + 2 + 2 + 3 + 200);
    }

    #[test]
    fn publish_v5_bytes() {
        let mut out = BytesMut::new();
        encode_publish(V5, &mut out, "a/b", b"hi", QoS::AtMostOnce, 0, false);
        assert_eq!(&out[..], &[0x30, 8, 0, 3, b'a', b'/', b'b', 0, b'h', b'i']);
        // what the encoder writes is what the decoder reads
        assert_eq!(
            decode5(&mut out, 64),
            Ok(Some(Packet::Publish(Publish {
                topic: "a/b".into(),
                payload: Bytes::from_static(b"hi"),
                qos: QoS::AtMostOnce,
                packet_id: None,
                dup: false,
            })))
        );
    }

    #[test]
    fn small_packets_v5() {
        assert_eq!(
            &encode_suback(V5, 7, &[suback::GRANTED_QOS_0, suback::V5_QUOTA_EXCEEDED])[..],
            &[0x90, 5, 0, 7, 0, 0x00, 0x97]
        );
        assert_eq!(
            &encode_unsuback(
                V5,
                9,
                &[unsuback::SUCCESS, unsuback::NO_SUBSCRIPTION_EXISTED]
            )[..],
            &[0xB0, 5, 0, 9, 0, 0x00, 0x11]
        );
        assert_eq!(
            &encode_disconnect(disconnect::SERVER_SHUTTING_DOWN)[..],
            &[0xE0, 2, 0x8B, 0]
        );
    }

    #[test]
    fn auth_is_a_protocol_error_in_v5_and_malformed_in_3_1_1() {
        assert!(matches!(
            decode5(&mut buf(&[0xF0, 0]), 64),
            Err(DecodeError::ProtocolError(_))
        ));
        assert!(matches!(
            decode(&mut buf(&[0xF0, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn varint_reader() {
        for (bytes, expected) in [
            (&[0u8][..], 0usize),
            (&[0x7F], 127),
            (&[0x80, 0x01], 128),
            (&[0xFF, 0xFF, 0xFF, 0x7F], 268_435_455),
        ] {
            let mut r = Reader(Bytes::copy_from_slice(bytes));
            assert_eq!(r.varint(), Ok(expected));
        }
        // five bytes, and a truncated one
        let mut r = Reader(Bytes::from_static(&[0x80, 0x80, 0x80, 0x80, 0x01]));
        assert!(r.varint().is_err());
        let mut r = Reader(Bytes::from_static(&[0x80]));
        assert!(r.varint().is_err());
    }

    // ---- QoS 1 and 2 ----

    /// Reads back what `encode_publish` wrote.
    fn publish_roundtrip(version: ProtocolVersion, qos: QoS, id: u16, dup: bool) -> Publish {
        let mut out = BytesMut::new();
        encode_publish(version, &mut out, "a/b", b"payload", qos, id, dup);
        match super::decode(&mut out, 1024, version) {
            Ok(Some(Packet::Publish(p))) => {
                assert!(out.is_empty());
                p
            }
            other => panic!("{other:?}"),
        }
    }

    #[test]
    fn publish_encode_decode_round_trip_at_every_qos() {
        for version in [V311, V5] {
            for qos in ALL_QOS {
                for dup in [false, true] {
                    // DUP at QoS 0 is not a valid packet, so it is not part of the round trip
                    if dup && qos == QoS::AtMostOnce {
                        continue;
                    }
                    let p = publish_roundtrip(version, qos, 0xABCD, dup);
                    assert_eq!(p.topic, "a/b");
                    assert_eq!(&p.payload[..], b"payload");
                    assert_eq!((p.qos, p.dup), (qos, dup));
                    let expected_id = (qos != QoS::AtMostOnce).then_some(0xABCD);
                    assert_eq!(p.packet_id, expected_id, "{version:?} {qos:?}");
                }
            }
        }
    }

    #[test]
    fn publish_encoding_bytes_for_qos_and_dup() {
        let enc = |version, qos, id, dup| {
            let mut out = BytesMut::new();
            encode_publish(version, &mut out, "a", b"x", qos, id, dup);
            out.to_vec()
        };
        // the packet id argument is ignored at QoS 0
        assert_eq!(
            enc(V311, QoS::AtMostOnce, 7, false),
            [0x30, 4, 0, 1, b'a', b'x']
        );
        assert_eq!(
            enc(V311, QoS::AtLeastOnce, 0x0102, false),
            [0x32, 6, 0, 1, b'a', 1, 2, b'x']
        );
        assert_eq!(
            enc(V311, QoS::ExactlyOnce, 9, true),
            [0x3C, 6, 0, 1, b'a', 0, 9, b'x']
        );
        // MQTT 5: packet id, then the empty property block, then the payload
        assert_eq!(
            enc(V5, QoS::AtLeastOnce, 9, true),
            [0x3A, 7, 0, 1, b'a', 0, 9, 0, b'x']
        );
    }

    type AckPacket = fn(u16, u8) -> Packet;
    type AckEncoder = fn(ProtocolVersion, u16, u8) -> Bytes;

    #[test]
    fn ack_packets_decode_in_3_1_1() {
        let cases: [(u8, AckPacket); 4] = [
            (0x40, |packet_id, reason| Packet::PubAck {
                packet_id,
                reason,
            }),
            (0x50, |packet_id, reason| Packet::PubRec {
                packet_id,
                reason,
            }),
            (0x62, |packet_id, reason| Packet::PubRel {
                packet_id,
                reason,
            }),
            (0x70, |packet_id, reason| Packet::PubComp {
                packet_id,
                reason,
            }),
        ];
        for (first, make) in cases {
            assert_eq!(
                decode(&mut buf(&[first, 2, 0x12, 0x34]), 64),
                Ok(Some(make(0x1234, 0))),
                "{first:#x}"
            );
            // nothing may follow the packet id in 3.1.1
            assert!(
                matches!(
                    decode(&mut buf(&[first, 3, 0x12, 0x34, 0]), 64),
                    Err(DecodeError::Malformed(_))
                ),
                "{first:#x}"
            );
            // missing or truncated packet id
            for raw in [&[first, 0][..], &[first, 1, 0x12]] {
                assert!(matches!(
                    decode(&mut buf(raw), 64),
                    Err(DecodeError::Malformed(_))
                ));
            }
        }
    }

    #[test]
    fn ack_packets_have_fixed_header_flags() {
        // PUBACK, PUBREC, PUBCOMP need flags 0, PUBREL needs 0b0010
        for (kind, required) in [(4u8, 0u8), (5, 0), (6, 0b0010), (7, 0)] {
            for flags in 0..16u8 {
                for version in [V311, V5] {
                    let raw = [kind << 4 | flags, 2, 0, 1];
                    let result = super::decode(&mut buf(&raw), 64, version);
                    if flags == required {
                        assert!(matches!(result, Ok(Some(_))), "{kind} {flags:#x}");
                    } else {
                        assert!(
                            matches!(result, Err(DecodeError::Malformed(_))),
                            "{kind} {flags:#x}"
                        );
                    }
                }
            }
        }
    }

    #[test]
    fn ack_packets_decode_in_v5_with_and_without_reason_and_properties() {
        // packet id only: success; with reason code; with reason code and empty properties; with a reason
        // string property (0x1F "x") and a user property
        let cases: [(&[u8], u8); 5] = [
            (&[0x50, 2, 0, 5], 0x00),
            (&[0x50, 3, 0, 5, 0x00], 0x00),
            (&[0x50, 3, 0, 5, 0x97], 0x97),
            (&[0x50, 4, 0, 5, 0x91, 0], 0x91),
            (
                &[
                    0x50, 15, 0, 5, 0x92, 11, 0x1F, 0, 1, b'x', 0x26, 0, 1, b'k', 0, 1, b'v',
                ],
                0x92,
            ),
        ];
        for (raw, reason) in cases {
            assert_eq!(
                decode5(&mut buf(raw), 64),
                Ok(Some(Packet::PubRec {
                    packet_id: 5,
                    reason
                })),
                "{raw:?}"
            );
        }
        assert_eq!(
            decode5(&mut buf(&[0x62, 3, 0, 5, 0x92]), 64),
            Ok(Some(Packet::PubRel {
                packet_id: 5,
                reason: puback::PACKET_IDENTIFIER_NOT_FOUND
            }))
        );
        assert_eq!(
            decode5(&mut buf(&[0x40, 3, 0, 5, 0x10]), 64),
            Ok(Some(Packet::PubAck {
                packet_id: 5,
                reason: 0x10
            }))
        );
        assert_eq!(
            decode5(&mut buf(&[0x70, 2, 0, 5]), 64),
            Ok(Some(Packet::PubComp {
                packet_id: 5,
                reason: 0
            }))
        );
        // property length behind the packet, and data after the properties
        for raw in [&[0x40u8, 4, 0, 5, 0, 9][..], &[0x40, 5, 0, 5, 0, 0, 0]] {
            assert!(matches!(
                decode5(&mut buf(raw), 64),
                Err(DecodeError::Malformed(_))
            ));
        }
    }

    #[test]
    fn ack_encoding_bytes() {
        let encoders: [(u8, AckEncoder); 4] = [
            (0x40, encode_puback),
            (0x50, encode_pubrec),
            (0x62, encode_pubrel),
            (0x70, encode_pubcomp),
        ];
        for (first, encode) in encoders {
            // 3.1.1 ignores the reason
            assert_eq!(&encode(V311, 0x0102, 0)[..], &[first, 2, 1, 2]);
            assert_eq!(&encode(V311, 0x0102, 0x97)[..], &[first, 2, 1, 2]);
            // MQTT 5: success needs no reason code, others append it without a property length
            assert_eq!(&encode(V5, 0x0102, 0)[..], &[first, 2, 1, 2]);
            assert_eq!(&encode(V5, 0x0102, 0x97)[..], &[first, 3, 1, 2, 0x97]);
            // what is written can be read back
            for version in [V311, V5] {
                let reason = if version == V5 { 0x91 } else { 0 };
                let mut b = BytesMut::from(&encode(version, 77, reason)[..]);
                let packet = super::decode(&mut b, 64, version).unwrap().unwrap();
                let (Packet::PubAck {
                    packet_id,
                    reason: r,
                }
                | Packet::PubRec {
                    packet_id,
                    reason: r,
                }
                | Packet::PubRel {
                    packet_id,
                    reason: r,
                }
                | Packet::PubComp {
                    packet_id,
                    reason: r,
                }) = packet
                else {
                    panic!("{packet:?}");
                };
                assert_eq!((packet_id, r), (77, reason));
            }
        }
        assert_eq!(puback::SUCCESS, 0);
        assert_eq!(suback::GRANTED_QOS_1, 1);
        assert_eq!(suback::GRANTED_QOS_2, 2);
    }

    #[test]
    fn subscribe_qos_in_both_versions() {
        for (bits, qos) in [
            (0u8, QoS::AtMostOnce),
            (1, QoS::AtLeastOnce),
            (2, QoS::ExactlyOnce),
        ] {
            let Ok(Some(Packet::Subscribe { entries, .. })) =
                decode(&mut buf(&[0x82, 6, 0, 1, 0, 1, b'a', bits]), 64)
            else {
                panic!("3.1.1 {bits}");
            };
            assert_eq!(entries[0].qos, qos);
            // MQTT 5 with No Local, retain as published and retain handling 2 set
            let options = bits | 0b100 | 0b1000 | 0b10_0000;
            let Ok(Some(Packet::Subscribe { entries, .. })) =
                decode5(&mut buf(&[0x82, 7, 0, 1, 0, 0, 1, b'a', options]), 64)
            else {
                panic!("v5 {bits}");
            };
            assert_eq!((entries[0].qos, entries[0].no_local), (qos, true));
        }
    }

    /// MQTT 5 CONNECT bytes, `connect5_bytes` with a bigger property block possible.
    fn decode_connect5(props: &[u8]) -> Result<Connect, DecodeError> {
        match decode5(&mut buf(&connect5_bytes("c", 10, props)), 1024) {
            Ok(Some(Packet::Connect(c))) => Ok(c),
            Ok(other) => panic!("{other:?}"),
            Err(e) => Err(e),
        }
    }

    #[test]
    fn connect_session_expiry_and_receive_maximum() {
        // defaults
        let c = decode_connect5(&[]).unwrap();
        assert_eq!((c.session_expiry, c.receive_maximum), (0, 65_535));
        let c = decode(&mut buf(&connect_bytes("c", 1)), 64)
            .unwrap()
            .unwrap();
        let Packet::Connect(c) = c else { panic!() };
        assert_eq!((c.session_expiry, c.receive_maximum), (0, 65_535));
        // values, also behind other properties and in any order
        let c = decode_connect5(&[0x11, 0, 0, 0x0E, 0x10, 0x21, 0x01, 0x00]).unwrap();
        assert_eq!((c.session_expiry, c.receive_maximum), (3600, 256));
        let props = [
            0x26, 0, 1, b'k', 0, 1, b'v', 0x21, 0, 5, 0x27, 0, 0, 1, 0, 0x11, 0xFF, 0xFF, 0xFF,
            0xFF,
        ];
        let c = decode_connect5(&props).unwrap();
        assert_eq!((c.session_expiry, c.receive_maximum), (u32::MAX, 5));
        let c = decode_connect5(&[0x21, 0xFF, 0xFF]).unwrap();
        assert_eq!((c.session_expiry, c.receive_maximum), (0, 65_535));
    }

    #[test]
    fn connect_duplicate_or_invalid_properties_are_errors() {
        for props in [
            &[0x11, 0, 0, 0, 1, 0x11, 0, 0, 0, 2][..],
            &[0x21, 0, 1, 0x21, 0, 2],
            &[0x21, 0, 1, 0x11, 0, 0, 0, 0, 0x21, 0, 1],
            // receive maximum 0
            &[0x21, 0, 0],
        ] {
            assert!(
                matches!(decode_connect5(props), Err(DecodeError::ProtocolError(_))),
                "{props:?}"
            );
        }
        // unknown property id, truncated values, string running over the block
        for props in [
            &[0x7F, 0][..],
            &[0x00, 0],
            &[0x11, 0, 0, 0],
            &[0x21, 0],
            &[0x03, 0, 5, b'x'],
            &[0x26, 0, 1, b'k'],
            &[0x0B, 0x80],
        ] {
            assert!(
                matches!(decode_connect5(props), Err(DecodeError::Malformed(_))),
                "{props:?}"
            );
        }
    }

    #[test]
    fn connect_debug_shows_session_expiry_and_receive_maximum() {
        let c = decode_connect5(&[0x11, 0, 0, 0, 9, 0x21, 0, 3]).unwrap();
        let text = format!("{c:?}");
        assert!(
            text.contains("session_expiry: 9") && text.contains("receive_maximum: 3"),
            "{text}"
        );
    }

    #[test]
    fn disconnect_session_expiry() {
        let expiry = |raw: &[u8], version| match super::decode(&mut buf(raw), 64, version) {
            Ok(Some(Packet::Disconnect { session_expiry })) => session_expiry,
            other => panic!("{other:?}"),
        };
        // reason code 0, properties: session expiry 120
        assert_eq!(
            expiry(&[0xE0, 7, 0x00, 5, 0x11, 0, 0, 0, 120], V5),
            Some(120)
        );
        // with a reason string before it; an explicit 0 is not the same as absent
        assert_eq!(
            expiry(&[0xE0, 11, 0x04, 9, 0x1F, 0, 1, b'x', 0x11, 0, 0, 0, 0], V5),
            Some(0)
        );
        // absent: no body, reason only, empty properties, other properties
        for raw in [
            &[0xE0u8, 0][..],
            &[0xE0, 1, 0x04],
            &[0xE0, 2, 0x04, 0],
            &[0xE0, 6, 0x04, 4, 0x1F, 0, 1, b'x'],
        ] {
            assert_eq!(expiry(raw, V5), None, "{raw:?}");
        }
        // 3.1.1 has no body and always reports None
        assert_eq!(expiry(&[0xE0, 0], V311), None);
        // duplicate and unknown properties
        assert!(matches!(
            decode5(
                &mut buf(&[0xE0, 12, 0, 10, 0x11, 0, 0, 0, 1, 0x11, 0, 0, 0, 2]),
                64
            ),
            Err(DecodeError::ProtocolError(_))
        ));
        assert!(matches!(
            decode5(&mut buf(&[0xE0, 4, 0, 2, 0x7F, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    /// Property block with the given content, run through the walker; returns the reported numbers.
    fn walk(content: &[u8]) -> Result<Vec<(u8, u32)>, DecodeError> {
        let mut bytes = Vec::new();
        put_varint_vec(&mut bytes, content.len());
        bytes.extend(content);
        bytes.extend([0xEE]); // trailing byte of the packet, must stay unread
        let mut r = Reader(Bytes::from(bytes));
        let mut seen = Vec::new();
        r.read_properties(|id, value| {
            seen.push((id, value));
            Ok(())
        })?;
        assert_eq!(r.u8(), Ok(0xEE));
        Ok(seen)
    }

    fn put_varint_vec(out: &mut Vec<u8>, value: usize) {
        let mut b = BytesMut::new();
        put_varint(&mut b, value);
        out.extend(&b[..]);
    }

    #[test]
    fn property_walker_knows_every_wire_type() {
        // byte properties
        for id in [0x01u8, 0x17, 0x19, 0x24, 0x25, 0x28, 0x29, 0x2A] {
            assert_eq!(walk(&[id, 0xAB]), Ok(vec![(id, 0xAB)]), "{id:#x}");
            assert!(walk(&[id]).is_err(), "{id:#x}");
        }
        // two byte
        for id in [0x13u8, 0x21, 0x22, 0x23] {
            assert_eq!(walk(&[id, 0x01, 0x02]), Ok(vec![(id, 0x0102)]), "{id:#x}");
            assert!(walk(&[id, 1]).is_err(), "{id:#x}");
        }
        // four byte
        for id in [0x02u8, 0x11, 0x18, 0x27] {
            assert_eq!(
                walk(&[id, 1, 2, 3, 4]),
                Ok(vec![(id, 0x0102_0304)]),
                "{id:#x}"
            );
            assert!(walk(&[id, 1, 2, 3]).is_err(), "{id:#x}");
        }
        // variable byte integer
        assert_eq!(walk(&[0x0B, 0x80, 0x01]), Ok(vec![(0x0B, 128)]));
        assert!(walk(&[0x0B, 0x80]).is_err());
        // strings and binary data are skipped, not reported
        for id in [0x03u8, 0x08, 0x09, 0x12, 0x15, 0x16, 0x1A, 0x1C, 0x1F] {
            assert_eq!(walk(&[id, 0, 2, b'h', b'i']), Ok(vec![]), "{id:#x}");
            assert_eq!(walk(&[id, 0, 0]), Ok(vec![]), "{id:#x}");
            assert!(walk(&[id, 0, 3, b'h', b'i']).is_err(), "{id:#x}");
            assert!(walk(&[id, 0]).is_err(), "{id:#x}");
        }
        // string pair
        assert_eq!(walk(&[0x26, 0, 1, b'k', 0, 2, b'v', b'v']), Ok(vec![]));
        assert!(walk(&[0x26, 0, 1, b'k']).is_err());
        assert!(walk(&[0x26, 0, 1, b'k', 0, 2, b'v']).is_err());
    }

    #[test]
    fn property_walker_handles_mixed_blocks_and_errors() {
        // empty block
        assert_eq!(walk(&[]), Ok(vec![]));
        // several properties of different types, in order, user property in the middle
        let content = [
            0x01, 1, 0x26, 0, 1, b'k', 0, 1, b'v', 0x21, 0, 9, 0x02, 0, 0, 1, 0, 0x03, 0, 1, b'x',
        ];
        assert_eq!(walk(&content), Ok(vec![(0x01, 1), (0x21, 9), (0x02, 256)]));
        // unknown ids, including 0 and ids between the known ones
        for id in [0x00u8, 0x04, 0x05, 0x0A, 0x0C, 0x14, 0x1B, 0x2B, 0x80, 0xFF] {
            assert!(
                matches!(walk(&[id, 0, 0]), Err(DecodeError::Malformed(_))),
                "{id:#x}"
            );
        }
        // block longer than the data
        let mut r = Reader(Bytes::from_static(&[5, 0x01, 1]));
        assert!(matches!(
            r.read_properties(|_, _| Ok(())),
            Err(DecodeError::Malformed(_))
        ));
        // a value may not run into the bytes behind the block
        let mut r = Reader(Bytes::from_static(&[2, 0x11, 0, 0, 0, 1]));
        assert!(r.read_properties(|_, _| Ok(())).is_err());
        // errors of the callback are passed on
        let mut r = Reader(Bytes::from_static(&[2, 0x01, 1]));
        assert_eq!(
            r.read_properties(|_, _| Err(DecodeError::ProtocolError("stop"))),
            Err(DecodeError::ProtocolError("stop"))
        );
    }

    #[test]
    fn new_reason_code_constants() {
        assert_eq!(disconnect::SESSION_TAKEN_OVER, 0x8E);
        assert_eq!(disconnect::RECEIVE_MAXIMUM_EXCEEDED, 0x93);
        assert_eq!(puback::PACKET_IDENTIFIER_IN_USE, 0x91);
        assert_eq!(puback::PACKET_IDENTIFIER_NOT_FOUND, 0x92);
        assert_eq!(puback::QUOTA_EXCEEDED, 0x97);
    }

    #[test]
    fn review_fixes() {
        // encode_publish never sets DUP at QoS 0
        let mut out = BytesMut::new();
        encode_publish(V311, &mut out, "a", b"x", QoS::AtMostOnce, 0, true);
        assert_eq!(out[0], 0x30);

        // DISCONNECT: data after the property block is malformed
        assert!(matches!(
            decode5(&mut buf(&[0xE0, 3, 0, 0, 0xFF]), 64),
            Err(DecodeError::Malformed(_))
        ));

        // string properties must be UTF-8, binary ones need not
        for id in [0x03u8, 0x08, 0x12, 0x15, 0x1A, 0x1C, 0x1F] {
            assert!(
                matches!(walk(&[id, 0, 1, 0xFF]), Err(DecodeError::Malformed(_))),
                "{id:#x}"
            );
        }
        for id in [0x09u8, 0x16] {
            assert_eq!(walk(&[id, 0, 1, 0xFF]), Ok(vec![]), "{id:#x}");
        }
        assert!(walk(&[0x26, 0, 1, 0xFF, 0, 1, b'v']).is_err());
        assert!(walk(&[0x26, 0, 1, b'k', 0, 1, 0xFF]).is_err());

        // acknowledgements with packet id 0
        for first in [0x40u8, 0x50, 0x62, 0x70] {
            for version in [V311, V5] {
                assert!(matches!(
                    super::decode(&mut buf(&[first, 2, 0, 0]), 64, version),
                    Err(DecodeError::Malformed(_))
                ));
            }
        }

        // v5 QoS 1 PUBLISH truncated after the packet id
        assert!(matches!(
            decode5(&mut buf(&[0x32, 5, 0, 1, b'a', 0, 1]), 64),
            Err(DecodeError::Malformed(_))
        ));

        // Maximum Packet Size
        let c = decode_connect5(&[]).unwrap();
        assert_eq!(c.maximum_packet_size, None);
        let c = decode_connect5(&[0x27, 0, 0, 4, 0]).unwrap();
        assert_eq!(c.maximum_packet_size, Some(1024));
        for props in [
            &[0x27u8, 0, 0, 0, 0][..],
            &[0x27, 0, 0, 0, 1, 0x27, 0, 0, 0, 2],
        ] {
            assert!(matches!(
                decode_connect5(props),
                Err(DecodeError::ProtocolError(_))
            ));
        }
        let text = format!("{c:?}");
        assert!(text.contains("maximum_packet_size: Some(1024)"), "{text}");
        let Ok(Some(Packet::Connect(c))) = decode(&mut buf(&connect_bytes("c", 1)), 64) else {
            panic!()
        };
        assert_eq!(c.maximum_packet_size, None);
    }
}
