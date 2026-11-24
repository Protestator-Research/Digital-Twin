//! QoS 1 and 2, resumable sessions: the handshakes in both directions, downgrade to the subscription QoS,
//! redelivery after a reconnect, offline queueing, session take-over and expiry, flow control. Raw clients
//! pin the bytes, rumqttc clients show that real clients cope.

mod common;

use std::future::Future;
use std::pin::Pin;
use std::sync::Arc;
use std::time::Duration;

use common::*;
use dt_server::auth::{AuthOutcome, Authenticator};
use dt_server::broker::{Broker, BrokerLimits, BrokerOptions, ProtocolMode};
use rumqttc::v5::mqttbytes::QoS as QoS5;
use rumqttc::{Packet, QoS, SubscribeReasonCode};
use tokio::io::AsyncWriteExt;

const SHORT: Duration = Duration::from_millis(400);

async fn both() -> Broker {
    start_mode(BrokerLimits::default(), ProtocolMode::Both).await
}

/// Polls until `cond` holds (the broker works asynchronously after a connection ended).
async fn eventually(what: &str, mut cond: impl FnMut() -> bool) {
    for _ in 0..250 {
        if cond() {
            return;
        }
        tokio::time::sleep(Duration::from_millis(20)).await;
    }
    panic!("timed out waiting for: {what}");
}

async fn wait_for_connections(broker: &Broker, expected: usize) {
    eventually("connections", || broker.connection_count() == expected).await;
}

/// A raw 3.1.1 subscriber that subscribed to `filter` with `qos`.
async fn raw_subscriber(addr: &str, id: &str, clean: bool, filter: &str, qos: u8) -> Raw {
    let (mut raw, _) = Raw::connected_session(addr, id, clean).await;
    raw.send(&subscribe_qos(1, &[(filter, qos)])).await;
    let (first, body) = raw.read_packet().await.unwrap();
    assert_eq!((first, &body[..]), (0x90, &[0, 1, qos][..]));
    raw
}

// --- round trips -------------------------------------------------------------------------------

#[tokio::test]
async fn qos1_round_trip_with_raw_clients_in_3_1_1() {
    let broker = both().await;
    let a = addr(&broker);
    let mut sub = raw_subscriber(&a, "sub", true, "t", 1).await;
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;

    publisher
        .send(&publish_with_id("t", b"hello", 1, 7, false))
        .await;
    assert_eq!(publisher.read_ack(PUBACK).await, (7, None));
    let got = sub.read_any_publish(false).await;
    assert_eq!(
        got,
        Received {
            topic: "t".into(),
            payload: b"hello".to_vec(),
            qos: 1,
            dup: false,
            packet_id: got.packet_id
        }
    );
    assert_ne!(got.packet_id, 0);
    sub.send(&ack(PUBACK, got.packet_id)).await;
    // the acknowledged message is not sent again, and the connection works on
    sub.send(&pingreq()).await;
    assert_eq!(sub.read_packet().await.unwrap().0, 0xD0);
    broker.shutdown().await;
}

#[tokio::test]
async fn qos2_round_trip_with_raw_clients_in_3_1_1() {
    let broker = both().await;
    let a = addr(&broker);
    let mut sub = raw_subscriber(&a, "sub", true, "t", 2).await;
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;

    // publisher side: PUBLISH, PUBREC, PUBREL, PUBCOMP
    publisher
        .send(&publish_with_id("t", b"once", 2, 3, false))
        .await;
    assert_eq!(publisher.read_ack(PUBREC).await, (3, None));
    // subscriber side: PUBLISH, PUBREC, PUBREL, PUBCOMP
    let got = sub.read_any_publish(false).await;
    assert_eq!((got.qos, got.payload.as_slice()), (2, &b"once"[..]));
    sub.send(&ack(PUBREC, got.packet_id)).await;
    assert_eq!(sub.read_ack(PUBREL).await, (got.packet_id, None));
    sub.send(&ack(PUBCOMP, got.packet_id)).await;
    publisher.send(&ack(PUBREL, 3)).await;
    assert_eq!(publisher.read_ack(PUBCOMP).await, (3, None));
    // exactly once
    assert!(sub.is_silent_for(SHORT).await);
    broker.shutdown().await;
}

#[tokio::test]
async fn qos1_and_qos2_round_trip_with_raw_clients_in_mqtt5() {
    let broker = both().await;
    let a = addr(&broker);
    let (mut sub, _) = Raw::connected5(&a, "sub5").await;
    assert_eq!(sub.subscribe5(1, &[("t", 2)]).await, [2]);
    let (mut publisher, _) = Raw::connected5(&a, "pub5").await;

    publisher.send(&publish5_with_id("t", b"one", 1, 11)).await;
    assert_eq!(publisher.read_ack(PUBACK).await, (11, None));
    let got = sub.read_any_publish(true).await;
    assert_eq!((got.qos, got.payload.as_slice()), (1, &b"one"[..]));
    sub.send(&ack(PUBACK, got.packet_id)).await;

    publisher.send(&publish5_with_id("t", b"two", 2, 12)).await;
    assert_eq!(publisher.read_ack(PUBREC).await, (12, None));
    let got = sub.read_any_publish(true).await;
    assert_eq!((got.qos, got.payload.as_slice()), (2, &b"two"[..]));
    sub.send(&ack(PUBREC, got.packet_id)).await;
    assert_eq!(sub.read_ack(PUBREL).await, (got.packet_id, None));
    sub.send(&ack(PUBCOMP, got.packet_id)).await;
    publisher.send(&ack(PUBREL, 12)).await;
    assert_eq!(publisher.read_ack(PUBCOMP).await, (12, None));
    assert!(sub.is_silent_for(SHORT).await);
    broker.shutdown().await;
}

#[tokio::test]
async fn qos1_and_qos2_round_trip_with_rumqttc_in_3_1_1() {
    let broker = both().await;
    let mut sub = Client::connect(broker.port(), "sub").await;
    let mut publisher = Client::connect(broker.port(), "pub").await;
    sub.client.subscribe("t", QoS::ExactlyOnce).await.unwrap();
    assert_eq!(
        sub.wait_suback().await,
        vec![SubscribeReasonCode::Success(QoS::ExactlyOnce)]
    );
    for (qos, payload) in [(QoS::AtLeastOnce, "one"), (QoS::ExactlyOnce, "two")] {
        publisher
            .client
            .publish("t", qos, false, payload)
            .await
            .unwrap();
        let got = sub
            .wait_for(|p| match p {
                Packet::Publish(p) => Some((p.qos, p.dup, p.payload.to_vec())),
                _ => None,
            })
            .await;
        assert_eq!(got, (qos, false, payload.as_bytes().to_vec()));
    }
    // the publisher's handshakes completed
    publisher
        .wait_for(|p| matches!(p, Packet::PubAck(_)).then_some(()))
        .await;
    publisher
        .wait_for(|p| matches!(p, Packet::PubComp(_)).then_some(()))
        .await;
    assert!(sub.no_publish_within(SHORT).await);
    broker.shutdown().await;
}

