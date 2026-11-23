//! Tests of the real backend (C++ facade). Only built with `--features ffi`.
//!
//! The error path tests need no Structura server. The live test is opt-in: it needs
//! `DT_TEST_SERVER_URL` (e.g. `http://127.0.0.1:18090`), `DT_TEST_USER`, `DT_TEST_PASSWORD`,
//! `DT_TEST_PROJECT_ID` and `DT_TEST_TWIN_ID` (a project with a digital twin on that server); without them it
//! is skipped. This test never starts a server.
#![cfg(feature = "ffi")]

use std::io::{Read, Write};
use std::net::{TcpListener, TcpStream};
use std::thread;
use std::time::Duration;

use dt_server::twin::ffi::FfiBackend;
use dt_server::twin::{BackendError, Credentials, StoreSettings, TwinBackend};
use uuid::Uuid;

fn password(username: &str, password: &str) -> Credentials {
    Credentials::Password {
        username: username.into(),
        password: password.into(),
    }
}

fn message(e: BackendError) -> String {
    match e {
        BackendError::Failed(m) | BackendError::Unauthorized(m) => m,
        other => panic!("unexpected error {other:?}"),
    }
}

#[test]
fn calls_without_login_return_errors() {
    let backend = FfiBackend::new();
    let err = backend
        .load_twin(Uuid::new_v4(), Uuid::new_v4())
        .expect_err("no session");
    assert!(message(err).contains("not logged in"));
    // unknown twin: no model loaded
    let err = backend
        .register_function(
            Uuid::new_v4(),
            r#"{"qualified_name":"a::b","linked_variables":[],"linked_variable_return":""}"#,
        )
        .expect_err("unknown twin");
    assert!(message(err).contains("no model"));
    // releasing an unknown twin is fine
    backend.release_twin(Uuid::new_v4()).expect("idempotent");
}

#[test]
fn login_to_an_unreachable_server_is_an_error_not_an_abort() {
    // a port that was just free: nothing listens there
    let port = {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        listener.local_addr().unwrap().port()
    };
    let backend = FfiBackend::new();
    let err = backend
        .login("127.0.0.1", port, &password("user", "secret-password"))
        .expect_err("connection refused");
    let text = message(err);
    assert!(!text.is_empty());
    assert!(
        !text.contains("secret-password"),
        "message leaks the password"
    );
    // still no session
    assert!(backend.load_twin(Uuid::new_v4(), Uuid::new_v4()).is_err());
}

#[test]
fn http_error_of_the_server_is_an_error() {
    // a server that answers every request with 500
    let listener = TcpListener::bind("127.0.0.1:0").unwrap();
    let port = listener.local_addr().unwrap().port();
    thread::spawn(move || {
        for stream in listener.incoming().flatten() {
            thread::spawn(move || answer_500(stream));
        }
    });
    let backend = FfiBackend::new();
    let err = backend
        .login("127.0.0.1", port, &password("user", "secret-password"))
        .expect_err("HTTP 500");
    assert!(!message(err).contains("secret-password"));
}

fn answer_500(mut stream: TcpStream) {
    let mut buf = [0u8; 4096];
    let _ = stream.read(&mut buf);
    let body = r#"{"error":"boom"}"#;
    let _ = write!(
        stream,
        "HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{body}",
        body.len()
    );
}

#[test]
fn unusable_application_keys_are_rejected_without_echoing_them() {
    let backend = FfiBackend::new();
    for bad in ["", "secret key", "secret\r\nX-Injected: 1", "secr\u{e9}t"] {
        let err = backend
            .login("127.0.0.1", 1, &Credentials::ApplicationKey(bad.into()))
            .expect_err("rejected");
        assert!(
            !message(err).contains("secr"),
            "the message echoes the key: {bad:?}"
        );
    }
}

/// Opt-in: against a real Structura server.
#[test]
fn live_load_twin_against_structura() {
    let Ok(url) = std::env::var("DT_TEST_SERVER_URL") else {
        eprintln!("SKIPPED live_load_twin_against_structura: DT_TEST_SERVER_URL is not set");
        return;
    };
    let (Ok(project), Ok(twin)) = (
        std::env::var("DT_TEST_PROJECT_ID"),
        std::env::var("DT_TEST_TWIN_ID"),
    ) else {
        eprintln!("SKIPPED live_load_twin_against_structura: DT_TEST_PROJECT_ID / DT_TEST_TWIN_ID not set");
        return;
    };
    let user = std::env::var("DT_TEST_USER").unwrap_or_else(|_| "admin".into());
    let password = std::env::var("DT_TEST_PASSWORD").unwrap_or_default();
    let (host, port) = split_url(&url);

    let backend = FfiBackend::new();
    backend
        .login(&host, port, &self::password(&user, &password))
        .expect("login");
    let project: Uuid = project.parse().expect("DT_TEST_PROJECT_ID is a uuid");
    let twin: Uuid = twin.parse().expect("DT_TEST_TWIN_ID is a uuid");
    let info = backend.load_twin(project, twin).expect("load twin");
    assert!(!info.name.is_empty());
    assert!(!info.elements.is_empty(), "the twin has no variables");
    eprintln!(
        "loaded twin {:?} with {} elements",
        info.name,
        info.elements.len()
    );
    for element in &info.elements {
        dt_server::payload::values::classify(element.initial_json.as_bytes())
            .unwrap_or_else(|e| panic!("initial value of {:?}: {e}", element.path));
    }
    backend.release_twin(twin).expect("release");
}

