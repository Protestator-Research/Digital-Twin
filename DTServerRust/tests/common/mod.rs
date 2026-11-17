//! Helpers for the integration tests: a hand written raw MQTT client (independent of the broker's own
//! codec) and a thin wrapper around the rumqttc v5 client.
#![allow(dead_code)]

use std::time::Duration;

use dt_server::broker::{Broker, BrokerLimits};
use rumqttc::v5::mqttbytes::v5::Packet;
use rumqttc::v5::{AsyncClient, Event, MqttOptions};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpStream;
use tokio::sync::mpsc;
use tokio::time::timeout;

pub const WAIT: Duration = Duration::from_secs(5);

pub async fn start(limits: BrokerLimits) -> Broker {
    Broker::start(0, limits)
        .await
        .expect("broker starts on an ephemeral port")
}

pub fn addr(broker: &Broker) -> String {
    format!("127.0.0.1:{}", broker.port())
}

// ---------------------------------------------------------------------------------------------
// Raw packets
// ---------------------------------------------------------------------------------------------

pub fn varint(mut v: usize) -> Vec<u8> {
    let mut out = Vec::new();
    loop {
        let mut b = (v % 128) as u8;
        v /= 128;
        if v > 0 {
            b |= 0x80;
        }
        out.push(b);
        if v == 0 {
            return out;
        }
    }
}

pub fn packet(first: u8, body: &[u8]) -> Vec<u8> {
    let mut out = vec![first];
    out.extend(varint(body.len()));
    out.extend_from_slice(body);
    out
}

fn string(s: &str) -> Vec<u8> {
    let mut out = (s.len() as u16).to_be_bytes().to_vec();
    out.extend_from_slice(s.as_bytes());
    out
}

pub fn connect(client_id: &str) -> Vec<u8> {
    let mut body = string("MQTT");
    body.extend([5, 0b10, 0, 30, 0]); // v5, clean start, keep alive 30, no properties
    body.extend(string(client_id));
    packet(0x10, &body)
}

/// CONNECT with arbitrary connect flags, properties and extra payload bytes (will, user name, password).
pub fn connect_raw(flags: u8, props: &[u8], client_id: &str, tail: &[u8]) -> Vec<u8> {
    let mut body = string("MQTT");
    body.extend([5, flags, 0, 30]);
    body.extend(varint(props.len()));
    body.extend_from_slice(props);
    body.extend(string(client_id));
    body.extend_from_slice(tail);
    packet(0x10, &body)
}

pub fn string_bytes(s: &str) -> Vec<u8> {
    string(s)
}

pub fn subscribe(packet_id: u16, filters: &[(&str, bool)]) -> Vec<u8> {
    let mut body = packet_id.to_be_bytes().to_vec();
    body.push(0);
    for (filter, no_local) in filters {
        body.extend(string(filter));
        body.push(if *no_local { 0b100 } else { 0 });
    }
    packet(0x82, &body)
}

pub fn unsubscribe(packet_id: u16, filters: &[&str]) -> Vec<u8> {
    let mut body = packet_id.to_be_bytes().to_vec();
    body.push(0);
    for filter in filters {
        body.extend(string(filter));
    }
    packet(0xA2, &body)
}

pub fn publish(topic: &str, payload: &[u8]) -> Vec<u8> {
    let mut body = string(topic);
    body.push(0);
    body.extend_from_slice(payload);
    packet(0x30, &body)
}

pub fn publish_qos1(topic: &str, payload: &[u8]) -> Vec<u8> {
    let mut body = string(topic);
    body.extend([0, 1, 0]); // packet id 1, no properties
    body.extend_from_slice(payload);
    packet(0x32, &body)
}

pub fn pingreq() -> Vec<u8> {
    vec![0xC0, 0]
}

// ---------------------------------------------------------------------------------------------
// Raw connection
// ---------------------------------------------------------------------------------------------

pub struct Raw {
    pub stream: TcpStream,
}

impl Raw {
    pub async fn open(addr: &str) -> Raw {
        Raw {
            stream: TcpStream::connect(addr).await.expect("tcp connect"),
        }
    }

    /// Opens a connection and completes the CONNECT / CONNACK handshake.
    pub async fn connected(addr: &str, client_id: &str) -> Raw {
        let mut raw = Raw::open(addr).await;
        raw.send(&connect(client_id)).await;
        let (first, body) = raw.read_packet().await.expect("CONNACK");
        assert_eq!(first, 0x20, "expected CONNACK");
        assert_eq!(body[1], 0x00, "CONNACK reason code");
        raw
    }

    /// Sends `connect` and returns the connection with the body of the CONNACK.
    pub async fn connected_with(addr: &str, connect: &[u8]) -> (Raw, Vec<u8>) {
        let mut raw = Raw::open(addr).await;
        raw.send(connect).await;
        let (first, body) = raw.read_packet().await.expect("CONNACK");
        assert_eq!(first, 0x20, "expected CONNACK");
        assert_eq!(body[1], 0x00, "CONNACK reason code");
        (raw, body)
    }

    pub async fn send(&mut self, bytes: &[u8]) {
        self.stream.write_all(bytes).await.expect("write");
    }

    /// Reads one packet (first byte and body). `None` if the connection is closed or reset, panics on timeout.
    pub async fn read_packet(&mut self) -> Option<(u8, Vec<u8>)> {
        self.read_packet_within(WAIT)
            .await
            .expect("timed out waiting for a packet")
    }

