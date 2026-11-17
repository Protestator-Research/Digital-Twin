//! Minimal MQTT v5 codec for exactly the packets the broker needs (QoS 0 only).
//!
//! Decoded: CONNECT, PUBLISH (QoS 0), SUBSCRIBE, UNSUBSCRIBE, PINGREQ, DISCONNECT. All other packet types
//! are reported as [`Packet::Ignored`]. Encoded: CONNACK, PUBLISH (QoS 0), SUBACK, UNSUBACK, PINGRESP,
//! DISCONNECT. Properties of incoming packets are skipped (bounds checked, not interpreted).
//!
//! The size check happens on the fixed header, before the body is buffered.

use bytes::{Buf, BufMut, Bytes, BytesMut};
use thiserror::Error;

/// Reason codes used by the broker.
pub mod reason {
    pub const SUCCESS: u8 = 0x00;
    pub const GRANTED_QOS_0: u8 = 0x00;
    pub const MALFORMED_PACKET: u8 = 0x81;
    pub const PROTOCOL_ERROR: u8 = 0x82;
    pub const SERVER_SHUTTING_DOWN: u8 = 0x8B;
    pub const KEEP_ALIVE_TIMEOUT: u8 = 0x8D;
    pub const TOPIC_FILTER_INVALID: u8 = 0x8F;
    pub const TOPIC_NAME_INVALID: u8 = 0x90;
    pub const QUOTA_EXCEEDED: u8 = 0x97;
    pub const PACKET_TOO_LARGE: u8 = 0x95;
}

#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum DecodeError {
    #[error("packet of {size} bytes exceeds the maximum packet size of {max} bytes")]
    PacketTooLarge { size: usize, max: usize },
    #[error("malformed packet: {0}")]
    Malformed(&'static str),
    #[error("unsupported protocol (only MQTT v5 is supported)")]
    UnsupportedProtocol,
    #[error("only QoS 0 is supported")]
    QosNotSupported,
    #[error("topic aliases are not supported (no Topic Alias Maximum announced)")]
    TopicAlias,
}

