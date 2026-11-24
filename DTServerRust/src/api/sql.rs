//! The SQL of the REST API. Every statement only reads. Table names are unqualified, so the schema is whatever
//! the `search_path` of the connection selects (`public` in production; the tests use their own schema).
//!
//! The statements are runtime-checked (`sqlx::query`), so the build needs no database. With the feature
//! `sql-syntax-check` a test parses them (and the schema file) with PostgreSQL's own parser.

pub const PROJECTS: &str = "\
SELECT project_id, name, first_seen FROM project ORDER BY first_seen, project_id";

pub const PROJECT_EXISTS: &str = "SELECT 1 FROM project WHERE project_id = $1";

pub const TWINS_OF_PROJECT: &str = "\
SELECT twin_id, name, first_seen, last_seen FROM twin WHERE project_id = $1 ORDER BY first_seen, twin_no";

pub const TWIN: &str = "SELECT twin_no, name FROM twin WHERE twin_id = $1";

/// Loaded commits of a twin, newest first.
pub const VERSIONS: &str = "\
SELECT commit_id, first_loaded, last_loaded FROM model_version WHERE twin_no = $1 \
ORDER BY last_loaded DESC, version_no DESC";

/// The version a request refers to: the commit `$2`, or (`$2` is NULL) the one loaded last.
pub const VERSION: &str = "\
SELECT version_no, commit_id FROM model_version \
WHERE twin_no = $1 AND ($2::uuid IS NULL OR commit_id = $2) \
ORDER BY last_loaded DESC, version_no DESC LIMIT 1";

pub const ELEMENTS: &str = "\
SELECT element_no, parent_no, kind, name, path, value_type FROM element WHERE version_no = $1 \
ORDER BY element_no";

/// State of a twin at `$3`. The checkpoint decides everything: the newest checkpoint at or before `$3` (ties:
/// the larger `change_no`) of any version of the twin `$1`, or only of the commit `$2` if given. That fixes the
/// version, and the state is the last row of every variable of that version from the checkpoint up to `$3`.
/// `$4` is NULL or a path: only that element and everything below it. `kind >= 4` are the variables.
///
/// The checkpoint (`taken_at`, `commit_id`) is returned in every row, from the same statement as the values, so
/// it is always the one the values were built from. A checkpoint without matching variables gives one row with
/// NULLs in the variable columns; no checkpoint gives no row.
///
/// The writer guarantees that `received_at` is monotonic per version and that `change_no` grows in write
/// order, which is what makes the lower bounds on both columns equivalent to "written after the checkpoint".
pub const STATE: &str = "WITH cp AS (     SELECT c.version_no, c.taken_at, c.change_no, mv.commit_id     FROM checkpoint c JOIN model_version mv ON mv.version_no = c.version_no     WHERE mv.twin_no = $1 AND ($2::uuid IS NULL OR mv.commit_id = $2) AND c.taken_at <= $3     ORDER BY c.taken_at DESC, c.change_no DESC LIMIT 1 ), latest AS (     SELECT DISTINCT ON (m.element_no) m.element_no, m.value_num, m.value_text, m.value_json,            m.received_at, m.ts, m.source     FROM measurement m, cp     WHERE m.element_no IN (SELECT element_no FROM element WHERE version_no = cp.version_no AND kind >= 4                            AND ($4::text IS NULL OR path = $4 OR starts_with(path, $4 || '/')))       AND m.received_at >= cp.taken_at AND m.received_at <= $3       AND m.change_no >= cp.change_no     ORDER BY m.element_no, m.change_no DESC ) SELECT cp.taken_at, cp.commit_id, e.path, e.kind, e.value_type, l.value_num, l.value_text, l.value_json,        l.received_at, l.ts, l.source FROM cp LEFT JOIN latest l ON true LEFT JOIN element e ON e.element_no = l.element_no ORDER BY e.path";

/// Does the twin have a variable (kind >= 4) of this path (`$2`), in the commit `$3` or (NULL) in any?
pub const VARIABLE_EXISTS: &str = "\
SELECT EXISTS ( \
    SELECT 1 FROM model_version mv JOIN element e ON e.version_no = mv.version_no \
    WHERE mv.twin_no = $1 AND e.path = $2 AND e.kind >= 4 AND ($3::uuid IS NULL OR mv.commit_id = $3))";

