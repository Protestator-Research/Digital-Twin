//! PostgreSQL implementation of [`Store`] (sqlx, tokio runtime, rustls). Runtime-checked queries only, no
//! compile-time `query!` macros (no database is available at build time).
//!
//! Every statement uses unqualified table names, so the schema is whatever the connection's `search_path`
//! selects (`public` in production; the tests use their own schema).

use std::str::FromStr;
use std::time::Duration;

use sqlx::postgres::{PgConnectOptions, PgPool, PgPoolOptions};
use sqlx::Row as _;
use sqlx::{Connection, PgConnection};
use tokio::sync::Mutex;
use uuid::Uuid;

use super::partition::Month;
use super::registry::TwinRecord;
use super::row::BatchColumns;
use super::store::{classify_sqlstate, is_partition_race, ErrorKind, Store, StoreError};

/// One statement per batch: the columns arrive as arrays and are unnested into rows. `value_json` is sent as
/// text and cast, so no array-of-jsonb encoding is needed.
const INSERT_BATCH: &str = "\
INSERT INTO measurement \
    (twin_id, project_id, element_path, ts, received_at, value_type, value_num, value_text, value_json) \
SELECT t.twin_id, t.project_id, t.element_path, t.ts, t.received_at, t.value_type, t.value_num, t.value_text, \
       t.value_json::jsonb \
FROM UNNEST($1::uuid[], $2::uuid[], $3::text[], $4::timestamptz[], $5::timestamptz[], $6::int2[], \
            $7::float8[], $8::text[], $9::text[]) \
     AS t(twin_id, project_id, element_path, ts, received_at, value_type, value_num, value_text, value_json)";

const UPSERT_TWIN: &str = "\
INSERT INTO twin (twin_id, project_id, name) VALUES ($1, $2, $3) \
ON CONFLICT (twin_id) DO UPDATE \
    SET project_id = EXCLUDED.project_id, name = EXCLUDED.name, last_seen = now()";

const LOAD_TWINS: &str = "SELECT twin_id, project_id, name FROM twin ORDER BY first_seen, twin_id";

/// Names of the partitions of `measurement` (resolved through the search_path).
const LIST_PARTITIONS: &str = "\
SELECT c.relname::text FROM pg_inherits i JOIN pg_class c ON c.oid = i.inhrelid \
WHERE i.inhparent = 'measurement'::regclass";

/// Key of the advisory lock that serializes partition DDL between sessions (and between servers).
const DDL_LOCK_KEY: i64 = 0x4454_5253_5054_0001;

/// Is the partition table attached to `measurement`? (A table of the same name that was detached is not.)
const PARTITION_ATTACHED: &str = "\
SELECT EXISTS (SELECT 1 FROM pg_inherits i JOIN pg_class c ON c.oid = i.inhrelid \
               WHERE i.inhparent = 'measurement'::regclass AND c.relname::text = $1)";

/// Server side time limits of every connection.
const STATEMENT_TIMEOUT_MS: &str = "30000";
const LOCK_TIMEOUT_MS: &str = "5000";

pub struct PgStore {
    pool: PgPool,
    /// Concurrent `CREATE TABLE ... PARTITION OF` for the same month can fail with a catalog unique violation,
    /// so the DDL of this process is serialized.
    ddl: Mutex<()>,
}

/// Turns a sqlx error into a [`StoreError`] without leaking connection details.
pub fn store_error(context: &str, error: &sqlx::Error) -> StoreError {
    let (kind, detail) = match error {
        sqlx::Error::Database(db) => (
            classify_sqlstate(db.code().as_deref()),
            format!(
                "{} (SQLSTATE {})",
                db.message(),
                db.code().as_deref().unwrap_or("?")
            ),
        ),
        sqlx::Error::Io(_)
        | sqlx::Error::Tls(_)
        | sqlx::Error::PoolTimedOut
        | sqlx::Error::PoolClosed
        | sqlx::Error::WorkerCrashed => (ErrorKind::Transient, error.to_string()),
        other => (ErrorKind::Permanent, other.to_string()),
    };
    StoreError::new(kind, format!("{context}: {detail}"))
}

/// Parses the URL. The error never contains (parts of) the URL.
pub fn parse_options(url: &str) -> Result<PgConnectOptions, StoreError> {
    PgConnectOptions::from_str(url).map_err(|_| {
        StoreError::new(
            ErrorKind::Permanent,
            "the history database URL is invalid (expected postgres://user:password@host:port/database)",
        )
    })
}

