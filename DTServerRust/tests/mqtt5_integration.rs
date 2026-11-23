//! MQTT 5 and protocol mode tests against the real broker on an ephemeral port.
//!
//! Raw socket tests use hand written packets (independent of the broker's own codec); rumqttc is used for
//! the real client behaviour (`rumqttc::v5` for MQTT 5, the top level API for MQTT 3.1.1).

mod common;

use std::time::Duration;

use common::*;
use dt_server::broker::{BrokerLimits, ProtocolMode};
use rumqttc::v5::mqttbytes::v5::SubscribeReasonCode as SubReason5;
use rumqttc::v5::mqttbytes::QoS as QoS5;
use rumqttc::{QoS, SubscribeReasonCode};

const SHORT: Duration = Duration::from_millis(400);

/// CONNACK properties the broker announces to MQTT 5 clients with a maximum packet size of 1024.
const CONNACK_PROPS_1024: [u8; 14] = [0x21, 0, 100, 0x25, 0, 0x27, 0, 0, 4, 0, 0x29, 0, 0x2A, 0];

// --- protocol modes --------------------------------------------------------------------------

#[tokio::test]
async fn default_mode_refuses_mqtt5_with_a_v5_connack() {
    let broker = start(BrokerLimits::default()).await;
    let mut raw = Raw::open(&addr(&broker)).await;
    raw.send(&connect5("v5")).await;
    // CONNACK in the MQTT 5 format: session present 0, reason 0x84 (unsupported protocol version), no properties
    let (first, body) = raw.read_packet().await.expect("CONNACK");
    assert_eq!((first, &body[..]), (0x20, &[0, 0x84, 0][..]));
    assert!(raw.read_until_closed().await.is_none());
    // a 3.1.1 client still works
    let _ok = Raw::connected(&addr(&broker), "v4").await;
    broker.shutdown().await;
}

#[tokio::test]
async fn mqtt5_only_mode_refuses_3_1_1_with_return_code_1() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::V5).await;
    let mut raw = Raw::open(&addr(&broker)).await;
    raw.send(&connect("v4")).await;
    let (first, body) = raw.read_packet().await.expect("CONNACK");
    assert_eq!((first, &body[..]), (0x20, &[0, 0x01][..]));
    assert!(raw.read_until_closed().await.is_none());

    // level 3 with the name MQTT is refused the same way, 3.1 (MQIsdp) is closed without reply
    let mut raw = Raw::open(&addr(&broker)).await;
    raw.send(&[0x10, 12, 0, 4, b'M', b'Q', b'T', b'T', 3, 2, 0, 10, 0, 0])
        .await;
    assert_eq!(raw.read_packet().await.unwrap().1, [0, 0x01]);
    assert!(raw.read_until_closed().await.is_none());

    // MQTT 5 is served
    let (_raw, body) = Raw::connected5(&addr(&broker), "v5").await;
    assert_eq!(&body[..2], &[0, 0]);
    broker.shutdown().await;
}

#[tokio::test]
async fn mqtt5_only_mode_with_a_real_client() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::V5).await;
    let mut sub = ClientV5::connect(broker.port(), "sub").await;
    let publisher = ClientV5::connect(broker.port(), "pub").await;
    assert!(!sub.connack.session_present);

    sub.client
        .subscribe("twin/temperature", QoS5::AtMostOnce)
        .await
        .unwrap();
    assert_eq!(
        sub.wait_suback().await,
        vec![SubReason5::Success(QoS5::AtMostOnce)]
    );
    publisher
        .client
        .publish("twin/temperature", QoS5::AtMostOnce, false, "21.5")
        .await
        .unwrap();
    assert_eq!(
        sub.next_publish().await,
        ("twin/temperature".to_owned(), b"21.5".to_vec())
    );
    broker.shutdown().await;
}

