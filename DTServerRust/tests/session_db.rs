//! MQTT sessions that survive a restart of the broker: the whole broker against a real PostgreSQL. Runs only when
//! `DT_TEST_DATABASE_URL` is set (a database in which the test user may create schemas); otherwise every test
//! prints a "SKIPPED" line and passes. The tests of the individual ops are in `src/broker/persist/db_tests.rs`.
//!
//! Each test uses its own schema (`dt_test_<uuid>`, selected through the `search_path`), which the broker fills
//! (it applies `sql/0001_mqtt_session.sql` itself) and which is dropped at the end, also when the test panics.
//! The URL is never printed.

mod common;

use std::str::FromStr;
use std::time::Duration;

use common::*;
use dt_server::broker::{Broker, BrokerLimits, BrokerOptions, ProtocolMode, SessionDatabase};
use sqlx::postgres::{PgConnectOptions, PgPool, PgPoolOptions};
use uuid::Uuid;

const SHORT: Duration = Duration::from_millis(400);

struct TestDb {
    url: String,
    schema: String,
}

impl TestDb {
    /// `None` (and a message) when `DT_TEST_DATABASE_URL` is not set.
    async fn create(test: &str) -> Option<TestDb> {
        let Ok(url) = std::env::var("DT_TEST_DATABASE_URL") else {
            eprintln!("SKIPPED {test}: DT_TEST_DATABASE_URL is not set (no PostgreSQL available)");
            return None;
        };
        let schema = format!("dt_test_{}", Uuid::new_v4().simple());
        let admin = PgPoolOptions::new()
            .max_connections(1)
            .connect_with(PgConnectOptions::from_str(&url).expect("valid DT_TEST_DATABASE_URL"))
            .await
            .expect("connect to the test database");
        // the name consists of a fixed prefix and hex digits only
        sqlx::query(sqlx::AssertSqlSafe(format!("CREATE SCHEMA {schema}")))
            .execute(&admin)
            .await
            .expect("create the test schema");
        admin.close().await;
        Some(TestDb { url, schema })
    }

    fn sessions(&self) -> Option<SessionDatabase> {
        Some(SessionDatabase {
            url: self.url.clone(),
            schema: Some(self.schema.clone()),
        })
    }

    /// A broker for both protocol versions on an ephemeral port that keeps its sessions in this schema.
    async fn broker(&self, limits: BrokerLimits) -> Broker {
        // The advisory lock of a broker that was shut down a moment ago goes with its connection, and the server
        // may need a few milliseconds to notice that it is closed: try again for a short while.
        let mut attempts = 0;
        loop {
            let started = Broker::start_with(
                0,
                limits.clone(),
                BrokerOptions {
                    protocol: ProtocolMode::Both,
                    sessions: self.sessions(),
                    ..BrokerOptions::default()
                },
            )
            .await;
            match started {
                Ok(broker) => return broker,
                Err(e) if attempts < 50 && e.to_string().contains("another broker uses") => {
                    attempts += 1;
                    tokio::time::sleep(Duration::from_millis(100)).await;
                }
                Err(e) => panic!("the broker starts with the session database: {e}"),
            }
        }
    }

    async fn pool(&self) -> PgPool {
        let options = PgConnectOptions::from_str(&self.url)
            .unwrap()
            .options([("search_path", self.schema.as_str())]);
        PgPoolOptions::new()
            .max_connections(2)
            .connect_with(options)
            .await
            .expect("connect")
    }

    async fn rows(&self, table: &str) -> i64 {
        sqlx::query_scalar(sqlx::AssertSqlSafe(format!("SELECT count(*) FROM {table}")))
            .fetch_one(&self.pool().await)
            .await
            .unwrap()
    }
}

impl Drop for TestDb {
    fn drop(&mut self) {
        // also runs when the test panicked: drop the schema from a thread with its own runtime
        let (url, schema) = (self.url.clone(), self.schema.clone());
        let _ = std::thread::spawn(move || {
            let runtime = tokio::runtime::Builder::new_current_thread()
                .enable_all()
                .build()
                .unwrap();
            runtime.block_on(async {
                if let Ok(pool) = PgPoolOptions::new()
                    .max_connections(1)
                    .connect_with(PgConnectOptions::from_str(&url).unwrap())
                    .await
                {
                    let _ = sqlx::query(sqlx::AssertSqlSafe(format!(
                        "DROP SCHEMA IF EXISTS {schema} CASCADE"
                    )))
                    .execute(&pool)
                    .await;
                    pool.close().await;
                }
            });
        })
        .join();
    }
}