#[tokio::test]
async fn qos1_and_qos2_round_trip_with_rumqttc_in_mqtt5() {
    use rumqttc::v5::Incoming;
    let broker = both().await;
    let mut sub = ClientV5::connect(broker.port(), "sub").await;
    let mut publisher = ClientV5::connect(broker.port(), "pub").await;
    sub.client.subscribe("t", QoS5::ExactlyOnce).await.unwrap();
    assert_eq!(
        sub.wait_suback().await,
        vec![rumqttc::v5::mqttbytes::v5::SubscribeReasonCode::Success(
            QoS5::ExactlyOnce
        )]
    );
    for (qos, payload) in [(QoS5::AtLeastOnce, "one"), (QoS5::ExactlyOnce, "two")] {
        publisher
            .client
            .publish("t", qos, false, payload)
            .await
            .unwrap();
        let got = sub
            .wait_for(|p| match p {
                Incoming::Publish(p) => Some((p.qos, p.dup, p.payload.to_vec())),
                _ => None,
            })
            .await;
        assert_eq!(got, (qos, false, payload.as_bytes().to_vec()));
    }
    publisher
        .wait_for(|p| matches!(p, Incoming::PubAck(_)).then_some(()))
        .await;
    publisher
        .wait_for(|p| matches!(p, Incoming::PubComp(_)).then_some(()))
        .await;
    assert!(sub.no_publish_within(SHORT).await);
    broker.shutdown().await;
}

// --- delivery QoS ------------------------------------------------------------------------------

