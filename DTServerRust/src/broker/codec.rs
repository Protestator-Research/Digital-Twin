//! Minimal MQTT 3.1.1 codec (protocol level 4) for exactly the packets the broker needs (QoS 0 only).
//!
//! Decoded: CONNECT, PUBLISH (QoS 0), SUBSCRIBE, UNSUBSCRIBE, PINGREQ, DISCONNECT. All other packet types
//! are reported as [`Packet::Ignored`]. Encoded: CONNACK, PUBLISH (QoS 0), SUBACK, UNSUBACK, PINGRESP.
//! MQTT 3.1.1 has no server-sent DISCONNECT and no properties, so none of that exists here.
//! Will, user name and password of a CONNECT are not interpreted.
//!
//! The size check happens on the fixed header, before the body is buffered.

use bytes::{Buf, BufMut, Bytes, BytesMut};
use thiserror::Error;

/// CONNACK return codes (MQTT 3.1.1, 3.2.2.3) used by the broker.
pub mod connack {
    pub const ACCEPTED: u8 = 0x00;
    /// Client identifier is correct UTF-8 but not allowed (here: empty id without clean session).
    pub const IDENTIFIER_REJECTED: u8 = 0x02;
    /// The server does not support the protocol level requested by the client.
    pub const UNACCEPTABLE_PROTOCOL_VERSION: u8 = 0x01;
}

/// SUBACK return codes (MQTT 3.1.1, 3.9.3).
pub mod suback {
    pub const GRANTED_QOS_0: u8 = 0x00;
    pub const FAILURE: u8 = 0x80;
}