async fn eventually(what: &str, mut cond: impl FnMut() -> bool) {
    for _ in 0..250 {
        if cond() {
            return;
        }
        tokio::time::sleep(Duration::from_millis(20)).await;
    }
    panic!("timed out waiting for: {what}");
}

const DISCONNECT: [u8; 2] = [0xE0, 0x00];

/// Ends a connection the way a client does and waits until the broker has handled it. `others` is the number of
/// connections that stay open (and are not counted as ended).
async fn disconnect(mut raw: Raw, broker: &Broker, others: usize) {
    raw.send(&DISCONNECT).await;
    drop(raw);
    eventually("the connection to end", || {
        broker.connection_count() == others
    })
    .await;
}

/// A round trip that tells that everything sent before was handled by the broker.
async fn sync(raw: &mut Raw) {
    raw.send(&pingreq()).await;
    assert_eq!(raw.read_packet().await.unwrap().0, 0xD0);
}

#[tokio::test]
async fn a_3_1_1_session_survives_a_restart_and_delivers_each_message_once() {
    let Some(db) = TestDb::create("a_3_1_1_session_survives_a_restart").await else {
        return;
    };
    let broker = db.broker(BrokerLimits::default()).await;
    let a = addr(&broker);

    // a persistent subscriber with a QoS 1 and a QoS 2 subscription goes offline
    let (mut keeper, present) = Raw::connected_session(&a, "keeper", false).await;
    assert!(!present);
    keeper
        .send(&subscribe_qos(1, &[("q1", 1), ("q2", 2)]))
        .await;
    assert_eq!(
        keeper.read_packet().await.unwrap(),
        (0x90, vec![0, 1, 1, 2])
    );
    disconnect(keeper, &broker, 0).await;

    // the messages are queued for the offline subscriber; the acknowledgements come after they are stored
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    publisher
        .send(&publish_with_id("q1", b"m1", 1, 1, false))
        .await;
    assert_eq!(publisher.read_ack(PUBACK).await, (1, None));
    publisher
        .send(&publish_with_id("q2", b"m2", 2, 2, false))
        .await;
    assert_eq!(publisher.read_ack(PUBREC).await, (2, None));
    publisher.send(&ack(PUBREL, 2)).await;
    assert_eq!(publisher.read_ack(PUBCOMP).await, (2, None));
    drop(publisher);

    // a second persistent client sends a QoS 2 PUBLISH and does not finish the handshake
    let (mut pub2, _) = Raw::connected_session(&a, "pub2", false).await;
    pub2.send(&publish_with_id("q2", b"m3", 2, 9, false)).await;
    assert_eq!(pub2.read_ack(PUBREC).await, (9, None));
    disconnect(pub2, &broker, 0).await;

    broker.shutdown().await;
    assert_eq!(
        db.rows("mqtt_session").await,
        2,
        "keeper and pub2; the clean session left nothing"
    );
    assert_eq!(db.rows("mqtt_subscription").await, 2);
    assert_eq!(db.rows("mqtt_outbound").await, 3);
    assert_eq!(db.rows("mqtt_inbound_qos2").await, 1);

    // the second start
    let broker = db.broker(BrokerLimits::default()).await;
    let a = addr(&broker);
    assert_eq!(broker.subscription_count(), 2, "the subscriptions are back");

    // pub2 repeats its PUBLISH, which the broker must recognise: no second delivery
    let (mut pub2, present) = Raw::connected_session(&a, "pub2", false).await;
    assert!(present);
    pub2.send(&publish_with_id("q2", b"m3", 2, 9, true)).await;
    assert_eq!(pub2.read_ack(PUBREC).await, (9, None));
    pub2.send(&ack(PUBREL, 9)).await;
    assert_eq!(pub2.read_ack(PUBCOMP).await, (9, None));
    sync(&mut pub2).await;

    // the subscriber comes back without subscribing again and gets each message once, as a repeat
    let (mut keeper, present) = Raw::connected_session(&a, "keeper", false).await;
    assert!(present, "session present after the restart");
    // a 3.1.1 client has Receive Maximum 65535: all three PUBLISH packets arrive at once, so they are read
    // first and the handshakes follow
    let m1 = keeper.read_any_publish(false).await;
    let m2 = keeper.read_any_publish(false).await;
    let m3 = keeper.read_any_publish(false).await;
    assert_eq!(
        (m1.topic.as_str(), m1.payload.as_slice(), m1.qos, m1.dup),
        ("q1", &b"m1"[..], 1, true)
    );
    for (m, expected) in [(&m2, &b"m2"[..]), (&m3, &b"m3"[..])] {
        assert_eq!(
            (m.topic.as_str(), m.payload.as_slice(), m.qos, m.dup),
            ("q2", expected, 2, true)
        );
    }
    keeper.send(&ack(PUBACK, m1.packet_id)).await;
    keeper.send(&ack(PUBREC, m2.packet_id)).await;
    keeper.send(&ack(PUBREC, m3.packet_id)).await;
    let mut released = vec![
        keeper.read_ack(PUBREL).await.0,
        keeper.read_ack(PUBREL).await.0,
    ];
    released.sort_unstable();
    let mut expected_ids = vec![m2.packet_id, m3.packet_id];
    expected_ids.sort_unstable();
    assert_eq!(released, expected_ids);
    keeper.send(&ack(PUBCOMP, m2.packet_id)).await;
    keeper.send(&ack(PUBCOMP, m3.packet_id)).await;
    assert!(
        keeper.is_silent_for(SHORT).await,
        "nothing is delivered twice"
    );
    sync(&mut keeper).await;

    // messages published now reach the restored subscription
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    publisher
        .send(&publish_with_id("q1", b"live", 1, 1, false))
        .await;
    publisher.read_ack(PUBACK).await;
    let live = keeper.read_any_publish(false).await;
    assert_eq!((live.payload.as_slice(), live.dup), (&b"live"[..], false));
    keeper.send(&ack(PUBACK, live.packet_id)).await;
    sync(&mut keeper).await;
    drop(publisher);

    broker.shutdown().await;
    assert_eq!(
        db.rows("mqtt_outbound").await,
        0,
        "everything was acknowledged"
    );
    assert_eq!(db.rows("mqtt_inbound_qos2").await, 0);
    assert_eq!(db.rows("mqtt_session").await, 2);
}

