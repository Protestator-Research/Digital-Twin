//! The REST API without a database: authentication, routing and the validation that happens before any query.
//! The pool points at a port nobody listens on; a request that reaches the database answers 503.

use std::future::Future;
use std::pin::Pin;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;

use axum::body::Body;
use axum::http::{header, Method, Request, StatusCode};
use axum::Router;
use base64::engine::general_purpose::STANDARD;
use base64::Engine as _;
use dt_server::api::{connect_lazy, router, BasicAuth};
use dt_server::auth::{AuthOutcome, Authenticator, LoginThrottle, ThrottleConfig};
use http_body_util::BodyExt;
use tower::ServiceExt;

/// Accepts `admin` / `pw`, is "down" for the user `down`, rejects everyone else. Counts the checks.
#[derive(Default)]
struct Stub {
    checks: AtomicUsize,
}

impl Authenticator for Stub {
    fn verify<'a>(
        &'a self,
        username: &'a str,
        password: &'a [u8],
    ) -> Pin<Box<dyn Future<Output = AuthOutcome> + Send + 'a>> {
        self.checks.fetch_add(1, Ordering::SeqCst);
        let outcome = match (username, password) {
            ("admin", b"pw") => AuthOutcome::Accepted,
            ("down", _) => AuthOutcome::Unavailable("backend down".into()),
            _ => AuthOutcome::Rejected,
        };
        Box::pin(async move { outcome })
    }
}

fn pool() -> sqlx::PgPool {
    connect_lazy("postgres://nobody:secret@127.0.0.1:1/none", None).unwrap()
}

fn open_router() -> Router {
    router(pool(), None)
}

fn protected_router(stub: Arc<Stub>) -> Router {
    protected_router_with(stub, LoginThrottle::new(ThrottleConfig::default()))
}

fn protected_router_with(stub: Arc<Stub>, throttle: Arc<LoginThrottle>) -> Router {
    router(pool(), Some(BasicAuth::new(stub, throttle, false)))
}

fn basic(user: &str, password: &str) -> String {
    format!("Basic {}", STANDARD.encode(format!("{user}:{password}")))
}

async fn call(
    router: &Router,
    method: Method,
    uri: &str,
    authorization: Option<&str>,
) -> (StatusCode, axum::http::HeaderMap, serde_json::Value) {
    call_from(router, method, uri, authorization, None, None).await
}

/// Like `call`, from the peer address `peer` and with the header `X-Forwarded-For: forwarded`.
async fn call_from(
    router: &Router,
    method: Method,
    uri: &str,
    authorization: Option<&str>,
    peer: Option<&str>,
    forwarded: Option<&str>,
) -> (StatusCode, axum::http::HeaderMap, serde_json::Value) {
    let mut request = Request::builder().method(method).uri(uri);
    if let Some(value) = authorization {
        request = request.header(header::AUTHORIZATION, value);
    }
    if let Some(value) = forwarded {
        request = request.header("x-forwarded-for", value);
    }
    if let Some(peer) = peer {
        request = request.extension(axum::extract::ConnectInfo(std::net::SocketAddr::new(
            peer.parse().unwrap(),
            40000,
        )));
    }
    let response = router
        .clone()
        .oneshot(request.body(Body::empty()).unwrap())
        .await
        .unwrap();
    let (parts, body) = response.into_parts();
    let bytes = body.collect().await.unwrap().to_bytes();
    let json = serde_json::from_slice(&bytes)
        .unwrap_or_else(|_| panic!("not JSON: {}", String::from_utf8_lossy(&bytes)));
    (parts.status, parts.headers, json)
}

async fn get(router: &Router, uri: &str) -> (StatusCode, serde_json::Value) {
    let (status, _, json) = call(router, Method::GET, uri, None).await;
    (status, json)
}

const TWIN: &str = "6f9619ff-8b86-d011-b42d-00c04fc964ff";

