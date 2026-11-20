//! Login at the broker: the credentials of the CONNECT are checked by an `Authenticator` before the connection
//! is accepted. A fake authenticator stands in for the SysML backend (the HTTP side is tested in
//! `auth::sysml`).

mod common;

use std::future::Future;
use std::pin::Pin;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::{Duration, Instant};

use common::*;
use dt_server::auth::{AuthOutcome, Authenticator, ThrottleConfig};
use dt_server::broker::{Broker, BrokerLimits, BrokerOptions, ProtocolMode};
use tokio::sync::Semaphore;

/// Decides by user name: `good` with password `secret` is accepted, `down` is unavailable, `hang` never
/// answers, `gated` is accepted once the gate is opened, everything else is rejected.
struct Fake {
    calls: AtomicUsize,
    gate: Semaphore,
}

impl Fake {
    fn new() -> Arc<Fake> {
        Arc::new(Fake {
            calls: AtomicUsize::new(0),
            gate: Semaphore::new(0),
        })
    }
}

/// The broker's handle on the fake (the test keeps its own `Arc` to look at the calls).
struct Handle(Arc<Fake>);

impl Authenticator for Handle {
    fn verify<'a>(
        &'a self,
        username: &'a str,
        password: &'a [u8],
    ) -> Pin<Box<dyn Future<Output = AuthOutcome> + Send + 'a>> {
        let fake = &self.0;
        fake.calls.fetch_add(1, Ordering::SeqCst);
        Box::pin(async move {
            match username {
                "good" if password == b"secret" => AuthOutcome::Accepted,
                "down" => AuthOutcome::Unavailable("backend down".into()),
                "hang" => std::future::pending().await,
                "gated" => {
                    let _ = fake.gate.acquire().await.unwrap();
                    AuthOutcome::Accepted
                }
                _ => AuthOutcome::Rejected,
            }
        })
    }
}

async fn start_auth(fake: &Arc<Fake>, mode: ProtocolMode, limits: BrokerLimits) -> Broker {
    // no blocking here, the throttle has its own tests
    let throttle = ThrottleConfig {
        max_failures: 0,
        max_pending_per_ip: 0,
        ..ThrottleConfig::default()
    };
    start_auth_throttled(fake, mode, limits, throttle).await
}

async fn start_auth_throttled(
    fake: &Arc<Fake>,
    mode: ProtocolMode,
    limits: BrokerLimits,
    throttle: ThrottleConfig,
) -> Broker {
    Broker::start_with(
        0,
        limits,
        BrokerOptions {
            protocol: mode,
            authenticator: Some(Arc::new(Handle(Arc::clone(fake)))),
            throttle,
        },
    )
    .await
    .expect("broker starts")
}

/// Payload of a CONNECT after the client id: user name and/or password.
fn credentials(user: Option<&str>, password: Option<&[u8]>) -> (u8, Vec<u8>) {
    let mut flags = 0b10; // clean session / clean start
    let mut tail = Vec::new();
    if let Some(user) = user {
        flags |= 0b1000_0000;
        tail.extend(string_bytes(user));
    }
    if let Some(password) = password {
        flags |= 0b0100_0000;
        tail.extend((password.len() as u16).to_be_bytes());
        tail.extend_from_slice(password);
    }
    (flags, tail)
}

/// CONNECT of the given protocol level (4 or 5) with credentials.
fn connect_with(level: u8, id: &str, user: Option<&str>, password: Option<&[u8]>) -> Vec<u8> {
    let (flags, tail) = credentials(user, password);
    if level == 4 {
        return connect_raw(flags, 30, id, &tail);
    }
    let mut body = string_bytes("MQTT");
    body.extend([5, flags]);
    body.extend(30u16.to_be_bytes());
    body.push(0); // no properties
    body.extend(string_bytes(id));
    body.extend(tail);
    packet(0x10, &body)
}

/// Return/reason code of a CONNACK body.
fn code(body: &[u8]) -> u8 {
    body[1]
}

async fn refused(addr: &str, connect: &[u8]) -> u8 {
    let mut raw = Raw::open(addr).await;
    raw.send(connect).await;
    let (first, body) = raw.read_packet().await.expect("CONNACK");
    assert_eq!(first, 0x20);
    assert!(
        raw.read_until_closed().await.is_none(),
        "the connection is closed after the refusal"
    );
    code(&body)
}