#[tokio::test]
async fn an_mqtt5_session_with_a_session_expiry_survives_a_restart() {
    let Some(db) = TestDb::create("an_mqtt5_session_survives_a_restart").await else {
        return;
    };
    let broker = db.broker(BrokerLimits::default()).await;
    let a = addr(&broker);

    let (mut keeper, body) = Raw::connected5_session(&a, "keeper5", false, 3600, &[]).await;
    assert_eq!(body[0], 0, "no session yet");
    assert_eq!(keeper.subscribe5(1, &[("t1", 1), ("t2", 2)]).await, [1, 2]);
    disconnect(keeper, &broker, 0).await;

    let (mut publisher, _) = Raw::connected5(&a, "pub5").await;
    publisher.send(&publish5_with_id("t1", b"one", 1, 1)).await;
    assert_eq!(publisher.read_ack(PUBACK).await, (1, None));
    publisher.send(&publish5_with_id("t2", b"two", 2, 2)).await;
    assert_eq!(publisher.read_ack(PUBREC).await, (2, None));
    publisher.send(&ack(PUBREL, 2)).await;
    assert_eq!(publisher.read_ack(PUBCOMP).await, (2, None));
    drop(publisher);
    broker.shutdown().await;
    assert_eq!(db.rows("mqtt_session").await, 1);
    assert_eq!(db.rows("mqtt_outbound").await, 2);
    let protocol: i16 = sqlx::query_scalar("SELECT protocol FROM mqtt_session")
        .fetch_one(&db.pool().await)
        .await
        .unwrap();
    assert_eq!(protocol, 5);

    let broker = db.broker(BrokerLimits::default()).await;
    let a = addr(&broker);
    assert_eq!(broker.subscription_count(), 2);
    let (mut keeper, body) = Raw::connected5_session(&a, "keeper5", false, 3600, &[]).await;
    assert_eq!(body[0], 1, "session present after the restart");
    let one = keeper.read_any_publish(true).await;
    assert_eq!(
        (one.topic.as_str(), one.payload.as_slice(), one.qos, one.dup),
        ("t1", &b"one"[..], 1, true)
    );
    keeper.send(&ack(PUBACK, one.packet_id)).await;
    let two = keeper.read_any_publish(true).await;
    assert_eq!(
        (two.topic.as_str(), two.payload.as_slice(), two.qos, two.dup),
        ("t2", &b"two"[..], 2, true)
    );
    keeper.send(&ack(PUBREC, two.packet_id)).await;
    assert_eq!(keeper.read_ack(PUBREL).await.0, two.packet_id);
    keeper.send(&ack(PUBCOMP, two.packet_id)).await;
    assert!(keeper.is_silent_for(SHORT).await);
    sync(&mut keeper).await;
    drop(keeper);
    broker.shutdown().await;
    assert_eq!(db.rows("mqtt_outbound").await, 0);
}

