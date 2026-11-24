//! The REST API against a real PostgreSQL. Runs only when `DT_TEST_DATABASE_URL` is set (a database in which the
//! test user may create schemas); otherwise every test prints a "SKIPPED" line and passes.
//!
//! Each test creates its own uniquely named schema (`dt_test_<uuid>`), loads `DigitalTwinStore/sql/0001_schema.sql`
//! into it, uses it as `search_path` and drops it afterwards (also when the test panics). Nothing else in the
//! database is touched. The URL is never printed.
//!
//! The data is written the way the C++ store writes it (see the comments of the schema): a twin with two loaded
//! commits, a checkpoint per commit and later changes.

use std::str::FromStr;

use axum::body::Body;
use axum::http::{Request, StatusCode};
use axum::Router;
use chrono::{DateTime, TimeZone, Utc};
use dt_server::api::{connect_lazy, router};
use http_body_util::BodyExt;
use serde_json::{json, Value};
use sqlx::postgres::{PgConnectOptions, PgPool, PgPoolOptions};
use sqlx::Row as _;
use tower::ServiceExt;
use uuid::Uuid;

const SCHEMA_SQL: &str = include_str!("../../DigitalTwinStore/sql/0001_schema.sql");

const PROJECT: Uuid = Uuid::from_u128(0x10);
const EMPTY_PROJECT: Uuid = Uuid::from_u128(0x11);
const TWIN: Uuid = Uuid::from_u128(0x20);
const TWIN_WITHOUT_VERSION: Uuid = Uuid::from_u128(0x21);
const OLD_COMMIT: Uuid = Uuid::from_u128(0x30);
const NEW_COMMIT: Uuid = Uuid::from_u128(0x31);
const FLIP_PROJECT: Uuid = Uuid::from_u128(0x12);
const FLIP_TWIN: Uuid = Uuid::from_u128(0x22);
const COMMIT_A: Uuid = Uuid::from_u128(0x40);
const COMMIT_B: Uuid = Uuid::from_u128(0x41);

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

    /// A writing connection pool (the test plays the C++ store) with the schema loaded.
    async fn writer(&self) -> PgPool {
        let options = PgConnectOptions::from_str(&self.url)
            .unwrap()
            .options([("search_path", self.schema.as_str())]);
        let pool = PgPoolOptions::new()
            .max_connections(2)
            .connect_with(options)
            .await
            .expect("connect");
        // plain statements only, each ended by a semicolon; comments are whole lines
        let sql: String = SCHEMA_SQL
            .lines()
            .map(|line| line.split("--").next().unwrap_or(""))
            .collect::<Vec<_>>()
            .join("\n");
        for statement in sql.split(';').map(str::trim).filter(|s| !s.is_empty()) {
            sqlx::query(sqlx::AssertSqlSafe(statement.to_owned()))
                .execute(&pool)
                .await
                .unwrap_or_else(|e| panic!("schema statement failed: {e}\n{statement}"));
        }
        // the store creates the partitions; one month holds all test data
        sqlx::query(
            "CREATE TABLE measurement_y2026m01 PARTITION OF measurement \
             FOR VALUES FROM ('2026-01-01 00:00:00+00') TO ('2026-02-01 00:00:00+00')",
        )
        .execute(&pool)
        .await
        .expect("create the partition");
        pool
    }

    /// The API as the server builds it: lazy read-only pool, no authentication.
    fn api(&self) -> Router {
        router(connect_lazy(&self.url, Some(&self.schema)).unwrap(), None)
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

/// 2026-01-10 at the given time (UTC).
fn t(hour: u32, minute: u32, second: u32) -> DateTime<Utc> {
    day(10, hour, minute, second)
}

fn day(day: u32, hour: u32, minute: u32, second: u32) -> DateTime<Utc> {
    Utc.with_ymd_and_hms(2026, 1, day, hour, minute, second)
        .unwrap()
}

async fn add_element(
    pool: &PgPool,
    version_no: i32,
    parent_no: Option<i32>,
    kind: i16,
    name: &str,
    path: &str,
    value_type: Option<i16>,
) -> i32 {
    sqlx::query(
        "INSERT INTO element (version_no, parent_no, kind, name, path, value_type) \
         VALUES ($1, $2, $3, $4, $5, $6) RETURNING element_no",
    )
    .bind(version_no)
    .bind(parent_no)
    .bind(kind)
    .bind(name)
    .bind(path)
    .bind(value_type)
    .fetch_one(pool)
    .await
    .unwrap()
    .get(0)
}

struct Row<'a> {
    element_no: i32,
    change_no: i64,
    received_at: DateTime<Utc>,
    ts: DateTime<Utc>,
    source: i16,
    checkpoint_no: Option<i64>,
    num: Option<f64>,
    text: Option<&'a str>,
    json: Option<Value>,
}

impl<'a> Row<'a> {
    fn new(element_no: i32, change_no: i64, received_at: DateTime<Utc>, source: i16) -> Self {
        Self {
            element_no,
            change_no,
            received_at,
            ts: received_at,
            source,
            checkpoint_no: None,
            num: None,
            text: None,
            json: None,
        }
    }

    fn num(mut self, value: f64) -> Self {
        self.num = Some(value);
        self
    }

    fn text(mut self, value: &'a str) -> Self {
        self.text = Some(value);
        self
    }

    fn json(mut self, value: Value) -> Self {
        self.json = Some(value);
        self
    }

    fn checkpoint(mut self, checkpoint_no: i64) -> Self {
        self.checkpoint_no = Some(checkpoint_no);
        self
    }