#[tokio::test]
async fn delivery_qos_is_the_lower_of_publish_and_subscription() {
    let broker = both().await;
    let a = addr(&broker);
    let mut s0 = raw_subscriber(&a, "s0", true, "t", 0).await;
    let mut s1 = raw_subscriber(&a, "s1", true, "t", 1).await;
    let mut s2 = raw_subscriber(&a, "s2", true, "t", 2).await;
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;

    for (qos, id) in [(0u8, 0u16), (1, 1), (2, 2)] {
        if qos == 0 {
            publisher.send(&publish("t", b"m")).await;
        } else {
            publisher
                .send(&publish_with_id("t", b"m", qos, id, false))
                .await;
            publisher
                .read_ack(if qos == 1 { PUBACK } else { PUBREC })
                .await;
        }
        for (raw, granted) in [(&mut s0, 0u8), (&mut s1, 1), (&mut s2, 2)] {
            let got = raw.read_any_publish(false).await;
            assert_eq!(
                got.qos,
                qos.min(granted),
                "published {qos}, granted {granted}"
            );
            // acknowledge so that nothing stays in flight
            match got.qos {
                1 => raw.send(&ack(PUBACK, got.packet_id)).await,
                2 => {
                    raw.send(&ack(PUBREC, got.packet_id)).await;
                    raw.read_ack(PUBREL).await;
                    raw.send(&ack(PUBCOMP, got.packet_id)).await;
                }
                _ => {}
            }
        }
        if qos == 2 {
            publisher.send(&ack(PUBREL, id)).await;
            publisher.read_ack(PUBCOMP).await;
        }
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn overlapping_filters_deliver_once_with_the_highest_granted_qos() {
    let broker = both().await;
    let a = addr(&broker);
    let mut sub = raw_subscriber(&a, "sub", true, "a/b", 0).await;
    sub.send(&subscribe_qos(2, &[("a/+", 1), ("#", 0)])).await;
    assert_eq!(sub.read_packet().await.unwrap(), (0x90, vec![0, 2, 1, 0]));
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    publisher
        .send(&publish_with_id("a/b", b"m", 2, 1, false))
        .await;
    publisher.read_ack(PUBREC).await;
    let got = sub.read_any_publish(false).await;
    assert_eq!(got.qos, 1);
    sub.send(&ack(PUBACK, got.packet_id)).await;
    assert!(sub.is_silent_for(SHORT).await, "one delivery per session");
    broker.shutdown().await;
}

// --- sessions: redelivery, offline queue, present flag -----------------------------------------

#[tokio::test]
async fn unacknowledged_publishes_are_sent_again_with_dup_after_a_reconnect() {
    let broker = both().await;
    let a = addr(&broker);
    let mut sub = raw_subscriber(&a, "keeper", false, "t", 1).await;
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    for (i, payload) in [b"first", b"other"].iter().enumerate() {
        publisher
            .send(&publish_with_id("t", *payload, 1, 1 + i as u16, false))
            .await;
        publisher.read_ack(PUBACK).await;
    }
    let first = sub.read_any_publish(false).await;
    let second = sub.read_any_publish(false).await;
    assert_eq!((first.dup, second.dup), (false, false));
    // the first is acknowledged, the second is not; then the connection breaks
    sub.send(&ack(PUBACK, first.packet_id)).await;
    drop(sub);

    let (mut sub, present) = Raw::connected_session(&a, "keeper", false).await;
    assert!(present, "the session was kept");
    let again = sub.read_any_publish(false).await;
    assert_eq!(
        again,
        Received {
            topic: "t".into(),
            payload: b"other".to_vec(),
            qos: 1,
            dup: true,
            packet_id: second.packet_id
        }
    );
    sub.send(&ack(PUBACK, again.packet_id)).await;
    sub.send(&pingreq()).await;
    assert_eq!(
        sub.read_packet().await.unwrap().0,
        0xD0,
        "nothing else was pending"
    );
    drop(sub);

    // acknowledged for good: a third connection gets nothing
    let (mut sub, present) = Raw::connected_session(&a, "keeper", false).await;
    assert!(present);
    assert!(sub.is_silent_for(SHORT).await);
    broker.shutdown().await;
}

#[tokio::test]
async fn a_qos2_flow_is_continued_with_pubrel_after_a_reconnect() {
    let broker = both().await;
    let a = addr(&broker);
    let mut sub = raw_subscriber(&a, "keeper", false, "t", 2).await;
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    for id in 1..=2u16 {
        publisher
            .send(&publish_with_id(
                "t",
                format!("m{id}").as_bytes(),
                2,
                id,
                false,
            ))
            .await;
        publisher.read_ack(PUBREC).await;
    }
    let first = sub.read_any_publish(false).await;
    let second = sub.read_any_publish(false).await;
    // PUBREC for the first only, then the connection breaks before PUBCOMP
    sub.send(&ack(PUBREC, first.packet_id)).await;
    sub.read_ack(PUBREL).await;
    drop(sub);

    let (mut sub, present) = Raw::connected_session(&a, "keeper", false).await;
    assert!(present);
    // PUBREL first (the PUBLISH must not be repeated), then the message that was not acknowledged
    assert_eq!(sub.read_ack(PUBREL).await, (first.packet_id, None));
    let again = sub.read_any_publish(false).await;
    assert_eq!(
        (again.payload.as_slice(), again.dup, again.packet_id),
        (&b"m2"[..], true, second.packet_id)
    );
    sub.send(&ack(PUBCOMP, first.packet_id)).await;
    sub.send(&ack(PUBREC, again.packet_id)).await;
    sub.read_ack(PUBREL).await;
    sub.send(&ack(PUBCOMP, again.packet_id)).await;
    sub.send(&pingreq()).await;
    assert_eq!(sub.read_packet().await.unwrap().0, 0xD0);
    broker.shutdown().await;
}

#[tokio::test]
async fn messages_for_an_offline_session_are_queued_in_order() {
    let broker = both().await;
    let a = addr(&broker);
    let sub = raw_subscriber(&a, "sleeper", false, "t/#", 1).await;
    drop(sub);
    wait_for_connections(&broker, 0).await;
    // the subscription outlives the connection
    assert_eq!(broker.subscription_count(), 1);

    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    for i in 0..3u16 {
        publisher
            .send(&publish_with_id(
                "t/x",
                format!("m{i}").as_bytes(),
                1,
                i + 1,
                false,
            ))
            .await;
        publisher.read_ack(PUBACK).await;
    }
    // QoS 0 is not kept for an offline session
    publisher.send(&publish("t/x", b"qos0")).await;
    publisher.send(&pingreq()).await;
    publisher.read_packet().await.unwrap();

    let (mut sub, present) = Raw::connected_session(&a, "sleeper", false).await;
    assert!(present);
    for i in 0..3 {
        let got = sub.read_any_publish(false).await;
        assert_eq!(
            (got.payload, got.qos, got.dup),
            (format!("m{i}").into_bytes(), 1, false)
        );
        sub.send(&ack(PUBACK, got.packet_id)).await;
    }
    assert!(sub.is_silent_for(SHORT).await);
    broker.shutdown().await;
}

#[tokio::test]
async fn offline_queueing_works_for_rumqttc_clients_in_both_versions() {
    let broker = both().await;
    // 3.1.1
    let mut keeper = Client::connect_with(broker.port(), "keeper", false).await;
    assert!(!keeper.connack.session_present);
    keeper
        .client
        .subscribe("t", QoS::AtLeastOnce)
        .await
        .unwrap();
    keeper.wait_suback().await;
    keeper.client.disconnect().await.unwrap();
    // wait for the end of the event stream: the connection is closed
    while keeper.events.recv().await.is_some() {}
    let mut publisher = Client::connect(broker.port(), "pub").await;
    publisher
        .client
        .publish("t", QoS::AtLeastOnce, false, "while away")
        .await
        .unwrap();
    // the PUBACK of the publisher means the message is queued
    publisher
        .wait_for(|p| matches!(p, Packet::PubAck(_)).then_some(()))
        .await;
    let mut keeper = Client::connect_with(broker.port(), "keeper", false).await;
    assert!(keeper.connack.session_present);
    assert_eq!(keeper.next_publish().await.1, b"while away");

    // MQTT 5
    let mut keeper = ClientV5::connect_with(broker.port(), "keeper5", false, Some(600)).await;
    assert!(!keeper.connack.session_present);
    keeper
        .client
        .subscribe("u", QoS5::AtLeastOnce)
        .await
        .unwrap();
    keeper.wait_suback().await;
    keeper.client.disconnect().await.unwrap();
    while keeper.events.recv().await.is_some() {}
    let mut publisher = ClientV5::connect(broker.port(), "pub5").await;
    publisher
        .client
        .publish("u", QoS5::AtLeastOnce, false, "while away")
        .await
        .unwrap();
    publisher
        .wait_for(|p| matches!(p, rumqttc::v5::Incoming::PubAck(_)).then_some(()))
        .await;
    let mut keeper = ClientV5::connect_with(broker.port(), "keeper5", false, Some(600)).await;
    assert!(keeper.connack.session_present);
    assert_eq!(keeper.next_publish().await.1, b"while away");
    broker.shutdown().await;
}

#[tokio::test]
async fn session_present_follows_the_clean_flag() {
    let broker = both().await;
    let a = addr(&broker);
    // 3.1.1
    let (raw, present) = Raw::connected_session(&a, "c", false).await;
    assert!(!present, "nothing to resume");
    drop(raw);
    let (raw, present) = Raw::connected_session(&a, "c", false).await;
    assert!(present);
    drop(raw);
    // a clean session destroys the old one, and leaves none behind
    let (raw, present) = Raw::connected_session(&a, "c", true).await;
    assert!(!present);
    drop(raw);
    let (raw, present) = Raw::connected_session(&a, "c", false).await;
    assert!(!present);
    drop(raw);

    // MQTT 5: the session lives only with an expiry
    let (raw, body) = Raw::connected5_session(&a, "c5", false, 0, &[]).await;
    assert_eq!(body[0], 0);
    drop(raw);
    wait_for_connections(&broker, 0).await;
    let (raw, body) = Raw::connected5_session(&a, "c5", false, 0, &[]).await;
    assert_eq!(body[0], 0, "expiry 0 ends the session with the connection");
    drop(raw);
    let (raw, _) = Raw::connected5_session(&a, "c5", false, 100, &[]).await;
    drop(raw);
    let (raw, body) = Raw::connected5_session(&a, "c5", false, 100, &[]).await;
    assert_eq!(body[0], 1);
    drop(raw);
    let (_raw, body) = Raw::connected5_session(&a, "c5", true, 100, &[]).await;
    assert_eq!(body[0], 0, "clean start");
    broker.shutdown().await;
}

// --- take over ---------------------------------------------------------------------------------

#[tokio::test]
async fn a_second_connection_with_the_same_client_id_takes_the_session_over() {
    let broker = both().await;
    let a = addr(&broker);

    // MQTT 5: the old connection gets DISCONNECT 0x8E
    let (mut old, _) = Raw::connected5_session(&a, "same", false, 600, &[]).await;
    assert_eq!(old.subscribe5(1, &[("t", 1)]).await, [1]);
    let (mut new, body) = Raw::connected5_session(&a, "same", false, 600, &[]).await;
    assert_eq!(body[0], 1, "the new connection resumes the session");
    assert_eq!(old.read_until_closed().await, Some((0xE0, vec![0x8E, 0])));
    // the subscription moved with the session, and the cleanup of the old connection did not disturb it
    let (mut publisher, _) = Raw::connected5(&a, "pub").await;
    publisher
        .send(&publish5_with_id("t", b"for new", 1, 1))
        .await;
    publisher.read_ack(PUBACK).await;
    assert_eq!(new.read_any_publish(true).await.payload, b"for new");
    assert_eq!(broker.subscription_count(), 1);

    // MQTT 3.1.1: just closed
    let (mut old, _) = Raw::connected_session(&a, "same311", false).await;
    old.send(&subscribe_qos(1, &[("u", 1)])).await;
    old.read_packet().await.unwrap();
    let (mut new, present) = Raw::connected_session(&a, "same311", false).await;
    assert!(present);
    assert!(old.read_until_closed().await.is_none());
    publisher
        .send(&publish5_with_id("u", b"for new", 1, 2))
        .await;
    publisher.read_ack(PUBACK).await;
    assert_eq!(new.read_any_publish(false).await.payload, b"for new");
    broker.shutdown().await;
}

#[tokio::test]
async fn a_take_over_with_clean_start_replaces_the_session() {
    let broker = both().await;
    let a = addr(&broker);
    let (mut old, _) = Raw::connected5_session(&a, "same", false, 600, &[]).await;
    old.subscribe5(1, &[("t", 1)]).await;
    let (_new, body) = Raw::connected5_session(&a, "same", true, 600, &[]).await;
    assert_eq!(body[0], 0);
    assert_eq!(old.read_until_closed().await, Some((0xE0, vec![0x8E, 0])));
    // the subscriptions of the destroyed session are gone
    assert_eq!(broker.subscription_count(), 0);
    broker.shutdown().await;
}

// --- session expiry ----------------------------------------------------------------------------

#[tokio::test]
async fn an_offline_session_expires_and_the_sweeper_removes_it() {
    let broker = both().await;
    let a = addr(&broker);
    let (mut raw, _) = Raw::connected5_session(&a, "short", false, 1, &[]).await;
    raw.subscribe5(1, &[("t", 1)]).await;
    drop(raw);
    wait_for_connections(&broker, 0).await;
    assert_eq!(broker.subscription_count(), 1, "kept while offline");
    // 1 s expiry plus up to one sweep interval
    eventually("expired session removed", || {
        broker.subscription_count() == 0
    })
    .await;
    let (_raw, body) = Raw::connected5_session(&a, "short", false, 1, &[]).await;
    assert_eq!(body[0], 0);
    broker.shutdown().await;
}

#[tokio::test]
async fn the_broker_caps_the_session_expiry_and_says_so() {
    let limits = BrokerLimits {
        session_expiry_max_s: 10,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::Both).await;
    let a = addr(&broker);
    let announced = |body: &[u8]| body.windows(5).any(|w| w == [0x11, 0, 0, 0, 10]);
    let (_c1, body) = Raw::connected5_session(&a, "big", false, 100, &[]).await;
    assert!(
        announced(&body),
        "CONNACK carries the shorter expiry: {body:?}"
    );
    let (_c2, body) = Raw::connected5_session(&a, "ok", false, 10, &[]).await;
    assert!(
        !announced(&body),
        "not announced when granted as asked: {body:?}"
    );
    let (_c3, body) = Raw::connected5_session(&a, "zero", true, 0, &[]).await;
    assert!(!body.contains(&0x11));
    broker.shutdown().await;
}

#[tokio::test]
async fn disconnect_can_lower_the_session_expiry_but_raising_zero_is_a_protocol_error() {
    let broker = both().await;
    let a = addr(&broker);
    let disconnect_with_expiry = |expiry: u32| {
        let mut p = vec![0xE0, 7, 0x00, 5, 0x11];
        p.extend(expiry.to_be_bytes());
        p
    };
    // expiry 100 lowered to 0: the session ends with the connection
    let (mut raw, _) = Raw::connected5_session(&a, "lower", false, 100, &[]).await;
    raw.subscribe5(1, &[("t", 0)]).await;
    raw.send(&disconnect_with_expiry(0)).await;
    assert!(raw.read_until_closed().await.is_none());
    wait_for_connections(&broker, 0).await;
    assert_eq!(broker.subscription_count(), 0);

    // expiry 0 cannot be raised by DISCONNECT: protocol error
    let (mut raw, _) = Raw::connected5_session(&a, "raise", false, 0, &[]).await;
    raw.subscribe5(1, &[("t", 0)]).await;
    raw.send(&disconnect_with_expiry(100)).await;
    assert_eq!(raw.read_until_closed().await, Some((0xE0, vec![0x82, 0])));
    wait_for_connections(&broker, 0).await;
    assert_eq!(broker.subscription_count(), 0);

    // 100 stays 100
    let (mut raw, _) = Raw::connected5_session(&a, "keep", false, 100, &[]).await;
    raw.subscribe5(1, &[("t", 0)]).await;
    raw.send(&packet(0xE0, &[0x00])).await;
    assert!(raw.read_until_closed().await.is_none());
    wait_for_connections(&broker, 0).await;
    assert_eq!(broker.subscription_count(), 1);
    broker.shutdown().await;
}

#[tokio::test]
async fn too_many_persistent_sessions_are_refused() {
    let limits = BrokerLimits {
        max_persistent_sessions: 1,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::Both).await;
    let a = addr(&broker);
    let (_first, _) = Raw::connected_session(&a, "first", false).await;
    // 3.1.1: server unavailable
    let mut raw = Raw::open(&a).await;
    raw.send(&connect_session("second", false)).await;
    assert_eq!(raw.read_packet().await.unwrap(), (0x20, vec![0, 0x03]));
    assert!(raw.read_until_closed().await.is_none());
    // MQTT 5: quota exceeded
    let mut raw = Raw::open(&a).await;
    raw.send(&connect5_session("second5", false, 60, &[])).await;
    assert_eq!(raw.read_packet().await.unwrap(), (0x20, vec![0, 0x97, 0]));
    // sessions that end with their connection are not counted, and the first can reconnect
    let (_clean, _) = Raw::connected_session(&a, "clean", true).await;
    let (_zero, _) = Raw::connected5_session(&a, "zero", false, 0, &[]).await;
    let (_again, present) = Raw::connected_session(&a, "first", false).await;
    assert!(present);
    broker.shutdown().await;
}

// --- who owns a session --------------------------------------------------------------------------

/// Accepts every user; the session layer is what is tested.
struct AcceptAll;

impl Authenticator for AcceptAll {
    fn verify<'a>(
        &'a self,
        _username: &'a str,
        _password: &'a [u8],
    ) -> Pin<Box<dyn Future<Output = AuthOutcome> + Send + 'a>> {
        Box::pin(async { AuthOutcome::Accepted })
    }
}