#[tokio::test]
async fn a_qos2_message_after_its_pubrec_is_not_sent_again_after_a_restart() {
    let Some(db) = TestDb::create("a_qos2_message_after_its_pubrec").await else {
        return;
    };
    let broker = db.broker(BrokerLimits::default()).await;
    let a = addr(&broker);
    let (mut keeper, _) = Raw::connected_session(&a, "keeper", false).await;
    keeper.send(&subscribe_qos(1, &[("t", 2)])).await;
    keeper.read_packet().await.unwrap();
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    publisher
        .send(&publish_with_id("t", b"x", 2, 1, false))
        .await;
    publisher.read_ack(PUBREC).await;
    let got = keeper.read_any_publish(false).await;
    // the subscriber answers with PUBREC and the connection breaks before PUBCOMP
    keeper.send(&ack(PUBREC, got.packet_id)).await;
    assert_eq!(keeper.read_ack(PUBREL).await.0, got.packet_id);
    sync(&mut keeper).await;
    disconnect(keeper, &broker, 1).await; // the publisher stays
    drop(publisher);
    broker.shutdown().await;
    let pubrec: bool = sqlx::query_scalar("SELECT pubrec_received FROM mqtt_outbound")
        .fetch_one(&db.pool().await)
        .await
        .unwrap();
    assert!(pubrec);

    let broker = db.broker(BrokerLimits::default()).await;
    let (mut keeper, present) = Raw::connected_session(&addr(&broker), "keeper", false).await;
    assert!(present);
    // PUBREL first: the PUBLISH must not be sent again
    assert_eq!(keeper.read_ack(PUBREL).await, (got.packet_id, None));
    keeper.send(&ack(PUBCOMP, got.packet_id)).await;
    assert!(keeper.is_silent_for(SHORT).await);
    sync(&mut keeper).await;
    drop(keeper);
    broker.shutdown().await;
    assert_eq!(db.rows("mqtt_outbound").await, 0);
}

#[tokio::test]
async fn clean_sessions_leave_no_rows() {
    let Some(db) = TestDb::create("clean_sessions_leave_no_rows").await else {
        return;
    };
    let broker = db.broker(BrokerLimits::default()).await;
    let a = addr(&broker);
    // 3.1.1 with clean session, and MQTT 5 with Session Expiry 0
    let (mut c311, _) = Raw::connected_session(&a, "clean311", true).await;
    c311.send(&subscribe_qos(1, &[("t", 2)])).await;
    c311.read_packet().await.unwrap();
    let (mut c5, _) = Raw::connected5_session(&a, "clean5", true, 0, &[]).await;
    assert_eq!(c5.subscribe5(1, &[("t", 2)]).await, [2]);
    let (mut publisher, _) = Raw::connected_session(&a, "pub", true).await;
    publisher
        .send(&publish_with_id("t", b"x", 2, 1, false))
        .await;
    publisher.read_ack(PUBREC).await;
    // the tables exist (the broker created them) and stay empty while the clients are connected
    for table in [
        "mqtt_session",
        "mqtt_subscription",
        "mqtt_outbound",
        "mqtt_inbound_qos2",
    ] {
        assert_eq!(db.rows(table).await, 0, "{table}");
    }
    c311.read_any_publish(false).await;
    c5.read_any_publish(true).await;
    disconnect(c311, &broker, 2).await; // c5 and the publisher stay
    drop(c5);
    drop(publisher);
    broker.shutdown().await;
    for table in [
        "mqtt_session",
        "mqtt_subscription",
        "mqtt_outbound",
        "mqtt_inbound_qos2",
    ] {
        assert_eq!(db.rows(table).await, 0, "{table}");
    }

    // a persistent session that is replaced by a clean start disappears from the database as well
    let broker = db.broker(BrokerLimits::default()).await;
    let a = addr(&broker);
    let (mut c, _) = Raw::connected_session(&a, "again", false).await;
    c.send(&subscribe_qos(1, &[("t", 1)])).await;
    c.read_packet().await.unwrap();
    disconnect(c, &broker, 0).await;
    assert_eq!(db.rows("mqtt_session").await, 1);
    let (c, present) = Raw::connected_session(&a, "again", true).await;
    assert!(!present);
    drop(c);
    eventually("the end of the connection", || {
        broker.connection_count() == 0
    })
    .await;
    broker.shutdown().await;
    for table in ["mqtt_session", "mqtt_subscription"] {
        assert_eq!(db.rows(table).await, 0, "{table}");
    }
}