    async fn insert(self, pool: &PgPool) {
        sqlx::query(
            "INSERT INTO measurement \
             (element_no, change_no, received_at, ts, source, checkpoint_no, value_num, value_text, value_json) \
             VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)",
        )
        .bind(self.element_no)
        .bind(self.change_no)
        .bind(self.received_at)
        .bind(self.ts)
        .bind(self.source)
        .bind(self.checkpoint_no)
        .bind(self.num)
        .bind(self.text)
        .bind(self.json)
        .execute(pool)
        .await
        .unwrap();
    }
}

async fn add_checkpoint(
    pool: &PgPool,
    version_no: i32,
    taken_at: DateTime<Utc>,
    change_no: i64,
    variables: i32,
) -> i64 {
    sqlx::query(
        "INSERT INTO checkpoint (version_no, taken_at, change_no, reason, variables) \
         VALUES ($1, $2, $3, 1, $4) RETURNING checkpoint_no",
    )
    .bind(version_no)
    .bind(taken_at)
    .bind(change_no)
    .bind(variables)
    .fetch_one(pool)
    .await
    .unwrap()
    .get(0)
}

async fn add_version(
    pool: &PgPool,
    twin_no: i32,
    commit: Uuid,
    first_loaded: DateTime<Utc>,
    last_loaded: DateTime<Utc>,
) -> i32 {
    sqlx::query(
        "INSERT INTO model_version (twin_no, commit_id, first_loaded, last_loaded) \
         VALUES ($1, $2, $3, $4) RETURNING version_no",
    )
    .bind(twin_no)
    .bind(commit)
    .bind(first_loaded)
    .bind(last_loaded)
    .fetch_one(pool)
    .await
    .unwrap()
    .get(0)
}