/// `http://host:port` -> (`http://host`, port)
fn split_url(url: &str) -> (String, u16) {
    let (scheme, rest) = url.split_once("://").unwrap_or(("http", url));
    let (host, port) = rest.rsplit_once(':').expect("URL with port");
    (
        format!("{scheme}://{host}"),
        port.trim_end_matches('/').parse().expect("port"),
    )
}

/// Offline contract test (no Structura server): the `getJson()` of every C++ value entity type passes
/// `classify` with the expected kind. Guards against the C++ and Rust wire formats drifting apart.
#[test]
fn cpp_get_json_matches_classify() {
    use dt_server::payload::values::{classify, Rational, Value};

    let samples = dt_server::twin::ffi::sample_initial_values();
    assert_eq!(samples.len(), 9, "one sample per type: {samples:?}");
    let value_of = |path: &str| {
        let sample = samples
            .iter()
            .find(|e| e.path == path)
            .unwrap_or_else(|| panic!("no sample {path}"));
        classify(sample.initial_json.as_bytes())
            .unwrap_or_else(|e| panic!("{path}: {:?} is rejected: {e}", sample.initial_json))
            .value
    };
    assert_eq!(value_of("real"), Value::Real(1.5));
    assert_eq!(value_of("real_zero"), Value::Real(0.0));
    assert_eq!(value_of("integer"), Value::Integer(-7));
    assert_eq!(value_of("boolean"), Value::Boolean(true));
    assert_eq!(
        value_of("complex"),
        Value::Complex {
            real: 1.0,
            imaginary: -2.5
        }
    );
    assert_eq!(
        value_of("rational"),
        Value::Rational(Rational::new(1, 3).unwrap())
    );
    assert_eq!(value_of("positive"), Value::Integer(3));
    assert_eq!(value_of("string"), Value::Text("hello".into()));
    assert_eq!(value_of("string_empty"), Value::Text(String::new()));
}

#[test]
fn update_variable_for_an_unknown_twin_is_an_error() {
    let backend = FfiBackend::new();
    let err = backend
        .update_variable(Uuid::new_v4(), "a/b", "1.5", 1_700_000_000_000)
        .expect_err("no model loaded");
    assert!(message(err).contains("no model"));
}

#[test]
fn store_configure_with_an_empty_url_is_an_error() {
    // Rejected before the once-only state of the process is touched, so this test is independent of the
    // lifecycle test below.
    let backend = FfiBackend::new();
    let settings = store_settings("");
    let err = backend.configure_store(&settings).expect_err("empty url");
    assert!(message(err).contains("url"));
}

fn store_settings(url: &str) -> StoreSettings {
    StoreSettings {
        database_url: url.to_string(),
        retention_days: 30,
        checkpoint_interval_s: 3600,
        queue_capacity: 0,
        batch_size: 0,
        flush_interval_ms: 0,
    }
}

/// The whole lifecycle of the history store in one test: it can be configured once per process, so the steps
/// that depend on the order (stats before and after configure, second configure, shutdown) must not be split
/// into tests that run in parallel threads. Nothing here needs a database: the store connects in the
/// background and keeps retrying; it must not leak the URL or fail the configuration.
#[test]
fn store_lifecycle_without_a_database() {
    let backend = FfiBackend::new();

    // not configured: disabled, all counters zero; shutdown without a store is not an error
    let stats = backend.store_stats();
    assert!(!stats.enabled && !stats.connected);
    assert_eq!(
        (
            stats.pending,
            stats.written,
            stats.dropped,
            stats.retries,
            stats.checkpoints
        ),
        (0, 0, 0, 0, 0)
    );
    backend
        .shutdown_store(Duration::from_millis(100))
        .expect("shutdown without a store");

    // a port that was just free: no database there
    let port = {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        listener.local_addr().unwrap().port()
    };
    let url = format!("postgres://dt_user:secret-db-password@127.0.0.1:{port}/dt");
    let settings = store_settings(&url);
    match backend.configure_store(&settings) {
        Ok(()) => {}
        Err(e) => {
            let text = message(e);
            assert!(
                !text.contains("secret-db-password"),
                "message leaks the password: {text}"
            );
            panic!("configure must not fail because the database is down: {text}");
        }
    }

    let stats = backend.store_stats();
    assert!(stats.enabled);
    assert!(!stats.connected, "there is no database");
    assert_eq!(stats.written, 0);

    // a second configuration is refused, also without leaking
    let err = backend
        .configure_store(&settings)
        .expect_err("second configure");
    let text = message(err);
    assert!(text.contains("already"), "{text}");
    assert!(!text.contains("secret-db-password"), "{text}");

    // an unknown twin is still an error with a store configured
    assert!(backend
        .update_variable(Uuid::new_v4(), "a/b", "1.5", 0)
        .is_err());

    // shutdown returns within the timeout (plus slack) and is idempotent
    let started = std::time::Instant::now();
    backend
        .shutdown_store(Duration::from_millis(500))
        .expect("shutdown");
    assert!(
        started.elapsed() < Duration::from_secs(10),
        "shutdown took {:?}",
        started.elapsed()
    );
    backend
        .shutdown_store(Duration::from_millis(500))
        .expect("second shutdown is not an error");
}