    /// `Err(())` on timeout, `Ok(None)` if the connection is closed.
    pub async fn read_packet_within(
        &mut self,
        limit: Duration,
    ) -> Result<Option<(u8, Vec<u8>)>, ()> {
        match timeout(limit, self.read_packet_inner()).await {
            Ok(r) => Ok(r),
            Err(_) => Err(()),
        }
    }

    async fn read_packet_inner(&mut self) -> Option<(u8, Vec<u8>)> {
        let first = self.stream.read_u8().await.ok()?;
        let mut len = 0usize;
        let mut shift = 0;
        loop {
            let b = self.stream.read_u8().await.ok()?;
            len |= usize::from(b & 0x7F) << shift;
            if b & 0x80 == 0 {
                break;
            }
            shift += 7;
        }
        let mut body = vec![0u8; len];
        self.stream.read_exact(&mut body).await.ok()?;
        Some((first, body))
    }

    /// Reads packets until the connection is closed. Returns the last packet seen (e.g. a DISCONNECT).
    pub async fn read_until_closed(&mut self) -> Option<(u8, Vec<u8>)> {
        let mut last = None;
        loop {
            match self.read_packet_within(WAIT).await {
                Ok(Some(p)) => last = Some(p),
                Ok(None) => return last,
                Err(()) => panic!("connection was not closed in time"),
            }
        }
    }

    /// Subscribes and waits for the SUBACK. Returns its reason codes.
    pub async fn subscribe(&mut self, packet_id: u16, filters: &[(&str, bool)]) -> Vec<u8> {
        self.send(&subscribe(packet_id, filters)).await;
        let (first, body) = self.read_packet().await.expect("SUBACK");
        assert_eq!(first, 0x90);
        assert_eq!(u16::from_be_bytes([body[0], body[1]]), packet_id);
        body[3..].to_vec()
    }

    /// Reads a PUBLISH and returns topic and payload.
    pub async fn read_publish(&mut self) -> (String, Vec<u8>) {
        let (first, body) = self.read_packet().await.expect("PUBLISH");
        assert_eq!(first & 0xF0, 0x30, "expected PUBLISH, got {first:#x}");
        let tlen = usize::from(u16::from_be_bytes([body[0], body[1]]));
        let topic = String::from_utf8(body[2..2 + tlen].to_vec()).unwrap();
        let props = usize::from(body[2 + tlen]);
        (topic, body[3 + tlen + props..].to_vec())
    }

    /// True if nothing arrives within `limit`.
    pub async fn is_silent_for(&mut self, limit: Duration) -> bool {
        matches!(self.read_packet_within(limit).await, Err(()))
    }
}

// ---------------------------------------------------------------------------------------------
// rumqttc
// ---------------------------------------------------------------------------------------------

pub struct Client {
    pub client: AsyncClient,
    pub events: mpsc::UnboundedReceiver<Event>,
    pub connack: rumqttc::v5::mqttbytes::v5::ConnAck,
}

impl Client {
    pub async fn connect(port: u16, id: &str) -> Client {
        let mut options = MqttOptions::new(id, "127.0.0.1", port);
        options.set_keep_alive(Duration::from_secs(30));
        let (client, mut eventloop) = AsyncClient::new(options, 16);
        let (tx, mut events) = mpsc::unbounded_channel();
        tokio::spawn(async move {
            while let Ok(event) = eventloop.poll().await {
                if tx.send(event).is_err() {
                    break;
                }
            }
        });
        let connack = loop {
            match timeout(WAIT, events.recv())
                .await
                .expect("CONNACK in time")
                .expect("event loop running")
            {
                Event::Incoming(Packet::ConnAck(ack)) => break ack,
                _ => continue,
            }
        };
        Client {
            client,
            events,
            connack,
        }
    }

    /// Waits for the next incoming packet that matches `pick`.
    pub async fn wait_for<T>(&mut self, mut pick: impl FnMut(&Packet) -> Option<T>) -> T {
        loop {
            match timeout(WAIT, self.events.recv())
                .await
                .expect("event in time")
                .expect("event loop running")
            {
                Event::Incoming(p) => {
                    if let Some(v) = pick(&p) {
                        return v;
                    }
                }
                Event::Outgoing(_) => {}
            }
        }
    }

    pub async fn wait_suback(&mut self) -> Vec<rumqttc::v5::mqttbytes::v5::SubscribeReasonCode> {
        self.wait_for(|p| match p {
            Packet::SubAck(s) => Some(s.return_codes.clone()),
            _ => None,
        })
        .await
    }

    pub async fn next_publish(&mut self) -> (String, Vec<u8>) {
        self.wait_for(|p| match p {
            Packet::Publish(p) => Some((
                String::from_utf8(p.topic.to_vec()).unwrap(),
                p.payload.to_vec(),
            )),
            _ => None,
        })
        .await
    }

    /// True if no PUBLISH arrives within `limit`.
    pub async fn no_publish_within(&mut self, limit: Duration) -> bool {
        let deadline = tokio::time::Instant::now() + limit;
        loop {
            match tokio::time::timeout_at(deadline, self.events.recv()).await {
                Err(_) => return true,
                Ok(None) => return true,
                Ok(Some(Event::Incoming(Packet::Publish(_)))) => return false,
                Ok(Some(_)) => {}
            }
        }
    }
}