/// Two projects (one without twins), two twins (one without a loaded version), and for the first twin
///
/// - the old commit (loaded 2026-01-05): `Motor` and `Motor/speed`; checkpoint at 08:30, then speed 1.5 at 08:40;
/// - the new commit (loaded 2026-01-10 10:00): eight variables of every value type; checkpoint at 10:00 with
///   their values, then speed 2.5 at 10:05, speed 3.5 with the derived `enabled` and `count` at 10:10, label at 10:20;
/// - a second checkpoint of the new commit at 10:30 (change number 2100), and around it rows that only the bounds of
///   the state query keep out: `Motor/speed` 7.77 and `Motor/extra` 7.77 at 10:30 with the smaller change number
///   2099 (the writer's clamp to the checkpoint time; `extra` has no checkpoint row, so only the lower bound on
///   `change_no` keeps it out), `Motor/ghost` 8.88 at 10:29 with the larger change number 2150 (only the lower
///   bound on `received_at` keeps it out); then speed 10.0 at 10:35.
async fn seed(pool: &PgPool) {
    for (id, name, first_seen) in [
        (PROJECT, Some("Plant"), day(1, 0, 0, 0)),
        (EMPTY_PROJECT, None, day(2, 0, 0, 0)),
    ] {
        sqlx::query("INSERT INTO project (project_id, name, first_seen) VALUES ($1, $2, $3)")
            .bind(id)
            .bind(name)
            .bind(first_seen)
            .execute(pool)
            .await
            .unwrap();
    }
    let mut twin_nos = Vec::new();
    for (id, name, first_seen) in [
        (TWIN, "Pump", day(5, 8, 0, 0)),
        (TWIN_WITHOUT_VERSION, "Valve", day(6, 8, 0, 0)),
    ] {
        let twin_no: i32 = sqlx::query(
            "INSERT INTO twin (twin_id, project_id, name, first_seen, last_seen) \
             VALUES ($1, $2, $3, $4, $4) RETURNING twin_no",
        )
        .bind(id)
        .bind(PROJECT)
        .bind(name)
        .bind(first_seen)
        .fetch_one(pool)
        .await
        .unwrap()
        .get(0);
        twin_nos.push(twin_no);
    }
    let twin_no = twin_nos[0];

    // old commit
    let v1 = add_version(pool, twin_no, OLD_COMMIT, day(5, 8, 0, 0), day(5, 9, 0, 0)).await;
    let motor = add_element(pool, v1, None, 2, "Motor", "Motor", None).await;
    let speed1 = add_element(pool, v1, Some(motor), 4, "speed", "Motor/speed", Some(1)).await;
    let cp1 = add_checkpoint(pool, v1, day(5, 8, 30, 0), 1000, 1).await;
    Row::new(speed1, 1000, day(5, 8, 30, 0), 3)
        .checkpoint(cp1)
        .num(1.0)
        .insert(pool)
        .await;
    Row::new(speed1, 1001, day(5, 8, 40, 0), 1)
        .num(1.5)
        .insert(pool)
        .await;

    // new commit
    let v2 = add_version(pool, twin_no, NEW_COMMIT, t(10, 0, 0), t(10, 0, 0)).await;
    let motor = add_element(pool, v2, None, 2, "Motor", "Motor", None).await;
    let speed = add_element(pool, v2, Some(motor), 4, "speed", "Motor/speed", Some(1)).await;
    let enabled = add_element(
        pool,
        v2,
        Some(motor),
        5,
        "enabled",
        "Motor/enabled",
        Some(3),
    )
    .await;
    let count = add_element(pool, v2, Some(motor), 6, "count", "Motor/count", Some(2)).await;
    let big = add_element(pool, v2, Some(motor), 6, "big", "Motor/big", Some(2)).await;
    let ratio = add_element(pool, v2, Some(motor), 4, "ratio", "Motor/ratio", Some(4)).await;
    let z = add_element(pool, v2, Some(motor), 4, "z", "Motor/z", Some(5)).await;
    let label = add_element(pool, v2, Some(motor), 6, "label", "Motor/label", Some(6)).await;
    // its path starts like `Motor/speed`, but it is a different element
    let speedometer = add_element(
        pool,
        v2,
        Some(motor),
        4,
        "speedometer",
        "Motor/speedometer",
        Some(1),
    )
    .await;

    let cp2 = add_checkpoint(pool, v2, t(10, 0, 0), 2000, 8).await;
    let at = t(10, 0, 0);
    Row::new(speed, 2000, at, 3)
        .checkpoint(cp2)
        .num(1.0)
        .insert(pool)
        .await;
    Row::new(enabled, 2000, at, 3)
        .checkpoint(cp2)
        .num(0.0)
        .insert(pool)
        .await;
    Row::new(count, 2000, at, 3)
        .checkpoint(cp2)
        .num(5.0)
        .insert(pool)
        .await;
    Row::new(big, 2000, at, 3)
        .checkpoint(cp2)
        .text("9007199254740993")
        .insert(pool)
        .await;
    Row::new(ratio, 2000, at, 3)
        .checkpoint(cp2)
        .text("1/2")
        .insert(pool)
        .await;
    Row::new(z, 2000, at, 3)
        .checkpoint(cp2)
        .json(json!({"imaginaryValue": 2.0, "realValue": 1.0}))
        .insert(pool)
        .await;
    Row::new(label, 2000, at, 3)
        .checkpoint(cp2)
        .text("idle")
        .insert(pool)
        .await;
    Row::new(speedometer, 2000, at, 3)
        .checkpoint(cp2)
        .num(0.0)
        .insert(pool)
        .await;

    // later changes: a client update, then one update with two derived values, then a string
    Row::new(speed, 2001, t(10, 5, 0), 1)
        .num(2.5)
        .insert(pool)
        .await;
    Row::new(speed, 2002, t(10, 10, 0), 1)
        .num(3.5)
        .insert(pool)
        .await;
    Row::new(enabled, 2002, t(10, 10, 0), 2)
        .num(1.0)
        .insert(pool)
        .await;
    Row::new(count, 2002, t(10, 10, 0), 2)
        .num(6.0)
        .insert(pool)
        .await;
    Row::new(label, 2003, t(10, 20, 0), 1)
        .text("run")
        .insert(pool)
        .await;

    // second checkpoint at 10:30 with a different value for everything that changed
    let extra = add_element(pool, v2, Some(motor), 4, "extra", "Motor/extra", Some(1)).await;
    let ghost = add_element(pool, v2, Some(motor), 4, "ghost", "Motor/ghost", Some(1)).await;
    let cp3 = add_checkpoint(pool, v2, t(10, 30, 0), 2100, 8).await;
    let at = t(10, 30, 0);
    Row::new(speed, 2100, at, 3)
        .checkpoint(cp3)
        .num(9.0)
        .insert(pool)
        .await;
    Row::new(enabled, 2100, at, 3)
        .checkpoint(cp3)
        .num(1.0)
        .insert(pool)
        .await;
    Row::new(count, 2100, at, 3)
        .checkpoint(cp3)
        .num(6.0)
        .insert(pool)
        .await;
    Row::new(big, 2100, at, 3)
        .checkpoint(cp3)
        .text("9007199254740993")
        .insert(pool)
        .await;
    Row::new(ratio, 2100, at, 3)
        .checkpoint(cp3)
        .text("1/2")
        .insert(pool)
        .await;
    Row::new(z, 2100, at, 3)
        .checkpoint(cp3)
        .json(json!({"imaginaryValue": 2.0, "realValue": 1.0}))
        .insert(pool)
        .await;
    Row::new(label, 2100, at, 3)
        .checkpoint(cp3)
        .text("run")
        .insert(pool)
        .await;
    Row::new(speedometer, 2100, at, 3)
        .checkpoint(cp3)
        .num(0.0)
        .insert(pool)
        .await;
    // clamped to the checkpoint time, but written before the checkpoint: a smaller change number, another value
    Row::new(speed, 2099, at, 1).num(7.77).insert(pool).await;
    Row::new(extra, 2099, at, 1).num(7.77).insert(pool).await;
    // before the checkpoint time, but a larger change number (violates the writer's guarantee on purpose)
    Row::new(ghost, 2150, t(10, 29, 0), 1)
        .num(8.88)
        .insert(pool)
        .await;
    Row::new(speed, 2101, t(10, 35, 0), 1)
        .num(10.0)
        .insert(pool)
        .await;
}

