//! Read-only REST API over the history database that the C++ core writes (`DigitalTwinStore`, schema in
//! `DigitalTwinStore/sql/0001_schema.sql`). See the README for the routes.
//!
//! - The API never writes. It should connect with a database role that may only `SELECT`; as a second line of
//!   defence every connection is also opened with `default_transaction_read_only = on`.
//! - Queries are runtime-checked (`sqlx::query`, no compile-time macros): the build needs no database.
//! - Errors are JSON, `{"error": "..."}`. A message never contains SQL, the database URL or a driver error; those
//!   are only logged.
//! - With MQTT authentication enabled the routes need HTTP Basic credentials (see [`auth`]); otherwise they are
//!   open and the API has to be bound to localhost or put behind a reverse proxy.
//! - The API documents itself: `/docs/` is a Swagger UI page and `/openapi.yaml` the OpenAPI document, both embedded
//!   in the binary (see [`docs`]) and behind the same authentication as the data routes.

pub mod auth;
pub mod docs;
pub mod model;
pub mod sql;

use std::net::SocketAddr;
use std::str::FromStr;
use std::sync::Arc;
use std::time::Duration;

use axum::extract::{Path, RawQuery, State};
use axum::http::{header, HeaderValue, StatusCode};
use axum::middleware;
use axum::response::{IntoResponse, Response};
use axum::routing::get;
use axum::{Json, Router};
use chrono::{DateTime, Utc};
use serde::Serialize;
use serde_json::Value;
use sqlx::postgres::{PgConnectOptions, PgPool, PgPoolOptions, PgRow};
use sqlx::Row as _;
use thiserror::Error;
use tokio::sync::oneshot;
use tokio::task::JoinHandle;
use tracing::{debug, error, warn};
use uuid::Uuid;

pub use auth::BasicAuth;
use model::{
    decode_value, format_time, kind_word, parse_uuid, source_word, value_type_word, Cursor,
    ElementsParams, SeriesParams, StateParams,
};

/// Time a single statement may run, enforced by the database.
const STATEMENT_TIMEOUT_MS: &str = "15000";
/// Connections of the pool.
const MAX_CONNECTIONS: u32 = 8;
/// Time a request waits for a free connection.
const ACQUIRE_TIMEOUT: Duration = Duration::from_secs(5);

/// Why a request was not answered with data.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ApiError {
    /// 400
    BadRequest(String),
    /// 404
    NotFound(String),
    /// 401: credentials are missing or wrong.
    Unauthorized,
    /// 503: the authentication service cannot decide.
    AuthUnavailable,
    /// 503: the database cannot be reached (or is shutting down, or has no free connection).
    DatabaseUnavailable,
    /// 503: a query hit the statement timeout.
    QueryTimeout,
    /// 500: anything else; the details are in the log.
    Internal,
}

impl ApiError {
    pub fn bad_request(message: impl Into<String>) -> Self {
        Self::BadRequest(message.into())
    }

    pub fn not_found(message: impl Into<String>) -> Self {
        Self::NotFound(message.into())
    }

    pub fn status(&self) -> StatusCode {
        match self {
            Self::BadRequest(_) => StatusCode::BAD_REQUEST,
            Self::NotFound(_) => StatusCode::NOT_FOUND,
            Self::Unauthorized => StatusCode::UNAUTHORIZED,
            Self::AuthUnavailable | Self::DatabaseUnavailable | Self::QueryTimeout => {
                StatusCode::SERVICE_UNAVAILABLE
            }
            Self::Internal => StatusCode::INTERNAL_SERVER_ERROR,
        }
    }

    /// The text of the `error` field.
    pub fn message(&self) -> &str {
        match self {
            Self::BadRequest(message) | Self::NotFound(message) => message,
            Self::Unauthorized => "authentication required: user name or password missing or wrong",
            Self::AuthUnavailable => "the authentication service is not available",
            Self::DatabaseUnavailable => "the history database is not available",
            Self::QueryTimeout => "the query took too long; narrow it down (path, from, to, limit)",
            Self::Internal => "internal error",
        }
    }
}