impl PgStore {
    /// Connects lazily-checked: fails if the server cannot be reached. `schema` selects the `search_path`
    /// (used by the tests).
    pub async fn connect(
        options: PgConnectOptions,
        schema: Option<&str>,
    ) -> Result<PgStore, StoreError> {
        // `statement_timeout` and `lock_timeout` apply to everything on these connections, including the DDL
        // (a lock wait on a partition drop must not hang the writer).
        let options = options.options([
            ("statement_timeout", STATEMENT_TIMEOUT_MS),
            ("lock_timeout", LOCK_TIMEOUT_MS),
        ]);
        let options = match schema {
            Some(schema) => options.options([("search_path", schema)]),
            None => options,
        };
        // One plain connection first: the pool only reports "timed out" and hides the actual reason (refused,
        // wrong password, unknown database).
        let probe = PgConnection::connect_with(&options)
            .await
            .map_err(|e| store_error("cannot connect to the history database", &e))?;
        let _ = probe.close().await;
        let pool = PgPoolOptions::new()
            .max_connections(4)
            .acquire_timeout(Duration::from_secs(10))
            .connect_with(options)
            .await
            .map_err(|e| store_error("cannot connect to the history database", &e))?;
        Ok(PgStore {
            pool,
            ddl: Mutex::new(()),
        })
    }

    /// Runs the embedded migrations (`migrations/*.sql`).
    pub async fn migrate(&self) -> Result<(), StoreError> {
        sqlx::migrate!("./migrations")
            .run(&self.pool)
            .await
            .map_err(|e| StoreError::new(ErrorKind::Permanent, format!("migration failed: {e}")))
    }

    /// Runs one DDL statement in a transaction holding the DDL advisory lock.
    async fn ddl(&self, sql: String) -> Result<(), sqlx::Error> {
        let mut tx = self.pool.begin().await?;
        sqlx::query("SELECT pg_advisory_xact_lock($1)")
            .bind(DDL_LOCK_KEY)
            .execute(&mut *tx)
            .await?;
        // audited by the callers: statements built from numbers computed by `Month`, never from external text
        sqlx::query(sqlx::AssertSqlSafe(sql))
            .execute(&mut *tx)
            .await?;
        tx.commit().await
    }

    async fn is_attached(&self, month: Month) -> Result<bool, StoreError> {
        sqlx::query(PARTITION_ATTACHED)
            .bind(month.partition_name())
            .fetch_one(&self.pool)
            .await
            .and_then(|row| row.try_get::<bool, _>(0))
            .map_err(|e| store_error("cannot check the partition", &e))
    }

    pub fn pool(&self) -> &PgPool {
        &self.pool
    }
}

impl Store for PgStore {
    async fn insert_batch(&self, rows: &BatchColumns) -> Result<(), StoreError> {
        sqlx::query(INSERT_BATCH)
            .bind(&rows.twin_id)
            .bind(&rows.project_id)
            .bind(&rows.element_path)
            .bind(&rows.ts)
            .bind(&rows.received_at)
            .bind(&rows.value_type)
            .bind(&rows.value_num)
            .bind(&rows.value_text)
            .bind(&rows.value_json)
            .execute(&self.pool)
            .await
            .map(|_| ())
            .map_err(|e| store_error("insert of a measurement batch failed", &e))
    }

    async fn create_partition(&self, month: Month) -> Result<(), StoreError> {
        let _ddl = self.ddl.lock().await;
        match self.ddl(month.create_sql()).await {
            Ok(()) => Ok(()),
            Err(e) => {
                // Another session (a second server on the same database) created it at the same moment:
                // that is fine if the partition is attached now.
                let race = matches!(&e, sqlx::Error::Database(db)
                    if is_partition_race(db.code().as_deref()));
                if race && self.is_attached(month).await? {
                    Ok(())
                } else {
                    Err(store_error("cannot create a partition", &e))
                }
            }
        }
    }