#[tokio::test]
async fn both_mode_connects_3_1_1_and_5_clients_that_exchange_messages() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::Both).await;
    let mut v4 = Client::connect(broker.port(), "v4").await;
    let mut v5 = ClientV5::connect(broker.port(), "v5").await;

    v4.client
        .subscribe("from/v5", QoS::AtMostOnce)
        .await
        .unwrap();
    assert_eq!(
        v4.wait_suback().await,
        vec![SubscribeReasonCode::Success(QoS::AtMostOnce)]
    );
    v5.client
        .subscribe("from/v4", QoS5::AtMostOnce)
        .await
        .unwrap();
    assert_eq!(
        v5.wait_suback().await,
        vec![SubReason5::Success(QoS5::AtMostOnce)]
    );

    v5.client
        .publish("from/v5", QoS5::AtMostOnce, false, "to 3.1.1")
        .await
        .unwrap();
    v4.client
        .publish("from/v4", QoS::AtMostOnce, false, "to 5")
        .await
        .unwrap();
    assert_eq!(
        v4.next_publish().await,
        ("from/v5".to_owned(), b"to 3.1.1".to_vec())
    );
    assert_eq!(
        v5.next_publish().await,
        ("from/v4".to_owned(), b"to 5".to_vec())
    );
    assert_eq!(broker.connection_count(), 2);
    broker.shutdown().await;
}

#[tokio::test]
async fn both_mode_raw_clients_see_the_encoding_of_their_own_version() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::Both).await;
    let a = addr(&broker);
    let mut v4 = Raw::connected(&a, "v4").await;
    let (mut v5, _) = Raw::connected5(&a, "v5").await;
    v4.subscribe(1, &["t"]).await;
    assert_eq!(v5.subscribe5(1, &[("t", 0)]).await, [0x00]);

    // a 3.1.1 publish reaches both, with the PUBLISH format of each receiver
    v4.send(&publish("t", b"one")).await;
    assert_eq!(v4.read_publish().await, ("t".to_owned(), b"one".to_vec()));
    assert_eq!(v5.read_publish5().await, ("t".to_owned(), b"one".to_vec()));
    // an MQTT 5 publish with properties arrives without them
    v5.send(&publish5("t", &[0x02, 0, 0, 0, 9], b"two")).await;
    assert_eq!(v4.read_publish().await, ("t".to_owned(), b"two".to_vec()));
    assert_eq!(v5.read_publish5().await, ("t".to_owned(), b"two".to_vec()));
    broker.shutdown().await;
}

#[tokio::test]
async fn hub_publishes_reach_mqtt5_subscribers() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::Both).await;
    let hub = broker.hub();
    let (mut v5, _) = Raw::connected5(&addr(&broker), "v5").await;
    v5.subscribe5(1, &[("srv/#", 0)]).await;
    hub.publish("srv/x", "from the hub").unwrap();
    assert_eq!(
        v5.read_publish5().await,
        ("srv/x".to_owned(), b"from the hub".to_vec())
    );
    // and client publishes reach the hub
    let mut sub = hub.subscribe("c/#", 4).unwrap();
    v5.send(&publish5("c/y", &[], b"from v5")).await;
    let m = tokio::time::timeout(WAIT, sub.recv())
        .await
        .unwrap()
        .unwrap();
    assert_eq!((&*m.topic, &m.payload[..]), ("c/y", &b"from v5"[..]));
    broker.shutdown().await;
}

// --- CONNECT / CONNACK -----------------------------------------------------------------------

#[tokio::test]
async fn connack_announces_what_the_broker_supports() {
    let limits = BrokerLimits {
        max_packet_size: 1024,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::V5).await;
    let (_raw, body) = Raw::connected5(&addr(&broker), "props").await;
    // session present 0, reason 0, then the property block: receive maximum 100, retain not available, maximum packet
    // size, subscription identifiers and shared subscriptions not available
    let mut expected = vec![0, 0, CONNACK_PROPS_1024.len() as u8];
    expected.extend(CONNACK_PROPS_1024);
    assert_eq!(body, expected);

    // the same through a real client
    let client = ClientV5::connect(broker.port(), "rumqttc").await;
    let props = client.connack.properties.as_ref().expect("properties");
    // Maximum QoS is not sent any more: absent means QoS 2
    assert_eq!(props.max_qos, None);
    assert_eq!(props.receive_max, Some(100));
    assert_eq!(props.retain_available, Some(0));
    assert_eq!(props.max_packet_size, Some(1024));
    assert_eq!(props.shared_subscription_available, Some(0));
    assert_eq!(props.assigned_client_identifier, None);
    broker.shutdown().await;
}