impl From<sqlx::Error> for ApiError {
    /// The driver error is logged, never returned: it can name tables, columns and hosts.
    fn from(e: sqlx::Error) -> Self {
        match e {
            sqlx::Error::Io(_)
            | sqlx::Error::Tls(_)
            | sqlx::Error::PoolTimedOut
            | sqlx::Error::PoolClosed
            | sqlx::Error::WorkerCrashed => {
                warn!("history database not available: {e}");
                Self::DatabaseUnavailable
            }
            sqlx::Error::Database(ref db) => {
                // class 08 connection exception, 53 insufficient resources, 57P0x shutdown of the server
                let code = db.code().map(|c| c.into_owned()).unwrap_or_default();
                if code == "57014" {
                    warn!("query canceled by the statement timeout: {e}");
                    Self::QueryTimeout
                } else if code.starts_with("08")
                    || code.starts_with("53")
                    || code.starts_with("57P0")
                {
                    warn!("history database not available: {e}");
                    Self::DatabaseUnavailable
                } else {
                    error!("database error in the REST API: {e}");
                    Self::Internal
                }
            }
            other => {
                error!("database error in the REST API: {other}");
                Self::Internal
            }
        }
    }
}

impl IntoResponse for ApiError {
    fn into_response(self) -> Response {
        let body = serde_json::json!({ "error": self.message() });
        let mut response = (self.status(), Json(body)).into_response();
        if self == ApiError::Unauthorized {
            response.headers_mut().insert(
                header::WWW_AUTHENTICATE,
                HeaderValue::from_static(r#"Basic realm="dt-history", charset="UTF-8""#),
            );
        }
        response
    }
}

type ApiResult<T> = Result<Json<T>, ApiError>;

#[derive(Clone)]
struct ApiState {
    pool: PgPool,
}

/// Problems while starting the API (the configuration is wrong or the port is taken). Never contains the
/// database URL.
#[derive(Debug, Error)]
pub enum ApiStartError {
    #[error("run.api.listen {0:?} is not a socket address (expected host:port, for example 127.0.0.1:8080)")]
    InvalidListen(String),
    #[error("cannot listen on {addr}: {source}")]
    Bind {
        addr: SocketAddr,
        source: std::io::Error,
    },
    #[error(
        "the API database URL is invalid (expected postgres://user:password@host:port/database)"
    )]
    InvalidDatabaseUrl,
}

/// The pool of the API. Connects lazily, so the API starts (and answers 503) while the database is down. Every
/// connection is read-only and has a statement timeout. `schema` sets the `search_path` (used by the tests).
pub fn connect_lazy(url: &str, schema: Option<&str>) -> Result<PgPool, ApiStartError> {
    let options = PgConnectOptions::from_str(url).map_err(|_| ApiStartError::InvalidDatabaseUrl)?;
    let options = options.options([
        ("statement_timeout", STATEMENT_TIMEOUT_MS),
        ("default_transaction_read_only", "on"),
    ]);
    let options = match schema {
        Some(schema) => options.options([("search_path", schema)]),
        None => options,
    };
    Ok(PgPoolOptions::new()
        .max_connections(MAX_CONNECTIONS)
        .acquire_timeout(ACQUIRE_TIMEOUT)
        .connect_lazy_with(options))
}

/// The routes. With `auth` every request (including unknown routes) needs valid Basic credentials.
pub fn router(pool: PgPool, auth: Option<BasicAuth>) -> Router {
    let router = Router::new()
        .route("/projects", get(list_projects))
        .route("/projects/{project_id}/twins", get(list_twins))
        .route("/twins/{twin_id}/versions", get(list_versions))
        .route("/twins/{twin_id}/elements", get(list_elements))
        .route("/twins/{twin_id}/state", get(get_state))
        .route("/twins/{twin_id}/series", get(get_series))
        .merge(docs::routes())
        .fallback(|| async { ApiError::not_found("no such route") })
        .method_not_allowed_fallback(|| async {
            (
                StatusCode::METHOD_NOT_ALLOWED,
                Json(serde_json::json!({ "error": "the API is read-only: only GET is allowed" })),
            )
        })
        .with_state(ApiState { pool });
    let router = match auth {
        Some(auth) => router.layer(middleware::from_fn_with_state(
            Arc::new(auth),
            auth::require_basic,
        )),
        None => router,
    };
    // outermost: errors of axum itself (a path segment that is not UTF-8, ...) are plain text; make them JSON
    router.layer(middleware::map_response(json_errors))
}