fn connect_as(level: u8, id: &str, user: &str, clean: bool) -> Vec<u8> {
    let flags = 0b1100_0000 | if clean { 0b10 } else { 0 };
    let mut tail = string_bytes(user);
    tail.extend([0, 1, b'p']);
    if level == 4 {
        return connect_raw(flags, 30, id, &tail);
    }
    let mut body = string_bytes("MQTT");
    body.extend([5, flags, 0, 30, 5, 0x11, 0, 0, 0x01, 0x00]); // session expiry 256
    body.extend(string_bytes(id));
    body.extend(tail);
    packet(0x10, &body)
}

#[tokio::test]
async fn a_session_belongs_to_the_user_that_created_it() {
    let broker = Broker::start_with(
        0,
        BrokerLimits::default(),
        BrokerOptions {
            protocol: ProtocolMode::Both,
            authenticator: Some(Arc::new(AcceptAll)),
            ..BrokerOptions::default()
        },
    )
    .await
    .unwrap();
    let a = addr(&broker);
    for (level, rejected) in [(4u8, 0x02u8), (5, 0x85)] {
        let id = format!("shared{level}");
        let mut alice = Raw::open(&a).await;
        alice.send(&connect_as(level, &id, "alice", false)).await;
        assert_eq!(alice.read_packet().await.unwrap().0, 0x20);
        drop(alice);
        wait_for_connections(&broker, 0).await;
        // another user is refused with a resume and with a clean start alike
        for clean in [false, true] {
            let mut bob = Raw::open(&a).await;
            bob.send(&connect_as(level, &id, "bob", clean)).await;
            let (first, body) = bob.read_packet().await.unwrap();
            assert_eq!(
                (first, body[1]),
                (0x20, rejected),
                "level {level} clean {clean}"
            );
            assert!(bob.read_until_closed().await.is_none());
        }
        // the refusals destroyed nothing: alice resumes
        let mut alice = Raw::open(&a).await;
        alice.send(&connect_as(level, &id, "alice", false)).await;
        let (_, body) = alice.read_packet().await.unwrap();
        assert_eq!((body[0], body[1]), (1, 0), "level {level}");
    }
    broker.shutdown().await;
}

