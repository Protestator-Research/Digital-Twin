//! History against a real PostgreSQL. Runs only when `DT_TEST_DATABASE_URL` is set (a database in which the
//! test user may create schemas); otherwise every test prints a "SKIPPED" line and passes.
//!
//! Each test creates its own uniquely named schema (`dt_test_<uuid>`), runs all statements with that schema
//! as `search_path`, and drops it afterwards (also when the test panics). Nothing else in the database is
//! touched. The URL is never printed.

use std::time::Duration;

use chrono::{DateTime, Duration as ChronoDuration, TimeZone, Utc};
use dt_server::broker::{Broker, BrokerLimits};
use dt_server::history::partition::Month;
use dt_server::history::pg::{parse_options, PgStore};
use dt_server::history::recorder::HistoryStats;
use dt_server::history::registry::TwinRecord;
use dt_server::history::row::{value_type, BatchColumns, Row};
use dt_server::history::store::{ErrorKind, Store};
use dt_server::history::writer::{maintenance_once, run_writer, WriterSettings};
use dt_server::history::{HistoryService, HistorySettings};
use dt_server::payload::time::format_time_point;
use dt_server::twin::LoadedTwin;
use sqlx::postgres::PgPoolOptions;
use sqlx::Row as _;
use uuid::Uuid;

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
            .connect_with(parse_options(&url).expect("valid DT_TEST_DATABASE_URL"))
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

    async fn store(&self) -> PgStore {
        let store = PgStore::connect(parse_options(&self.url).unwrap(), Some(&self.schema))
            .await
            .expect("connect");
        store.migrate().await.expect("migrate");
        store
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
                    .connect_with(parse_options(&url).unwrap())
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

fn ts(y: i32, m: u32, d: u32) -> DateTime<Utc> {
    Utc.with_ymd_and_hms(y, m, d, 12, 0, 0).unwrap()
}

fn cols(entries: &[(&str, DateTime<Utc>)]) -> BatchColumns {
    let mut c = BatchColumns::default();
    for (path, t) in entries {
        c.twin_id.push(Uuid::from_u128(1));
        c.project_id.push(Uuid::from_u128(2));
        c.element_path.push((*path).to_owned());
        c.ts.push(*t);
        c.received_at.push(*t);
        c.value_type.push(value_type::REAL);
        c.value_num.push(Some(1.5));
        c.value_text.push(None);
        c.value_json.push(None);
    }
    c
}

async fn count(store: &PgStore, table: &str) -> i64 {
    sqlx::query(sqlx::AssertSqlSafe(format!("SELECT count(*) FROM {table}")))
        .fetch_one(store.pool())
        .await
        .unwrap()
        .get(0)
}

#[tokio::test]
async fn migrations_create_the_partitioned_schema_and_are_repeatable() {
    let Some(db) = TestDb::create("migrations").await else {
        return;
    };
    let store = db.store().await;
    store.migrate().await.expect("second run is a no-op");
    let partitioned: bool = sqlx::query(
        "SELECT EXISTS (SELECT 1 FROM pg_partitioned_table WHERE partrelid = 'measurement'::regclass)",
    )
    .fetch_one(store.pool())
    .await
    .unwrap()
    .get(0);
    assert!(partitioned);
    assert_eq!(count(&store, "twin").await, 0);
    store.close().await;
}

#[tokio::test]
async fn partitions_are_created_listed_and_dropped() {
    let Some(db) = TestDb::create("partitions").await else {
        return;
    };
    let store = db.store().await;
    let months = [
        Month::new(2026, 12).unwrap(),
        Month::new(2027, 1).unwrap(),
        Month::new(2026, 3).unwrap(),
    ];
    for m in months {
        store.create_partition(m).await.unwrap();
        store.create_partition(m).await.expect("IF NOT EXISTS");
    }
    // a partition with a foreign name is not ours and is left alone
    sqlx::query(
        "CREATE TABLE measurement_foreign PARTITION OF measurement \
         FOR VALUES FROM ('2030-01-01 00:00:00+00') TO ('2030-02-01 00:00:00+00')",
    )
    .execute(store.pool())
    .await
    .unwrap();
    assert_eq!(
        store.list_partitions().await.unwrap(),
        vec![
            Month::new(2026, 3).unwrap(),
            Month::new(2026, 12).unwrap(),
            Month::new(2027, 1).unwrap()
        ]
    );
    store
        .drop_partition(Month::new(2026, 3).unwrap())
        .await
        .unwrap();
    store
        .drop_partition(Month::new(2026, 3).unwrap())
        .await
        .expect("IF EXISTS");
    assert_eq!(store.list_partitions().await.unwrap().len(), 2);
    store.close().await;
}

#[tokio::test]
async fn batch_insert_stores_all_value_types_in_the_right_partitions() {
    let Some(db) = TestDb::create("insert").await else {
        return;
    };
    let store = db.store().await;
    store
        .create_partition(Month::new(2026, 11).unwrap())
        .await
        .unwrap();
    store
        .create_partition(Month::new(2026, 12).unwrap())
        .await
        .unwrap();

    let mut c = cols(&[
        ("real", ts(2026, 11, 30)),
        ("int", ts(2026, 12, 1)),
        ("rat", ts(2026, 11, 2)),
        ("cpx", ts(2026, 12, 2)),
    ]);
    c.value_type = vec![
        value_type::REAL,
        value_type::INTEGER,
        value_type::RATIONAL,
        value_type::COMPLEX,
    ];
    c.value_num = vec![Some(1.5), Some(9007199254740992.0), None, None];
    c.value_text = vec![
        None,
        Some("9007199254740993".into()),
        Some("3/4".into()),
        None,
    ];
    c.value_json = vec![
        None,
        None,
        None,
        Some(r#"{"imaginary_value":-2.5,"real_value":1.0}"#.into()),
    ];
    store
        .insert_batch(&c)
        .await
        .expect("one statement for the whole batch");

    assert_eq!(count(&store, "measurement").await, 4);
    let rows = sqlx::query(
        "SELECT element_path, tableoid::regclass::text AS part, value_type, value_num, value_text, \
                value_json->>'real_value' AS re FROM measurement ORDER BY element_path",
    )
    .fetch_all(store.pool())
    .await
    .unwrap();
    let get = |i: usize| {
        let r = &rows[i];
        (
            r.get::<String, _>("element_path"),
            r.get::<String, _>("part"),
            r.get::<i16, _>("value_type"),
            r.get::<Option<f64>, _>("value_num"),
            r.get::<Option<String>, _>("value_text"),
            r.get::<Option<String>, _>("re"),
        )
    };
    let complex = get(0);
    assert_eq!(
        (complex.0.as_str(), complex.1.as_str(), complex.2),
        ("cpx", "measurement_y2026m12", 5)
    );
    assert_eq!(
        complex.5.as_deref().and_then(|s| s.parse::<f64>().ok()),
        Some(1.0)
    );
    assert_eq!(get(1).1, "measurement_y2026m12");
    assert_eq!(get(1).4.as_deref(), Some("9007199254740993"));
    assert_eq!(get(2).0, "rat");
    assert_eq!(get(2).1, "measurement_y2026m11");
    assert_eq!(get(2).4.as_deref(), Some("3/4"));
    assert_eq!(get(3).3, Some(1.5));
    store.close().await;
}

#[tokio::test]
async fn database_errors_are_classified() {
    let Some(db) = TestDb::create("errors").await else {
        return;
    };
    let store = db.store().await;
    store
        .create_partition(Month::new(2026, 11).unwrap())
        .await
        .unwrap();

    // no partition for the month: 23514
    let err = store
        .insert_batch(&cols(&[("a", ts(2026, 5, 1))]))
        .await
        .expect_err("no partition");
    assert_eq!(err.kind, ErrorKind::MissingPartition, "{err}");
    // and nothing of the batch was inserted (all or nothing)
    let err = store
        .insert_batch(&cols(&[("ok", ts(2026, 11, 1)), ("b", ts(2026, 5, 1))]))
        .await
        .expect_err("no partition");
    assert_eq!(err.kind, ErrorKind::MissingPartition);
    assert_eq!(count(&store, "measurement").await, 0);

    // a NUL character is not valid in PostgreSQL text: a data error, the row can be isolated
    let err = store
        .insert_batch(&cols(&[("bad\0path", ts(2026, 11, 1))]))
        .await
        .expect_err("NUL in text");
    assert_eq!(err.kind, ErrorKind::BadData, "{err}");
    store.close().await;
}

#[tokio::test]
async fn twin_registry_upsert_collision_and_load() {
    let Some(db) = TestDb::create("registry").await else {
        return;
    };
    let store = db.store().await;
    let a = TwinRecord {
        twin_id: Uuid::from_u128(1),
        project_id: Uuid::from_u128(10),
        name: "Pump".into(),
    };
    let b = TwinRecord {
        twin_id: Uuid::from_u128(2),
        project_id: Uuid::from_u128(10),
        name: "Valve".into(),
    };
    store.upsert_twin(&a).await.unwrap();
    store.upsert_twin(&b).await.unwrap();
    let first_seen: DateTime<Utc> = sqlx::query("SELECT first_seen FROM twin WHERE twin_id = $1")
        .bind(a.twin_id)
        .fetch_one(store.pool())
        .await
        .unwrap()
        .get(0);
    tokio::time::sleep(Duration::from_millis(50)).await;
    // update: new project, last_seen moves, first_seen stays
    let a2 = TwinRecord {
        project_id: Uuid::from_u128(11),
        ..a.clone()
    };
    store.upsert_twin(&a2).await.unwrap();
    let row = sqlx::query("SELECT first_seen, last_seen, project_id FROM twin WHERE twin_id = $1")
        .bind(a.twin_id)
        .fetch_one(store.pool())
        .await
        .unwrap();
    assert_eq!(row.get::<DateTime<Utc>, _>("first_seen"), first_seen);
    assert!(row.get::<DateTime<Utc>, _>("last_seen") > first_seen);
    assert_eq!(row.get::<Uuid, _>("project_id"), Uuid::from_u128(11));
    // a name owned by another twin is refused by the unique index
    let thief = TwinRecord {
        twin_id: Uuid::from_u128(3),
        project_id: Uuid::from_u128(10),
        name: "Pump".into(),
    };
    assert_eq!(
        store.upsert_twin(&thief).await.unwrap_err().kind,
        ErrorKind::BadData
    );
    assert_eq!(store.load_twins().await.unwrap(), vec![a2, b]);
    store.close().await;
}

#[tokio::test]
async fn retention_drops_only_old_partitions_in_the_database() {
    let Some(db) = TestDb::create("retention").await else {
        return;
    };
    let store = db.store().await;
    let now = Utc::now();
    let old = Month::of(now - ChronoDuration::days(200));
    let recent = Month::of(now - ChronoDuration::days(5));
    store.create_partition(old).await.unwrap();
    store.create_partition(recent).await.unwrap();
    store
        .insert_batch(&cols(&[("old", old.start() + ChronoDuration::hours(1))]))
        .await
        .unwrap();

    // 0 keeps everything
    let report = maintenance_once(&store, now, 0).await;
    assert!(report.errors.is_empty() && report.dropped.is_empty());
    assert!(store.list_partitions().await.unwrap().contains(&old));

    let report = maintenance_once(&store, now, 30).await;
    assert!(report.errors.is_empty(), "{:?}", report.errors);
    assert_eq!(report.dropped, vec![old]);
    let months = store.list_partitions().await.unwrap();
    assert!(!months.contains(&old));
    assert!(months.contains(&recent));
    assert!(months.contains(&Month::of(now)));
    assert!(months.contains(&Month::of(now).next()));
    assert_eq!(count(&store, "measurement").await, 0);
    store.close().await;
}

fn value_payload(value: f64, at: DateTime<Utc>) -> String {
    format!(
        r#"{{"time_point":"{}","value":{value}}}"#,
        format_time_point(at)
    )
}

/// The whole pipeline: client publishes -> broker -> hub -> recorder -> PostgreSQL, a restart, and an old
/// timestamp whose partition is created on demand.
#[tokio::test]
async fn recorder_end_to_end_and_registry_survives_a_restart() {
    let Some(db) = TestDb::create("end_to_end").await else {
        return;
    };
    let settings = || HistorySettings {
        queue_capacity: 1000,
        batch_size: 100,
        flush_interval: Duration::from_millis(50),
        retention_days: 0,
        maintenance_interval: Duration::from_secs(3600),
        backoff_initial: Duration::from_millis(50),
        backoff_max: Duration::from_millis(500),
        shutdown_flush_limit: Duration::from_secs(5),
        permanent_attempts: 3,
        call_timeout: Duration::from_secs(45),
        registry_queue: 16,
        report_interval: Duration::from_secs(3600),
    };
    let (url, schema) = (db.url.clone(), db.schema.clone());
    let connect = move |_| {
        let (url, schema) = (url.clone(), schema.clone());
        async move {
            let store = PgStore::connect(parse_options(&url).unwrap(), Some(&schema))
                .await
                .ok()?;
            store.migrate().await.ok()?;
            Some(store)
        }
    };
    let verify = db.store().await;

    // --- first run: the twin is registered by the twin logic ---
    let broker = Broker::start(0, BrokerLimits::default()).await.unwrap();
    let service = HistoryService::start_with(broker.hub(), settings(), connect.clone()).unwrap();
    service.observer().twin_loaded(&LoadedTwin {
        twin_id: Uuid::from_u128(5),
        project_id: Uuid::from_u128(6),
        name: "Pump".into(),
    });
    // the server's own initial value must not be recorded
    broker
        .hub()
        .publish("Pump/Motor/temp", value_payload(0.0, Utc::now()))
        .unwrap();
    let now = Utc::now();
    let old = now - ChronoDuration::days(120);
    publish(
        broker.port(),
        &[
            ("Pump/Motor/temp", value_payload(21.5, now)),
            ("Pump/Motor/temp", value_payload(22.5, old)),
            ("Unknown/x", value_payload(1.0, now)),
            ("Pump/Motor/temp", "garbage".to_owned()),
        ],
    )
    .await;
    wait_for_rows(&verify, 2).await;
    broker.shutdown().await;
    service.shutdown().await;
    let stats_rows = sqlx::query(
        "SELECT twin_id, project_id, element_path, value_num FROM measurement ORDER BY ts",
    )
    .fetch_all(verify.pool())
    .await
    .unwrap();
    assert_eq!(
        stats_rows.len(),
        2,
        "initial value, unknown twin and garbage are not recorded"
    );
    assert_eq!(stats_rows[0].get::<f64, _>("value_num"), 22.5);
    assert_eq!(stats_rows[1].get::<Uuid, _>("twin_id"), Uuid::from_u128(5));
    assert_eq!(
        stats_rows[1].get::<Uuid, _>("project_id"),
        Uuid::from_u128(6)
    );
    assert_eq!(stats_rows[1].get::<String, _>("element_path"), "Motor/temp");
    // the old value's month was created on demand
    assert!(verify
        .list_partitions()
        .await
        .unwrap()
        .contains(&Month::of(old)));

    // --- restart: nobody calls twin_loaded, the registry comes from the database ---
    let broker = Broker::start(0, BrokerLimits::default()).await.unwrap();
    let service = HistoryService::start_with(broker.hub(), settings(), connect).unwrap();
    for _ in 0..500 {
        if service.registered_twins() == 1 {
            break;
        }
        tokio::time::sleep(Duration::from_millis(10)).await;
    }
    assert_eq!(service.registered_twins(), 1);
    publish(
        broker.port(),
        &[("Pump/Motor/temp", value_payload(23.5, Utc::now()))],
    )
    .await;
    wait_for_rows(&verify, 3).await;
    broker.shutdown().await;
    service.shutdown().await;
    verify.close().await;
}

async fn wait_for_rows(store: &PgStore, expected: i64) {
    for _ in 0..500 {
        if count(store, "measurement").await >= expected {
            return;
        }
        tokio::time::sleep(Duration::from_millis(20)).await;
    }
    panic!(
        "expected {expected} rows, found {}",
        count(store, "measurement").await
    );
}

async fn publish(port: u16, messages: &[(&str, String)]) {
    use rumqttc::{AsyncClient, MqttOptions, QoS};
    let (client, mut eventloop) =
        AsyncClient::new(MqttOptions::new("db-test", "127.0.0.1", port), 16);
    let poller = tokio::spawn(async move { while eventloop.poll().await.is_ok() {} });
    for (topic, payload) in messages {
        client
            .publish(*topic, QoS::AtMostOnce, false, payload.clone())
            .await
            .unwrap();
    }
    tokio::time::sleep(Duration::from_millis(300)).await;
    client.disconnect().await.ok();
    poller.abort();
}

#[tokio::test]
async fn concurrent_partition_creation_is_not_an_error() {
    let Some(db) = TestDb::create("concurrent_ddl").await else {
        return;
    };
    // two stores = two pools = two independent sessions, like two servers on the same database
    let a = std::sync::Arc::new(db.store().await);
    let b = std::sync::Arc::new(db.store().await);
    let month = Month::new(2031, 7).unwrap();
    let mut tasks = Vec::new();
    for i in 0..16 {
        let store = if i % 2 == 0 { a.clone() } else { b.clone() };
        tasks.push(tokio::spawn(
            async move { store.create_partition(month).await },
        ));
    }
    for task in tasks {
        task.await
            .unwrap()
            .expect("a lost race (42P07 / 23505) is success when the partition is attached");
    }
    assert_eq!(
        a.list_partitions().await.unwrap(),
        vec![month],
        "exactly one partition, attached"
    );
    // the retention drop and a concurrent create do not fail either
    let (drop_result, create_result) =
        tokio::join!(b.drop_partition(month), a.create_partition(month));
    drop_result.unwrap();
    create_result.unwrap();
    a.close().await;
    b.close().await;
}

#[tokio::test]
async fn a_detached_table_of_the_same_name_is_an_error_not_a_stall() {
    let Some(db) = TestDb::create("detached").await else {
        return;
    };
    let store = db.store().await;
    let month = Month::new(2031, 8).unwrap();
    store.create_partition(month).await.unwrap();
    sqlx::query("ALTER TABLE measurement DETACH PARTITION measurement_y2031m08")
        .execute(store.pool())
        .await
        .unwrap();
    // IF NOT EXISTS is a no-op for the detached table: the partition is not attached, so this is a real error
    let err = store.create_partition(month).await;
    // (42P07 is only benign if the table is attached; here CREATE IF NOT EXISTS succeeds silently, so the
    // insert path has to deal with the resulting 23514)
    assert!(err.is_ok() || err.unwrap_err().kind == ErrorKind::Permanent);
    let insert = store
        .insert_batch(&cols(&[("a", ts(2031, 8, 5))]))
        .await
        .unwrap_err();
    assert_eq!(insert.kind, ErrorKind::MissingPartition, "{insert}");

    // the writer must not retry forever: the rows are isolated, dropped and counted, later data is written
    store
        .create_partition(Month::new(2031, 9).unwrap())
        .await
        .unwrap();
    let stats = std::sync::Arc::new(HistoryStats::default());
    let (tx, rx) = tokio::sync::mpsc::channel(10);
    let (_stop_tx, stop_rx) = tokio::sync::watch::channel(false);
    let settings = WriterSettings {
        batch_size: 10,
        flush_interval: Duration::from_millis(20),
        backoff_initial: Duration::from_millis(20),
        backoff_max: Duration::from_millis(100),
        shutdown_flush_limit: Duration::from_secs(2),
        permanent_attempts: 3,
    };
    let store = std::sync::Arc::new(store);
    let writer = tokio::spawn(run_writer(
        store.clone(),
        rx,
        settings,
        stats.clone(),
        stop_rx,
    ));
    let row = |path: &str, t: DateTime<Utc>| Row {
        twin_id: Uuid::from_u128(1),
        project_id: Uuid::from_u128(2),
        element_path: path.to_owned(),
        ts: t,
        received_at: t,
        value_type: value_type::REAL,
        value_num: Some(1.0),
        value_text: None,
        value_json: None,
    };
    tx.send(row("poison", ts(2031, 8, 5))).await.unwrap();
    tx.send(row("fine", ts(2031, 9, 5))).await.unwrap();
    drop(tx);
    tokio::time::timeout(Duration::from_secs(30), writer)
        .await
        .expect("writer ends")
        .unwrap();
    let s = stats.snapshot();
    assert_eq!((s.recorded, s.rejected), (1, 1));
    assert_eq!(count(&store, "measurement").await, 1);
    store.close().await;
}

#[tokio::test]
async fn connections_have_server_side_time_limits() {
    let Some(db) = TestDb::create("timeouts").await else {
        return;
    };
    let store = db.store().await;
    let statement: String = sqlx::query("SHOW statement_timeout")
        .fetch_one(store.pool())
        .await
        .unwrap()
        .get(0);
    let lock: String = sqlx::query("SHOW lock_timeout")
        .fetch_one(store.pool())
        .await
        .unwrap()
        .get(0);
    assert_eq!((statement.as_str(), lock.as_str()), ("30s", "5s"));

    // a partition drop that waits for a lock held elsewhere fails within about lock_timeout, as a transient error
    let month = Month::new(2031, 10).unwrap();
    store.create_partition(month).await.unwrap();
    let mut holder = store.pool().acquire().await.unwrap();
    sqlx::query("BEGIN").execute(&mut *holder).await.unwrap();
    sqlx::query("LOCK TABLE measurement_y2031m10 IN ACCESS EXCLUSIVE MODE")
        .execute(&mut *holder)
        .await
        .unwrap();
    let started = std::time::Instant::now();
    let err = store
        .drop_partition(month)
        .await
        .expect_err("blocked by the lock");
    assert_eq!(err.kind, ErrorKind::Transient, "{err}");
    assert!(started.elapsed() < Duration::from_secs(20));
    sqlx::query("ROLLBACK").execute(&mut *holder).await.unwrap();
    drop(holder);
    store
        .drop_partition(month)
        .await
        .expect("works after the lock is released");
    store.close().await;
}

#[tokio::test]
async fn string_and_huge_integer_values_are_stored() {
    let Some(db) = TestDb::create("strings").await else {
        return;
    };
    let store = db.store().await;
    store.create_partition(Month::of(Utc::now())).await.unwrap();
    let now = Utc::now();
    let payload = |value: &str| {
        dt_server::payload::values::classify(
            format!(
                r#"{{"time_point":"{}","value":{value}}}"#,
                format_time_point(now)
            )
            .as_bytes(),
        )
        .unwrap()
    };
    let rows = vec![
        Row::new(
            Uuid::from_u128(1),
            Uuid::from_u128(2),
            "text",
            &payload(r#""hello world""#),
            now,
        ),
        Row::new(
            Uuid::from_u128(1),
            Uuid::from_u128(2),
            "big",
            &payload("18446744073709551615"),
            now,
        ),
    ];
    store
        .insert_batch(&BatchColumns::from_rows(&rows))
        .await
        .unwrap();
    let text = sqlx::query(
        "SELECT value_type, value_num, value_text FROM measurement WHERE element_path = 'text'",
    )
    .fetch_one(store.pool())
    .await
    .unwrap();
    assert_eq!(text.get::<i16, _>(0), value_type::STRING);
    assert_eq!(text.get::<Option<f64>, _>(1), None);
    assert_eq!(
        text.get::<Option<String>, _>(2).as_deref(),
        Some("hello world")
    );
    let big = sqlx::query(
        "SELECT value_type, value_num, value_text FROM measurement WHERE element_path = 'big'",
    )
    .fetch_one(store.pool())
    .await
    .unwrap();
    assert_eq!(big.get::<i16, _>(0), value_type::INTEGER);
    assert_eq!(
        big.get::<Option<String>, _>(2).as_deref(),
        Some("18446744073709551615")
    );
    store.close().await;
}