/// A twin whose commits alternate: commit A runs on day 1, commit B on days 2 and 3, A is loaded again on day 4
/// (which sets its `last_loaded` to day 4 and writes a new checkpoint of A).
async fn seed_flip(pool: &PgPool) {
    sqlx::query("INSERT INTO project (project_id, name, first_seen) VALUES ($1, 'Gears', $2)")
        .bind(FLIP_PROJECT)
        .bind(day(1, 0, 0, 0))
        .execute(pool)
        .await
        .unwrap();
    let twin_no: i32 = sqlx::query(
        "INSERT INTO twin (twin_id, project_id, name, first_seen, last_seen) \
         VALUES ($1, $2, 'Gear', $3, $3) RETURNING twin_no",
    )
    .bind(FLIP_TWIN)
    .bind(FLIP_PROJECT)
    .bind(day(1, 8, 0, 0))
    .fetch_one(pool)
    .await
    .unwrap()
    .get(0);
    let va = add_version(pool, twin_no, COMMIT_A, day(1, 8, 0, 0), day(4, 8, 0, 0)).await;
    let pos_a = add_element(pool, va, None, 4, "pos", "Gear/pos", Some(1)).await;
    let vb = add_version(pool, twin_no, COMMIT_B, day(2, 8, 0, 0), day(2, 8, 0, 0)).await;
    let pos_b = add_element(pool, vb, None, 4, "pos", "Gear/pos", Some(1)).await;

    let cp = add_checkpoint(pool, va, day(1, 8, 0, 0), 3000, 1).await;
    Row::new(pos_a, 3000, day(1, 8, 0, 0), 3)
        .checkpoint(cp)
        .num(1.0)
        .insert(pool)
        .await;
    Row::new(pos_a, 3001, day(1, 12, 0, 0), 1)
        .num(1.5)
        .insert(pool)
        .await;
    let cp = add_checkpoint(pool, vb, day(2, 8, 0, 0), 3100, 1).await;
    Row::new(pos_b, 3100, day(2, 8, 0, 0), 3)
        .checkpoint(cp)
        .num(20.0)
        .insert(pool)
        .await;
    Row::new(pos_b, 3101, day(3, 12, 0, 0), 1)
        .num(21.0)
        .insert(pool)
        .await;
    // A is loaded again: the last stored state of A, as a checkpoint
    let cp = add_checkpoint(pool, va, day(4, 8, 0, 0), 3200, 1).await;
    Row::new(pos_a, 3200, day(4, 8, 0, 0), 3)
        .checkpoint(cp)
        .num(1.5)
        .insert(pool)
        .await;
}

async fn get(router: &Router, uri: &str) -> (StatusCode, Value) {
    let response = router
        .clone()
        .oneshot(Request::builder().uri(uri).body(Body::empty()).unwrap())
        .await
        .unwrap();
    let status = response.status();
    let bytes = response.into_body().collect().await.unwrap().to_bytes();
    let json = serde_json::from_slice(&bytes)
        .unwrap_or_else(|_| panic!("not JSON: {}", String::from_utf8_lossy(&bytes)));
    (status, json)
}

async fn ok(router: &Router, uri: &str) -> Value {
    let (status, json) = get(router, uri).await;
    assert_eq!(status, StatusCode::OK, "{uri}: {json}");
    json
}

async fn status_of(router: &Router, uri: &str) -> StatusCode {
    let (status, json) = get(router, uri).await;
    assert!(
        json["error"].is_string() || status.is_success(),
        "{uri}: {json}"
    );
    status
}

/// The entry of a state response for a path.
fn entry<'a>(state: &'a Value, path: &str) -> &'a Value {
    state["variables"]
        .as_array()
        .unwrap()
        .iter()
        .find(|e| e["path"] == path)
        .unwrap_or_else(|| panic!("{path} not in {state}"))
}

fn paths(state: &Value) -> Vec<&str> {
    state["variables"]
        .as_array()
        .unwrap()
        .iter()
        .map(|e| e["path"].as_str().unwrap())
        .collect()
}

#[tokio::test]
async fn lists_projects_twins_and_versions() {
    let Some(db) = TestDb::create("lists").await else {
        return;
    };
    seed(&db.writer().await).await;
    let api = db.api();

    let projects = ok(&api, "/projects").await;
    assert_eq!(
        projects,
        json!([
            {"project_id": PROJECT, "name": "Plant", "first_seen": "2026-01-01T00:00:00Z"},
            {"project_id": EMPTY_PROJECT, "name": null, "first_seen": "2026-01-02T00:00:00Z"},
        ])
    );

    let twins = ok(&api, &format!("/projects/{PROJECT}/twins")).await;
    assert_eq!(
        twins,
        json!([
            {"twin_id": TWIN, "name": "Pump",
             "first_seen": "2026-01-05T08:00:00Z", "last_seen": "2026-01-05T08:00:00Z"},
            {"twin_id": TWIN_WITHOUT_VERSION, "name": "Valve",
             "first_seen": "2026-01-06T08:00:00Z", "last_seen": "2026-01-06T08:00:00Z"},
        ])
    );
    assert_eq!(
        ok(&api, &format!("/projects/{EMPTY_PROJECT}/twins")).await,
        json!([])
    );
    assert_eq!(
        status_of(&api, &format!("/projects/{}/twins", Uuid::from_u128(99))).await,
        StatusCode::NOT_FOUND
    );

    // newest commit first
    let versions = ok(&api, &format!("/twins/{TWIN}/versions")).await;
    assert_eq!(
        versions,
        json!([
            {"commit": NEW_COMMIT, "first_loaded": "2026-01-10T10:00:00Z", "last_loaded": "2026-01-10T10:00:00Z"},
            {"commit": OLD_COMMIT, "first_loaded": "2026-01-05T08:00:00Z", "last_loaded": "2026-01-05T09:00:00Z"},
        ])
    );
    assert_eq!(
        ok(&api, &format!("/twins/{TWIN_WITHOUT_VERSION}/versions")).await,
        json!([])
    );
    assert_eq!(
        status_of(&api, &format!("/twins/{}/versions", Uuid::from_u128(99))).await,
        StatusCode::NOT_FOUND
    );
}