#[derive(Debug, Clone, PartialEq, Eq, Error)]
pub enum DecodeError {
    #[error("packet of {size} bytes exceeds the maximum packet size of {max} bytes")]
    PacketTooLarge { size: usize, max: usize },
    #[error("malformed packet: {0}")]
    Malformed(&'static str),
    /// Protocol name is not `MQTT` (e.g. MQTT 3.1 `MQIsdp`): the connection is just closed.
    #[error("unsupported protocol (only MQTT 3.1.1 is supported)")]
    UnsupportedProtocol,
    /// Protocol name `MQTT` with a level other than 4 (e.g. MQTT 5): CONNACK 0x01, then close
    /// ([MQTT-3.1.2-2]).
    #[error("unsupported protocol level (only MQTT 3.1.1, level 4, is supported)")]
    UnacceptableProtocolLevel,
    #[error("only QoS 0 is supported")]
    QosNotSupported,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Connect {
    pub client_id: String,
    pub keep_alive: u16,
    pub clean_session: bool,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Publish {
    pub topic: String,
    pub payload: Bytes,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SubscribeEntry {
    pub filter: String,
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
    /// A well formed packet of a type the broker does not act on (PUBACK, ...).
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
            // [MQTT-3.3.1-2]: DUP must be 0 for QoS 0
            if flags & 0b1000 != 0 && (flags >> 1) & 0b11 == 0 {
                return Err(DecodeError::Malformed("DUP flag on a QoS 0 PUBLISH"));
            }
            match (flags >> 1) & 0b11 {
                0 => {}
                3 => return Err(DecodeError::Malformed("QoS 3")),
                _ => return Err(DecodeError::QosNotSupported),
            }
            let topic = r.string()?;
            Ok(Packet::Publish(Publish {
                topic,
                payload: r.0,
            }))
        }
        SUBSCRIBE => {
            let packet_id = r.u16()?;
            let mut entries = Vec::new();
            while r.0.has_remaining() {
                let filter = r.string()?;
                let options = r.u8()?;
                // 3.1.1: only bits 0-1 (requested QoS) are used, QoS 3 and any of bits 2-7 are malformed
                if options & 0xFC != 0 || options & 0b11 == 0b11 {
                    return Err(DecodeError::Malformed("requested QoS byte"));
                }
                entries.push(SubscribeEntry { filter });
            }
            if entries.is_empty() {
                return Err(DecodeError::Malformed("SUBSCRIBE without filters"));
            }
            Ok(Packet::Subscribe { packet_id, entries })
        }
        UNSUBSCRIBE => {
            let packet_id = r.u16()?;
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
        DISCONNECT => Ok(Packet::Disconnect),
        0 | 15 => Err(DecodeError::Malformed("reserved packet type")),
        other => Ok(Packet::Ignored(other)),
    }
}

fn decode_connect(r: &mut Reader) -> Result<Packet, DecodeError> {
    let name = r.string().map_err(|_| DecodeError::UnsupportedProtocol)?;
    if name != "MQTT" {
        return Err(DecodeError::UnsupportedProtocol);
    }
    if r.u8()? != 4 {
        return Err(DecodeError::UnacceptableProtocolLevel);
    }
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
    if flags & 0b1000_0000 == 0 && flags & 0b0100_0000 != 0 {
        return Err(DecodeError::Malformed(
            "password flag without user name flag",
        ));
    }
    let keep_alive = r.u16()?;
    let client_id = r.string()?;
    // Will topic/message, user name and password follow the client id; only their flags are checked above,
    // the content is not parsed or used (no authentication yet).
    Ok(Packet::Connect(Connect {
        client_id,
        keep_alive,
        clean_session: flags & 0b10 != 0,
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

/// CONNACK with session_present = false (the broker keeps no session state) and the given return code.
pub fn encode_connack(return_code: u8) -> Bytes {
    Bytes::copy_from_slice(&[0x20, 0x02, 0x00, return_code])
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
pub fn publish_encoded_len(topic_len: usize, payload_len: usize) -> Option<usize> {
    // the topic length is a 16 bit field
    if topic_len > usize::from(u16::MAX) {
        return None;
    }
    let remaining = 2usize.checked_add(topic_len)?.checked_add(payload_len)?;
    (remaining <= MAX_REMAINING_LENGTH).then(|| 1 + varint_len(remaining) + remaining)
}

/// Appends a QoS 0 PUBLISH to `out`. The caller makes sure the packet fits (see [`publish_encoded_len`]).
pub fn encode_publish(out: &mut BytesMut, topic: &str, payload: &[u8]) {
    let remaining = 2 + topic.len() + payload.len();
    out.reserve(5 + remaining);
    put_fixed_header(out, 0x30, remaining);
    out.put_u16(topic.len() as u16);
    out.put_slice(topic.as_bytes());
    out.put_slice(payload);
}

/// SUBACK with one return code ([`suback`]) per requested filter.
pub fn encode_suback(packet_id: u16, return_codes: &[u8]) -> Bytes {
    let mut out = BytesMut::with_capacity(8 + return_codes.len());
    put_fixed_header(&mut out, 0x90, 2 + return_codes.len());
    out.put_u16(packet_id);
    out.put_slice(return_codes);
    out.freeze()
}

pub fn encode_unsuback(packet_id: u16) -> Bytes {
    let id = packet_id.to_be_bytes();
    Bytes::copy_from_slice(&[0xB0, 0x02, id[0], id[1]])
}

pub fn encode_pingresp() -> Bytes {
    Bytes::from_static(&[0xD0, 0x00])
}

#[cfg(test)]
mod tests {
    use super::*;

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
            })
        );
    }

    #[test]
    fn connect_ignores_will_user_name_and_password() {
        // flags: user name, password, will flag, clean session; payload: id "a", will topic/message, user, pass
        let mut body = vec![
            0,
            4,
            b'M',
            b'Q',
            b'T',
            b'T',
            4,
            0b1100_0110,
            0,
            5,
            0,
            1,
            b'a',
        ];
        body.extend([0, 1, b'w', 0, 1, b'm', 0, 1, b'u', 0, 1, b'p']);
        let mut raw = vec![0x10, body.len() as u8];
        raw.extend(body);
        let p = decode(&mut buf(&raw), 1024).unwrap().unwrap();
        assert!(matches!(
            p,
            Packet::Connect(Connect {
                clean_session: true,
                ..
            })
        ));
    }

    #[test]
    fn connect_rejects_other_protocols() {
        // MQTT 5 (level 5, with a properties byte)
        let raw = [0x10, 13, 0, 4, b'M', b'Q', b'T', b'T', 5, 2, 0, 10, 0, 0, 0];
        assert_eq!(
            decode(&mut buf(&raw), 1024),
            Err(DecodeError::UnacceptableProtocolLevel)
        );
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
        for flags in [0b0010_1110u8, 0b1000_0010, 0b1100_0010] {
            assert!(
                matches!(
                    decode(&mut buf(&with_flags(flags)), 1024),
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
        assert_eq!(decode(&mut b, 16), Ok(Some(Packet::Disconnect)));
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
                payload: Bytes::from_static(b"hi")
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
                payload: Bytes::from_static(&[2, 0x01, 1])
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
                        filter: "a/#".into()
                    },
                    SubscribeEntry { filter: "b".into() },
                    SubscribeEntry { filter: "c".into() },
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
        assert_eq!(
            decode(&mut buf(&[0x40, 2, 0, 1]), 64),
            Ok(Some(Packet::Ignored(4)))
        );
    }

    #[test]
    fn connack_bytes() {
        assert_eq!(
            &encode_connack(connack::ACCEPTED)[..],
            &[0x20, 0x02, 0x00, 0x00]
        );
        assert_eq!(
            &encode_connack(connack::IDENTIFIER_REJECTED)[..],
            &[0x20, 0x02, 0x00, 0x02]
        );
    }

    #[test]
    fn publish_bytes_and_long_remaining_length() {
        let mut out = BytesMut::new();
        encode_publish(&mut out, "a/b", b"hi");
        assert_eq!(&out[..], &[0x30, 7, 0, 3, b'a', b'/', b'b', b'h', b'i']);

        let payload = vec![7u8; 300];
        let mut out = BytesMut::new();
        encode_publish(&mut out, "t", &payload);
        // remaining = 2 + 1 + 300 = 303 = 0xAF 0x02
        assert_eq!(&out[..3], &[0x30, 0xAF, 0x02]);
        let decoded = decode(&mut out, 1024).unwrap().unwrap();
        assert_eq!(
            decoded,
            Packet::Publish(Publish {
                topic: "t".into(),
                payload: Bytes::from(payload)
            })
        );
    }

    #[test]
    fn publish_encoded_len_matches_encoder() {
        for (t, l) in [(1usize, 0usize), (3, 300), (10, 20_000)] {
            let mut out = BytesMut::new();
            encode_publish(&mut out, &"t".repeat(t), &vec![2; l]);
            assert_eq!(publish_encoded_len(t, l), Some(out.len()));
        }
        assert_eq!(publish_encoded_len(1, 268_435_455), None);
        // topic length does not fit into the 16 bit length field
        assert!(publish_encoded_len(65_535, 0).is_some());
        assert_eq!(publish_encoded_len(65_536, 0), None);
    }

    #[test]
    fn small_packets() {
        assert_eq!(
            &encode_suback(7, &[suback::GRANTED_QOS_0, suback::FAILURE])[..],
            &[0x90, 4, 0, 7, 0x00, 0x80]
        );
        assert_eq!(&encode_unsuback(9)[..], &[0xB0, 2, 0, 9]);
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
}