// --- flow control ---------------------------------------------------------------------------------

#[tokio::test]
async fn no_more_than_the_clients_receive_maximum_is_in_flight() {
    let broker = both().await;
    let a = addr(&broker);
    // MQTT 5 client with Receive Maximum 2
    let (mut sub, _) = Raw::connected5_session(&a, "slow", true, 0, &[0x21, 0, 2]).await;
    assert_eq!(sub.subscribe5(1, &[("t", 1)]).await, [1]);
    let (mut publisher, _) = Raw::connected5(&a, "pub").await;
    for i in 0..5u16 {
        publisher
            .send(&publish5_with_id("t", format!("m{i}").as_bytes(), 1, i + 1))
            .await;
        publisher.read_ack(PUBACK).await;
    }
    let m0 = sub.read_any_publish(true).await;
    let m1 = sub.read_any_publish(true).await;
    assert_eq!((&m0.payload[..], &m1.payload[..]), (&b"m0"[..], &b"m1"[..]));
    assert!(sub.is_silent_for(SHORT).await, "window of 2 is full");
    // every acknowledgement lets the next message in order through
    sub.send(&ack(PUBACK, m0.packet_id)).await;
    let m2 = sub.read_any_publish(true).await;
    assert_eq!(m2.payload, b"m2");
    assert!(sub.is_silent_for(SHORT).await);
    sub.send(&ack(PUBACK, m1.packet_id)).await;
    sub.send(&ack(PUBACK, m2.packet_id)).await;
    assert_eq!(sub.read_any_publish(true).await.payload, b"m3");
    assert_eq!(sub.read_any_publish(true).await.payload, b"m4");
    // the packet identifiers of messages in flight at the same time differ
    assert_ne!(m0.packet_id, m1.packet_id);
    broker.shutdown().await;
}

#[tokio::test]
async fn a_full_session_queue_drops_messages_but_does_not_disconnect_the_client() {
    let limits = BrokerLimits {
        max_pending_send_bytes: 100,
        max_consecutive_send_overflows: 3,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::Both).await;
    let a = addr(&broker);
    let (mut sub, _) = Raw::connected5_session(&a, "slow", false, 600, &[0x21, 0, 1]).await;
    sub.subscribe5(1, &[("t", 1)]).await;
    let (mut publisher, _) = Raw::connected5(&a, "pub").await;
    // 41 bytes each: two fit into 100 bytes, the other three are dropped
    for i in 0..5u16 {
        publisher
            .send(&publish5_with_id("t", &[b'x'; 40], 1, i + 1))
            .await;
        publisher.read_ack(PUBACK).await;
    }
    let first = sub.read_any_publish(true).await;
    assert!(sub.is_silent_for(SHORT).await, "window of 1");
    sub.send(&ack(PUBACK, first.packet_id)).await;
    let second = sub.read_any_publish(true).await;
    sub.send(&ack(PUBACK, second.packet_id)).await;
    // the connection was not kicked and nothing else is queued
    assert!(sub.is_silent_for(SHORT).await);
    sub.send(&pingreq()).await;
    assert_eq!(sub.read_packet().await.unwrap().0, 0xD0);
    broker.shutdown().await;
}

#[tokio::test]
async fn messages_larger_than_the_clients_maximum_packet_size_are_not_sent() {
    let broker = both().await;
    let a = addr(&broker);
    // Maximum Packet Size 40
    let (mut sub, _) = Raw::connected5_session(&a, "small", true, 0, &[0x27, 0, 0, 0, 40]).await;
    sub.subscribe5(1, &[("t", 1)]).await;
    let (mut publisher, _) = Raw::connected5(&a, "pub").await;
    publisher
        .send(&publish5_with_id("t", &[b'x'; 100], 1, 1))
        .await;
    publisher.read_ack(PUBACK).await;
    publisher.send(&publish5_with_id("t", b"fits", 1, 2)).await;
    publisher.read_ack(PUBACK).await;
    let got = sub.read_any_publish(true).await;
    assert_eq!(got.payload, b"fits");
    assert!(sub.is_silent_for(SHORT).await);
    broker.shutdown().await;
}

// --- publishes from a client: duplicates, limits ----------------------------------------------------

#[tokio::test]
async fn a_repeated_qos2_publish_with_the_same_id_is_delivered_once() {
    let broker = both().await;
    let a = addr(&broker);
    let mut sub = raw_subscriber(&a, "sub", true, "t", 2).await;
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    publisher
        .send(&publish_with_id("t", b"once", 2, 5, false))
        .await;
    assert_eq!(publisher.read_ack(PUBREC).await, (5, None));
    // the client did not see the PUBREC and repeats the PUBLISH
    publisher
        .send(&publish_with_id("t", b"once", 2, 5, true))
        .await;
    assert_eq!(publisher.read_ack(PUBREC).await, (5, None));
    let got = sub.read_any_publish(false).await;
    assert_eq!(got.payload, b"once");
    sub.send(&ack(PUBREC, got.packet_id)).await;
    sub.read_ack(PUBREL).await;
    sub.send(&ack(PUBCOMP, got.packet_id)).await;
    assert!(sub.is_silent_for(SHORT).await, "delivered once");
    // PUBREL ends it; the same id is then a new message
    publisher.send(&ack(PUBREL, 5)).await;
    assert_eq!(publisher.read_ack(PUBCOMP).await, (5, None));
    publisher
        .send(&publish_with_id("t", b"again", 2, 5, false))
        .await;
    publisher.read_ack(PUBREC).await;
    assert_eq!(sub.read_any_publish(false).await.payload, b"again");
    broker.shutdown().await;
}

#[tokio::test]
async fn pubrel_for_an_unknown_id_is_answered_per_version() {
    let broker = both().await;
    let a = addr(&broker);
    let (mut v311, _) = Raw::connected_session(&a, "v311", true).await;
    v311.send(&ack(PUBREL, 9)).await;
    assert_eq!(v311.read_ack(PUBCOMP).await, (9, None));
    let (mut v5, _) = Raw::connected5(&a, "v5").await;
    v5.send(&ack(PUBREL, 9)).await;
    assert_eq!(v5.read_ack(PUBCOMP).await, (9, Some(0x92)));
    broker.shutdown().await;
}