#[tokio::test]
async fn connect_properties_are_skipped() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::V5).await;
    // session expiry (0x11), receive maximum (0x21), maximum packet size (0x27), user property (0x26)
    let props = [
        0x11, 0, 0, 0, 60, 0x21, 0, 10, 0x27, 0, 0, 1, 0, 0x26, 0, 1, b'k', 0, 1, b'v',
    ];
    let mut raw = Raw::open(&addr(&broker)).await;
    raw.send(&connect5_raw(0b10, 30, &props, "withprops")).await;
    let (first, body) = raw.read_packet().await.unwrap();
    assert_eq!((first, &body[..2]), (0x20, &[0u8, 0][..]));
    raw.send(&pingreq()).await;
    assert_eq!(raw.read_packet().await.unwrap().0, 0xD0);
    broker.shutdown().await;
}

#[tokio::test]
async fn empty_client_id_is_assigned_in_mqtt5_and_checked_in_3_1_1() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::Both).await;
    let a = addr(&broker);
    // MQTT 5: allowed with clean start 1 and 0, the assigned id (0x12) is announced
    for flags in [0b10u8, 0] {
        let mut raw = Raw::open(&a).await;
        raw.send(&connect5_raw(flags, 30, &[], "")).await;
        let (first, body) = raw.read_packet().await.unwrap();
        assert_eq!((first, &body[..2]), (0x20, &[0u8, 0][..]));
        // property block: the 14 bytes of the standard announcements, then the assigned id as the last
        // property: 0x12, length (2 bytes), "dt-..."
        let props = &body[3..];
        let at = CONNACK_PROPS_1024.len();
        assert_eq!(props[at], 0x12);
        let len = usize::from(u16::from_be_bytes([props[at + 1], props[at + 2]]));
        let id = std::str::from_utf8(&props[at + 3..at + 3 + len]).unwrap();
        assert!(id.starts_with("dt-"), "{id}");
        assert_eq!(at + 3 + len, props.len());
    }
    // 3.1.1 without clean session is still refused with 0x02
    let mut raw = Raw::open(&a).await;
    raw.send(&connect_raw(0, 30, "", &[])).await;
    assert_eq!(raw.read_packet().await.unwrap().1, [0, 0x02]);
    assert!(raw.read_until_closed().await.is_none());
    broker.shutdown().await;
}

#[tokio::test]
async fn empty_client_id_with_a_real_mqtt5_client() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::V5).await;
    let client = ClientV5::connect(broker.port(), "").await;
    let assigned = client
        .connack
        .properties
        .as_ref()
        .and_then(|p| p.assigned_client_identifier.clone());
    assert!(assigned.is_some_and(|id| id.starts_with("dt-")));
    broker.shutdown().await;
}

// --- SUBSCRIBE / UNSUBSCRIBE -----------------------------------------------------------------