#[tokio::test]
async fn a_session_that_expired_while_the_broker_was_down_is_gone_at_the_next_start() {
    let Some(db) = TestDb::create("a_session_that_expired_while_the_broker_was_down").await else {
        return;
    };
    let limits = BrokerLimits {
        session_expiry_max_s: 3,
        ..BrokerLimits::default()
    };
    let broker = db.broker(limits.clone()).await;
    let a = addr(&broker);
    let (mut c, _) = Raw::connected_session(&a, "short", false).await;
    c.send(&subscribe_qos(1, &[("t", 1)])).await;
    c.read_packet().await.unwrap();
    disconnect(c, &broker, 0).await;
    broker.shutdown().await;
    assert_eq!(db.rows("mqtt_session").await, 1);

    // the expiry is 3 s: the broker is stopped well before the session expires, the next start comes after
    tokio::time::sleep(Duration::from_millis(3_400)).await;
    let broker = db.broker(limits).await;
    assert_eq!(broker.subscription_count(), 0);
    assert_eq!(db.rows("mqtt_session").await, 0, "removed at the start");
    let (c, present) = Raw::connected_session(&addr(&broker), "short", false).await;
    assert!(!present);
    drop(c);
    broker.shutdown().await;
}

#[tokio::test]
async fn a_session_that_was_connected_when_the_broker_stopped_starts_its_expiry_at_the_next_start()
{
    let Some(db) = TestDb::create("a_session_that_was_connected_when_the_broker_stopped").await
    else {
        return;
    };
    let broker = db.broker(BrokerLimits::default()).await;
    let (mut c, _) = Raw::connected_session(&addr(&broker), "cut-off", false).await;
    c.send(&subscribe_qos(1, &[("t", 1)])).await;
    c.read_packet().await.unwrap();
    // the row says "connected" (no disconnect time) while the client is there
    let marked: Option<i64> = sqlx::query_scalar("SELECT disconnected_at_ms FROM mqtt_session")
        .fetch_one(&db.pool().await)
        .await
        .unwrap();
    assert_eq!(marked, None);
    broker.shutdown().await;
    drop(c);
    // simulate a crash instead of a clean stop: the row still says "connected"
    sqlx::query("UPDATE mqtt_session SET disconnected_at_ms = NULL")
        .execute(&db.pool().await)
        .await
        .unwrap();
    let broker = db.broker(BrokerLimits::default()).await;
    let stamped: Option<i64> = sqlx::query_scalar("SELECT disconnected_at_ms FROM mqtt_session")
        .fetch_one(&db.pool().await)
        .await
        .unwrap();
    assert!(stamped.is_some(), "stamped with the start time");
    assert_eq!(broker.subscription_count(), 1);
    broker.shutdown().await;
}

// The tests below need no database.

#[tokio::test]
async fn an_unreachable_session_database_stops_the_start_without_telling_the_password() {
    let result = Broker::start_with(
        0,
        BrokerLimits::default(),
        BrokerOptions {
            sessions: Some(SessionDatabase {
                url: "postgres://dt:secret-pw@127.0.0.1:1/db".to_owned(),
                schema: None,
            }),
            ..BrokerOptions::default()
        },
    )
    .await;
    let Err(error) = result else {
        panic!("nothing listens on port 1");
    };
    let shown = format!("{error} {error:?}");
    assert!(shown.contains("session database"), "{shown}");
    assert!(
        !shown.contains("secret-pw") && !shown.contains("127.0.0.1"),
        "{shown}"
    );
}

#[tokio::test]
async fn an_invalid_session_database_url_stops_the_start_without_echoing_it() {
    let result = Broker::start_with(
        0,
        BrokerLimits::default(),
        BrokerOptions {
            sessions: Some(SessionDatabase {
                url: "this is not a url secret-pw".to_owned(),
                schema: None,
            }),
            ..BrokerOptions::default()
        },
    )
    .await;
    let Err(error) = result else {
        panic!("the URL is invalid");
    };
    let shown = format!("{error} {error:?}");
    assert!(shown.contains("not a valid"), "{shown}");
    assert!(!shown.contains("secret-pw"), "{shown}");
}