/// Turns an error response that is not JSON (they come from axum's extractors) into the usual JSON body. The
/// original text is dropped: it can echo the request.
async fn json_errors(response: Response) -> Response {
    let status = response.status();
    let is_json = response
        .headers()
        .get(header::CONTENT_TYPE)
        .and_then(|v| v.to_str().ok())
        .is_some_and(|v| v.starts_with("application/json"));
    if (status.is_client_error() || status.is_server_error()) && !is_json {
        let message = status.canonical_reason().unwrap_or("error").to_lowercase();
        return (status, Json(serde_json::json!({ "error": message }))).into_response();
    }
    response
}

/// The running API server.
pub struct ApiService {
    addr: SocketAddr,
    stop: oneshot::Sender<()>,
    task: JoinHandle<()>,
}

impl ApiService {
    /// Binds `listen`, connects (lazily) to the database and serves in the background.
    pub async fn start(
        listen: &str,
        database_url: &str,
        auth: Option<BasicAuth>,
    ) -> Result<ApiService, ApiStartError> {
        let addr: SocketAddr = listen
            .trim()
            .parse()
            .map_err(|_| ApiStartError::InvalidListen(listen.to_owned()))?;
        let pool = connect_lazy(database_url, None)?;
        let listener = tokio::net::TcpListener::bind(addr)
            .await
            .map_err(|source| ApiStartError::Bind { addr, source })?;
        let addr = listener.local_addr().unwrap_or(addr);
        let app = router(pool, auth).into_make_service_with_connect_info::<SocketAddr>();
        let (stop, stopped) = oneshot::channel::<()>();
        let task = tokio::spawn(async move {
            let served = axum::serve(listener, app).with_graceful_shutdown(async {
                let _ = stopped.await;
            });
            if let Err(e) = served.await {
                error!("the REST API server ended with an error: {e}");
            }
        });
        Ok(ApiService { addr, stop, task })
    }

    /// The address the API listens on.
    pub fn local_addr(&self) -> SocketAddr {
        self.addr
    }

    /// Stops accepting connections and waits (at most 5 seconds) for running requests.
    pub async fn shutdown(self) {
        let _ = self.stop.send(());
        if tokio::time::timeout(Duration::from_secs(5), self.task)
            .await
            .is_err()
        {
            warn!("the REST API did not stop within 5 seconds");
        }
    }
}

// ---------------------------------------------------------------------------------------------
// Responses
// ---------------------------------------------------------------------------------------------

#[derive(Serialize)]
struct ProjectOut {
    project_id: Uuid,
    name: Option<String>,
    first_seen: String,
}

#[derive(Serialize)]
struct TwinOut {
    twin_id: Uuid,
    name: String,
    first_seen: String,
    last_seen: String,
}

#[derive(Serialize)]
struct VersionOut {
    commit: Uuid,
    first_loaded: String,
    last_loaded: String,
}

#[derive(Serialize)]
struct ElementOut {
    element_no: i32,
    parent_no: Option<i32>,
    kind: &'static str,
    name: String,
    path: String,
    value_type: Option<&'static str>,
}

#[derive(Serialize)]
struct ElementsOut {
    twin_id: Uuid,
    commit: Uuid,
    elements: Vec<ElementOut>,
}

#[derive(Serialize)]
struct StateEntryOut {
    path: String,
    kind: &'static str,
    value_type: Option<&'static str>,
    value: Value,
    received_at: String,
    ts: String,
    source: &'static str,
}

#[derive(Serialize)]
struct StateOut {
    twin_id: Uuid,
    commit: Uuid,
    /// The time the state is for.
    at: String,
    /// The checkpoint the state was built from.
    checkpoint_at: String,
    variables: Vec<StateEntryOut>,
}