#[tokio::test]
async fn a_pubrec_for_an_unknown_id_is_answered_with_pubrel() {
    let broker = both().await;
    let a = addr(&broker);
    let (mut v311, _) = Raw::connected_session(&a, "v311", true).await;
    v311.send(&ack(PUBREC, 77)).await;
    assert_eq!(v311.read_ack(PUBREL).await, (77, None));
    let (mut v5, _) = Raw::connected5(&a, "v5").await;
    v5.send(&ack(PUBREC, 77)).await;
    assert_eq!(v5.read_ack(PUBREL).await, (77, Some(0x92)));
    // unknown PUBACK and PUBCOMP are ignored: the connection goes on
    v5.send(&ack(PUBACK, 78)).await;
    v5.send(&ack(PUBCOMP, 79)).await;
    v5.send(&pingreq()).await;
    assert_eq!(v5.read_packet().await.unwrap().0, 0xD0);
    broker.shutdown().await;
}

#[tokio::test]
async fn an_mqtt5_pubrec_with_an_error_ends_the_flow_without_pubrel() {
    let broker = both().await;
    let a = addr(&broker);
    let (mut sub, _) = Raw::connected5_session(&a, "sub", true, 0, &[0x21, 0, 1]).await;
    sub.subscribe5(1, &[("t", 2)]).await;
    let (mut publisher, _) = Raw::connected5(&a, "pub").await;
    for i in 0..2u16 {
        publisher
            .send(&publish5_with_id("t", format!("m{i}").as_bytes(), 2, i + 1))
            .await;
        publisher.read_ack(PUBREC).await;
    }
    let first = sub.read_any_publish(true).await;
    // 0x80: unspecified error. No PUBREL comes; the window is free for the next message
    sub.send(&ack5(PUBREC, first.packet_id, 0x80)).await;
    let second = sub.read_any_publish(true).await;
    assert_eq!(second.payload, b"m1");
    broker.shutdown().await;
}

#[tokio::test]
async fn a_topic_beyond_the_limits_is_not_delivered_but_acknowledged_per_version() {
    let limits = BrokerLimits {
        max_topic_length: 8,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::Both).await;
    let a = addr(&broker);
    let mut sub = raw_subscriber(&a, "sub", true, "#", 1).await;
    let long = "a/very/long/topic";

    let (mut v5, _) = Raw::connected5(&a, "v5").await;
    v5.send(&publish5_with_id(long, b"x", 1, 1)).await;
    assert_eq!(v5.read_ack(PUBACK).await, (1, Some(0x97)));
    v5.send(&publish5_with_id(long, b"x", 2, 2)).await;
    assert_eq!(v5.read_ack(PUBREC).await, (2, Some(0x97)));
    // no state was kept: PUBREL is unknown
    v5.send(&ack(PUBREL, 2)).await;
    assert_eq!(v5.read_ack(PUBCOMP).await, (2, Some(0x92)));

    let (mut v311, _) = Raw::connected_session(&a, "v311", true).await;
    v311.send(&publish_with_id(long, b"x", 1, 1, false)).await;
    assert_eq!(v311.read_ack(PUBACK).await, (1, None));
    v311.send(&publish_with_id(long, b"x", 2, 2, false)).await;
    assert_eq!(v311.read_ack(PUBREC).await, (2, None));
    // the identifier is remembered until PUBREL: a repeat is just acknowledged
    v311.send(&publish_with_id(long, b"x", 2, 2, true)).await;
    assert_eq!(v311.read_ack(PUBREC).await, (2, None));
    v311.send(&ack(PUBREL, 2)).await;
    assert_eq!(v311.read_ack(PUBCOMP).await, (2, None));
    // QoS 0 is dropped
    v311.send(&publish(long, b"x")).await;
    v311.send(&pingreq()).await;
    assert_eq!(v311.read_packet().await.unwrap().0, 0xD0);
    assert!(sub.is_silent_for(SHORT).await);
    broker.shutdown().await;
}