/// Time series of the variable `$2` of the twin `$1`, over all versions or only the commit `$3`. `$4` / `$5`
/// bound `received_at` (inclusive, NULL = open), `$6` is the number of rows to return. `$7`..`$10` are the cursor
/// (all NULL for the first page): only rows after that row in the order `(received_at, change_no, element_no,
/// source)` are returned. The order is total for the rows of one variable, so pages neither repeat nor skip rows
/// that share a `received_at`. The plain bound on `received_at` next to the row comparison is redundant, but it is
/// what partition pruning and the index on `(element_no, received_at)` can use, so deep pages do not re-read the
/// history.
pub const SERIES: &str = "SELECT m.received_at, m.ts, m.change_no, m.element_no, m.source, m.value_num, m.value_text, m.value_json,        e.value_type, mv.commit_id FROM model_version mv JOIN element e ON e.version_no = mv.version_no AND e.path = $2 AND e.kind >= 4 JOIN measurement m ON m.element_no = e.element_no WHERE mv.twin_no = $1 AND ($3::uuid IS NULL OR mv.commit_id = $3)   AND ($4::timestamptz IS NULL OR m.received_at >= $4)   AND ($5::timestamptz IS NULL OR m.received_at <= $5)   AND ($7::timestamptz IS NULL OR m.received_at >= $7)   AND ($7::timestamptz IS NULL OR (m.received_at, m.change_no, m.element_no, m.source) > ($7, $8::bigint, $9::integer, $10::smallint)) ORDER BY m.received_at ASC, m.change_no ASC, m.element_no ASC, m.source ASC LIMIT $6";

#[cfg(test)]
mod tests {
    use super::*;

    const ALL: [&str; 10] = [
        PROJECTS,
        PROJECT_EXISTS,
        TWINS_OF_PROJECT,
        TWIN,
        VERSIONS,
        VERSION,
        ELEMENTS,
        STATE,
        VARIABLE_EXISTS,
        SERIES,
    ];

    /// The API never writes.
    #[test]
    fn statements_only_read() {
        for sql in ALL {
            let upper = sql.to_uppercase();
            assert!(
                upper.starts_with("SELECT") || upper.starts_with("WITH"),
                "{sql}"
            );
            for word in [
                "INSERT", "UPDATE", "DELETE", "CREATE", "DROP", "ALTER", "TRUNCATE", "GRANT",
                "COPY", "NEXTVAL",
            ] {
                assert!(
                    !upper
                        .split(|c: char| !c.is_ascii_alphanumeric() && c != '_')
                        .any(|w| w == word),
                    "{word} in {sql}"
                );
            }
        }
    }

    /// The highest parameter of each statement is the number of values it is bound with in `mod.rs`.
    #[test]
    fn parameter_numbers_are_contiguous() {
        let highest = |sql: &str| {
            (1..=10)
                .rev()
                .find(|n| sql.contains(&format!("${n}")))
                .unwrap_or(0)
        };
        let expected = [
            (PROJECTS, 0),
            (PROJECT_EXISTS, 1),
            (TWINS_OF_PROJECT, 1),
            (TWIN, 1),
            (VERSIONS, 1),
            (VERSION, 2),
            (ELEMENTS, 1),
            (STATE, 4),
            (VARIABLE_EXISTS, 3),
            (SERIES, 10),
        ];
        for (sql, n) in expected {
            assert_eq!(highest(sql), n, "{sql}");
            for i in 1..=n {
                assert!(sql.contains(&format!("${i}")), "${i} missing in {sql}");
            }
        }
    }

    /// The real PostgreSQL parser (libpg_query) accepts the schema and every statement of the API. This checks
    /// the syntax only; the statements are executed by the database tests (`DT_TEST_DATABASE_URL`).
    #[cfg(feature = "sql-syntax-check")]
    #[test]
    fn schema_and_api_sql_are_valid_postgresql() {
        let schema = include_str!("../../../DigitalTwinStore/sql/0001_schema.sql");
        let parsed = pg_query::parse(schema).expect("the schema parses");
        assert_eq!(
            parsed.protobuf.stmts.len(),
            9,
            "4 tables, checkpoint + index, sequence, measurement + index"
        );
        for sql in ALL {
            pg_query::parse(sql).unwrap_or_else(|e| panic!("{e}: {sql}"));
        }
    }
}