#[derive(Serialize)]
struct PointOut {
    commit: Uuid,
    received_at: String,
    /// Number of the update that wrote the row; with `received_at` it orders rows that arrived in the same instant.
    change_no: i64,
    ts: String,
    source: &'static str,
    value_type: Option<&'static str>,
    value: Value,
}

#[derive(Serialize)]
struct SeriesOut {
    twin_id: Uuid,
    path: String,
    limit: i64,
    /// More rows than `limit` exist in the requested range; continue with `next`.
    more: bool,
    /// With `more`: pass it as `after` to get the next page, without skipping or repeating rows. Otherwise null.
    next: Option<String>,
    points: Vec<PointOut>,
}

// ---------------------------------------------------------------------------------------------
// Handlers
// ---------------------------------------------------------------------------------------------

fn time(row: &PgRow, column: &str) -> Result<String, sqlx::Error> {
    row.try_get::<DateTime<Utc>, _>(column).map(format_time)
}

/// The decoded value of a row with the columns `value_num`, `value_text`, `value_json` and the given type.
fn value(row: &PgRow, value_type: Option<i16>) -> Result<Value, sqlx::Error> {
    Ok(decode_value(
        value_type,
        row.try_get("value_num")?,
        row.try_get::<Option<String>, _>("value_text")?.as_deref(),
        row.try_get("value_json")?,
    ))
}

async fn list_projects(State(state): State<ApiState>) -> ApiResult<Vec<ProjectOut>> {
    let rows = sqlx::query(sql::PROJECTS).fetch_all(&state.pool).await?;
    let projects = rows
        .iter()
        .map(|row| {
            Ok(ProjectOut {
                project_id: row.try_get("project_id")?,
                name: row.try_get("name")?,
                first_seen: time(row, "first_seen")?,
            })
        })
        .collect::<Result<_, sqlx::Error>>()?;
    Ok(Json(projects))
}

async fn list_twins(
    State(state): State<ApiState>,
    Path(project_id): Path<String>,
) -> ApiResult<Vec<TwinOut>> {
    let project_id = parse_uuid(&project_id, "project_id")?;
    let exists = sqlx::query(sql::PROJECT_EXISTS)
        .bind(project_id)
        .fetch_optional(&state.pool)
        .await?;
    if exists.is_none() {
        return Err(ApiError::not_found("unknown project"));
    }
    let rows = sqlx::query(sql::TWINS_OF_PROJECT)
        .bind(project_id)
        .fetch_all(&state.pool)
        .await?;
    let twins = rows
        .iter()
        .map(|row| {
            Ok(TwinOut {
                twin_id: row.try_get("twin_id")?,
                name: row.try_get("name")?,
                first_seen: time(row, "first_seen")?,
                last_seen: time(row, "last_seen")?,
            })
        })
        .collect::<Result<_, sqlx::Error>>()?;
    Ok(Json(twins))
}

/// `twin_no` of a twin id, or 404.
async fn find_twin(pool: &PgPool, twin_id: &str) -> Result<(Uuid, i32), ApiError> {
    let twin_id = parse_uuid(twin_id, "twin_id")?;
    let row = sqlx::query(sql::TWIN)
        .bind(twin_id)
        .fetch_optional(pool)
        .await?
        .ok_or_else(|| ApiError::not_found("unknown twin"))?;
    Ok((twin_id, row.try_get("twin_no")?))
}

/// `(version_no, commit)`: the commit asked for, or else the one loaded last.
async fn find_version(
    pool: &PgPool,
    twin_no: i32,
    commit: Option<Uuid>,
) -> Result<(i32, Uuid), ApiError> {
    let row = sqlx::query(sql::VERSION)
        .bind(twin_no)
        .bind(commit)
        .fetch_optional(pool)
        .await?
        .ok_or_else(|| {
            ApiError::not_found(if commit.is_some() {
                "the twin has no such commit"
            } else {
                "the twin has no loaded version yet"
            })
        })?;
    Ok((row.try_get("version_no")?, row.try_get("commit_id")?))
}