#[tokio::test]
async fn suback_and_unsuback_carry_mqtt5_reason_codes() {
    let limits = BrokerLimits {
        max_subscriptions_per_session: 2,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::V5).await;
    let (mut raw, _) = Raw::connected5(&addr(&broker), "codes").await;
    // QoS 2 is granted as requested; shared subscriptions are not supported; "a/#/b" is invalid; the third filter
    // exceeds the quota of 2
    let codes = raw
        .subscribe5(
            1,
            &[
                ("a", 2),
                ("$share/g/x", 0),
                ("a/#/b", 0),
                ("b", 0),
                ("c", 0),
            ],
        )
        .await;
    assert_eq!(codes, [0x02, 0x9E, 0x8F, 0x00, 0x97]);

    // UNSUBACK: one reason code per filter, 0x11 for a filter the session did not have
    raw.send(&unsubscribe5(9, &["a", "nope"])).await;
    let (first, body) = raw.read_packet().await.unwrap();
    assert_eq!((first, &body[..]), (0xB0, &[0, 9, 0, 0x00, 0x11][..]));
    broker.shutdown().await;
}

#[tokio::test]
async fn real_client_gets_mqtt5_reason_codes() {
    let limits = BrokerLimits {
        max_subscriptions_per_session: 1,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::Both).await;
    let mut c = ClientV5::connect(broker.port(), "rc").await;
    c.client.subscribe("a", QoS5::AtLeastOnce).await.unwrap();
    assert_eq!(
        c.wait_suback().await,
        vec![SubReason5::Success(QoS5::AtLeastOnce)]
    );
    c.client.subscribe("b", QoS5::AtMostOnce).await.unwrap();
    assert_eq!(c.wait_suback().await, vec![SubReason5::QuotaExceeded]);
    c.client
        .subscribe("$share/g/c", QoS5::AtMostOnce)
        .await
        .unwrap();
    assert_eq!(
        c.wait_suback().await,
        vec![SubReason5::SharedSubscriptionsNotSupported]
    );
    broker.shutdown().await;
}

#[tokio::test]
async fn invalid_subscription_options_are_a_malformed_packet() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::V5).await;
    // QoS 3, retain handling 3, reserved bits 6 and 7
    for options in [0x03u8, 0x30, 0x40, 0x80] {
        let (mut raw, _) = Raw::connected5(&addr(&broker), "bad").await;
        raw.send(&subscribe5(1, &[("t", options)])).await;
        assert_eq!(
            raw.read_until_closed().await,
            Some((0xE0, vec![0x81, 0])),
            "options {options:#x}"
        );
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn no_local_keeps_own_messages_away() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::V5).await;
    let a = addr(&broker);
    let (mut me, _) = Raw::connected5(&a, "me").await;
    let (mut other, _) = Raw::connected5(&a, "other").await;
    // No Local on "mine", a normal subscription on "shared"
    me.subscribe5(1, &[("mine", 0b100), ("shared", 0)]).await;
    other.subscribe5(1, &[("mine", 0), ("shared", 0)]).await;

    me.send(&publish5("mine", &[], b"own")).await;
    // the other client gets it, the publisher does not
    assert_eq!(other.read_publish5().await.1, b"own");
    me.send(&publish5("shared", &[], b"normal")).await;
    // the first PUBLISH the publisher sees is the one on the subscription without No Local
    assert_eq!(
        me.read_publish5().await,
        ("shared".to_owned(), b"normal".to_vec())
    );
    assert!(me.is_silent_for(SHORT).await);

    // a message from another client is delivered to the No Local subscriber
    other.send(&publish5("mine", &[], b"foreign")).await;
    assert_eq!(
        me.read_publish5().await,
        ("mine".to_owned(), b"foreign".to_vec())
    );
    broker.shutdown().await;
}

// --- server DISCONNECT with reason codes -----------------------------------------------------