#[tokio::test]
async fn bad_requests_are_answered_as_json_before_any_query() {
    let router = open_router();
    let cases = [
        "/twins/not-a-uuid/versions".to_owned(),
        "/projects/xyz/twins".to_owned(),
        format!("/twins/{TWIN}/elements?commit=1"),
        format!("/twins/{TWIN}/elements?unknown=1"),
        format!("/twins/{TWIN}/state?at=yesterday"),
        format!("/twins/{TWIN}/state?path=a%00b"),
        format!("/twins/{TWIN}/series"),
        format!("/twins/{TWIN}/series?path=a&limit=0"),
        format!("/twins/{TWIN}/series?path=a&limit=10001"),
        format!("/twins/{TWIN}/series?path=a&from=2026-01-02T00:00:00Z&to=2026-01-01T00:00:00Z"),
    ];
    for uri in cases {
        let (status, json) = get(&router, &uri).await;
        assert_eq!(status, StatusCode::BAD_REQUEST, "{uri}");
        assert!(
            json["error"].as_str().is_some_and(|m| !m.is_empty()),
            "{uri}"
        );
    }
}

#[tokio::test]
async fn unknown_routes_and_methods_are_json_errors() {
    let router = open_router();
    let (status, json) = get(&router, "/nothing").await;
    assert_eq!(status, StatusCode::NOT_FOUND);
    assert!(json["error"].is_string());
    for method in [Method::POST, Method::PUT, Method::DELETE, Method::PATCH] {
        let (status, _, json) = call(&router, method, "/projects", None).await;
        assert_eq!(status, StatusCode::METHOD_NOT_ALLOWED);
        assert!(json["error"].is_string());
    }
}

#[tokio::test]
async fn an_unreachable_database_is_503_without_details() {
    let (status, json) = get(&open_router(), "/projects").await;
    assert_eq!(status, StatusCode::SERVICE_UNAVAILABLE);
    let shown = json.to_string();
    for secret in ["secret", "nobody", "127.0.0.1", "postgres://"] {
        assert!(!shown.contains(secret), "{shown}");
    }
}

#[tokio::test]
async fn requests_need_credentials_when_authentication_is_on() {
    let stub = Arc::new(Stub::default());
    let router = protected_router(Arc::clone(&stub));

    // missing, malformed and wrong credentials: 401 with a challenge, and nothing behind it is reached
    for header_value in [
        None,
        Some("Bearer token".to_owned()),
        Some("Basic !!!".to_owned()),
        Some(basic("admin", "wrong")),
        Some(basic("nobody", "pw")),
    ] {
        let (status, headers, json) =
            call(&router, Method::GET, "/projects", header_value.as_deref()).await;
        assert_eq!(status, StatusCode::UNAUTHORIZED, "{header_value:?}");
        assert!(headers[header::WWW_AUTHENTICATE]
            .to_str()
            .unwrap()
            .starts_with("Basic"));
        assert!(json["error"].is_string());
    }
    // unknown routes are protected as well
    let (status, _, _) = call(&router, Method::GET, "/nothing", None).await;
    assert_eq!(status, StatusCode::UNAUTHORIZED);

    // valid credentials get through: to the validation (400) and to the database (503 here)
    let good = basic("admin", "pw");
    let (status, _, _) = call(&router, Method::GET, "/twins/x/versions", Some(&good)).await;
    assert_eq!(status, StatusCode::BAD_REQUEST);
    let (status, _, _) = call(&router, Method::GET, "/projects", Some(&good)).await;
    assert_eq!(status, StatusCode::SERVICE_UNAVAILABLE);

    // an authentication service that cannot decide is 503, not 401
    let (status, _, json) =
        call(&router, Method::GET, "/projects", Some(&basic("down", "x"))).await;
    assert_eq!(status, StatusCode::SERVICE_UNAVAILABLE);
    assert!(json["error"].as_str().unwrap().contains("authentication"));
}

#[tokio::test]
async fn repeated_failures_are_throttled_like_for_mqtt() {
    let stub = Arc::new(Stub::default());
    let router = protected_router(Arc::clone(&stub));
    let max = ThrottleConfig::default().max_failures;
    assert!(max > 0);
    for _ in 0..max {
        let (status, _, _) = call(
            &router,
            Method::GET,
            "/projects",
            Some(&basic("mallory", "x")),
        )
        .await;
        assert_eq!(status, StatusCode::UNAUTHORIZED);
    }
    let checks = stub.checks.load(Ordering::SeqCst);
    assert_eq!(checks, max as usize);
    // blocked now: answered without asking the backend again
    let (status, _, _) = call(
        &router,
        Method::GET,
        "/projects",
        Some(&basic("mallory", "x")),
    )
    .await;
    assert_eq!(status, StatusCode::UNAUTHORIZED);
    assert_eq!(stub.checks.load(Ordering::SeqCst), checks);
}