async fn list_versions(
    State(state): State<ApiState>,
    Path(twin_id): Path<String>,
) -> ApiResult<Vec<VersionOut>> {
    let (_, twin_no) = find_twin(&state.pool, &twin_id).await?;
    let rows = sqlx::query(sql::VERSIONS)
        .bind(twin_no)
        .fetch_all(&state.pool)
        .await?;
    let versions = rows
        .iter()
        .map(|row| {
            Ok(VersionOut {
                commit: row.try_get("commit_id")?,
                first_loaded: time(row, "first_loaded")?,
                last_loaded: time(row, "last_loaded")?,
            })
        })
        .collect::<Result<_, sqlx::Error>>()?;
    Ok(Json(versions))
}

async fn list_elements(
    State(state): State<ApiState>,
    Path(twin_id): Path<String>,
    RawQuery(query): RawQuery,
) -> ApiResult<ElementsOut> {
    let params = ElementsParams::parse(query.as_deref())?;
    let (twin_id, twin_no) = find_twin(&state.pool, &twin_id).await?;
    let (version_no, commit) = find_version(&state.pool, twin_no, params.commit).await?;
    let rows = sqlx::query(sql::ELEMENTS)
        .bind(version_no)
        .fetch_all(&state.pool)
        .await?;
    let elements = rows
        .iter()
        .map(|row| {
            Ok(ElementOut {
                element_no: row.try_get("element_no")?,
                parent_no: row.try_get("parent_no")?,
                kind: kind_word(row.try_get("kind")?),
                name: row.try_get("name")?,
                path: row.try_get("path")?,
                value_type: value_type_word(row.try_get("value_type")?),
            })
        })
        .collect::<Result<_, sqlx::Error>>()?;
    Ok(Json(ElementsOut {
        twin_id,
        commit,
        elements,
    }))
}

async fn get_state(
    State(state): State<ApiState>,
    Path(twin_id): Path<String>,
    RawQuery(query): RawQuery,
) -> ApiResult<StateOut> {
    let params = StateParams::parse(query.as_deref(), Utc::now())?;
    let (twin_id, twin_no) = find_twin(&state.pool, &twin_id).await?;
    if params.commit.is_some() {
        // an unknown commit is a 404 of its own
        find_version(&state.pool, twin_no, params.commit).await?;
    }
    // One statement decides the version, the checkpoint and the values. Without `commit` the newest checkpoint of
    // any version at or before `at` is the one that was valid then, whatever was loaded since (a reload of an old
    // commit sets its `last_loaded` again, which says nothing about the time asked for).
    let rows = sqlx::query(sql::STATE)
        .bind(twin_no)
        .bind(params.commit)
        .bind(params.at)
        .bind(params.path.as_deref())
        .fetch_all(&state.pool)
        .await?;
    let Some(first) = rows.first() else {
        // The state is only defined from a checkpoint on: the twin was loaded later, or the data of that time was
        // removed by the retention.
        // only a missing version is a 404; a database error of the follow-up query is passed on as it is
        let message = match find_version(&state.pool, twin_no, None).await {
            Ok(_) => "no state is stored for this time: there is no checkpoint at or before it",
            Err(ApiError::NotFound(_)) => "the twin has no loaded version yet",
            Err(other) => return Err(other),
        };
        return Err(ApiError::not_found(message));
    };
    let commit: Uuid = first.try_get("commit_id")?;
    let checkpoint_at = time(first, "taken_at")?;
    let mut variables = Vec::new();
    for row in &rows {
        // a checkpoint without variables gives one row without a path
        let Some(path) = row.try_get::<Option<String>, _>("path")? else {
            continue;
        };
        let value_type: Option<i16> = row.try_get("value_type")?;
        variables.push(StateEntryOut {
            path,
            kind: kind_word(row.try_get("kind")?),
            value_type: value_type_word(value_type),
            value: value(row, value_type)?,
            received_at: time(row, "received_at")?,
            ts: time(row, "ts")?,
            source: source_word(row.try_get("source")?),
        });
    }
    Ok(Json(StateOut {
        twin_id,
        commit,
        at: format_time(params.at),
        checkpoint_at,
        variables,
    }))
}

