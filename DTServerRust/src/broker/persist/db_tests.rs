//! The session store against a real PostgreSQL. Runs only when `DT_TEST_DATABASE_URL` is set (a database in which
//! the test user may create schemas); otherwise every test prints a "SKIPPED" line and passes.
//!
//! Each test uses its own schema (`dt_test_<uuid>`, selected through the `search_path`), which is dropped at the
//! end, also when the test panics. The URL is never printed. The ops are tested here and not in `tests/` because
//! `Op` and `SessionStore` are internal; the whole broker is tested in `tests/session_db.rs`.

use std::str::FromStr;
use std::sync::atomic::{AtomicBool, Ordering};

use sqlx::postgres::PgConnectOptions;
use uuid::Uuid;

use super::*;

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

    fn database(&self) -> SessionDatabase {
        SessionDatabase {
            url: self.url.clone(),
            schema: Some(self.schema.clone()),
        }
    }

    /// A connection pool of the test itself (another connection than the ones of the store).
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

    async fn open_at(
        &self,
        now_ms: i64,
        limits: &BrokerLimits,
    ) -> (Arc<PgSessionStore>, Vec<SessionSnapshot>) {
        // The advisory lock of a store that was shut down a moment ago goes with its connection, and the server
        // may need a few milliseconds to notice that it is closed: try again for a short while.
        let mut attempts = 0;
        loop {
            match PgSessionStore::connect(&self.database(), now_ms, limits).await {
                Ok(opened) => return opened,
                Err(SessionStoreError::InUse) if attempts < 50 => {
                    attempts += 1;
                    tokio::time::sleep(Duration::from_millis(100)).await;
                }
                Err(e) => panic!("the store opens: {e}"),
            }
        }
    }

    async fn open(&self) -> Arc<PgSessionStore> {
        self.open_at(0, &BrokerLimits::default()).await.0
    }
}