/// The throttle is the one of the broker: a user name blocked there (over MQTT) is blocked for the API.
#[tokio::test]
async fn a_user_blocked_over_mqtt_is_blocked_for_the_api() {
    let stub = Arc::new(Stub::default());
    let throttle = LoginThrottle::new(ThrottleConfig::default());
    for _ in 0..ThrottleConfig::default().max_failures {
        throttle.record_failure("admin", "10.0.0.1".parse().unwrap());
    }
    let router = protected_router_with(Arc::clone(&stub), throttle);
    let (status, _, _) = call(
        &router,
        Method::GET,
        "/twins/x/versions",
        Some(&basic("admin", "pw")),
    )
    .await;
    assert_eq!(
        status,
        StatusCode::UNAUTHORIZED,
        "even with the right password"
    );
    assert_eq!(stub.checks.load(Ordering::SeqCst), 0);
}

#[tokio::test]
async fn errors_of_the_framework_are_json_too() {
    // a path segment that is not UTF-8 is rejected by axum before the handler runs
    let router = open_router();
    let (status, headers, json) = call(&router, Method::GET, "/twins/%ff%fe/versions", None).await;
    assert_eq!(status, StatusCode::BAD_REQUEST);
    assert!(headers[header::CONTENT_TYPE]
        .to_str()
        .unwrap()
        .starts_with("application/json"));
    assert!(json["error"].is_string());
}

/// The status of a request for a versions list (400 for the bad id when the login passed) of one user from an address.
async fn login_status(
    router: &Router,
    user: &str,
    password: &str,
    peer: &str,
    forwarded: Option<&str>,
) -> StatusCode {
    call_from(
        router,
        Method::GET,
        "/twins/x/versions",
        Some(&basic(user, password)),
        Some(peer),
        forwarded,
    )
    .await
    .0
}

/// Like on the MQTT side an address is blocked after too many failures, whatever the user names: one password
/// tried against many user names does not go on for ever.
#[tokio::test]
async fn one_address_cannot_spray_many_user_names() {
    let stub = Arc::new(Stub::default());
    let router = protected_router(Arc::clone(&stub));
    let max = ThrottleConfig::default().max_failures;
    for n in 0..max {
        let status = login_status(&router, &format!("victim{n}"), "pw", "10.0.0.9", None).await;
        assert_eq!(status, StatusCode::UNAUTHORIZED);
    }
    let checks = stub.checks.load(Ordering::SeqCst);
    assert_eq!(checks, max as usize);
    // blocked now, also with a right password and without a check
    assert_eq!(
        login_status(&router, "admin", "pw", "10.0.0.9", None).await,
        StatusCode::UNAUTHORIZED
    );
    assert_eq!(stub.checks.load(Ordering::SeqCst), checks);
    // another address is not affected (400: past the login, the id is invalid)
    assert_eq!(
        login_status(&router, "admin", "pw", "10.0.0.10", None).await,
        StatusCode::BAD_REQUEST
    );
    // requests without credentials count nowhere
    for _ in 0..(3 * max) {
        let (status, _, _) = call_from(
            &router,
            Method::GET,
            "/projects",
            None,
            Some("10.0.0.11"),
            None,
        )
        .await;
        assert_eq!(status, StatusCode::UNAUTHORIZED);
    }
    assert_eq!(
        login_status(&router, "admin", "pw", "10.0.0.11", None).await,
        StatusCode::BAD_REQUEST
    );
}