#[tokio::test]
async fn too_many_qos2_ids_awaiting_pubrel_disconnect_mqtt5_clients_only() {
    let limits = BrokerLimits {
        receive_maximum: 2,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::Both).await;
    let a = addr(&broker);
    let (mut v5, _) = Raw::connected5(&a, "v5").await;
    for id in 1..=2u16 {
        v5.send(&publish5_with_id("t", b"x", 2, id)).await;
        v5.read_ack(PUBREC).await;
    }
    v5.send(&publish5_with_id("t", b"x", 2, 3)).await;
    // 0x93: receive maximum exceeded
    assert_eq!(v5.read_until_closed().await, Some((0xE0, vec![0x93, 0])));

    // a repeat of a waiting id is no new one, and a PUBREL makes room
    let (mut v5, _) = Raw::connected5(&a, "v5b").await;
    for id in 1..=2u16 {
        v5.send(&publish5_with_id("t", b"x", 2, id)).await;
        v5.read_ack(PUBREC).await;
    }
    v5.send(&publish5_with_id("t", b"x", 2, 2)).await;
    v5.read_ack(PUBREC).await;
    v5.send(&ack(PUBREL, 1)).await;
    v5.read_ack(PUBCOMP).await;
    v5.send(&publish5_with_id("t", b"x", 2, 3)).await;
    v5.read_ack(PUBREC).await;

    // 3.1.1 has no such limit
    let (mut v311, _) = Raw::connected_session(&a, "v311", true).await;
    for id in 1..=5u16 {
        v311.send(&publish_with_id("t", b"x", 2, id, false)).await;
        v311.read_ack(PUBREC).await;
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn pipelined_publishes_are_acknowledged_in_order() {
    let broker = both().await;
    let a = addr(&broker);
    let (mut raw, _) = Raw::connected_session(&a, "pipe", true).await;
    let mut bytes = Vec::new();
    for id in 1..=50u16 {
        let qos = 1 + (id % 2) as u8;
        bytes.extend(publish_with_id("t", b"x", qos, id, false));
    }
    raw.send(&bytes).await;
    for id in 1..=50u16 {
        let first = if id % 2 == 1 { PUBREC } else { PUBACK };
        assert_eq!(raw.read_ack(first).await, (id, None));
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn qos0_still_flows_to_the_subscriber_of_a_resumed_session() {
    let broker = both().await;
    let a = addr(&broker);
    let sub = raw_subscriber(&a, "keeper", false, "t", 1).await;
    drop(sub);
    wait_for_connections(&broker, 0).await;
    let (mut sub, present) = Raw::connected_session(&a, "keeper", false).await;
    assert!(present);
    // a client publishes with QoS 0: delivered with QoS 0 to the connected session
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    publisher.send(&publish("t", b"from a client")).await;
    let got = sub.read_any_publish(false).await;
    assert_eq!((got.qos, got.payload), (0, b"from a client".to_vec()));
    broker.shutdown().await;
}

#[tokio::test]
async fn the_old_connection_cannot_change_the_subscriptions_after_a_take_over() {
    let broker = both().await;
    let a = addr(&broker);
    let (mut old, _) = Raw::connected_session(&a, "same", false).await;
    let (mut new, present) = Raw::connected_session(&a, "same", false).await;
    assert!(present);
    // whatever the old connection still manages to send is not applied to the session
    // (one write: the broker may have closed the socket already)
    let mut bytes = subscribe_qos(1, &[("stolen", 1)]);
    bytes.extend(unsubscribe(2, &["mine"]));
    let _ = old.stream.write_all(&bytes).await;
    assert!(old.read_until_closed().await.is_none());
    assert_eq!(broker.subscription_count(), 0);
    new.send(&subscribe_qos(1, &[("mine", 1)])).await;
    new.read_packet().await.unwrap();
    assert_eq!(broker.subscription_count(), 1);
    // and the old connection cannot remove them
    assert!(new.is_silent_for(SHORT).await);
    broker.shutdown().await;
}

// --- identifiers that are stored with the session ---------------------------------------------

struct CountingAuth(Arc<std::sync::atomic::AtomicUsize>);

impl Authenticator for CountingAuth {
    fn verify<'a>(
        &'a self,
        _username: &'a str,
        _password: &'a [u8],
    ) -> Pin<Box<dyn Future<Output = AuthOutcome> + Send + 'a>> {
        self.0.fetch_add(1, std::sync::atomic::Ordering::SeqCst);
        Box::pin(async { AuthOutcome::Accepted })
    }
}

#[tokio::test]
async fn an_overlong_client_id_or_user_name_is_refused_before_anything_else_happens() {
    let calls = Arc::new(std::sync::atomic::AtomicUsize::new(0));
    let broker = Broker::start_with(
        0,
        BrokerLimits::default(),
        BrokerOptions {
            protocol: ProtocolMode::Both,
            authenticator: Some(Arc::new(CountingAuth(Arc::clone(&calls)))),
            ..BrokerOptions::default()
        },
    )
    .await
    .unwrap();
    let a = addr(&broker);
    let long = "x".repeat(1025);
    let longest_ok = "y".repeat(1024);

    // 3.1.1: client identifier -> 0x02, user name -> 0x04; and MQTT 5: 0x85 / 0x86
    let mut raw = Raw::open(&a).await;
    raw.send(&connect_as(4, &long, "user", true)).await;
    assert_eq!(raw.read_packet().await.unwrap(), (0x20, vec![0, 0x02]));
    let mut raw = Raw::open(&a).await;
    raw.send(&connect_as(4, "id", &long, true)).await;
    assert_eq!(raw.read_packet().await.unwrap(), (0x20, vec![0, 0x04]));
    let mut raw = Raw::open(&a).await;
    raw.send(&connect_as(5, &long, "user", true)).await;
    let (first, body) = raw.read_packet().await.unwrap();
    assert_eq!((first, body[1]), (0x20, 0x85));
    let mut raw = Raw::open(&a).await;
    raw.send(&connect_as(5, "id", &long, true)).await;
    let (first, body) = raw.read_packet().await.unwrap();
    assert_eq!((first, body[1]), (0x20, 0x86));
    assert_eq!(
        calls.load(std::sync::atomic::Ordering::SeqCst),
        0,
        "the backend was not asked"
    );

    // the limit itself is fine
    let mut raw = Raw::open(&a).await;
    raw.send(&connect_as(4, &longest_ok, &longest_ok, true))
        .await;
    assert_eq!(raw.read_packet().await.unwrap(), (0x20, vec![0, 0]));
    assert_eq!(calls.load(std::sync::atomic::Ordering::SeqCst), 1);
    broker.shutdown().await;
}

// --- server publishes --------------------------------------------------------------------------

#[tokio::test]
async fn a_server_publish_is_delivered_at_qos1_at_most() {
    let broker = both().await;
    let a = addr(&broker);
    let mut s0 = raw_subscriber(&a, "s0", true, "srv", 0).await;
    let mut s1 = raw_subscriber(&a, "s1", true, "srv", 1).await;
    let mut s2 = raw_subscriber(&a, "s2", true, "srv", 2).await;
    broker.hub().publish("srv", &b"hello"[..]).unwrap();

    let got = s0.read_any_publish(false).await;
    assert_eq!((got.payload.as_slice(), got.qos), (&b"hello"[..], 0));
    for raw in [&mut s1, &mut s2] {
        let got = raw.read_any_publish(false).await;
        assert_eq!((got.payload.as_slice(), got.qos), (&b"hello"[..], 1));
        assert_ne!(got.packet_id, 0);
        raw.send(&ack(PUBACK, got.packet_id)).await;
        // acknowledged: nothing is sent again
        assert!(raw.is_silent_for(SHORT).await);
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn a_server_publish_is_not_queued_for_an_offline_session_but_a_client_publish_is() {
    let broker = both().await;
    let a = addr(&broker);
    let sub = raw_subscriber(&a, "sleeper", false, "srv", 1).await;
    drop(sub);
    wait_for_connections(&broker, 0).await;
    broker
        .hub()
        .publish("srv", &b"from the server"[..])
        .unwrap();
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    publisher
        .send(&publish_with_id("srv", b"from a client", 1, 1, false))
        .await;
    publisher.read_ack(PUBACK).await;

    let (mut sub, present) = Raw::connected_session(&a, "sleeper", false).await;
    assert!(present);
    let got = sub.read_any_publish(false).await;
    assert_eq!(
        (got.payload.as_slice(), got.qos),
        (&b"from a client"[..], 1),
        "only the client's message was kept"
    );
    sub.send(&ack(PUBACK, got.packet_id)).await;
    assert!(sub.is_silent_for(SHORT).await, "nothing from the server");
    // connected again: server messages reach it
    broker.hub().publish("srv", &b"now"[..]).unwrap();
    assert_eq!(sub.read_any_publish(false).await.payload, b"now");
    broker.shutdown().await;
}

// --- Receive Maximum after a resume, No Local with QoS 1, shutdown with open handshakes -----------

#[tokio::test]
async fn the_receive_maximum_of_a_resuming_client_limits_the_resent_backlog() {
    let broker = both().await;
    let a = addr(&broker);
    let window = [0x21, 0, 2]; // Receive Maximum 2
    let (mut sub, _) = Raw::connected5_session(&a, "resumer", false, 600, &window).await;
    assert_eq!(sub.subscribe5(1, &[("t", 1)]).await, [1]);
    let (mut publisher, _) = Raw::connected5(&a, "pub").await;
    for i in 0..5u16 {
        publisher
            .send(&publish5_with_id("t", format!("m{i}").as_bytes(), 1, i + 1))
            .await;
        publisher.read_ack(PUBACK).await;
    }
    let m0 = sub.read_any_publish(true).await;
    let m1 = sub.read_any_publish(true).await;
    assert_eq!((&m0.payload[..], &m1.payload[..]), (&b"m0"[..], &b"m1"[..]));
    assert!(sub.is_silent_for(SHORT).await);
    // the connection breaks with two messages in flight and three behind them
    drop(sub);

    let (mut sub, body) = Raw::connected5_session(&a, "resumer", false, 600, &window).await;
    assert_eq!(body[0], 1, "session present");
    // the two that were in flight come first, as repeats, and the window of two is full again
    let again0 = sub.read_any_publish(true).await;
    let again1 = sub.read_any_publish(true).await;
    assert_eq!(
        (&again0.payload[..], again0.dup, again0.packet_id),
        (&b"m0"[..], true, m0.packet_id)
    );
    assert_eq!(
        (&again1.payload[..], again1.dup, again1.packet_id),
        (&b"m1"[..], true, m1.packet_id)
    );
    assert!(
        sub.is_silent_for(SHORT).await,
        "never more than 2 unacknowledged"
    );
    // each acknowledgement lets exactly the next message of the backlog through
    sub.send(&ack(PUBACK, again0.packet_id)).await;
    let m2 = sub.read_any_publish(true).await;
    assert_eq!((&m2.payload[..], m2.dup), (&b"m2"[..], false));
    assert!(sub.is_silent_for(SHORT).await);
    sub.send(&ack(PUBACK, again1.packet_id)).await;
    sub.send(&ack(PUBACK, m2.packet_id)).await;
    assert_eq!(sub.read_any_publish(true).await.payload, b"m3");
    assert_eq!(sub.read_any_publish(true).await.payload, b"m4");
    broker.shutdown().await;
}

#[tokio::test]
async fn no_local_applies_to_qos1_and_a_second_matching_subscription_still_delivers() {
    let broker = both().await;
    let a = addr(&broker);
    let (mut me, _) = Raw::connected5(&a, "me").await;
    // QoS 1 (bits 0-1) with No Local (bit 2)
    assert_eq!(me.subscribe5(1, &[("t", 0b101)]).await, [1]);
    let (mut other, _) = Raw::connected5(&a, "other").await;
    assert_eq!(other.subscribe5(1, &[("t", 1)]).await, [1]);

    me.send(&publish5_with_id("t", b"mine", 1, 1)).await;
    assert_eq!(me.read_ack(PUBACK).await, (1, None));
    let got = other.read_any_publish(true).await;
    assert_eq!((got.payload.as_slice(), got.qos), (&b"mine"[..], 1));
    other.send(&ack(PUBACK, got.packet_id)).await;
    assert!(
        me.is_silent_for(SHORT).await,
        "No Local: a session does not get its own message"
    );

    // another matching subscription of the same session without No Local counts: one delivery, at the QoS
    // of that subscription (2), although the No Local subscription of QoS 1 matches as well
    assert_eq!(me.subscribe5(2, &[("#", 2)]).await, [2]);
    me.send(&publish5_with_id("t", b"twice", 2, 2)).await;
    // the broker delivers while it dispatches and acknowledges afterwards: the PUBLISH comes first
    let own = me.read_any_publish(true).await;
    assert_eq!((own.payload.as_slice(), own.qos), (&b"twice"[..], 2));
    assert_eq!(me.read_ack(PUBREC).await, (2, None));
    assert!(me.is_silent_for(SHORT).await, "delivered once");
    let got = other.read_any_publish(true).await;
    assert_eq!((got.payload.as_slice(), got.qos), (&b"twice"[..], 1));
    broker.shutdown().await;
}

/// A subscriber with every kind of open handshake: a QoS 1 and a QoS 2 PUBLISH nobody acknowledged, and a
/// QoS 2 message that is past its PUBREC. Returns the connection.
async fn subscriber_with_open_handshakes(a: &str, publisher: &mut Raw) -> Raw {
    let (mut sub, _) = Raw::connected5_session(a, "busy", false, 600, &[]).await;
    assert_eq!(sub.subscribe5(1, &[("t", 2)]).await, [2]);
    for (i, qos, first) in [(1u16, 1u8, PUBACK), (2, 2, PUBREC), (3, 2, PUBREC)] {
        publisher
            .send(&publish5_with_id("t", format!("m{i}").as_bytes(), qos, i))
            .await;
        publisher.read_ack(first).await;
    }
    let _unacknowledged1 = sub.read_any_publish(true).await;
    let _unacknowledged2 = sub.read_any_publish(true).await;
    let third = sub.read_any_publish(true).await;
    sub.send(&ack(PUBREC, third.packet_id)).await;
    assert_eq!(sub.read_ack(PUBREL).await.0, third.packet_id);
    sub
}

#[tokio::test]
async fn shutdown_with_unacknowledged_messages_stops_in_time_and_does_not_panic() {
    // Broker::shutdown
    let broker = both().await;
    let a = addr(&broker);
    let (mut publisher, _) = Raw::connected5(&a, "pub").await;
    let mut sub = subscriber_with_open_handshakes(&a, &mut publisher).await;
    // and an offline session with a queue
    let off = raw_subscriber(&a, "off", false, "t", 1).await;
    drop(off);
    wait_for_connections(&broker, 2).await;
    publisher.send(&publish5_with_id("t", b"q", 1, 4)).await;
    publisher.read_ack(PUBACK).await;
    let started = std::time::Instant::now();
    broker.shutdown().await;
    assert!(
        started.elapsed() < Duration::from_secs(3),
        "took {:?}",
        started.elapsed()
    );
    // the last thing the client sees is DISCONNECT 0x8B, server shutting down
    assert_eq!(sub.read_until_closed().await, Some((0xE0, vec![0x8B, 0])));

    // dropping the broker starts the same shutdown without waiting
    let broker = both().await;
    let a = addr(&broker);
    let (mut publisher, _) = Raw::connected5(&a, "pub").await;
    let mut sub = subscriber_with_open_handshakes(&a, &mut publisher).await;
    drop(broker);
    assert_eq!(sub.read_until_closed().await, Some((0xE0, vec![0x8B, 0])));
}

#[tokio::test]
async fn qos0_messages_larger_than_the_clients_maximum_packet_size_are_not_sent_either() {
    let broker = both().await;
    let a = addr(&broker);
    // Maximum Packet Size 40, subscription at QoS 0
    let (mut sub, _) = Raw::connected5_session(&a, "small", true, 0, &[0x27, 0, 0, 0, 40]).await;
    assert_eq!(sub.subscribe5(1, &[("t", 0)]).await, [0]);
    let (mut publisher, _) = Raw::connected5(&a, "pub").await;
    publisher.send(&publish5("t", &[], &[b'x'; 100])).await;
    publisher.send(&publish5("t", &[], b"fits")).await;
    let got = sub.read_any_publish(true).await;
    assert_eq!((got.payload.as_slice(), got.qos), (&b"fits"[..], 0));
    assert!(sub.is_silent_for(SHORT).await);
    broker.shutdown().await;
}

#[tokio::test]
async fn a_receive_maximum_of_zero_in_the_limits_is_announced_as_one() {
    let limits = BrokerLimits {
        receive_maximum: 0,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::Both).await;
    let (_raw, body) = Raw::connected5(&addr(&broker), "c").await;
    // the Receive Maximum is the first property of the CONNACK
    assert_eq!(&body[3..6], &[0x21, 0, 1]);
    broker.shutdown().await;
}
