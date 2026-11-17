//! Integration tests against the real broker on an ephemeral port.

mod common;

use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;

use bytes::Bytes;
use dt_server::broker::Broker;
use dt_server::hub::HubError;
use rumqttc::v5::mqttbytes::v5::{Packet, PublishProperties};
use tokio::io::AsyncWriteExt;

use std::time::Duration;

use common::*;
use dt_server::broker::BrokerLimits;
use rumqttc::v5::mqttbytes::v5::{Filter, SubscribeReasonCode};
use rumqttc::v5::mqttbytes::QoS;

const SHORT: Duration = Duration::from_millis(400);

// --- clients written with rumqttc -------------------------------------------------------------

#[tokio::test]
async fn publish_subscribe_round_trip_and_connack_properties() {
    let broker = start(BrokerLimits::default()).await;
    let mut sub = Client::connect(broker.port(), "sub").await;
    let publisher = Client::connect(broker.port(), "pub").await;

    let props = sub.connack.properties.clone().expect("CONNACK properties");
    assert!(!sub.connack.session_present);
    assert_eq!(props.max_packet_size, Some(262_144));
    assert_eq!(props.server_keep_alive, Some(60));

    sub.client
        .subscribe("twin/temperature", QoS::AtMostOnce)
        .await
        .unwrap();
    assert_eq!(
        sub.wait_suback().await,
        vec![SubscribeReasonCode::Success(QoS::AtMostOnce)]
    );

    publisher
        .client
        .publish("twin/temperature", QoS::AtMostOnce, false, "21.5")
        .await
        .unwrap();
    assert_eq!(
        sub.next_publish().await,
        ("twin/temperature".to_owned(), b"21.5".to_vec())
    );
    broker.shutdown().await;
}

#[tokio::test]
async fn wildcards_and_deduplication() {
    let broker = start(BrokerLimits::default()).await;
    let mut sub = Client::connect(broker.port(), "sub").await;
    let publisher = Client::connect(broker.port(), "pub").await;

    // "a/b" and "a/+" and "#" all match a/b, the session still gets it once
    for filter in ["a/b", "a/+", "#", "x/+/z"] {
        sub.client.subscribe(filter, QoS::AtMostOnce).await.unwrap();
        sub.wait_suback().await;
    }
    publisher
        .client
        .publish("a/b", QoS::AtMostOnce, false, "1")
        .await
        .unwrap();
    publisher
        .client
        .publish("x/y/z", QoS::AtMostOnce, false, "2")
        .await
        .unwrap();
    assert_eq!(sub.next_publish().await.0, "a/b");
    assert_eq!(sub.next_publish().await.0, "x/y/z");
    assert!(sub.no_publish_within(SHORT).await, "duplicate delivery");
    broker.shutdown().await;
}

#[tokio::test]
async fn wildcard_does_not_deliver_non_matching_topics() {
    let broker = start(BrokerLimits::default()).await;
    let mut sub = Client::connect(broker.port(), "sub").await;
    let publisher = Client::connect(broker.port(), "pub").await;
    sub.client
        .subscribe("twin/+/value", QoS::AtMostOnce)
        .await
        .unwrap();
    sub.wait_suback().await;
    sub.client
        .subscribe("other/#", QoS::AtMostOnce)
        .await
        .unwrap();
    sub.wait_suback().await;

    publisher
        .client
        .publish("twin/a/other", QoS::AtMostOnce, false, "no")
        .await
        .unwrap();
    publisher
        .client
        .publish("twin/a/b/value", QoS::AtMostOnce, false, "no")
        .await
        .unwrap();
    publisher
        .client
        .publish("twin/a/value", QoS::AtMostOnce, false, "yes")
        .await
        .unwrap();
    publisher
        .client
        .publish("other", QoS::AtMostOnce, false, "yes2")
        .await
        .unwrap();
    assert_eq!(
        sub.next_publish().await,
        ("twin/a/value".to_owned(), b"yes".to_vec())
    );
    assert_eq!(
        sub.next_publish().await,
        ("other".to_owned(), b"yes2".to_vec())
    );
    assert!(sub.no_publish_within(SHORT).await);
    broker.shutdown().await;
}

#[tokio::test]
async fn no_local_is_respected() {
    let broker = start(BrokerLimits::default()).await;
    let mut a = Client::connect(broker.port(), "a").await;
    let mut b = Client::connect(broker.port(), "b").await;

    let mut filter = Filter::new("t", QoS::AtMostOnce);
    filter.nolocal = true;
    a.client.subscribe_many(vec![filter]).await.unwrap();
    a.wait_suback().await;
    b.client.subscribe("t", QoS::AtMostOnce).await.unwrap();
    b.wait_suback().await;

    a.client
        .publish("t", QoS::AtMostOnce, false, "from a")
        .await
        .unwrap();
    assert_eq!(b.next_publish().await.1, b"from a".to_vec());
    assert!(
        a.no_publish_within(SHORT).await,
        "no-local subscriber received its own message"
    );

    // the other direction still works
    b.client
        .publish("t", QoS::AtMostOnce, false, "from b")
        .await
        .unwrap();
    assert_eq!(a.next_publish().await.1, b"from b".to_vec());
    broker.shutdown().await;
}