impl DecodeError {
    /// DISCONNECT reason code to send, `None` if the connection is just closed.
    pub fn disconnect_reason(&self) -> Option<u8> {
        match self {
            DecodeError::PacketTooLarge { .. } => Some(reason::PACKET_TOO_LARGE),
            DecodeError::Malformed(_) => Some(reason::MALFORMED_PACKET),
            DecodeError::QosNotSupported | DecodeError::TopicAlias => Some(reason::PROTOCOL_ERROR),
            DecodeError::UnsupportedProtocol => None,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Connect {
    pub client_id: String,
    pub keep_alive: u16,
    pub clean_start: bool,
    /// Maximum Packet Size property of the client (0x27), if sent.
    pub max_packet_size: Option<u32>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Publish {
    pub topic: String,
    /// Raw property bytes (without length prefix) without Topic Alias and Subscription Identifier.
    pub properties: Bytes,
    pub payload: Bytes,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SubscribeEntry {
    pub filter: String,
    pub no_local: bool,
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
    Disconnect,
    /// A well formed packet of a type the broker does not act on (PUBACK, AUTH, ...).
    Ignored(u8),
}

const CONNECT: u8 = 1;
const PUBLISH: u8 = 3;
const SUBSCRIBE: u8 = 8;
const UNSUBSCRIBE: u8 = 10;
const PINGREQ: u8 = 12;
const DISCONNECT: u8 = 14;

/// Tries to decode one packet from the front of `buf`. Returns `Ok(None)` if more bytes are needed;
/// nothing is consumed in that case. `max_packet_size` is checked as soon as the length is known.
pub fn decode(buf: &mut BytesMut, max_packet_size: u32) -> Result<Option<Packet>, DecodeError> {
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
    decode_body(first, frame).map(Some)
}

fn decode_body(first: u8, body: Bytes) -> Result<Packet, DecodeError> {
    let kind = first >> 4;
    let flags = first & 0x0F;
    let mut r = Reader(body);
    let required_flags = match kind {
        CONNECT | PINGREQ | DISCONNECT => Some(0),
        SUBSCRIBE | UNSUBSCRIBE => Some(0b0010),
        _ => None,
    };
    if required_flags.is_some_and(|f| f != flags) {
        return Err(DecodeError::Malformed("reserved fixed header flags"));
    }
    match kind {
        CONNECT => decode_connect(&mut r),
        PUBLISH => {
            match (flags >> 1) & 0b11 {
                0 => {}
                3 => return Err(DecodeError::Malformed("QoS 3")),
                _ => return Err(DecodeError::QosNotSupported),
            }
            let topic = r.string()?;
            let properties = forwardable_publish_properties(&r.properties()?)?;
            Ok(Packet::Publish(Publish {
                topic,
                properties,
                payload: r.0,
            }))
        }
        SUBSCRIBE => {
            let packet_id = r.u16()?;
            r.skip_properties()?;
            let mut entries = Vec::new();
            while r.0.has_remaining() {
                let filter = r.string()?;
                let options = r.u8()?;
                // reserved bits, QoS 3 and Retain Handling 3 are malformed
                if options & 0xC0 != 0 || options & 0b11 == 0b11 || (options >> 4) & 0b11 == 0b11 {
                    return Err(DecodeError::Malformed("subscription options"));
                }
                entries.push(SubscribeEntry {
                    filter,
                    no_local: options & 0b100 != 0,
                });
            }
            if entries.is_empty() {
                return Err(DecodeError::Malformed("SUBSCRIBE without filters"));
            }
            Ok(Packet::Subscribe { packet_id, entries })
        }
        UNSUBSCRIBE => {
            let packet_id = r.u16()?;
            r.skip_properties()?;
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
        // A DISCONNECT may consist of the fixed header only (remaining length 0).
        DISCONNECT => Ok(Packet::Disconnect),
        0 => Err(DecodeError::Malformed("reserved packet type")),
        other => Ok(Packet::Ignored(other)),
    }
}

fn decode_connect(r: &mut Reader) -> Result<Packet, DecodeError> {
    let name = r.string().map_err(|_| DecodeError::UnsupportedProtocol)?;
    if name != "MQTT" {
        return Err(DecodeError::UnsupportedProtocol);
    }
    if r.u8()? != 5 {
        return Err(DecodeError::UnsupportedProtocol);
    }
    let flags = r.u8()?;
    if flags & 1 != 0 {
        return Err(DecodeError::Malformed("reserved connect flag"));
    }
    let keep_alive = r.u16()?;
    let properties = r.properties()?;
    let mut max_packet_size = None;
    walk_properties(&properties, |id, value| {
        if id == 0x27 {
            let size = u32::from_be_bytes([value[0], value[1], value[2], value[3]]);
            if size == 0 {
                return Err(DecodeError::Malformed("maximum packet size 0"));
            }
            max_packet_size = Some(size);
        }
        Ok(())
    })?;
    let client_id = r.string()?;
    // Will, user name and password are not used by the broker (no authentication yet).
    Ok(Packet::Connect(Connect {
        client_id,
        keep_alive,
        clean_start: flags & 0b10 != 0,
        max_packet_size,
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

    fn string(&mut self) -> Result<String, DecodeError> {
        let len = usize::from(self.u16()?);
        if self.0.remaining() < len {
            return Err(DecodeError::Malformed("truncated string"));
        }
        let raw = self.0.split_to(len);
        String::from_utf8(raw.to_vec()).map_err(|_| DecodeError::Malformed("invalid UTF-8"))
    }

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

    fn skip_properties(&mut self) -> Result<(), DecodeError> {
        self.properties().map(|_| ())
    }

    /// Reads the property section and returns its bytes (without the length prefix).
    fn properties(&mut self) -> Result<Bytes, DecodeError> {
        let len = self.varint()?;
        if self.0.remaining() < len {
            return Err(DecodeError::Malformed("truncated properties"));
        }
        Ok(self.0.split_to(len))
    }
}

/// Length of the value of property `id` at the start of `rest`.
fn property_value_len(id: u8, rest: &[u8]) -> Result<usize, DecodeError> {
    let bad = DecodeError::Malformed("property");
    let prefixed = |at: usize| -> Result<usize, DecodeError> {
        let hdr = rest
            .get(at..at + 2)
            .ok_or(DecodeError::Malformed("property"))?;
        Ok(2 + usize::from(u16::from_be_bytes([hdr[0], hdr[1]])))
    };
    match id {
        0x01 | 0x17 | 0x19 | 0x24 | 0x25 | 0x28 | 0x29 | 0x2A => Ok(1),
        0x13 | 0x21 | 0x22 | 0x23 => Ok(2),
        0x02 | 0x11 | 0x18 | 0x27 => Ok(4),
        0x0B => {
            let n = rest.iter().take(4).position(|b| b & 0x80 == 0).ok_or(bad)?;
            Ok(n + 1)
        }
        0x03 | 0x08 | 0x09 | 0x12 | 0x15 | 0x16 | 0x1A | 0x1C | 0x1F => prefixed(0),
        0x26 => {
            let first = prefixed(0)?;
            Ok(first + prefixed(first)?)
        }
        _ => Err(bad),
    }
}

/// Calls `visit(id, value)` for every property; fails on unknown ids and truncated values.
fn walk_properties(
    props: &[u8],
    mut visit: impl FnMut(u8, &[u8]) -> Result<(), DecodeError>,
) -> Result<(), DecodeError> {
    let mut pos = 0;
    while pos < props.len() {
        let id = props[pos];
        let len = property_value_len(id, &props[pos + 1..])?;
        let value = props
            .get(pos + 1..pos + 1 + len)
            .ok_or(DecodeError::Malformed("truncated property"))?;
        visit(id, value)?;
        pos += 1 + len;
    }
    Ok(())
}

/// Removes Topic Alias (0x23) and Subscription Identifier (0x0B) from the PUBLISH properties. A topic alias
/// cannot be resolved because the broker announces no Topic Alias Maximum.
fn forwardable_publish_properties(props: &Bytes) -> Result<Bytes, DecodeError> {
    let mut out = BytesMut::with_capacity(props.len());
    let mut pos = 0;
    walk_properties(props, |id, value| {
        let end = pos + 1 + value.len();
        match id {
            0x23 => return Err(DecodeError::TopicAlias),
            0x0B => {}
            _ => out.put_slice(&props[pos..end]),
        }
        pos = end;
        Ok(())
    })?;
    Ok(out.freeze())
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

/// CONNACK with session_present = false, success and the properties maximum packet size and server keep alive.
/// `assigned_client_id` is announced as Assigned Client Identifier (0x12) when the client sent an empty id.
pub fn encode_connack(
    max_packet_size: u32,
    server_keep_alive: u16,
    assigned_client_id: Option<&str>,
) -> Bytes {
    let props_len = 8 + assigned_client_id.map_or(0, |id| 3 + id.len());
    let mut out = BytesMut::with_capacity(16 + props_len);
    let mut props_len_bytes = BytesMut::new();
    put_varint(&mut props_len_bytes, props_len);
    // ack flags + reason code + properties length + properties
    put_fixed_header(&mut out, 0x20, 2 + props_len_bytes.len() + props_len);
    out.put_u8(0); // session present = false
    out.put_u8(reason::SUCCESS);
    out.put_slice(&props_len_bytes);
    out.put_u8(0x27); // maximum packet size
    out.put_u32(max_packet_size);
    out.put_u8(0x13); // server keep alive
    out.put_u16(server_keep_alive);
    if let Some(id) = assigned_client_id {
        out.put_u8(0x12);
        out.put_u16(id.len() as u16);
        out.put_slice(id.as_bytes());
    }
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

/// Size in bytes of an encoded QoS 0 PUBLISH, `None` if it cannot be encoded at all.
pub fn publish_encoded_len(
    topic_len: usize,
    properties_len: usize,
    payload_len: usize,
) -> Option<usize> {
    let remaining = 2usize
        .checked_add(topic_len)?
        .checked_add(varint_len(properties_len))?
        .checked_add(properties_len)?
        .checked_add(payload_len)?;
    (remaining <= MAX_REMAINING_LENGTH).then(|| 1 + varint_len(remaining) + remaining)
}

/// Appends a QoS 0 PUBLISH to `out`. `properties` are raw property bytes without length prefix.
/// The caller makes sure the packet fits (see [`publish_encoded_len`]).
pub fn encode_publish(out: &mut BytesMut, topic: &str, properties: &[u8], payload: &[u8]) {
    let remaining =
        2 + topic.len() + varint_len(properties.len()) + properties.len() + payload.len();
    out.reserve(5 + remaining);
    put_fixed_header(out, 0x30, remaining);
    out.put_u16(topic.len() as u16);
    out.put_slice(topic.as_bytes());
    put_varint(out, properties.len());
    out.put_slice(properties);
    out.put_slice(payload);
}

pub fn encode_suback(packet_id: u16, reasons: &[u8]) -> Bytes {
    let mut out = BytesMut::with_capacity(8 + reasons.len());
    put_fixed_header(&mut out, 0x90, 2 + 1 + reasons.len());
    out.put_u16(packet_id);
    out.put_u8(0);
    out.put_slice(reasons);
    out.freeze()
}

pub fn encode_unsuback(packet_id: u16, reasons: &[u8]) -> Bytes {
    let mut out = BytesMut::with_capacity(8 + reasons.len());
    put_fixed_header(&mut out, 0xB0, 2 + 1 + reasons.len());
    out.put_u16(packet_id);
    out.put_u8(0);
    out.put_slice(reasons);
    out.freeze()
}

pub fn encode_pingresp() -> Bytes {
    Bytes::from_static(&[0xD0, 0x00])
}

pub fn encode_disconnect(reason_code: u8) -> Bytes {
    Bytes::copy_from_slice(&[0xE0, 0x01, reason_code])
}

#[cfg(test)]
mod tests {
    use super::*;

    fn buf(bytes: &[u8]) -> BytesMut {
        BytesMut::from(bytes)
    }

    fn connect_bytes(client_id: &str, keep_alive: u16) -> Vec<u8> {
        let mut body = vec![0, 4, b'M', b'Q', b'T', b'T', 5, 0b10];
        body.extend(keep_alive.to_be_bytes());
        body.push(0); // properties
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
                clean_start: true,
                max_packet_size: None
            })
        );
        assert!(b.is_empty());
    }

    #[test]
    fn connect_with_properties_and_empty_client_id() {
        // properties: session expiry (0x11) 4 bytes
        let body: Vec<u8> = vec![
            0, 4, b'M', b'Q', b'T', b'T', 5, 0, 0, 10, 5, 0x11, 0, 0, 0, 1, 0, 0,
        ];
        let mut raw = vec![0x10, body.len() as u8];
        raw.extend(body);
        let p = decode(&mut buf(&raw), 1024).unwrap().unwrap();
        assert_eq!(
            p,
            Packet::Connect(Connect {
                client_id: String::new(),
                keep_alive: 10,
                clean_start: false,
                max_packet_size: None
            })
        );
    }

    #[test]
    fn connect_rejects_other_protocols() {
        // MQTT 3.1.1
        let raw = [0x10, 12, 0, 4, b'M', b'Q', b'T', b'T', 4, 2, 0, 10, 0, 0];
        assert_eq!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::UnsupportedProtocol)
        );
        // wrong protocol name
        let raw = [0x10, 12, 0, 4, b'X', b'Q', b'T', b'T', 5, 2, 0, 10, 0, 0];
        assert_eq!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::UnsupportedProtocol)
        );
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
        assert_eq!(decode(&mut b, 16), Ok(Some(Packet::Disconnect)));
        assert_eq!(decode(&mut b, 16), Ok(None));
    }

    #[test]
    fn disconnect_with_reason_code() {
        assert_eq!(
            decode(&mut buf(&[0xE0, 0x01, 0x04]), 16),
            Ok(Some(Packet::Disconnect))
        );
    }

    #[test]
    fn publish_qos0() {
        // topic "a/b", no properties, payload "hi"
        let raw = [0x30, 8, 0, 3, b'a', b'/', b'b', 0, b'h', b'i'];
        assert_eq!(
            decode(&mut buf(&raw), 1024),
            Ok(Some(Packet::Publish(Publish {
                topic: "a/b".into(),
                properties: Bytes::new(),
                payload: Bytes::from_static(b"hi")
            })))
        );
    }

    #[test]
    fn publish_with_retain_dup_flags_and_properties() {
        // retain bit set, payload format indicator property (0x01, 1)
        let raw = [0x31, 9, 0, 1, b'a', 2, 0x01, 1, b'x', b'y', b'z'];
        let p = decode(&mut buf(&raw), 1024).unwrap().unwrap();
        assert_eq!(
            p,
            Packet::Publish(Publish {
                topic: "a".into(),
                properties: Bytes::from_static(&[0x01, 1]),
                payload: Bytes::from_static(b"xyz")
            })
        );
    }

    #[test]
    fn publish_qos1_and_qos2_are_rejected() {
        for flags in [0x32u8, 0x34] {
            let raw = [flags, 7, 0, 1, b'a', 0, 1, 0, 0];
            assert_eq!(
                decode(&mut buf(&raw), 1024),
                Err(DecodeError::QosNotSupported),
                "flags {flags:#x}"
            );
        }
        assert_eq!(
            DecodeError::QosNotSupported.disconnect_reason(),
            Some(reason::PROTOCOL_ERROR)
        );
    }

    #[test]
    fn subscribe_with_options() {
        // id 7, no props, "a/#" qos 0 + no_local, "b" qos 1
        let raw = [
            0x82, 13, 0, 7, 0, 0, 3, b'a', b'/', b'#', 0b100, 0, 1, b'b', 1,
        ];
        let p = decode(&mut buf(&raw), 1024).unwrap().unwrap();
        assert_eq!(
            p,
            Packet::Subscribe {
                packet_id: 7,
                entries: vec![
                    SubscribeEntry {
                        filter: "a/#".into(),
                        no_local: true
                    },
                    SubscribeEntry {
                        filter: "b".into(),
                        no_local: false
                    },
                ]
            }
        );
    }

    #[test]
    fn subscribe_without_filters_or_with_bad_options_is_malformed() {
        assert!(matches!(
            decode(&mut buf(&[0x82, 3, 0, 1, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
        assert!(matches!(
            decode(&mut buf(&[0x82, 7, 0, 1, 0, 0, 1, b'a', 0xC0]), 64),
            Err(DecodeError::Malformed(_))
        ));
        assert!(matches!(
            decode(&mut buf(&[0x82, 7, 0, 1, 0, 0, 1, b'a', 0x03]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn unsubscribe() {
        let raw = [0xA2, 8, 0, 9, 0, 0, 3, b'a', b'/', b'b'];
        assert_eq!(
            decode(&mut buf(&raw), 64),
            Ok(Some(Packet::Unsubscribe {
                packet_id: 9,
                filters: vec!["a/b".into()]
            }))
        );
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
        // property length behind the packet
        assert!(matches!(
            decode(&mut buf(&[0x30, 4, 0, 1, b'a', 9]), 64),
            Err(DecodeError::Malformed(_))
        ));
        // invalid UTF-8 topic
        assert!(matches!(
            decode(&mut buf(&[0x30, 4, 0, 1, 0xFF, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn reserved_and_unknown_types() {
        assert!(matches!(
            decode(&mut buf(&[0x00, 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
        assert_eq!(
            decode(&mut buf(&[0x40, 2, 0, 1]), 64),
            Ok(Some(Packet::Ignored(4)))
        );
    }

    #[test]
    fn connack_bytes() {
        let ack = encode_connack(262_144, 60, None);
        assert_eq!(
            &ack[..],
            &[0x20, 0x0B, 0x00, 0x00, 0x08, 0x27, 0x00, 0x04, 0x00, 0x00, 0x13, 0x00, 0x3C]
        );
    }

    #[test]
    fn publish_bytes_and_long_remaining_length() {
        let mut out = BytesMut::new();
        encode_publish(&mut out, "a/b", &[], b"hi");
        assert_eq!(&out[..], &[0x30, 8, 0, 3, b'a', b'/', b'b', 0, b'h', b'i']);

        let payload = vec![7u8; 300];
        let mut out = BytesMut::new();
        encode_publish(&mut out, "t", &[], &payload);
        // remaining = 2 + 1 + 1 + 300 = 304 = 0xB0 0x02
        assert_eq!(&out[..3], &[0x30, 0xB0, 0x02]);
        let decoded = decode(&mut out, 1024).unwrap().unwrap();
        assert_eq!(
            decoded,
            Packet::Publish(Publish {
                topic: "t".into(),
                properties: Bytes::new(),
                payload: Bytes::from(payload)
            })
        );
    }

    fn props_publish(props: &[u8], topic: &str, payload: &[u8]) -> Vec<u8> {
        let mut body = (topic.len() as u16).to_be_bytes().to_vec();
        body.extend(topic.as_bytes());
        body.push(props.len() as u8);
        body.extend(props);
        body.extend(payload);
        let mut out = vec![0x30, body.len() as u8];
        out.extend(body);
        out
    }

    #[test]
    fn publish_properties_are_kept_except_alias_and_subscription_id() {
        // response topic "r", correlation data [1,2], user property k=v, subscription id 5, content type "c"
        let mut props = vec![
            0x08, 0, 1, b'r', 0x09, 0, 2, 1, 2, 0x26, 0, 1, b'k', 0, 1, b'v',
        ];
        let kept = props.clone();
        props.extend([0x0B, 5]);
        props.extend([0x03, 0, 1, b'c']);
        let mut expected = kept;
        expected.extend([0x03, 0, 1, b'c']);
        let raw = props_publish(&props, "t", b"p");
        let Some(Packet::Publish(p)) = decode(&mut buf(&raw), 1024).unwrap() else {
            panic!("not a publish")
        };
        assert_eq!(&p.properties[..], &expected[..]);
        assert_eq!(&p.payload[..], b"p");
    }

    #[test]
    fn publish_with_topic_alias_is_a_protocol_error() {
        for topic in ["t", ""] {
            let raw = props_publish(&[0x23, 0, 1], topic, b"p");
            assert_eq!(decode(&mut buf(&raw), 1024), Err(DecodeError::TopicAlias));
        }
        assert_eq!(
            DecodeError::TopicAlias.disconnect_reason(),
            Some(reason::PROTOCOL_ERROR)
        );
    }

    #[test]
    fn publish_with_unknown_or_truncated_property_is_malformed() {
        for props in [
            &[0x7F, 0][..],
            &[0x08, 0, 5, b'r'][..],
            &[0x26, 0, 1, b'k'][..],
        ] {
            let raw = props_publish(props, "t", b"");
            assert!(matches!(
                decode(&mut buf(&raw), 1024),
                Err(DecodeError::Malformed(_))
            ));
        }
    }

    #[test]
    fn reserved_flags_are_checked() {
        for raw in [
            &[0x83u8, 6, 0, 1, 0, 0, 1, b'a', 0][..], // SUBSCRIBE flags 3
            &[0x80, 6, 0, 1, 0, 0, 1, b'a', 0][..],
            &[0xA0, 5, 0, 1, 0, 0, 1, b'a'][..],
            &[0xC1, 0][..],
            &[0xE1, 0][..],
            &[0x11, 0][..],
        ] {
            assert!(
                matches!(decode(&mut buf(raw), 64), Err(DecodeError::Malformed(_))),
                "{raw:x?}"
            );
        }
        // QoS 3 PUBLISH
        assert!(matches!(
            decode(&mut buf(&[0x36, 4, 0, 1, b'a', 0]), 64),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn retain_handling_3_is_malformed() {
        for options in [0x30u8, 0x20, 0x10] {
            let raw = [0x82, 7, 0, 1, 0, 0, 1, b'a', options];
            let r = decode(&mut buf(&raw), 64);
            if options == 0x30 {
                assert!(matches!(r, Err(DecodeError::Malformed(_))));
            } else {
                assert!(matches!(r, Ok(Some(Packet::Subscribe { .. }))));
            }
        }
    }

    #[test]
    fn connect_maximum_packet_size_property() {
        let body: Vec<u8> = vec![
            0, 4, b'M', b'Q', b'T', b'T', 5, 0, 0, 10, 5, 0x27, 0, 0, 2, 0, 0, 0,
        ];
        let mut raw = vec![0x10, body.len() as u8];
        raw.extend(body);
        let Some(Packet::Connect(c)) = decode(&mut buf(&raw), 1024).unwrap() else {
            panic!()
        };
        assert_eq!(c.max_packet_size, Some(512));
        // 0 is not allowed
        raw[15] = 0;
        raw[16] = 0;
        assert!(matches!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::Malformed(_))
        ));
    }

    #[test]
    fn connack_with_assigned_client_id() {
        let ack = encode_connack(1, 2, Some("ab"));
        assert_eq!(
            &ack[..],
            &[0x20, 16, 0, 0, 13, 0x27, 0, 0, 0, 1, 0x13, 0, 2, 0x12, 0, 2, b'a', b'b']
        );
    }

    #[test]
    fn publish_encoded_len_matches_encoder() {
        for (t, p, l) in [(1usize, 0usize, 0usize), (3, 5, 300), (10, 200, 20_000)] {
            let mut out = BytesMut::new();
            encode_publish(&mut out, &"t".repeat(t), &vec![1; p], &vec![2; l]);
            assert_eq!(publish_encoded_len(t, p, l), Some(out.len()));
        }
        assert_eq!(publish_encoded_len(1, 0, 268_435_455), None);
    }

    #[test]
    fn small_packets() {
        assert_eq!(
            &encode_suback(7, &[0x00, 0x97, 0x8F])[..],
            &[0x90, 6, 0, 7, 0, 0x00, 0x97, 0x8F]
        );
        assert_eq!(&encode_unsuback(9, &[0x00])[..], &[0xB0, 4, 0, 9, 0, 0]);
        assert_eq!(&encode_pingresp()[..], &[0xD0, 0]);
        assert_eq!(
            &encode_disconnect(reason::PROTOCOL_ERROR)[..],
            &[0xE0, 1, 0x82]
        );
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
            let mut r = Reader(out.freeze());
            assert_eq!(r.varint(), Ok(value));
        }
    }
}
