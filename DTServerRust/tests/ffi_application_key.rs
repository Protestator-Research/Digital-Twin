//! Tests of the application key login of the real backend (C++ facade), against a fake SysMLv2 server that records
//! every request. Only built with `--features ffi`.
//!
//! The facade keeps one session per process, and a successful login replaces it. These tests install sessions, so
//! they live in their own test binary (own process): the tests of `ffi_backend.rs` rely on "no session yet".
#![cfg(feature = "ffi")]

use std::io::{Read, Write};
use std::net::TcpListener;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

use dt_server::twin::ffi::FfiBackend;
use dt_server::twin::{BackendError, Credentials, TwinBackend};
use uuid::Uuid;

/// Both tests use the one session of the process: one at a time.
static SESSION: Mutex<()> = Mutex::new(());

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

/// A fake SysMLv2 server that records the head of every request it receives and answers each with `status`.
fn recording_server(status: &'static str) -> (u16, Arc<Mutex<Vec<String>>>) {
    server(status, false)
}

/// Like `recording_server`; with `echo` the JSON body of the answer repeats the `Authorization` header of the
/// request in its `error` field, like a badly written server that reflects what it was sent.
fn server(status: &'static str, echo: bool) -> (u16, Arc<Mutex<Vec<String>>>) {
    let listener = TcpListener::bind("127.0.0.1:0").unwrap();
    let port = listener.local_addr().unwrap().port();
    let requests = Arc::new(Mutex::new(Vec::new()));
    let seen = Arc::clone(&requests);
    thread::spawn(move || {
        for stream in listener.incoming().flatten() {
            let seen = Arc::clone(&seen);
            thread::spawn(move || {
                let mut stream = stream;
                let mut head = Vec::new();
                let mut buf = [0u8; 1024];
                while !head.windows(4).any(|w| w == b"\r\n\r\n") {
                    match stream.read(&mut buf) {
                        Ok(0) | Err(_) => break,
                        Ok(n) => head.extend_from_slice(&buf[..n]),
                    }
                }
                seen.lock()
                    .unwrap()
                    .push(String::from_utf8_lossy(&head).into_owned());
                let request = String::from_utf8_lossy(&head).into_owned();
                let body = if echo {
                    let auth = header_of(&request, "authorization").unwrap_or_default();
                    format!(r#"{{"error":"rejected {auth}"}}"#)
                } else {
                    r#"{"error":"nope"}"#.to_owned()
                };
                let _ = write!(
                    stream,
                    "HTTP/1.1 {status}\r\nContent-Type: application/json\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{body}",
                    body.len()
                );
            });
        }
    });
    (port, requests)
}

/// The value of a header in a recorded request head (name compared case-insensitively).
fn header_of(request: &str, name: &str) -> Option<String> {
    request.lines().skip(1).find_map(|line| {
        let (key, value) = line.split_once(':')?;
        key.trim()
            .eq_ignore_ascii_case(name)
            .then(|| value.trim().to_owned())
    })
}

/// The application key is the bearer token: logging in makes no request at all, and the first request of the twin
/// load carries `Authorization: Bearer <key>`. A 401 of the server is the unauthorized error.
#[test]
fn application_key_login_sends_nothing_and_the_load_sends_the_key_as_bearer_token() {
    let _session = SESSION.lock().unwrap_or_else(|e| e.into_inner());
    let (port, requests) = recording_server("401 Unauthorized");
    let backend = FfiBackend::new();
    let key = "sysml_ak_0123456789abcdef";
    backend
        .login("127.0.0.1", port, &Credentials::ApplicationKey(key.into()))
        .expect("setting the key makes no request, so it cannot fail");
    // nothing was sent at login time (give a late request a moment to show up)
    thread::sleep(Duration::from_millis(300));
    assert!(
        requests.lock().unwrap().is_empty(),
        "requests at login time: {:?}",
        requests.lock().unwrap()
    );

    let project = Uuid::new_v4();
    let err = backend
        .load_twin(project, Uuid::new_v4())
        .expect_err("the fake server answers 401");
    match &err {
        BackendError::Unauthorized(text) => {
            assert!(!text.contains(key), "the message leaks the key: {text}")
        }
        other => panic!("expected the unauthorized error, got {other:?}"),
    }
    let seen = requests.lock().unwrap().clone();
    assert_eq!(
        seen.len(),
        1,
        "exactly the one request of the load: {seen:?}"
    );
    assert!(
        seen[0].starts_with(&format!("GET /projects/{project}/twins")),
        "{}",
        seen[0]
    );
    assert_eq!(
        header_of(&seen[0], "authorization").as_deref(),
        Some(format!("Bearer {key}").as_str())
    );
    assert!(seen.iter().all(|r| !r.contains("/login")), "{seen:?}");
}

/// The password login, for comparison, does make a login request first (and its 401 is the "wrong credentials"
/// error, not the session error of a load).
#[test]
fn password_login_makes_a_login_request() {
    let _session = SESSION.lock().unwrap_or_else(|e| e.into_inner());
    let (port, requests) = recording_server("401 Unauthorized");
    let backend = FfiBackend::new();
    let err = backend
        .login("127.0.0.1", port, &password("user", "secret-password"))
        .expect_err("401");
    assert!(!message(err).contains("secret-password"));
    let seen = requests.lock().unwrap().clone();
    assert_eq!(seen.len(), 1, "{seen:?}");
    assert!(seen[0].starts_with("POST /login"), "{}", seen[0]);
}

/// A 401 with an application key is the rejected key: the message is only the server's own detail, without the
/// advice for an expired session ("log in again") that the message for a password session carries.
#[test]
fn the_message_of_a_rejected_key_has_no_session_expired_advice() {
    let _session = SESSION.lock().unwrap_or_else(|e| e.into_inner());
    let (port, _) = recording_server("401 Unauthorized");
    let backend = FfiBackend::new();
    backend
        .login(
            "127.0.0.1",
            port,
            &Credentials::ApplicationKey("sysml_ak_detail".into()),
        )
        .unwrap();
    let text = match backend.load_twin(Uuid::new_v4(), Uuid::new_v4()) {
        Err(BackendError::Unauthorized(text)) => text,
        other => panic!("expected the unauthorized error: {other:?}"),
    };
    assert!(text.contains("nope"), "the server's detail is kept: {text}");
    assert!(
        !text.contains("log in again") && !text.contains("expired"),
        "{text}"
    );
    // for comparison: with a password session the library's advice is part of the message
    // (not tested here: it needs a successful password login, which the fake server does not offer)
}

/// Defence in depth: whatever text the library puts into an error, the application key is replaced before it
/// crosses the bridge. The fake server echoes the `Authorization` header into the body of a 500.
#[test]
fn the_application_key_is_removed_from_error_messages() {
    let _session = SESSION.lock().unwrap_or_else(|e| e.into_inner());
    let (port, requests) = server("500 Internal Server Error", true);
    let backend = FfiBackend::new();
    let key = "sysml_ak_echoed_9f8e7d";
    backend
        .login("127.0.0.1", port, &Credentials::ApplicationKey(key.into()))
        .unwrap();
    let err = backend
        .load_twin(Uuid::new_v4(), Uuid::new_v4())
        .expect_err("the server answers 500");
    let text = message(err);
    assert!(!text.contains(key), "the key is in the message: {text}");
    // the echo really reached the library's message, so this is the redaction at work and not a body that is
    // never shown
    let seen = requests.lock().unwrap().clone();
    assert_eq!(seen.len(), 1, "{seen:?}");
    assert!(
        text.contains("<application key>"),
        "the body of the answer is not part of the message, the test proves nothing: {text}"
    );
}