#[tokio::test]
async fn unsubscribe_stops_delivery() {
    let broker = start(BrokerLimits::default()).await;
    let mut sub = Client::connect(broker.port(), "sub").await;
    let publisher = Client::connect(broker.port(), "pub").await;
    sub.client.subscribe("t", QoS::AtMostOnce).await.unwrap();
    sub.wait_suback().await;
    publisher
        .client
        .publish("t", QoS::AtMostOnce, false, "1")
        .await
        .unwrap();
    sub.next_publish().await;

    sub.client.unsubscribe("t").await.unwrap();
    sub.wait_for(|p| matches!(p, rumqttc::v5::mqttbytes::v5::Packet::UnsubAck(_)).then_some(()))
        .await;
    publisher
        .client
        .publish("t", QoS::AtMostOnce, false, "2")
        .await
        .unwrap();
    assert!(sub.no_publish_within(SHORT).await);
    broker.shutdown().await;
}

// --- hub --------------------------------------------------------------------------------------

#[tokio::test]
async fn hub_receives_client_publishes_and_publishes_to_clients() {
    let broker = start(BrokerLimits::default()).await;
    let hub = broker.hub();
    let mut internal = hub.subscribe("twin/#", 16).unwrap();
    let mut only_connect = hub.subscribe("connectToTwin", 16).unwrap();

    let mut client = Client::connect(broker.port(), "client").await;
    client
        .client
        .subscribe("server/+", QoS::AtMostOnce)
        .await
        .unwrap();
    client.wait_suback().await;

    // client -> hub
    client
        .client
        .publish("twin/x/value", QoS::AtMostOnce, false, "42")
        .await
        .unwrap();
    client
        .client
        .publish("connectToTwin", QoS::AtMostOnce, false, r#"{"a":1}"#)
        .await
        .unwrap();
    let msg = tokio::time::timeout(WAIT, internal.recv())
        .await
        .unwrap()
        .unwrap();
    assert_eq!(
        (&*msg.topic, &msg.payload[..]),
        ("twin/x/value", &b"42"[..])
    );
    let msg = tokio::time::timeout(WAIT, only_connect.recv())
        .await
        .unwrap()
        .unwrap();
    assert_eq!(
        (&*msg.topic, &msg.payload[..]),
        ("connectToTwin", &br#"{"a":1}"#[..])
    );
    // the client is not an internal subscriber of its own other topic
    assert!(internal.try_recv().is_err());

    // hub -> client
    hub.publish("server/status", "hello").unwrap();
    assert_eq!(
        client.next_publish().await,
        ("server/status".to_owned(), b"hello".to_vec())
    );
    // internal publishes also reach internal subscribers
    hub.publish("twin/y/value", "7").unwrap();
    let msg = tokio::time::timeout(WAIT, internal.recv())
        .await
        .unwrap()
        .unwrap();
    assert_eq!(&*msg.topic, "twin/y/value");
    broker.shutdown().await;
}

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn slow_internal_subscriber_never_blocks_the_broker() {
    let broker = start(BrokerLimits::default()).await;
    let hub = broker.hub();
    let slow = hub.subscribe("#", 1).unwrap(); // never read
    let mut client = Client::connect(broker.port(), "client").await;
    client.client.subscribe("t", QoS::AtMostOnce).await.unwrap();
    client.wait_suback().await;

    let publisher = Client::connect(broker.port(), "pub").await;
    for i in 0..50 {
        publisher
            .client
            .publish("t", QoS::AtMostOnce, false, format!("{i}"))
            .await
            .unwrap();
    }
    for i in 0..50 {
        assert_eq!(client.next_publish().await.1, format!("{i}").into_bytes());
    }
    assert_eq!(slow.dropped(), 49);
    assert_eq!(hub.dropped_total(), 49);
    broker.shutdown().await;
}

// --- protocol violations and limits (raw sockets) ---------------------------------------------

#[tokio::test]
async fn qos1_publish_gets_protocol_error_disconnect() {
    let broker = start(BrokerLimits::default()).await;
    let mut raw = Raw::connected(&addr(&broker), "q1").await;
    raw.send(&publish_qos1("t", b"x")).await;
    let (first, body) = raw.read_packet().await.expect("DISCONNECT");
    assert_eq!(first, 0xE0);
    assert_eq!(body[0], 0x82, "reason code protocol_error");
    assert!(
        raw.read_packet().await.is_none(),
        "connection is closed after DISCONNECT"
    );
    broker.shutdown().await;
}

#[tokio::test]
async fn socket_without_connect_is_closed_after_the_timeout() {
    let limits = BrokerLimits {
        connect_timeout_s: 1,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let mut raw = Raw::open(&addr(&broker)).await;
    let started = std::time::Instant::now();
    assert!(raw.read_until_closed().await.is_none());
    let elapsed = started.elapsed();
    assert!(
        elapsed >= Duration::from_millis(900),
        "closed too early: {elapsed:?}"
    );
    assert!(
        elapsed < Duration::from_secs(4),
        "closed too late: {elapsed:?}"
    );
    broker.shutdown().await;
}

#[tokio::test]
async fn trickling_bytes_does_not_extend_the_connect_timeout() {
    let limits = BrokerLimits {
        connect_timeout_s: 1,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let mut raw = Raw::open(&addr(&broker)).await;
    let bytes = connect("slow");
    let started = std::time::Instant::now();
    // one byte every 300 ms: the CONNECT would only be complete after 5 s
    let mut closed_after = None;
    for byte in &bytes[..bytes.len() - 1] {
        if raw.stream.write_all(&[*byte]).await.is_err() {
            closed_after = Some(started.elapsed());
            break;
        }
        if let Ok(read) = raw.read_packet_within(Duration::from_millis(300)).await {
            assert!(read.is_none(), "unexpected packet");
            closed_after = Some(started.elapsed());
            break;
        }
    }
    let closed_after = closed_after.expect("connection was not closed while trickling");
    assert!(
        closed_after >= Duration::from_millis(900) && closed_after < Duration::from_millis(2500),
        "closed after {closed_after:?}"
    );
    broker.shutdown().await;
}

#[tokio::test]
async fn first_packet_not_connect_closes_the_connection() {
    let broker = start(BrokerLimits::default()).await;
    for bytes in [pingreq(), publish("t", b"x"), subscribe(1, &[("t", false)])] {
        let mut raw = Raw::open(&addr(&broker)).await;
        raw.send(&bytes).await;
        assert!(
            raw.read_until_closed().await.is_none(),
            "no packet expected before the close"
        );
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn mqtt_311_client_is_refused() {
    let broker = start(BrokerLimits::default()).await;
    let mut raw = Raw::open(&addr(&broker)).await;
    raw.send(&[0x10, 12, 0, 4, b'M', b'Q', b'T', b'T', 4, 2, 0, 10, 0, 0])
        .await;
    assert!(raw.read_until_closed().await.is_none());
    broker.shutdown().await;
}

#[tokio::test]
async fn second_connect_is_a_protocol_error() {
    let broker = start(BrokerLimits::default()).await;
    let mut raw = Raw::connected(&addr(&broker), "twice").await;
    raw.send(&connect("twice")).await;
    let (first, body) = raw.read_until_closed().await.expect("DISCONNECT");
    assert_eq!((first, body[0]), (0xE0, 0x82));
    broker.shutdown().await;
}

#[tokio::test]
async fn subscription_quota_and_invalid_filters() {
    let limits = BrokerLimits {
        max_subscriptions_per_session: 2,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let mut raw = Raw::connected(&addr(&broker), "quota").await;

    // third distinct filter exceeds the quota, an update of an existing filter does not count
    assert_eq!(
        raw.subscribe(1, &[("a", false), ("b", false), ("c", false)])
            .await,
        vec![0x00, 0x00, 0x97]
    );
    assert_eq!(raw.subscribe(2, &[("a", true)]).await, vec![0x00]);
    assert_eq!(raw.subscribe(3, &[("d", false)]).await, vec![0x97]);

    // freeing a slot makes room again
    raw.send(&unsubscribe(4, &["b"])).await;
    let (first, body) = raw.read_packet().await.unwrap();
    assert_eq!((first, &body[..]), (0xB0, &[0, 4, 0, 0][..]));
    assert_eq!(raw.subscribe(5, &[("d", false)]).await, vec![0x00]);

    // invalid filters
    let long = "x".repeat(513);
    let deep = vec!["l"; 17].join("/");
    let reasons = raw
        .subscribe(
            6,
            &[
                (&long, false),
                (&deep, false),
                ("a/#/b", false),
                ("a", false),
            ],
        )
        .await;
    assert_eq!(reasons, vec![0x8F, 0x8F, 0x8F, 0x00]);
    broker.shutdown().await;
}

#[tokio::test]
async fn quota_is_reported_to_rumqttc_clients() {
    let limits = BrokerLimits {
        max_subscriptions_per_session: 1,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let mut c = Client::connect(broker.port(), "q").await;
    c.client.subscribe("a", QoS::AtMostOnce).await.unwrap();
    assert_eq!(
        c.wait_suback().await,
        vec![SubscribeReasonCode::Success(QoS::AtMostOnce)]
    );
    c.client.subscribe("b", QoS::AtMostOnce).await.unwrap();
    assert_eq!(
        c.wait_suback().await,
        vec![SubscribeReasonCode::QuotaExceeded]
    );
    broker.shutdown().await;
}

#[tokio::test]
async fn oversize_packet_closes_the_connection() {
    let limits = BrokerLimits {
        max_packet_size: 1024,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;

    // complete oversize packet
    let mut raw = Raw::connected(&addr(&broker), "big").await;
    raw.send(&publish("t", &vec![b'x'; 2000])).await;
    let last = raw.read_until_closed().await.expect("DISCONNECT");
    assert_eq!((last.0, last.1[0]), (0xE0, 0x95), "packet_too_large");

    // only the header of a huge packet: rejected without waiting for the body
    let mut raw = Raw::connected(&addr(&broker), "huge").await;
    raw.send(&[0x30, 0xFF, 0xFF, 0xFF, 0x7F]).await;
    let last = raw.read_until_closed().await.expect("DISCONNECT");
    assert_eq!((last.0, last.1[0]), (0xE0, 0x95));

    // exactly at the limit is accepted
    let mut ok = Raw::connected(&addr(&broker), "exact").await;
    // fixed header 3 bytes (two byte remaining length), topic 3, properties length 1 => 7 bytes of overhead
    let message = publish("t", &vec![b'x'; 1024 - 7]);
    assert_eq!(message.len(), 1024);
    ok.send(&message).await;
    ok.send(&pingreq()).await;
    assert_eq!(ok.read_packet().await.unwrap().0, 0xD0);
    broker.shutdown().await;
}

#[tokio::test]
async fn per_ip_connection_limit() {
    let limits = BrokerLimits {
        max_connections_per_ip: 2,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let a = Raw::connected(&addr(&broker), "a").await;
    let _b = Raw::connected(&addr(&broker), "b").await;

    // the third connection is accepted by the kernel and closed right away
    let mut third = Raw::open(&addr(&broker)).await;
    third.send(&connect("c")).await;
    assert!(
        third.read_until_closed().await.is_none(),
        "no CONNACK for a rejected connection"
    );

    // a slot becomes free when a client leaves
    drop(a);
    let mut accepted = false;
    for _ in 0..50 {
        let mut raw = Raw::open(&addr(&broker)).await;
        raw.send(&connect("d")).await;
        if matches!(
            raw.read_packet_within(Duration::from_millis(500)).await,
            Ok(Some((0x20, _)))
        ) {
            accepted = true;
            break;
        }
        tokio::time::sleep(Duration::from_millis(100)).await;
    }
    assert!(accepted, "slot was not released");
    broker.shutdown().await;
}

#[tokio::test]
async fn total_connection_limit() {
    let limits = BrokerLimits {
        max_connections: 2,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let _a = Raw::connected(&addr(&broker), "a").await;
    let _b = Raw::connected(&addr(&broker), "b").await;
    let mut third = Raw::open(&addr(&broker)).await;
    third.send(&connect("c")).await;
    assert!(third.read_until_closed().await.is_none());
    broker.shutdown().await;
}

#[tokio::test]
async fn keep_alive_is_enforced_with_the_server_keep_alive() {
    let limits = BrokerLimits {
        server_keep_alive_s: 1,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;

    // announced in CONNACK
    let mut raw = Raw::open(&addr(&broker)).await;
    raw.send(&connect("ka")).await;
    let (_, body) = raw.read_packet().await.unwrap();
    assert_eq!(
        &body[2..],
        &[8, 0x27, 0, 4, 0, 0, 0x13, 0, 1],
        "CONNACK properties"
    );

    // a client that pings keeps its connection (3 pings over 2.1 s, limit is 1.5 s without traffic)
    for _ in 0..3 {
        tokio::time::sleep(Duration::from_millis(700)).await;
        raw.send(&pingreq()).await;
        assert_eq!(raw.read_packet().await.unwrap().0, 0xD0);
    }
    // silence is punished after 1.5 x keep alive
    let started = std::time::Instant::now();
    let last = raw.read_until_closed().await.expect("DISCONNECT");
    assert_eq!((last.0, last.1[0]), (0xE0, 0x8D));
    let elapsed = started.elapsed();
    assert!(
        elapsed >= Duration::from_millis(1300) && elapsed < Duration::from_secs(4),
        "{elapsed:?}"
    );
    broker.shutdown().await;
}

#[tokio::test]
async fn publish_exceeding_topic_limits_is_dropped_without_disconnect() {
    let broker = start(BrokerLimits::default()).await;
    let mut sub = Raw::connected(&addr(&broker), "sub").await;
    sub.subscribe(1, &[("#", false)]).await;
    let mut raw = Raw::connected(&addr(&broker), "pub").await;
    raw.send(&publish(&vec!["l"; 17].join("/"), b"x")).await;
    raw.send(&publish(&"t".repeat(513), b"x")).await;
    raw.send(&publish("ok", b"y")).await;
    assert_eq!(sub.read_publish().await, ("ok".to_owned(), b"y".to_vec()));
    assert!(sub.is_silent_for(SHORT).await);
    // still connected
    raw.send(&pingreq()).await;
    assert_eq!(raw.read_packet().await.unwrap().0, 0xD0);
    broker.shutdown().await;
}

#[tokio::test]
async fn publish_with_wildcard_topic_is_rejected() {
    let broker = start(BrokerLimits::default()).await;
    let mut raw = Raw::connected(&addr(&broker), "w").await;
    raw.send(&publish("a/+", b"x")).await;
    let last = raw.read_until_closed().await.expect("DISCONNECT");
    assert_eq!((last.0, last.1[0]), (0xE0, 0x90));
    broker.shutdown().await;
}

#[tokio::test(flavor = "multi_thread", worker_threads = 2)]
async fn slow_client_is_dropped_and_others_are_not_affected() {
    // The limit is much larger than the lag the publisher allows for the healthy client (8 x 100 KB), so
    // this also holds when the machine is busy.
    let limits = BrokerLimits {
        max_pending_send_bytes: 2_000_000,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let hub = broker.hub();

    // never reads after subscribing
    let mut slow = Raw::connected(&addr(&broker), "slow").await;
    slow.subscribe(1, &[("big", false)]).await;
    let mut healthy = Raw::connected(&addr(&broker), "healthy").await;
    healthy.subscribe(1, &[("big", false)]).await;

    let received = Arc::new(AtomicUsize::new(0));
    let counter = Arc::clone(&received);
    let reader = tokio::spawn(async move {
        for _ in 0..300 {
            assert_eq!(healthy.read_publish().await.1.len(), 100_000);
            counter.fetch_add(1, Ordering::SeqCst);
        }
        healthy
    });

    // 300 x 100 KB = 30 MB, far more than the socket buffers of the stalled client can hold
    let payload = vec![b'z'; 100_000];
    for sent in 0..300usize {
        while sent - received.load(Ordering::SeqCst) > 8 {
            tokio::time::sleep(Duration::from_millis(1)).await;
        }
        hub.publish("big", payload.clone()).unwrap();
    }
    // the healthy client got everything
    let healthy = tokio::time::timeout(Duration::from_secs(60), reader)
        .await
        .unwrap()
        .unwrap();

    // the stalled client was disconnected: draining its socket ends in EOF or a reset
    let mut drained = 0usize;
    loop {
        match slow.read_packet_within(Duration::from_secs(10)).await {
            Ok(Some(_)) => drained += 1,
            Ok(None) => break,
            Err(()) => panic!("slow client still connected after {drained} packets"),
        }
    }
    assert!(drained < 300, "slow client received all messages");
    drop(healthy);
    wait_for_connections(&broker, 0).await;
    broker.shutdown().await;
}

#[tokio::test]
async fn shutdown_disconnects_clients_gracefully() {
    let broker = start(BrokerLimits::default()).await;
    let mut raw = Raw::connected(&addr(&broker), "x").await;
    let address = addr(&broker);
    let started = std::time::Instant::now();
    broker.shutdown().await;
    assert!(started.elapsed() < Duration::from_secs(3));
    let last = raw.read_until_closed().await.expect("DISCONNECT");
    assert_eq!((last.0, last.1[0]), (0xE0, 0x8B), "server shutting down");
    // the listener is gone
    assert!(tokio::net::TcpStream::connect(address).await.is_err());
}

#[tokio::test]
async fn abrupt_disconnect_removes_subscriptions_and_releases_the_slot() {
    let limits = BrokerLimits {
        max_connections_per_ip: 1,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let hub = broker.hub();
    {
        let mut raw = Raw::connected(&addr(&broker), "gone").await;
        raw.subscribe(1, &[("t", false), ("u/#", false)]).await;
        assert_eq!(broker.subscription_count(), 2);
        assert_eq!(broker.connection_count(), 1);
    } // dropped without DISCONNECT
    wait_for_connections(&broker, 0).await;
    assert_eq!(broker.subscription_count(), 0);
    // the only slot of this IP is free again
    let mut another = Raw::connected(&addr(&broker), "another").await;
    another.subscribe(1, &[("t", false)]).await;
    hub.publish("t", "y").unwrap();
    assert_eq!(
        another.read_publish().await,
        ("t".to_owned(), b"y".to_vec())
    );
    broker.shutdown().await;
}

// --- bounded memory, hub lifecycle, properties, protocol details -------------------------------

#[tokio::test]
async fn ping_flood_without_reading_drops_the_session() {
    let limits = BrokerLimits {
        max_pending_send_bytes: 64 * 1024,
        max_connections_per_ip: 1,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let mut raw = Raw::connected(&addr(&broker), "flood").await;
    // never read: the PINGRESPs pile up first in the socket buffers, then in the broker's queue
    let chunk: Vec<u8> = [0xC0u8, 0].repeat(32 * 1024);
    let mut failed = false;
    for _ in 0..1000 {
        match tokio::time::timeout(WAIT, raw.stream.write_all(&chunk)).await {
            Ok(Ok(())) => {}
            Ok(Err(_)) => {
                failed = true;
                break;
            }
            Err(_) => panic!("broker stopped reading but did not drop the session"),
        }
    }
    assert!(failed, "session was not dropped after 64 MB of PINGREQs");
    wait_for_connections(&broker, 0).await;
    // the slot is usable again
    let _again = Raw::connected(&addr(&broker), "again").await;
    broker.shutdown().await;
}

#[tokio::test]
async fn hub_is_closed_after_shutdown() {
    let broker = start(BrokerLimits::default()).await;
    let hub = broker.hub();
    let mut sub = hub.subscribe("#", 4).unwrap();
    broker.shutdown().await;
    let ended = tokio::time::timeout(WAIT, sub.recv())
        .await
        .expect("recv returns");
    assert!(ended.is_none());
    assert_eq!(hub.publish("t", "x").unwrap_err(), HubError::Closed);
    assert_eq!(hub.subscribe("#", 1).unwrap_err(), HubError::Closed);
}

#[tokio::test]
async fn dropping_the_broker_shuts_it_down_like_shutdown() {
    let broker = start(BrokerLimits::default()).await;
    let address = addr(&broker);
    let hub = broker.hub();
    let mut sub = hub.subscribe("#", 4).unwrap();
    let mut raw = Raw::connected(&address, "x").await;
    drop(broker);
    let last = raw.read_until_closed().await.expect("DISCONNECT");
    assert_eq!((last.0, last.1[0]), (0xE0, 0x8B));
    assert!(tokio::time::timeout(WAIT, sub.recv())
        .await
        .unwrap()
        .is_none());
    assert_eq!(hub.publish("t", "x").unwrap_err(), HubError::Closed);
    assert!(tokio::net::TcpStream::connect(address).await.is_err());
}

#[tokio::test]
async fn publish_properties_are_forwarded() {
    let broker = start(BrokerLimits::default()).await;
    let mut internal = broker.hub().subscribe("req/#", 4).unwrap();
    let mut sub = Client::connect(broker.port(), "sub").await;
    let publisher = Client::connect(broker.port(), "pub").await;
    sub.client
        .subscribe("req/#", QoS::AtMostOnce)
        .await
        .unwrap();
    sub.wait_suback().await;

    let props = PublishProperties {
        response_topic: Some("resp/topic".to_owned()),
        correlation_data: Some(Bytes::from_static(b"corr-1")),
        user_properties: vec![("k".to_owned(), "v".to_owned())],
        content_type: Some("application/json".to_owned()),
        ..Default::default()
    };
    publisher
        .client
        .publish_with_properties("req/a", QoS::AtMostOnce, false, "{}", props.clone())
        .await
        .unwrap();
    let got = sub
        .wait_for(|p| match p {
            Packet::Publish(p) => Some(p.clone()),
            _ => None,
        })
        .await;
    let got_props = got.properties.expect("properties are forwarded");
    assert_eq!(got_props.response_topic, props.response_topic);
    assert_eq!(got_props.correlation_data, props.correlation_data);
    assert_eq!(got_props.user_properties, props.user_properties);
    assert_eq!(got_props.content_type, props.content_type);
    assert_eq!(&got.payload[..], b"{}");

    // and the hub sees the raw property bytes
    let msg = tokio::time::timeout(WAIT, internal.recv())
        .await
        .unwrap()
        .unwrap();
    assert!(!msg.properties.is_empty());
    assert!(msg.properties.windows(6).any(|w| w == b"corr-1"));
    broker.shutdown().await;
}

#[tokio::test]
async fn topic_alias_is_a_protocol_error() {
    let broker = start(BrokerLimits::default()).await;
    for topic in ["t", ""] {
        let mut raw = Raw::connected(&addr(&broker), "alias").await;
        let mut body = string_bytes(topic);
        body.extend([3, 0x23, 0, 1]); // properties: topic alias 1
        body.extend(b"x");
        raw.send(&packet(0x30, &body)).await;
        let last = raw.read_until_closed().await.expect("DISCONNECT");
        assert_eq!((last.0, last.1[0]), (0xE0, 0x82), "topic {topic:?}");
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn nothing_but_connack_is_sent_before_the_connection_is_established() {
    let broker = start(BrokerLimits::default()).await;
    // QoS 1 PUBLISH as first packet
    let mut raw = Raw::open(&addr(&broker)).await;
    raw.send(&publish_qos1("t", b"x")).await;
    assert!(raw.read_until_closed().await.is_none());
    // malformed CONNECT: reserved flag set
    let mut raw = Raw::open(&addr(&broker)).await;
    raw.send(&connect_raw(0b11, &[], "c", &[])).await;
    assert!(raw.read_until_closed().await.is_none());
    // malformed CONNECT: truncated client id
    let mut raw = Raw::open(&addr(&broker)).await;
    let mut body = string_bytes("MQTT");
    body.extend([5, 2, 0, 30, 0, 0, 9, b'a']);
    raw.send(&packet(0x10, &body)).await;
    assert!(raw.read_until_closed().await.is_none());
    // oversize first packet
    let limits = BrokerLimits {
        max_packet_size: 64,
        ..BrokerLimits::default()
    };
    let small = start(limits).await;
    let mut raw = Raw::open(&addr(&small)).await;
    raw.send(&connect("x".repeat(100).as_str())).await;
    assert!(raw.read_until_closed().await.is_none());
    small.shutdown().await;
    broker.shutdown().await;
}

#[tokio::test]
async fn hub_publish_rejects_payloads_that_do_not_fit() {
    let broker = start(BrokerLimits::default()).await;
    let hub = broker.hub();
    // encoded size = 1 + 3 (remaining length) + 2 + 1 (topic) + 1 (properties length) + payload
    assert_eq!(hub.publish("t", vec![0u8; 262_144 - 8]), Ok(()));
    assert_eq!(
        hub.publish("t", vec![0u8; 262_144 - 7]),
        Err(HubError::PayloadTooLarge)
    );
    assert_eq!(
        hub.publish("t", vec![0u8; 300_000_000]),
        Err(HubError::PayloadTooLarge)
    );
    broker.shutdown().await;
}

#[tokio::test]
async fn publishes_larger_than_the_client_maximum_are_dropped_for_that_client() {
    let broker = start(BrokerLimits::default()).await;
    let hub = broker.hub();
    // Maximum Packet Size property (0x27) = 100
    let connect = connect_raw(0b10, &[0x27, 0, 0, 0, 100], "limited", &[]);
    let (mut limited, _) = Raw::connected_with(&addr(&broker), &connect).await;
    limited.subscribe(1, &[("t", false)]).await;
    let mut unlimited = Raw::connected(&addr(&broker), "unlimited").await;
    unlimited.subscribe(1, &[("t", false)]).await;

    // more oversize messages than max_consecutive_send_overflows (3): must not count as overflow
    for _ in 0..5 {
        hub.publish("t", vec![b'x'; 200]).unwrap();
    }
    hub.publish("t", vec![b'y'; 10]).unwrap();
    for _ in 0..5 {
        assert_eq!(unlimited.read_publish().await.1.len(), 200);
    }
    assert_eq!(
        limited.read_publish().await,
        ("t".to_owned(), vec![b'y'; 10])
    );
    assert_eq!(hub.oversize_dropped_total(), 5);
    assert_eq!(broker.connection_count(), 2);
    broker.shutdown().await;
}

#[tokio::test]
async fn empty_client_id_gets_an_assigned_identifier() {
    let broker = start(BrokerLimits::default()).await;
    let mut ids = Vec::new();
    for _ in 0..2 {
        let (_raw, body) =
            Raw::connected_with(&addr(&broker), &connect_raw(0b10, &[], "", &[])).await;
        // ack flags, reason code, property length, 0x27 + u32, 0x13 + u16, then 0x12 + string
        assert_eq!(body[2] as usize, body.len() - 3);
        assert_eq!(body[11], 0x12);
        let len = usize::from(u16::from_be_bytes([body[12], body[13]]));
        assert!(len > 0);
        ids.push(body[14..14 + len].to_vec());
    }
    assert_ne!(ids[0], ids[1]);
    // an explicit id is not echoed
    let (_raw, body) = Raw::connected_with(&addr(&broker), &connect("mine")).await;
    assert_eq!(body.len(), 3 + 8);
    broker.shutdown().await;
}

#[tokio::test]
async fn reserved_flags_and_retain_handling_are_malformed() {
    let broker = start(BrokerLimits::default()).await;
    let cases: Vec<Vec<u8>> = vec![
        vec![0x80, 6, 0, 1, 0, 0, 1, b'a', 0], // SUBSCRIBE with flags 0
        vec![0x83, 6, 0, 1, 0, 0, 1, b'a', 0], // SUBSCRIBE with flags 3
        vec![0xA0, 5, 0, 1, 0, 0, 1, b'a'],    // UNSUBSCRIBE with flags 0
        vec![0xC1, 0],                         // PINGREQ with flags 1
        vec![0xE1, 0],                         // DISCONNECT with flags 1
        vec![0x82, 7, 0, 1, 0, 0, 1, b'a', 0x30], // Retain Handling 3
    ];
    for bytes in cases {
        let mut raw = Raw::connected(&addr(&broker), "flags").await;
        raw.send(&bytes).await;
        let last = raw.read_until_closed().await.expect("DISCONNECT");
        assert_eq!((last.0, last.1[0]), (0xE0, 0x81), "{bytes:x?}");
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn connect_with_will_credentials_and_properties() {
    let broker = start(BrokerLimits::default()).await;
    // flags: user name, password, will retain=0 qos=0, will flag, clean start
    let flags = 0b1100_0110;
    let props = [
        0x11, 0, 0, 0, 10, // session expiry
        0x21, 0, 5, // receive maximum
        0x26, 0, 1, b'k', 0, 1, b'v', // user property
    ];
    let mut tail = vec![0]; // will properties length
    tail.extend(string_bytes("will/topic"));
    tail.extend([0, 3, b'b', b'y', b'e']); // will payload
    tail.extend(string_bytes("user"));
    tail.extend([0, 2, 0xDE, 0xAD]); // password (binary data)
    let (mut raw, _) =
        Raw::connected_with(&addr(&broker), &connect_raw(flags, &props, "full", &tail)).await;
    raw.send(&pingreq()).await;
    assert_eq!(raw.read_packet().await.unwrap().0, 0xD0);
    broker.shutdown().await;
}

#[tokio::test]
async fn subscribe_with_subscription_identifier_and_disconnect_with_properties() {
    let broker = start(BrokerLimits::default()).await;
    let hub = broker.hub();
    let mut raw = Raw::connected(&addr(&broker), "subid").await;
    // SUBSCRIBE id 3, properties: subscription identifier 7, filter "t", options 0
    raw.send(&packet(0x82, &[0, 3, 2, 0x0B, 7, 0, 1, b't', 0]))
        .await;
    let (first, body) = raw.read_packet().await.unwrap();
    assert_eq!((first, &body[..]), (0x90, &[0, 3, 0, 0][..]));
    hub.publish("t", "x").unwrap();
    assert_eq!(raw.read_publish().await, ("t".to_owned(), b"x".to_vec()));
    // DISCONNECT with reason code and properties (session expiry)
    raw.send(&packet(0xE0, &[0x00, 5, 0x11, 0, 0, 0, 0])).await;
    assert!(raw.read_until_closed().await.is_none());
    wait_for_connections(&broker, 0).await;
    assert_eq!(broker.subscription_count(), 0);
    broker.shutdown().await;
}

#[tokio::test]
async fn tracker_slot_is_released_after_keep_alive_timeout_and_decode_error() {
    let limits = BrokerLimits {
        server_keep_alive_s: 1,
        max_connections_per_ip: 2,
        ..BrokerLimits::default()
    };
    let broker = start(limits).await;
    let mut idle = Raw::connected(&addr(&broker), "idle").await;
    let mut bad = Raw::connected(&addr(&broker), "bad").await;
    bad.send(&publish_qos1("t", b"x")).await;
    bad.read_until_closed().await;
    // the decode error released its slot while the idle client still holds one
    for _ in 0..50 {
        if broker.connection_count() == 1 {
            break;
        }
        tokio::time::sleep(Duration::from_millis(50)).await;
    }
    assert_eq!(broker.connection_count(), 1);
    let last = idle.read_until_closed().await.expect("DISCONNECT");
    assert_eq!(last.1[0], 0x8D);
    wait_for_connections(&broker, 0).await;
    broker.shutdown().await;
}

/// Waits until the broker counts `expected` connections.
async fn wait_for_connections(broker: &Broker, expected: usize) {
    for _ in 0..100 {
        if broker.connection_count() == expected {
            return;
        }
        tokio::time::sleep(Duration::from_millis(50)).await;
    }
    panic!(
        "expected {expected} connections, broker counts {}",
        broker.connection_count()
    );
}