    async fn list_partitions(&self) -> Result<Vec<Month>, StoreError> {
        let rows = sqlx::query(LIST_PARTITIONS)
            .fetch_all(&self.pool)
            .await
            .map_err(|e| store_error("cannot list the partitions", &e))?;
        let mut months = Vec::new();
        for row in rows {
            let name: String = row
                .try_get(0)
                .map_err(|e| store_error("cannot read a partition name", &e))?;
            // partitions created by somebody else (other names) are left alone
            if let Some(month) = Month::from_partition_name(&name) {
                months.push(month);
            }
        }
        months.sort();
        Ok(months)
    }

    async fn drop_partition(&self, month: Month) -> Result<(), StoreError> {
        let _ddl = self.ddl.lock().await;
        self.ddl(month.drop_sql())
            .await
            .map_err(|e| store_error("cannot drop a partition", &e))
    }

    async fn upsert_twin(&self, twin: &TwinRecord) -> Result<(), StoreError> {
        sqlx::query(UPSERT_TWIN)
            .bind(twin.twin_id)
            .bind(twin.project_id)
            .bind(&twin.name)
            .execute(&self.pool)
            .await
            .map(|_| ())
            .map_err(|e| store_error("cannot store the twin", &e))
    }

    async fn load_twins(&self) -> Result<Vec<TwinRecord>, StoreError> {
        let rows = sqlx::query_as::<_, (Uuid, Uuid, String)>(LOAD_TWINS)
            .fetch_all(&self.pool)
            .await
            .map_err(|e| store_error("cannot load the twin registry", &e))?;
        Ok(rows
            .into_iter()
            .map(|(twin_id, project_id, name)| TwinRecord {
                twin_id,
                project_id,
                name,
            })
            .collect())
    }

    async fn close(&self) {
        self.pool.close().await;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn invalid_url_error_does_not_echo_the_url() {
        let err = parse_options("not a url with secret-password").unwrap_err();
        assert!(!err.message.contains("secret-password"));
        let ok = parse_options("postgres://user:secret@localhost:5432/db").unwrap();
        assert_eq!(ok.get_host(), "localhost");
        assert_eq!(ok.get_database(), Some("db"));
    }

    /// The real PostgreSQL parser (libpg_query) accepts every statement. This checks the syntax only; the
    /// statements are executed by the database tests (`DT_TEST_DATABASE_URL`).
    #[cfg(feature = "sql-syntax-check")]
    #[test]
    fn all_sql_is_syntactically_valid_postgresql() {
        let migration = include_str!("../../migrations/0001_init.sql");
        let parsed = pg_query::parse(migration).expect("migration parses");
        assert_eq!(
            parsed.protobuf.stmts.len(),
            4,
            "twin, unique index, measurement, index"
        );
        for sql in [
            INSERT_BATCH,
            UPSERT_TWIN,
            LOAD_TWINS,
            LIST_PARTITIONS,
            PARTITION_ATTACHED,
            "SELECT pg_advisory_xact_lock($1)",
        ] {
            pg_query::parse(sql).unwrap_or_else(|e| panic!("{e}: {sql}"));
        }
        for (year, month) in [(2026, 11), (2026, 12), (2027, 1), (2000, 1)] {
            let month = Month::new(year, month).unwrap();
            pg_query::parse(&month.create_sql()).expect("partition DDL parses");
            pg_query::parse(&month.drop_sql()).expect("drop DDL parses");
        }
    }

    /// PostgreSQL requires every unique constraint (and primary key) of a partitioned table to contain the
    /// partition key; `measurement` has none, and the registry table is not partitioned.
    #[test]
    fn measurement_has_no_unique_constraints() {
        let migration = include_str!("../../migrations/0001_init.sql");
        let measurement = migration
            .split("CREATE TABLE measurement")
            .nth(1)
            .and_then(|rest| rest.split(';').next())
            .expect("measurement definition");
        let sql = measurement.to_uppercase();
        assert!(sql.contains("PARTITION BY RANGE (TS)"));
        assert!(!sql.contains("PRIMARY KEY") && !sql.contains("UNIQUE"));
    }

    #[test]
    fn insert_statement_binds_nine_arrays_in_one_statement() {
        // one INSERT ... SELECT FROM UNNEST, not one statement per row
        assert_eq!(INSERT_BATCH.matches("INSERT INTO").count(), 1);
        for n in 1..=9 {
            assert!(INSERT_BATCH.contains(&format!("${n}::")), "parameter {n}");
        }
        assert!(!INSERT_BATCH.contains("$10"));
        assert!(INSERT_BATCH.contains("t.value_json::jsonb"));
    }
}