/// With `behind_proxy` the last `X-Forwarded-For` entry is the client; without it the header means nothing.
#[tokio::test]
async fn the_forwarded_header_is_only_used_behind_a_proxy() {
    let max = ThrottleConfig::default().max_failures;
    let spray = |router: Router, forwarded: &'static str| async move {
        for n in 0..max {
            let status = login_status(
                &router,
                &format!("victim{n}"),
                "pw",
                "10.0.0.1",
                Some(forwarded),
            )
            .await;
            assert_eq!(status, StatusCode::UNAUTHORIZED);
        }
        router
    };

    // behind a proxy: the attacker behind the proxy is blocked, the other client behind the same proxy is not
    let stub = Arc::new(Stub::default());
    let throttle = LoginThrottle::new(ThrottleConfig::default());
    let proxied = router(pool(), Some(BasicAuth::new(stub.clone(), throttle, true)));
    let proxied = spray(proxied, "1.1.1.1, 203.0.113.66").await;
    assert_eq!(
        login_status(
            &proxied,
            "admin",
            "pw",
            "10.0.0.1",
            Some("1.1.1.1, 203.0.113.66")
        )
        .await,
        StatusCode::UNAUTHORIZED
    );
    assert_eq!(
        login_status(
            &proxied,
            "admin",
            "pw",
            "10.0.0.1",
            Some("203.0.113.66, 203.0.113.7")
        )
        .await,
        StatusCode::BAD_REQUEST,
        "another client, even if it claims the attacker's address to the left"
    );
    // a missing or unusable header falls back to the proxy's own address, which nobody was blocked on
    assert_eq!(
        login_status(&proxied, "admin", "pw", "10.0.0.1", None).await,
        StatusCode::BAD_REQUEST
    );

    // not behind a proxy: the header is ignored, so a changing header does not help the attacker
    let stub = Arc::new(Stub::default());
    let direct = protected_router(Arc::clone(&stub));
    for n in 0..max {
        let forwarded = format!("198.51.100.{n}");
        let status = login_status(
            &direct,
            &format!("victim{n}"),
            "pw",
            "10.0.0.1",
            Some(&forwarded),
        )
        .await;
        assert_eq!(status, StatusCode::UNAUTHORIZED);
    }
    assert_eq!(
        login_status(&direct, "admin", "pw", "10.0.0.1", Some("203.0.113.7")).await,
        StatusCode::UNAUTHORIZED,
        "still the same peer"
    );
}

struct Gated {
    release: tokio::sync::Notify,
}

impl Authenticator for Gated {
    fn verify<'a>(
        &'a self,
        _: &'a str,
        _: &'a [u8],
    ) -> Pin<Box<dyn Future<Output = AuthOutcome> + Send + 'a>> {
        Box::pin(async move {
            self.release.notified().await;
            AuthOutcome::Accepted
        })
    }
}

/// Only a few checks of one address run at the same time, and the slot is only held while verifying.
#[tokio::test]
async fn pending_checks_are_limited_per_address_and_only_while_verifying() {
    let config = ThrottleConfig {
        max_pending_per_ip: 1,
        ..ThrottleConfig::default()
    };
    // (a) while a verification of an address is running, another one of the same address is refused with 503
    let gated = Arc::new(Gated {
        release: tokio::sync::Notify::new(),
    });
    let auth = BasicAuth::new(gated.clone(), LoginThrottle::new(config.clone()), false);
    let router_a = router(pool(), Some(auth));
    let first = {
        let router = router_a.clone();
        tokio::spawn(async move { login_status(&router, "a", "p", "10.0.0.1", None).await })
    };
    tokio::time::sleep(std::time::Duration::from_millis(200)).await;
    assert_eq!(
        login_status(&router_a, "b", "p", "10.0.0.1", None).await,
        StatusCode::SERVICE_UNAVAILABLE
    );
    gated.release.notify_one();
    assert_eq!(first.await.unwrap(), StatusCode::BAD_REQUEST);

    // (b) a request that passed the login and is now served (here: waiting for the unreachable database) does not
    // count: the next login of the same address is checked at once
    let stub = Arc::new(Stub::default());
    let router_b = protected_router_with(stub, LoginThrottle::new(config));
    let slow = {
        let router = router_b.clone();
        tokio::spawn(async move {
            call_from(
                &router,
                Method::GET,
                "/projects",
                Some(&basic("admin", "pw")),
                Some("10.0.0.1"),
                None,
            )
            .await
        })
    };
    tokio::time::sleep(std::time::Duration::from_millis(200)).await;
    assert!(
        !slow.is_finished(),
        "the first request is still being served"
    );
    assert_eq!(
        login_status(&router_b, "admin", "pw", "10.0.0.1", None).await,
        StatusCode::BAD_REQUEST
    );
    slow.abort();
}