#[tokio::test]
async fn elements_of_a_commit() {
    let Some(db) = TestDb::create("elements").await else {
        return;
    };
    seed(&db.writer().await).await;
    let api = db.api();

    // default: the version loaded last
    let new = ok(&api, &format!("/twins/{TWIN}/elements")).await;
    assert_eq!(new["commit"], json!(NEW_COMMIT));
    let elements = new["elements"].as_array().unwrap();
    assert_eq!(elements.len(), 11);
    let motor = &elements[0];
    assert_eq!(
        (
            motor["kind"].clone(),
            motor["name"].clone(),
            motor["path"].clone()
        ),
        (json!("component"), json!("Motor"), json!("Motor"))
    );
    assert_eq!(motor["parent_no"], Value::Null);
    assert_eq!(motor["value_type"], Value::Null);
    let by_path = |path: &str| elements.iter().find(|e| e["path"] == path).unwrap();
    for (path, kind, value_type) in [
        ("Motor/speed", "measurable", "real"),
        ("Motor/enabled", "controllable", "boolean"),
        ("Motor/count", "attribute", "integer"),
        ("Motor/ratio", "measurable", "rational"),
        ("Motor/z", "measurable", "complex"),
        ("Motor/label", "attribute", "string"),
    ] {
        let e = by_path(path);
        assert_eq!(e["kind"], kind, "{path}");
        assert_eq!(e["value_type"], value_type, "{path}");
        assert_eq!(e["parent_no"], motor["element_no"], "{path}");
    }

    // an older commit has its own tree
    let old = ok(&api, &format!("/twins/{TWIN}/elements?commit={OLD_COMMIT}")).await;
    assert_eq!(old["commit"], json!(OLD_COMMIT));
    let old_paths: Vec<_> = old["elements"]
        .as_array()
        .unwrap()
        .iter()
        .map(|e| e["path"].as_str().unwrap())
        .collect();
    assert_eq!(old_paths, ["Motor", "Motor/speed"]);

    assert_eq!(
        status_of(
            &api,
            &format!("/twins/{TWIN}/elements?commit={}", Uuid::from_u128(99))
        )
        .await,
        StatusCode::NOT_FOUND
    );
    assert_eq!(
        status_of(&api, &format!("/twins/{TWIN_WITHOUT_VERSION}/elements")).await,
        StatusCode::NOT_FOUND
    );
    assert_eq!(
        status_of(&api, &format!("/twins/{TWIN}/elements?commit=nope")).await,
        StatusCode::BAD_REQUEST
    );
}

#[tokio::test]
async fn state_at_a_time_is_checkpoint_plus_later_changes() {
    let Some(db) = TestDb::create("state").await else {
        return;
    };
    seed(&db.writer().await).await;
    let api = db.api();
    let state = |query: &str| format!("/twins/{TWIN}/state?{query}");

    // just after the first change: speed changed, everything else is the checkpoint
    let s = ok(&api, &state("at=2026-01-10T10:07:00Z")).await;
    assert_eq!(s["commit"], json!(NEW_COMMIT));
    assert_eq!(s["at"], json!("2026-01-10T10:07:00Z"));
    assert_eq!(s["checkpoint_at"], json!("2026-01-10T10:00:00Z"));
    assert_eq!(s["variables"].as_array().unwrap().len(), 8);
    assert_eq!(
        entry(&s, "Motor/speed"),
        &json!({
            "path": "Motor/speed", "kind": "measurable", "value_type": "real", "value": 2.5,
            "received_at": "2026-01-10T10:05:00Z", "ts": "2026-01-10T10:05:00Z", "source": "client"
        })
    );
    assert_eq!(entry(&s, "Motor/speed")["source"], json!("client"));
    assert_eq!(entry(&s, "Motor/enabled")["value"], json!(false));
    assert_eq!(entry(&s, "Motor/enabled")["source"], json!("checkpoint"));
    // one value of every type
    assert_eq!(entry(&s, "Motor/count")["value"], json!(5));
    assert_eq!(entry(&s, "Motor/big")["value"], json!("9007199254740993"));
    assert_eq!(entry(&s, "Motor/ratio")["value"], json!("1/2"));
    assert_eq!(
        entry(&s, "Motor/z")["value"],
        json!({"imaginaryValue": 2.0, "realValue": 1.0})
    );
    assert_eq!(entry(&s, "Motor/label")["value"], json!("idle"));
    assert_eq!(entry(&s, "Motor/label")["value_type"], json!("string"));
    // ordered by path
    let listed = paths(&s);
    let mut sorted = listed.clone();
    sorted.sort_unstable();
    assert_eq!(listed, sorted);

    // `at` is inclusive; changes of one change number are seen together, derived values as such
    let s = ok(&api, &state("at=2026-01-10T10:10:00Z")).await;
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(3.5));
    assert_eq!(entry(&s, "Motor/enabled")["value"], json!(true));
    assert_eq!(entry(&s, "Motor/enabled")["source"], json!("derived"));
    assert_eq!(entry(&s, "Motor/count")["value"], json!(6));
    // one second before it, none of them
    let s = ok(&api, &state("at=2026-01-10T10:09:59Z")).await;
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(2.5));
    assert_eq!(entry(&s, "Motor/enabled")["value"], json!(false));

    // the exact time of the checkpoint is the checkpoint
    let s = ok(&api, &state("at=2026-01-10T10:00:00Z")).await;
    assert_eq!(entry(&s, "Motor/speed")["source"], json!("checkpoint"));

    // an offset is understood (+ has to be sent as %2B)
    let s = ok(&api, &state("at=2026-01-10T11:07:00%2B01:00")).await;
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(2.5));

    // default: now, which is after all data
    let s = ok(&api, &state("")).await;
    assert_eq!(entry(&s, "Motor/label")["value"], json!("run"));
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(10.0));
    assert_eq!(s["checkpoint_at"], json!("2026-01-10T10:30:00Z"));

    // The second checkpoint (10:30, change number 2100) and the rows around it. Just before it the first
    // checkpoint is the base; the rows of 10:29 and 10:30 are not seen yet.
    let s = ok(&api, &state("at=2026-01-10T10:29:59Z")).await;
    assert_eq!(s["checkpoint_at"], json!("2026-01-10T10:00:00Z"));
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(3.5));
    assert_eq!(entry(&s, "Motor/label")["value"], json!("run"));
    assert!(!paths(&s).contains(&"Motor/extra"));
    // At it, the second checkpoint is the base: its values win, and neither the clamped row with the smaller
    // change number (`extra`: 7.77) nor the row before its time (`ghost`: 8.88) belongs to the state.
    let s = ok(&api, &state("at=2026-01-10T10:30:00Z")).await;
    assert_eq!(s["checkpoint_at"], json!("2026-01-10T10:30:00Z"));
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(9.0));
    assert_eq!(entry(&s, "Motor/speed")["source"], json!("checkpoint"));
    assert_eq!(entry(&s, "Motor/enabled")["value"], json!(true));
    assert_eq!(entry(&s, "Motor/enabled")["source"], json!("checkpoint"));
    assert_eq!(s["variables"].as_array().unwrap().len(), 8);
    assert!(!paths(&s).contains(&"Motor/extra"), "{s}");
    assert!(!paths(&s).contains(&"Motor/ghost"), "{s}");
    // after it, the later change is applied on top
    let s = ok(&api, &state("at=2026-01-10T10:36:00Z")).await;
    assert_eq!(s["checkpoint_at"], json!("2026-01-10T10:30:00Z"));
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(10.0));
    assert_eq!(entry(&s, "Motor/speed")["source"], json!("client"));
    assert!(!paths(&s).contains(&"Motor/extra"));
    assert!(!paths(&s).contains(&"Motor/ghost"));

    // An older commit, with its own checkpoint. Without a commit the newest checkpoint at or before the time
    // decides, so 09:59:59 on day 10 is still commit OLD.
    let old = |at: &str| format!("/twins/{TWIN}/state?commit={OLD_COMMIT}&at={at}");
    let s = ok(&api, &old("2026-01-05T08:45:00Z")).await;
    assert_eq!(s["commit"], json!(OLD_COMMIT));
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(1.5));
    let s = ok(&api, &old("2026-01-05T08:35:00Z")).await;
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(1.0));
    let s = ok(&api, &state("at=2026-01-10T09:59:59Z")).await;
    assert_eq!(s["commit"], json!(OLD_COMMIT));
    assert_eq!(s["checkpoint_at"], json!("2026-01-05T08:30:00Z"));
    assert_eq!(entry(&s, "Motor/speed")["value"], json!(1.5));
    let s = ok(&api, &state("at=2026-01-10T10:00:00Z")).await;
    assert_eq!(s["commit"], json!(NEW_COMMIT));

    // no checkpoint at or before the time: 404 with an explanation, not an empty state
    for uri in [
        state("at=2025-06-01T00:00:00Z"),
        state("at=2026-01-05T08:29:59Z"),
        old("2026-01-05T08:29:59Z"),
        // the new commit has no checkpoint before 10:00, whatever the other commit has
        format!("/twins/{TWIN}/state?commit={NEW_COMMIT}&at=2026-01-10T09:59:59Z"),
    ] {
        let (status, json) = get(&api, &uri).await;
        assert_eq!(status, StatusCode::NOT_FOUND, "{uri}");
        assert!(
            json["error"].as_str().unwrap().contains("checkpoint"),
            "{json}"
        );
    }
    assert_eq!(
        status_of(&api, &format!("/twins/{TWIN_WITHOUT_VERSION}/state")).await,
        StatusCode::NOT_FOUND
    );
    assert_eq!(
        status_of(
            &api,
            &format!("/twins/{TWIN}/state?commit={}", Uuid::from_u128(99))
        )
        .await,
        StatusCode::NOT_FOUND
    );
    assert_eq!(
        status_of(&api, &state("at=today")).await,
        StatusCode::BAD_REQUEST
    );
}