#[tokio::test]
async fn protocol_violations_end_with_a_disconnect_reason_code() {
    let limits = BrokerLimits {
        max_packet_size: 64,
        ..BrokerLimits::default()
    };
    let broker = start_mode(limits, ProtocolMode::Both).await;
    let a = addr(&broker);
    let cases: Vec<(&str, Vec<u8>, u8)> = vec![
        ("QoS 3 publish", packet(0x36, &[0, 1, b't', 0]), 0x81),
        ("second CONNECT", connect5("again"), 0x82),
        ("AUTH packet", vec![0xF0, 0], 0x82),
        ("wildcard topic", publish5("a/#", &[], b"x"), 0x90),
        ("oversize packet", publish5("t", &[], &[b'x'; 100]), 0x95),
        (
            "huge packet header",
            vec![0x30, 0xFF, 0xFF, 0xFF, 0x7F],
            0x95,
        ),
        ("truncated properties", packet(0x30, &[0, 1, b't', 9]), 0x81),
    ];
    for (name, bytes, reason) in cases {
        let (mut raw, _) = Raw::connected5(&a, "violator").await;
        raw.send(&bytes).await;
        assert_eq!(
            raw.read_until_closed().await,
            Some((0xE0, vec![reason, 0])),
            "{name}"
        );
    }
    // the same violations close a 3.1.1 connection without any packet
    let mut raw = Raw::connected(&a, "violator311").await;
    raw.send(&packet(0x36, &[0, 1, b't', 0, 1])).await;
    assert!(raw.read_until_closed().await.is_none());
    broker.shutdown().await;
}

#[tokio::test]
async fn keep_alive_timeout_sends_a_disconnect_to_mqtt5_clients_only() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::Both).await;
    let a = addr(&broker);
    let mut v5 = Raw::open(&a).await;
    v5.send(&connect5_raw(0b10, 1, &[], "ka5")).await;
    assert_eq!(v5.read_packet().await.unwrap().0, 0x20);
    let mut v4 = Raw::connected_keep_alive(&a, "ka4", 1).await;

    let started = std::time::Instant::now();
    assert_eq!(v5.read_until_closed().await, Some((0xE0, vec![0x8D, 0])));
    assert!(started.elapsed() >= Duration::from_millis(1300));
    assert!(v4.read_until_closed().await.is_none());
    broker.shutdown().await;
}

#[tokio::test]
async fn shutdown_sends_a_disconnect_to_mqtt5_clients_only() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::Both).await;
    let a = addr(&broker);
    let (mut v5, _) = Raw::connected5(&a, "v5").await;
    let mut v4 = Raw::connected(&a, "v4").await;
    broker.shutdown().await;
    // 0x8B: server shutting down
    assert_eq!(v5.read_until_closed().await, Some((0xE0, vec![0x8B, 0])));
    assert!(v4.read_until_closed().await.is_none());
}

#[tokio::test]
async fn client_disconnect_variants_are_accepted() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::V5).await;
    let a = addr(&broker);
    // without reason code, with reason code, with reason code and properties
    for bytes in [
        vec![0xE0u8, 0],
        vec![0xE0, 1, 0x00],
        vec![0xE0, 2, 0x04, 0],
        vec![0xE0, 7, 0x04, 5, 0x1F, 0, 2, b'h', b'i'],
    ] {
        let (mut raw, _) = Raw::connected5(&a, "bye").await;
        raw.send(&bytes).await;
        assert!(raw.read_until_closed().await.is_none(), "{bytes:?}");
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn ping_and_publish_properties_work_in_mqtt5() {
    let broker = start_mode(BrokerLimits::default(), ProtocolMode::V5).await;
    let (mut raw, _) = Raw::connected5(&addr(&broker), "ping").await;
    raw.send(&pingreq()).await;
    assert_eq!(raw.read_packet().await.unwrap(), (0xD0, vec![]));
    raw.subscribe5(1, &[("t", 0)]).await;
    // content type (0x03), user property (0x26), correlation data (0x09) are skipped
    let props = [
        0x03, 0, 1, b'x', 0x26, 0, 1, b'k', 0, 1, b'v', 0x09, 0, 2, 1, 2,
    ];
    raw.send(&publish5("t", &props, b"payload")).await;
    assert_eq!(
        raw.read_publish5().await,
        ("t".to_owned(), b"payload".to_vec())
    );
    // an empty payload is fine
    raw.send(&publish5("t", &[], b"")).await;
    assert_eq!(raw.read_publish5().await, ("t".to_owned(), vec![]));
    broker.shutdown().await;
}