impl Drop for TestDb {
    fn drop(&mut self) {
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

/// Waits until everything submitted so far is stored.
async fn flush(store: &PgSessionStore) {
    let (tx, rx) = oneshot::channel();
    store.barrier(Box::new(move || {
        let _ = tx.send(());
    }));
    tokio::time::timeout(Duration::from_secs(10), rx)
        .await
        .expect("the barrier ran in time")
        .expect("the barrier was not dropped");
}

async fn count(pool: &PgPool, table: &str, session: i64) -> i64 {
    sqlx::query_scalar(sqlx::AssertSqlSafe(format!(
        "SELECT count(*) FROM {table} WHERE session_id = $1"
    )))
    .bind(session)
    .fetch_one(pool)
    .await
    .unwrap()
}

fn entry(
    seq: u64,
    packet_id: u16,
    qos: QoS,
    pubrec: bool,
    origin: Origin,
    payload: &[u8],
) -> StoredEntry {
    StoredEntry {
        seq,
        packet_id,
        qos,
        pubrec_received: pubrec,
        origin,
        topic: Arc::from(format!("t/{seq}")),
        payload: Bytes::copy_from_slice(payload),
        received_at_ms: 1_000 + seq as i64,
    }
}

fn session(id: SessionId, client_id: &str) -> SessionSnapshot {
    SessionSnapshot {
        id,
        client_id: client_id.to_owned(),
        username: Some("user".to_owned()),
        version: ProtocolVersion::V5,
        expiry_s: 100,
        disconnected_at_ms: None,
        subscriptions: vec![
            ("a/#".to_owned(), QoS::AtLeastOnce, false),
            ("b".to_owned(), QoS::ExactlyOnce, true),
        ],
        outbound: vec![
            entry(1, 1, QoS::AtLeastOnce, false, Origin::Client, b"one"),
            entry(2, 2, QoS::ExactlyOnce, true, Origin::Server, &[0, 255, 0]),
        ],
        inbound_qos2: vec![7],
    }
}

#[tokio::test]
async fn every_op_kind_is_applied_and_read_back() {
    let Some(db) = TestDb::create("every_op_kind_is_applied_and_read_back").await else {
        return;
    };
    let check = db.pool().await;
    let store = db.open().await;
    let full = session(1, "c1");

    store.submit(Op::PutSession(full.clone()));
    flush(&store).await;
    let row: (i64, String, Option<String>, i16, i64, Option<i64>) = sqlx::query_as(
        "SELECT session_id, client_id, username, protocol, expiry_s, disconnected_at_ms FROM mqtt_session",
    )
    .fetch_one(&check)
    .await
    .unwrap();
    assert_eq!(row, (1, "c1".into(), Some("user".into()), 5, 100, None));
    assert_eq!(count(&check, "mqtt_subscription", 1).await, 2);
    assert_eq!(count(&check, "mqtt_outbound", 1).await, 2);
    assert_eq!(count(&check, "mqtt_inbound_qos2", 1).await, 1);
    let out: (i32, i16, bool, i16, String, Vec<u8>, i64) = sqlx::query_as(
        "SELECT packet_id, qos, pubrec_received, origin, topic, payload, received_at_ms \
         FROM mqtt_outbound WHERE seq = 2",
    )
    .fetch_one(&check)
    .await
    .unwrap();
    assert_eq!(out, (2, 2, true, 1, "t/2".into(), vec![0, 255, 0], 1_002));

    store.submit(Op::UpdateSession {
        id: 1,
        expiry_s: 50,
        disconnected_at_ms: Some(123),
    });
    store.submit(Op::SetSubscription {
        id: 1,
        filter: "a/#".into(),
        qos: QoS::ExactlyOnce,
        no_local: true,
    });
    store.submit(Op::SetSubscription {
        id: 1,
        filter: "c".into(),
        qos: QoS::AtMostOnce,
        no_local: false,
    });
    store.submit(Op::RemoveSubscription {
        id: 1,
        filter: "b".into(),
    });
    store.submit(Op::Enqueue {
        id: 1,
        entry: entry(3, 3, QoS::AtLeastOnce, false, Origin::Client, b"three"),
    });
    store.submit(Op::OutboundPubrec { id: 1, seq: 1 });
    store.submit(Op::DeleteOutbound { id: 1, seq: 2 });
    store.submit(Op::AddInbound {
        id: 1,
        packet_id: 8,
    });
    store.submit(Op::RemoveInbound {
        id: 1,
        packet_id: 7,
    });
    flush(&store).await;

    let row: (i64, Option<i64>) =
        sqlx::query_as("SELECT expiry_s, disconnected_at_ms FROM mqtt_session")
            .fetch_one(&check)
            .await
            .unwrap();
    assert_eq!(row, (50, Some(123)));
    let subs: Vec<(String, i16, bool)> =
        sqlx::query_as("SELECT filter, qos, no_local FROM mqtt_subscription ORDER BY filter")
            .fetch_all(&check)
            .await
            .unwrap();
    assert_eq!(
        subs,
        vec![("a/#".to_owned(), 2, true), ("c".to_owned(), 0, false)]
    );
    let queue: Vec<(i64, bool)> =
        sqlx::query_as("SELECT seq, pubrec_received FROM mqtt_outbound ORDER BY seq")
            .fetch_all(&check)
            .await
            .unwrap();
    assert_eq!(queue, vec![(1, true), (3, false)]);
    let inbound: Vec<i32> = sqlx::query_scalar("SELECT packet_id FROM mqtt_inbound_qos2")
        .fetch_all(&check)
        .await
        .unwrap();
    assert_eq!(inbound, vec![8]);

    // what is stored comes back as the same snapshot (the broker stamps the connect time)
    store.shutdown(Duration::from_secs(5)).await;
    let (store, loaded) = db.open_at(500, &BrokerLimits::default()).await;
    assert_eq!(loaded.len(), 1);
    let loaded = &loaded[0];
    assert_eq!(
        (
            loaded.id,
            loaded.client_id.as_str(),
            loaded.version,
            loaded.expiry_s
        ),
        (1, "c1", ProtocolVersion::V5, 50)
    );
    assert_eq!(loaded.disconnected_at_ms, Some(123));
    assert_eq!(
        loaded.subscriptions,
        vec![
            ("a/#".to_owned(), QoS::ExactlyOnce, true),
            ("c".to_owned(), QoS::AtMostOnce, false)
        ]
    );
    let seqs: Vec<u64> = loaded.outbound.iter().map(|e| e.seq).collect();
    assert_eq!(seqs, vec![1, 3]);
    assert!(loaded.outbound[0].pubrec_received);
    assert_eq!(&loaded.outbound[1].payload[..], b"three");
    assert_eq!(loaded.inbound_qos2, vec![8]);

    store.submit(Op::DeleteSession { id: 1 });
    flush(&store).await;
    for table in [
        "mqtt_session",
        "mqtt_subscription",
        "mqtt_outbound",
        "mqtt_inbound_qos2",
    ] {
        let rows: i64 =
            sqlx::query_scalar(sqlx::AssertSqlSafe(format!("SELECT count(*) FROM {table}")))
                .fetch_one(&check)
                .await
                .unwrap();
        assert_eq!(rows, 0, "{table} (the children go with the session)");
    }
    store.shutdown(Duration::from_secs(5)).await;
}

#[tokio::test]
async fn ops_for_a_missing_session_are_harmless() {
    let Some(db) = TestDb::create("ops_for_a_missing_session_are_harmless").await else {
        return;
    };
    let check = db.pool().await;
    let store = db.open().await;
    // the batch must succeed as a whole: no foreign key violation, no error from an update that matches nothing
    let gone = 99;
    let ops = vec![
        Op::UpdateSession {
            id: gone,
            expiry_s: 1,
            disconnected_at_ms: Some(1),
        },
        Op::SetSubscription {
            id: gone,
            filter: "f".into(),
            qos: QoS::AtLeastOnce,
            no_local: false,
        },
        Op::RemoveSubscription {
            id: gone,
            filter: "f".into(),
        },
        Op::Enqueue {
            id: gone,
            entry: entry(1, 1, QoS::AtLeastOnce, false, Origin::Client, b"x"),
        },
        Op::OutboundPubrec { id: gone, seq: 1 },
        Op::DeleteOutbound { id: gone, seq: 1 },
        Op::AddInbound {
            id: gone,
            packet_id: 1,
        },
        Op::RemoveInbound {
            id: gone,
            packet_id: 1,
        },
        Op::DeleteSession { id: gone },
    ];
    run_batch(&check, &ops)
        .await
        .expect("ops for a missing session do not fail");
    for table in [
        "mqtt_session",
        "mqtt_subscription",
        "mqtt_outbound",
        "mqtt_inbound_qos2",
    ] {
        let rows: i64 =
            sqlx::query_scalar(sqlx::AssertSqlSafe(format!("SELECT count(*) FROM {table}")))
                .fetch_one(&check)
                .await
                .unwrap();
        assert_eq!(rows, 0, "{table}");
    }
    // the same through the store, with a real session in the same batch
    for op in ops {
        store.submit(op);
    }
    store.submit(Op::PutSession(session(1, "c1")));
    store.submit(Op::DeleteOutbound { id: 1, seq: 77 });
    flush(&store).await;
    assert_eq!(count(&check, "mqtt_session", 1).await, 1);
    assert_eq!(count(&check, "mqtt_outbound", 1).await, 2);
    // a dropped Enqueue followed by its DeleteOutbound is harmless: the delete finds nothing
    store.submit(Op::DeleteOutbound { id: 1, seq: 5 });
    flush(&store).await;
    assert_eq!(count(&check, "mqtt_outbound", 1).await, 2);
    store.shutdown(Duration::from_secs(5)).await;
}

#[tokio::test]
async fn a_put_session_replaces_an_older_session_with_the_same_client_id_or_id() {
    let Some(db) = TestDb::create("a_put_session_replaces_an_older_session").await else {
        return;
    };
    let check = db.pool().await;
    let store = db.open().await;
    store.submit(Op::PutSession(session(1, "same")));
    store.submit(Op::PutSession(session(2, "other")));
    flush(&store).await;
    // same client id, new session id: the old session and its children are gone
    let mut newer = session(3, "same");
    newer.subscriptions = vec![("x".to_owned(), QoS::AtMostOnce, false)];
    newer.outbound.clear();
    newer.inbound_qos2.clear();
    store.submit(Op::PutSession(newer));
    flush(&store).await;
    assert_eq!(count(&check, "mqtt_session", 1).await, 0);
    assert_eq!(count(&check, "mqtt_subscription", 1).await, 0);
    assert_eq!(count(&check, "mqtt_outbound", 1).await, 0);
    assert_eq!(count(&check, "mqtt_subscription", 3).await, 1);
    assert_eq!(count(&check, "mqtt_outbound", 3).await, 0);
    assert_eq!(count(&check, "mqtt_session", 2).await, 1);
    // same session id, new client id
    let mut again = session(2, "renamed");
    again.subscriptions.truncate(1);
    store.submit(Op::PutSession(again));
    flush(&store).await;
    let name: String =
        sqlx::query_scalar("SELECT client_id FROM mqtt_session WHERE session_id = 2")
            .fetch_one(&check)
            .await
            .unwrap();
    assert_eq!(name, "renamed");
    assert_eq!(count(&check, "mqtt_subscription", 2).await, 1);
    store.shutdown(Duration::from_secs(5)).await;
}

#[tokio::test]
async fn barriers_run_in_order_and_only_after_the_earlier_ops_are_visible() {
    let Some(db) = TestDb::create("barriers_run_in_order").await else {
        return;
    };
    let check = db.pool().await;
    let store = db.open().await;
    let (tx, mut rx) = mpsc::unbounded_channel();
    // more than one batch (BATCH_SIZE items)
    let n = 400i64;
    for i in 1..=n {
        store.submit(Op::PutSession(SessionSnapshot {
            subscriptions: vec![],
            outbound: vec![],
            inbound_qos2: vec![],
            ..session(i as SessionId, &format!("c{i}"))
        }));
        let tx = tx.clone();
        store.barrier(Box::new(move || {
            let _ = tx.send(i);
        }));
    }
    for expected in 1..=n {
        let got = tokio::time::timeout(Duration::from_secs(20), rx.recv())
            .await
            .expect("barrier in time")
            .unwrap();
        assert_eq!(
            got, expected,
            "barriers run in the order of their submission"
        );
        // another connection sees the session: it is committed
        assert_eq!(
            count(&check, "mqtt_session", expected).await,
            1,
            "session {expected}"
        );
    }
    // a barrier without ops runs as well
    flush(&store).await;
    store.shutdown(Duration::from_secs(5)).await;
}

#[tokio::test]
async fn connect_loads_what_was_stored_drops_expired_sessions_and_stamps_the_rest() {
    let Some(db) = TestDb::create("connect_loads_expires_and_stamps").await else {
        return;
    };
    let check = db.pool().await;
    let limits = BrokerLimits {
        session_expiry_max_s: 100,
        ..BrokerLimits::default()
    };
    let now = 100_000;
    let store = db.open().await;
    let with = |id, name: &str, expiry_s, disconnected_at_ms| SessionSnapshot {
        expiry_s,
        disconnected_at_ms,
        ..session(id, name)
    };
    store.submit(Op::PutSession(with(1, "connected", 100, None)));
    // expired: 1_000 + 10 s <= now
    store.submit(Op::PutSession(with(2, "expired", 10, Some(1_000))));
    // not expired: 99_000 + 100 s > now; its queue is stored out of order and has to come back ordered
    let mut offline = with(3, "offline", 100, Some(99_000));
    offline.outbound = vec![
        entry(5, 5, QoS::AtLeastOnce, false, Origin::Client, b"b"),
        entry(2, 2, QoS::ExactlyOnce, true, Origin::Server, b"a"),
    ];
    store.submit(Op::PutSession(offline));
    // stored with a longer expiry than the limit allows: counted with the limit, so exactly due now
    store.submit(Op::PutSession(with(4, "capped", 1_000_000, Some(0))));
    flush(&store).await;
    store.shutdown(Duration::from_secs(5)).await;

    let (store, loaded) = db.open_at(now, &limits).await;
    let ids: Vec<SessionId> = loaded.iter().map(|s| s.id).collect();
    assert_eq!(ids, vec![1, 3]);
    assert_eq!(
        loaded[0].disconnected_at_ms,
        Some(now),
        "stamped at the start"
    );
    assert_eq!(loaded[1].disconnected_at_ms, Some(99_000));
    assert_eq!(loaded[1].subscriptions.len(), 2);
    let seqs: Vec<u64> = loaded[1].outbound.iter().map(|e| e.seq).collect();
    assert_eq!(seqs, vec![2, 5], "in queue order");
    assert_eq!(loaded[1].inbound_qos2, vec![7]);
    assert_eq!(count(&check, "mqtt_session", 2).await, 0);
    assert_eq!(count(&check, "mqtt_subscription", 2).await, 0);
    let stamped: Option<i64> =
        sqlx::query_scalar("SELECT disconnected_at_ms FROM mqtt_session WHERE session_id = 1")
            .fetch_one(&check)
            .await
            .unwrap();
    assert_eq!(stamped, Some(now));
    store.shutdown(Duration::from_secs(5)).await;

    // opening again is idempotent (the schema exists) and finds the same two
    let (store, loaded) = db.open_at(now, &limits).await;
    assert_eq!(loaded.len(), 2);
    store.shutdown(Duration::from_secs(5)).await;
}

#[tokio::test]
async fn rows_that_cannot_be_understood_do_not_keep_the_store_from_starting() {
    let Some(db) = TestDb::create("rows_that_cannot_be_understood").await else {
        return;
    };
    let check = db.pool().await;
    let store = db.open().await;
    store.submit(Op::PutSession(session(1, "good")));
    store.submit(Op::PutSession(session(2, "bad-protocol")));
    flush(&store).await;
    store.shutdown(Duration::from_secs(5)).await;
    sqlx::query("UPDATE mqtt_session SET protocol = 9 WHERE session_id = 2")
        .execute(&check)
        .await
        .unwrap();
    sqlx::query("UPDATE mqtt_outbound SET qos = 7 WHERE session_id = 1 AND seq = 1")
        .execute(&check)
        .await
        .unwrap();
    sqlx::query("UPDATE mqtt_subscription SET qos = 5 WHERE session_id = 1 AND filter = 'b'")
        .execute(&check)
        .await
        .unwrap();
    let (store, loaded) = db.open_at(0, &BrokerLimits::default()).await;
    assert_eq!(loaded.len(), 1);
    assert_eq!(loaded[0].subscriptions.len(), 1);
    assert_eq!(loaded[0].outbound.len(), 1);
    store.shutdown(Duration::from_secs(5)).await;
}

#[tokio::test]
async fn a_change_the_database_refuses_is_skipped_and_the_rest_is_applied() {
    let Some(db) = TestDb::create("a_change_the_database_refuses_is_skipped").await else {
        return;
    };
    let check = db.pool().await;
    let store = db.open().await;
    // a Writer of the test, so that the ops are guaranteed to form one batch
    let (_tx, rx) = mpsc::unbounded_channel();
    let (_abort, abort_rx) = watch::channel(false);
    let mut writer = Writer {
        pool: db.pool().await,
        rx,
        backlog: Arc::new(Backlog::new(10, 10)),
        abort: abort_rx,
        failing: false,
        last_warned: None,
    };
    let mut bad = entry(1, 1, QoS::AtLeastOnce, false, Origin::Client, b"x");
    // PostgreSQL text cannot hold a NUL: the database refuses this one (SQLSTATE 22021)
    bad.topic = Arc::from("a\0b");
    let ops = vec![
        Op::PutSession(SessionSnapshot {
            outbound: vec![],
            ..session(1, "c1")
        }),
        Op::Enqueue { id: 1, entry: bad },
        Op::Enqueue {
            id: 1,
            entry: entry(2, 2, QoS::AtLeastOnce, false, Origin::Client, b"fine"),
        },
        Op::PutSession(SessionSnapshot {
            outbound: vec![],
            ..session(2, "c2")
        }),
    ];
    assert!(
        run_batch(&check, &ops).await.is_err(),
        "the batch as a whole is refused"
    );
    assert_eq!(count(&check, "mqtt_session", 1).await, 0, "and rolled back");
    assert!(writer.apply(&ops).await.is_ok());
    assert_eq!(count(&check, "mqtt_session", 1).await, 1);
    assert_eq!(count(&check, "mqtt_session", 2).await, 1);
    let seqs: Vec<i64> = sqlx::query_scalar("SELECT seq FROM mqtt_outbound")
        .fetch_all(&check)
        .await
        .unwrap();
    assert_eq!(seqs, vec![2]);
    assert!(
        !writer.failing,
        "a refused change is not a connection problem"
    );

    // through the store: the barrier behind the bad op still runs
    store.submit(Op::Enqueue {
        id: 1,
        entry: StoredEntry {
            topic: Arc::from("x\0y"),
            ..entry(9, 9, QoS::AtLeastOnce, false, Origin::Client, b"x")
        },
    });
    store.submit(Op::PutSession(session(3, "c3")));
    flush(&store).await;
    assert_eq!(count(&check, "mqtt_session", 3).await, 1);
    store.shutdown(Duration::from_secs(5)).await;
}

#[tokio::test]
async fn shutdown_stores_what_was_submitted_and_the_store_then_does_nothing() {
    let Some(db) = TestDb::create("shutdown_stores_what_was_submitted").await else {
        return;
    };
    let check = db.pool().await;
    let store = db.open().await;
    for id in 1..=50 {
        store.submit(Op::PutSession(session(id, &format!("c{id}"))));
    }
    store.shutdown(Duration::from_secs(10)).await;
    let rows: i64 = sqlx::query_scalar("SELECT count(*) FROM mqtt_session")
        .fetch_one(&check)
        .await
        .unwrap();
    assert_eq!(rows, 50);
    assert_eq!(store.queued(), 0);

    // afterwards nothing is accepted: no op, and a barrier is dropped unrun
    let ran = Arc::new(AtomicBool::new(false));
    let flag = Arc::clone(&ran);
    store.submit(Op::DeleteSession { id: 1 });
    store.barrier(Box::new(move || flag.store(true, Ordering::SeqCst)));
    tokio::time::sleep(Duration::from_millis(300)).await;
    assert!(!ran.load(Ordering::SeqCst));
    assert_eq!(count(&check, "mqtt_session", 1).await, 1);
    // and a second shutdown returns at once
    store.shutdown(Duration::from_secs(1)).await;
}

#[tokio::test]
async fn only_one_broker_can_use_a_schema_at_a_time() {
    let Some(db) = TestDb::create("only_one_broker_can_use_a_schema_at_a_time").await else {
        return;
    };
    let other = TestDb::create("only_one_broker_can_use_a_schema_at_a_time (other schema)")
        .await
        .unwrap();
    let first = db.open().await;
    match PgSessionStore::connect(&db.database(), 0, &BrokerLimits::default()).await {
        Err(SessionStoreError::InUse) => {}
        Err(other) => panic!("wrong error: {other}"),
        Ok(_) => panic!("a second store got the same schema"),
    }
    // the failed attempt left nothing behind that blocks the first one
    flush(&first).await;
    // another schema is another database for the broker
    let elsewhere = other.open().await;
    elsewhere.shutdown(Duration::from_secs(5)).await;
    // the lock goes with the store
    first.shutdown(Duration::from_secs(5)).await;
    let again = db.open().await;
    again.shutdown(Duration::from_secs(5)).await;
}

#[tokio::test]
async fn existing_tables_are_not_created_again() {
    let Some(db) = TestDb::create("existing_tables_are_not_created_again").await else {
        return;
    };
    let check = db.pool().await;
    let store = db.open().await;
    store.shutdown(Duration::from_secs(5)).await;
    // with all tables in place the start runs no DDL (so a role without CREATE can start the broker). The DDL
    // path takes the advisory lock `sql::SCHEMA_LOCK` first: while another session holds that lock, a start that
    // would create something blocks, and one that finds everything in place does not.
    assert!(tables_exist(&mut check.acquire().await.unwrap())
        .await
        .unwrap());
    let mut blocker = check.acquire().await.unwrap();
    sqlx::query("SELECT pg_advisory_lock(7364501)")
        .execute(&mut *blocker)
        .await
        .unwrap();
    let store = tokio::time::timeout(Duration::from_secs(10), db.open())
        .await
        .expect("a start that has nothing to create does not wait for the schema lock");
    store.shutdown(Duration::from_secs(5)).await;
    sqlx::query("SELECT pg_advisory_unlock(7364501)")
        .execute(&mut *blocker)
        .await
        .unwrap();
    drop(blocker);
    sqlx::query("DROP TABLE mqtt_inbound_qos2")
        .execute(&check)
        .await
        .unwrap();
    assert!(!tables_exist(&mut check.acquire().await.unwrap())
        .await
        .unwrap());
    // one missing table: the next start creates it (the others stay as they are)
    let store = db.open().await;
    assert!(tables_exist(&mut check.acquire().await.unwrap())
        .await
        .unwrap());
    store.shutdown(Duration::from_secs(5)).await;
}

/// A wrong password is refused with the SQLSTATE only (needs a server that checks passwords and a URL of the
/// form `postgres://user:password@host/db`; otherwise the test says so and passes).
#[tokio::test]
async fn a_refused_login_is_reported_by_its_sqlstate_without_secrets() {
    let Some(db) = TestDb::create("a_refused_login_is_reported_by_its_sqlstate").await else {
        return;
    };
    let Some((scheme_and_user, rest)) = db
        .url
        .split_once("://")
        .and_then(|(scheme, rest)| {
            rest.split_once('@')
                .map(|(userinfo, tail)| (scheme, userinfo, tail))
        })
        .and_then(|(scheme, userinfo, tail)| {
            userinfo
                .split_once(':')
                .map(|(user, _)| (format!("{scheme}://{user}"), tail.to_owned()))
        })
    else {
        eprintln!("SKIPPED a_refused_login: DT_TEST_DATABASE_URL has no user:password@ part");
        return;
    };
    let database = SessionDatabase {
        url: format!("{scheme_and_user}:wrong-secret-pw@{rest}"),
        schema: Some(db.schema.clone()),
    };
    match PgSessionStore::connect(&database, 0, &BrokerLimits::default()).await {
        Err(SessionStoreError::Database(code)) => {
            assert!(code.contains("28P01") || code.contains("28000"), "{code}");
            let shown = format!("{}", SessionStoreError::Database(code));
            assert!(!shown.contains("wrong-secret-pw"), "{shown}");
        }
        Err(other) => panic!("unexpected error: {other}"),
        Ok((store, _)) => {
            // trust authentication: every password is accepted, nothing to check
            store.shutdown(Duration::from_secs(5)).await;
            eprintln!("SKIPPED a_refused_login: the server does not check passwords (trust authentication)");
        }
    }
}