/// Commit A runs, then B, then A is loaded again (its `last_loaded` is the latest): the state at a time is the one
/// of the commit that was running then, not of the commit loaded last.
#[tokio::test]
async fn state_without_a_commit_follows_the_checkpoints_not_last_loaded() {
    let Some(db) = TestDb::create("flip").await else {
        return;
    };
    seed_flip(&db.writer().await).await;
    let api = db.api();
    let state = |at: &str| format!("/twins/{FLIP_TWIN}/state?at={at}");
    let check = |s: &Value, commit: Uuid, pos: f64, checkpoint_at: &str| {
        assert_eq!(s["commit"], json!(commit), "{s}");
        assert_eq!(entry(s, "Gear/pos")["value"], json!(pos), "{s}");
        assert_eq!(s["checkpoint_at"], json!(checkpoint_at), "{s}");
    };

    check(
        &ok(&api, &state("2026-01-01T10:00:00Z")).await,
        COMMIT_A,
        1.0,
        "2026-01-01T08:00:00Z",
    );
    check(
        &ok(&api, &state("2026-01-01T12:00:00Z")).await,
        COMMIT_A,
        1.5,
        "2026-01-01T08:00:00Z",
    );
    // A is still the version loaded last, but B was running
    check(
        &ok(&api, &state("2026-01-02T20:00:00Z")).await,
        COMMIT_B,
        20.0,
        "2026-01-02T08:00:00Z",
    );
    check(
        &ok(&api, &state("2026-01-03T13:00:00Z")).await,
        COMMIT_B,
        21.0,
        "2026-01-02T08:00:00Z",
    );
    // after A was loaded again its new checkpoint decides
    check(
        &ok(&api, &state("2026-01-04T09:00:00Z")).await,
        COMMIT_A,
        1.5,
        "2026-01-04T08:00:00Z",
    );
    check(
        &ok(&api, &state("2026-01-09T00:00:00Z")).await,
        COMMIT_A,
        1.5,
        "2026-01-04T08:00:00Z",
    );
    // an explicit commit uses the checkpoints of that commit only
    let (status, _) = get(
        &api,
        &format!("/twins/{FLIP_TWIN}/state?commit={COMMIT_B}&at=2026-01-01T10:00:00Z"),
    )
    .await;
    assert_eq!(status, StatusCode::NOT_FOUND);
    let s = ok(
        &api,
        &format!("/twins/{FLIP_TWIN}/state?commit={COMMIT_A}&at=2026-01-02T20:00:00Z"),
    )
    .await;
    check(&s, COMMIT_A, 1.5, "2026-01-01T08:00:00Z");
    assert_eq!(
        status_of(&api, &state("2025-12-31T00:00:00Z")).await,
        StatusCode::NOT_FOUND
    );

    // `elements` without a commit is the version loaded last (documented), which is A here
    let e = ok(&api, &format!("/twins/{FLIP_TWIN}/elements")).await;
    assert_eq!(e["commit"], json!(COMMIT_A));
    let versions = ok(&api, &format!("/twins/{FLIP_TWIN}/versions")).await;
    assert_eq!(versions[0]["commit"], json!(COMMIT_A));
    assert_eq!(versions[1]["commit"], json!(COMMIT_B));
}