#[tokio::test]
async fn correct_credentials_are_accepted() {
    let fake = Fake::new();
    let broker = start_auth(&fake, ProtocolMode::Both, BrokerLimits::default()).await;
    for level in [4u8, 5] {
        let mut raw = Raw::open(&addr(&broker)).await;
        raw.send(&connect_with(level, "c", Some("good"), Some(b"secret")))
            .await;
        let (first, body) = raw.read_packet().await.expect("CONNACK");
        assert_eq!((first, code(&body)), (0x20, 0), "level {level}");
        // and the session works
        if level == 4 {
            assert_eq!(raw.subscribe(1, &["a/b"]).await, [0]);
        } else {
            assert_eq!(raw.subscribe5(1, &[("a/b", 0)]).await, [0]);
        }
    }
    assert_eq!(fake.calls.load(Ordering::SeqCst), 2);
    broker.shutdown().await;
}

#[tokio::test]
async fn wrong_credentials_are_refused_with_bad_user_name_or_password() {
    let fake = Fake::new();
    let broker = start_auth(&fake, ProtocolMode::Both, BrokerLimits::default()).await;
    for (level, expected) in [(4u8, 0x04u8), (5, 0x86)] {
        let a = addr(&broker);
        // wrong password, unknown user
        for (user, password) in [("good", &b"wrong"[..]), ("nobody", b"secret")] {
            let connect = connect_with(level, "c", Some(user), Some(password));
            assert_eq!(refused(&a, &connect).await, expected, "level {level}");
        }
        assert_eq!(broker.connection_count(), 0);
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn missing_credentials_are_refused_without_asking_the_backend() {
    let fake = Fake::new();
    let broker = start_auth(&fake, ProtocolMode::Both, BrokerLimits::default()).await;
    for (level, expected) in [(4u8, 0x04u8), (5, 0x86)] {
        let a = addr(&broker);
        for (user, password) in [
            (None, None),
            (Some("good"), None), // user name without password
        ] {
            let connect = connect_with(level, "c", user, password);
            assert_eq!(refused(&a, &connect).await, expected, "level {level}");
        }
    }
    assert_eq!(fake.calls.load(Ordering::SeqCst), 0);
    broker.shutdown().await;
}

#[tokio::test]
async fn unavailable_backend_is_server_unavailable() {
    let fake = Fake::new();
    let broker = start_auth(&fake, ProtocolMode::Both, BrokerLimits::default()).await;
    for (level, expected) in [(4u8, 0x03u8), (5, 0x88)] {
        let connect = connect_with(level, "c", Some("down"), Some(b"x"));
        assert_eq!(refused(&addr(&broker), &connect).await, expected);
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn shutdown_during_a_pending_check_closes_the_connection() {
    let fake = Fake::new();
    let broker = start_auth(&fake, ProtocolMode::Both, BrokerLimits::default()).await;
    let mut raw = Raw::open(&addr(&broker)).await;
    raw.send(&connect_with(4, "c", Some("hang"), Some(b"x")))
        .await;
    // the check is running, nothing is answered
    assert!(raw.is_silent_for(Duration::from_millis(300)).await);
    assert_eq!(fake.calls.load(Ordering::SeqCst), 1);
    let started = Instant::now();
    broker.shutdown().await;
    assert!(started.elapsed() < Duration::from_secs(3));
    assert!(raw.read_until_closed().await.is_none());
}

#[tokio::test]
async fn a_check_that_outlasts_the_connect_timeout_is_answered_with_server_unavailable() {
    let fake = Fake::new();
    let limits = BrokerLimits {
        connect_timeout_s: 2,
        ..BrokerLimits::default()
    };
    let broker = start_auth(&fake, ProtocolMode::Both, limits).await;
    for (level, expected) in [(4u8, 0x03u8), (5, 0x88)] {
        let started = Instant::now();
        let connect = connect_with(level, "c", Some("hang"), Some(b"x"));
        assert_eq!(refused(&addr(&broker), &connect).await, expected);
        // before the CONNECT deadline of 2 s
        assert!(
            started.elapsed() < Duration::from_millis(2000),
            "level {level}"
        );
    }
    broker.shutdown().await;
}

#[tokio::test]
async fn repeated_failures_block_the_user_without_asking_the_backend() {
    let fake = Fake::new();
    let throttle = ThrottleConfig {
        max_failures: 2,
        window: Duration::from_secs(60),
        block: Duration::from_secs(30),
        max_pending_per_ip: 0,
    };
    let broker =
        start_auth_throttled(&fake, ProtocolMode::Both, BrokerLimits::default(), throttle).await;
    let a = addr(&broker);
    for _ in 0..2 {
        let connect = connect_with(4, "c", Some("good"), Some(b"wrong"));
        assert_eq!(refused(&a, &connect).await, 0x04);
    }
    assert_eq!(fake.calls.load(Ordering::SeqCst), 2);
    // now blocked, even with the right password, in both versions, and the backend is not asked
    for (level, expected) in [(4u8, 0x04u8), (5, 0x86)] {
        let connect = connect_with(level, "c", Some("good"), Some(b"secret"));
        assert_eq!(refused(&a, &connect).await, expected);
    }
    assert_eq!(fake.calls.load(Ordering::SeqCst), 2);
    broker.shutdown().await;
}

#[tokio::test]
async fn too_many_pending_checks_of_one_address_are_refused() {
    let fake = Fake::new();
    let throttle = ThrottleConfig {
        max_failures: 0,
        max_pending_per_ip: 1,
        ..ThrottleConfig::default()
    };
    let broker =
        start_auth_throttled(&fake, ProtocolMode::Both, BrokerLimits::default(), throttle).await;
    let mut first = Raw::open(&addr(&broker)).await;
    first
        .send(&connect_with(4, "c1", Some("gated"), Some(b"x")))
        .await;
    assert!(first.is_silent_for(Duration::from_millis(300)).await);
    // the second check of the same address is not started
    let connect = connect_with(5, "c2", Some("good"), Some(b"secret"));
    assert_eq!(refused(&addr(&broker), &connect).await, 0x88);
    assert_eq!(fake.calls.load(Ordering::SeqCst), 1);
    fake.gate.add_permits(1);
    let (first_byte, body) = first.read_packet().await.expect("CONNACK");
    assert_eq!((first_byte, code(&body)), (0x20, 0));
    broker.shutdown().await;
}

#[tokio::test]
async fn pipelined_packets_wait_for_the_verdict() {
    let fake = Fake::new();
    let broker = start_auth(&fake, ProtocolMode::Both, BrokerLimits::default()).await;
    let mut raw = Raw::open(&addr(&broker)).await;
    // CONNECT, SUBSCRIBE and PINGREQ in one write
    let mut bytes = connect_with(4, "c", Some("gated"), Some(b"x"));
    bytes.extend(subscribe(7, &["t/#"]));
    bytes.extend(pingreq());
    raw.send(&bytes).await;
    assert!(raw.is_silent_for(Duration::from_millis(300)).await);
    assert_eq!(broker.subscription_count(), 0, "nothing is processed yet");
    fake.gate.add_permits(1);
    let (first, body) = raw.read_packet().await.expect("CONNACK");
    assert_eq!((first, code(&body)), (0x20, 0));
    let (first, body) = raw.read_packet().await.expect("SUBACK");
    assert_eq!((first, &body[..2]), (0x90, &[0, 7][..]));
    let (first, _) = raw.read_packet().await.expect("PINGRESP");
    assert_eq!(first, 0xD0);
    assert_eq!(broker.subscription_count(), 1);
    broker.shutdown().await;
}

#[tokio::test]
async fn pipelined_packets_behind_a_refused_connect_are_not_processed() {
    let fake = Fake::new();
    let broker = start_auth(&fake, ProtocolMode::Both, BrokerLimits::default()).await;
    let mut raw = Raw::open(&addr(&broker)).await;
    let mut bytes = connect_with(4, "c", Some("good"), Some(b"wrong"));
    bytes.extend(subscribe(7, &["t/#"]));
    raw.send(&bytes).await;
    let (first, body) = raw.read_packet().await.expect("CONNACK");
    assert_eq!((first, code(&body)), (0x20, 0x04));
    assert!(raw.read_until_closed().await.is_none());
    assert_eq!(broker.subscription_count(), 0);
    broker.shutdown().await;
}

#[tokio::test]
async fn without_an_authenticator_credentials_are_not_required() {
    let broker = start(BrokerLimits::default()).await;
    let _raw = Raw::connected(&addr(&broker), "anon").await;
    // credentials that are present are ignored
    let _raw = Raw::connected_with(
        &addr(&broker),
        &connect_with(4, "c", Some("whoever"), Some(b"x")),
    )
    .await;
    broker.shutdown().await;
}