async fn get_series(
    State(state): State<ApiState>,
    Path(twin_id): Path<String>,
    RawQuery(query): RawQuery,
) -> ApiResult<SeriesOut> {
    let params = SeriesParams::parse(query.as_deref())?;
    let (twin_id, twin_no) = find_twin(&state.pool, &twin_id).await?;
    if let Some(commit) = params.commit {
        find_version(&state.pool, twin_no, Some(commit)).await?;
    }
    let known: bool = sqlx::query(sql::VARIABLE_EXISTS)
        .bind(twin_no)
        .bind(&params.path)
        .bind(params.commit)
        .fetch_one(&state.pool)
        .await?
        .try_get(0)?;
    if !known {
        return Err(ApiError::not_found(
            "the twin has no variable with this path",
        ));
    }
    // one row more than asked for tells whether there are more
    let mut rows = sqlx::query(sql::SERIES)
        .bind(twin_no)
        .bind(&params.path)
        .bind(params.commit)
        .bind(params.from)
        .bind(params.to)
        .bind(params.limit + 1)
        .bind(params.after.map(|c| c.received_at))
        .bind(params.after.map(|c| c.change_no))
        .bind(params.after.map(|c| c.element_no))
        .bind(params.after.map(|c| c.source))
        .fetch_all(&state.pool)
        .await?;
    let more = rows.len() as i64 > params.limit;
    rows.truncate(params.limit as usize);
    let next = match rows.last() {
        Some(last) if more => Some(
            Cursor {
                received_at: last.try_get("received_at")?,
                change_no: last.try_get("change_no")?,
                element_no: last.try_get("element_no")?,
                source: last.try_get("source")?,
            }
            .format(),
        ),
        _ => None,
    };
    let points = rows
        .iter()
        .map(|row| {
            let value_type: Option<i16> = row.try_get("value_type")?;
            Ok(PointOut {
                commit: row.try_get("commit_id")?,
                received_at: time(row, "received_at")?,
                change_no: row.try_get("change_no")?,
                ts: time(row, "ts")?,
                source: source_word(row.try_get("source")?),
                value_type: value_type_word(value_type),
                value: value(row, value_type)?,
            })
        })
        .collect::<Result<_, sqlx::Error>>()?;
    debug!(points = rows.len(), more, "series answered");
    Ok(Json(SeriesOut {
        twin_id,
        path: params.path,
        limit: params.limit,
        more,
        next,
        points,
    }))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn errors_map_to_status_and_json() {
        for (error, status) in [
            (ApiError::bad_request("x"), 400),
            (ApiError::not_found("x"), 404),
            (ApiError::Unauthorized, 401),
            (ApiError::AuthUnavailable, 503),
            (ApiError::DatabaseUnavailable, 503),
            (ApiError::QueryTimeout, 503),
            (ApiError::Internal, 500),
        ] {
            assert_eq!(error.status().as_u16(), status, "{error:?}");
            assert!(!error.message().is_empty());
        }
    }

    #[test]
    fn driver_errors_do_not_reach_the_client() {
        let leaking =
            sqlx::Error::Protocol("relation \"measurement\" ... postgres://u:pw@h/db".into());
        let error = ApiError::from(leaking);
        assert_eq!(error, ApiError::Internal);
        assert!(!error.message().contains("postgres://"));
        assert_eq!(
            ApiError::from(sqlx::Error::PoolTimedOut),
            ApiError::DatabaseUnavailable
        );
    }

    #[test]
    fn invalid_urls_are_not_echoed() {
        let error = connect_lazy("not a url with secret-password", None).unwrap_err();
        assert!(!error.to_string().contains("secret-password"));
    }

    #[tokio::test]
    async fn invalid_listen_addresses_are_rejected() {
        for bad in ["", "localhost", "8080", "1.2.3.4", "::1:80:x"] {
            let result = ApiService::start(bad, "postgres://u:p@localhost/db", None).await;
            assert!(
                matches!(result, Err(ApiStartError::InvalidListen(_))),
                "{bad:?}"
            );
        }
    }
}
