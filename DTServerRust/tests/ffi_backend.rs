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

use dt_server::twin::ffi::FfiBackend;
use dt_server::twin::{BackendError, TwinBackend};
use uuid::Uuid;

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
        .login("127.0.0.1", port, "user", "secret-password")
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
        .login("127.0.0.1", port, "user", "secret-password")
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
    backend.login(&host, port, &user, &password).expect("login");
    let project: Uuid = project.parse().expect("DT_TEST_PROJECT_ID is a uuid");
    let twin: Uuid = twin.parse().expect("DT_TEST_TWIN_ID is a uuid");
    let info = backend.load_twin(project, twin).expect("load twin");
    assert!(!info.name.is_empty());
    eprintln!(
        "loaded twin {:?} with {} elements",
        info.name,
        info.elements.len()
    );
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