#[tokio::test]
async fn state_can_be_restricted_to_a_subtree() {
    let Some(db) = TestDb::create("subtree").await else {
        return;
    };
    seed(&db.writer().await).await;
    let api = db.api();
    let state = |path: &str| format!("/twins/{TWIN}/state?at=2026-01-10T10:07:00Z&path={path}");

    // a variable: only that one, not `Motor/speedometer`
    assert_eq!(
        paths(&ok(&api, &state("Motor%2Fspeed")).await),
        ["Motor/speed"]
    );
    // a trailing separator is ignored
    assert_eq!(
        paths(&ok(&api, &state("Motor%2Fspeed%2F")).await),
        ["Motor/speed"]
    );
    // a component: all variables below it
    assert_eq!(paths(&ok(&api, &state("Motor")).await).len(), 8);
    // a prefix that is not a whole path level, and a path that does not exist: no entries
    assert!(paths(&ok(&api, &state("Motor%2Fsp")).await).is_empty());
    assert!(paths(&ok(&api, &state("Other")).await).is_empty());
    // characters of LIKE patterns are plain characters
    assert!(paths(&ok(&api, &state("Mo%25")).await).is_empty());
    assert!(paths(&ok(&api, &state("Motor%2F_peed")).await).is_empty());
}

#[tokio::test]
async fn series_of_a_variable() {
    let Some(db) = TestDb::create("series").await else {
        return;
    };
    seed(&db.writer().await).await;
    let api = db.api();
    let series = |query: &str| format!("/twins/{TWIN}/series?path=Motor%2Fspeed{query}");
    let values = |s: &Value| -> Vec<(String, Value, String)> {
        s["points"]
            .as_array()
            .unwrap()
            .iter()
            .map(|p| {
                (
                    p["received_at"].as_str().unwrap().to_owned(),
                    p["value"].clone(),
                    p["commit"].as_str().unwrap().to_owned(),
                )
            })
            .collect()
    };
    let (old, new) = (OLD_COMMIT.to_string(), NEW_COMMIT.to_string());

    // across all versions of the twin, ascending in time, each point says which commit
    let s = ok(&api, &series("")).await;
    assert_eq!(s["twin_id"], json!(TWIN));
    assert_eq!(s["path"], json!("Motor/speed"));
    assert_eq!(s["limit"], json!(1000));
    assert_eq!(s["more"], json!(false));
    assert_eq!(
        values(&s),
        [
            ("2026-01-05T08:30:00Z".to_owned(), json!(1.0), old.clone()),
            ("2026-01-05T08:40:00Z".to_owned(), json!(1.5), old.clone()),
            ("2026-01-10T10:00:00Z".to_owned(), json!(1.0), new.clone()),
            ("2026-01-10T10:05:00Z".to_owned(), json!(2.5), new.clone()),
            ("2026-01-10T10:10:00Z".to_owned(), json!(3.5), new.clone()),
            // two rows of the same instant: the clamped client row (change 2099), then the checkpoint (2100)
            ("2026-01-10T10:30:00Z".to_owned(), json!(7.77), new.clone()),
            ("2026-01-10T10:30:00Z".to_owned(), json!(9.0), new.clone()),
            ("2026-01-10T10:35:00Z".to_owned(), json!(10.0), new.clone()),
        ]
    );
    assert_eq!(s["next"], Value::Null);
    assert_eq!(
        s["points"][0],
        json!({
            "commit": OLD_COMMIT, "received_at": "2026-01-05T08:30:00Z", "change_no": 1000,
            "ts": "2026-01-05T08:30:00Z", "source": "checkpoint", "value_type": "real", "value": 1.0
        })
    );

    // one commit
    let s = ok(&api, &series(&format!("&commit={NEW_COMMIT}"))).await;
    assert_eq!(values(&s).len(), 6);
    assert!(values(&s).iter().all(|(_, _, c)| *c == new));

    // limit: the first rows, and whether there are more
    let s = ok(&api, &series("&limit=2")).await;
    assert_eq!(s["limit"], json!(2));
    assert_eq!(s["more"], json!(true));
    assert_eq!(values(&s).len(), 2);
    assert_eq!(values(&s)[1].0, "2026-01-05T08:40:00Z");
    assert!(s["next"].is_string());
    let s = ok(&api, &series("&limit=8")).await;
    assert_eq!((s["more"].clone(), values(&s).len()), (json!(false), 8));
    assert_eq!(s["next"], Value::Null);
    let s = ok(&api, &series("&limit=7")).await;
    assert_eq!((s["more"].clone(), values(&s).len()), (json!(true), 7));

    // from and to are inclusive
    let s = ok(
        &api,
        &series("&from=2026-01-10T10:00:00Z&to=2026-01-10T10:05:00Z"),
    )
    .await;
    assert_eq!(
        values(&s).iter().map(|v| v.1.clone()).collect::<Vec<_>>(),
        [json!(1.0), json!(2.5)]
    );
    let s = ok(&api, &series("&from=2026-01-10T10:05:01Z")).await;
    assert_eq!(values(&s).len(), 4);
    let s = ok(&api, &series("&to=2026-01-01T00:00:00Z")).await;
    assert!(values(&s).is_empty());
    assert_eq!(s["more"], json!(false));

    // values of other types
    let s = ok(&api, &format!("/twins/{TWIN}/series?path=Motor/label")).await;
    assert_eq!(
        values(&s).iter().map(|v| v.1.clone()).collect::<Vec<_>>(),
        [json!("idle"), json!("run"), json!("run")]
    );

    // unknown path, a path that is not a variable, bad parameters
    assert_eq!(
        status_of(&api, &format!("/twins/{TWIN}/series?path=Nothing")).await,
        StatusCode::NOT_FOUND
    );
    assert_eq!(
        status_of(&api, &format!("/twins/{TWIN}/series?path=Motor")).await,
        StatusCode::NOT_FOUND
    );
    assert_eq!(
        status_of(&api, &series(&format!("&commit={}", Uuid::from_u128(99)))).await,
        StatusCode::NOT_FOUND
    );
    // `Motor/label` does not exist in the old commit
    assert_eq!(
        status_of(
            &api,
            &format!("/twins/{TWIN}/series?path=Motor/label&commit={OLD_COMMIT}")
        )
        .await,
        StatusCode::NOT_FOUND
    );
    for bad in [
        "&limit=0",
        "&limit=10001",
        "&limit=x",
        "&from=x",
        "&bogus=1",
        "&after=x",
    ] {
        assert_eq!(
            status_of(&api, &series(bad)).await,
            StatusCode::BAD_REQUEST,
            "{bad}"
        );
    }
    assert_eq!(
        status_of(
            &api,
            &format!("/twins/{}/series?path=Motor/speed", Uuid::from_u128(99))
        )
        .await,
        StatusCode::NOT_FOUND
    );
}

/// Pages of a series neither repeat nor skip rows, also when rows share a `received_at` across the page border.
#[tokio::test]
async fn series_pages_with_a_cursor_cover_every_row_once() {
    let Some(db) = TestDb::create("paging").await else {
        return;
    };
    seed(&db.writer().await).await;
    let api = db.api();
    let uri = |query: &str| format!("/twins/{TWIN}/series?path=Motor%2Fspeed{query}");
    let key = |s: &Value| -> Vec<(String, i64)> {
        s["points"]
            .as_array()
            .unwrap()
            .iter()
            .map(|p| {
                (
                    p["received_at"].as_str().unwrap().to_owned(),
                    p["change_no"].as_i64().unwrap(),
                )
            })
            .collect()
    };
    let all = key(&ok(&api, &uri("")).await);
    assert_eq!(all.len(), 8);
    // the rows around the border of the first page share the instant 10:30:00
    assert_eq!(all[5], ("2026-01-10T10:30:00Z".to_owned(), 2099));
    assert_eq!(all[6], ("2026-01-10T10:30:00Z".to_owned(), 2100));

    // any page size, from 1 to more than everything: following `next` gives exactly the rows, once, in order
    for limit in 1..=9 {
        let mut collected = Vec::new();
        let mut after = String::new();
        for _ in 0..20 {
            let page = ok(&api, &uri(&format!("&limit={limit}{after}"))).await;
            collected.extend(key(&page));
            match page["next"].as_str() {
                Some(next) => {
                    assert_eq!(page["more"], json!(true));
                    after = format!("&after={next}");
                }
                None => {
                    assert_eq!(page["more"], json!(false));
                    break;
                }
            }
        }
        assert_eq!(collected, all, "limit {limit}");
    }

    // a page that ends between the two tied rows continues with the second one
    let first = ok(&api, &uri("&limit=6")).await;
    assert_eq!(key(&first).last().unwrap().1, 2099);
    let next = first["next"].as_str().unwrap().to_owned();
    let second = ok(&api, &uri(&format!("&limit=6&after={next}"))).await;
    assert_eq!(key(&second)[0], ("2026-01-10T10:30:00Z".to_owned(), 2100));
    assert_eq!(second["points"][0]["value"], json!(9.0));
    assert_eq!(second["more"], json!(false));
    // the cursor works together with from, to and commit
    let page = ok(
        &api,
        &uri(&format!(
            "&after={next}&to=2026-01-10T10:30:00Z&commit={NEW_COMMIT}"
        )),
    )
    .await;
    assert_eq!(key(&page).len(), 1);
}

/// The pool of the API cannot write, whatever the database role may do.
#[tokio::test]
async fn the_api_connection_is_read_only() {
    let Some(db) = TestDb::create("read_only").await else {
        return;
    };
    drop(db.writer().await);
    let pool = connect_lazy(&db.url, Some(&db.schema)).unwrap();
    let error = sqlx::query("INSERT INTO project (project_id, name) VALUES ($1, 'x')")
        .bind(Uuid::from_u128(1))
        .execute(&pool)
        .await
        .expect_err("a write must fail");
    let code = error
        .as_database_error()
        .and_then(|e| e.code().map(|c| c.into_owned()));
    // 25006: read_only_sql_transaction
    assert_eq!(code.as_deref(), Some("25006"), "{error}");
    pool.close().await;
}
