//! The twin logic against a real broker on an ephemeral port, with a fake backend (no C++ needed).

mod common;

use std::collections::{HashMap, HashSet};
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::mpsc as std_mpsc;
use std::sync::{Arc, Mutex};
use std::time::Duration;

use common::*;
use dt_server::broker::{Broker, BrokerLimits};
use dt_server::config::SysmlConfig;
use dt_server::payload::twin::{ConnectToTwin, FunctionRegistrationRequest};
use dt_server::payload::values::{classify, IntegerValue, Value};
use dt_server::twin::{
    BackendError, LoadedTwin, TwinBackend, TwinElement, TwinInfo, TwinObserver, TwinOptions,
    TwinService, UnavailableBackend,
};
use rumqttc::v5::mqttbytes::QoS;
use uuid::Uuid;

const PROJECT: Uuid = Uuid::from_u128(0xAAAA);

fn twin_id(n: u128) -> Uuid {
    Uuid::from_u128(n)
}

#[derive(Default)]
struct FakeState {
    login_calls: usize,
    login_failures_left: usize,
    loads: Vec<(Uuid, Uuid)>,
    registered: Vec<(Uuid, String)>,
    released: Vec<Uuid>,
}

#[derive(Default)]
struct FakeBackend {
    state: Mutex<FakeState>,
    twins: Mutex<HashMap<Uuid, TwinInfo>>,
    failing: Mutex<HashSet<Uuid>>,
    /// The next load of these twins is refused as unauthorized (expired session).
    unauthorized_once: Mutex<HashSet<Uuid>>,
    /// While set, `load_twin` blocks until a unit is sent.
    gate: Mutex<Option<std_mpsc::Receiver<()>>>,
    /// Every load takes this long.
    load_delay: Mutex<Duration>,
    running: AtomicUsize,
    max_running: AtomicUsize,
}

impl FakeBackend {
    fn with_twin(id: Uuid, name: &str, elements: &[&str]) -> Arc<FakeBackend> {
        let backend = FakeBackend::default();
        backend.add_twin(id, name, elements);
        Arc::new(backend)
    }

    /// Every element gets the initial value `Integer 0`.
    fn add_twin(&self, id: Uuid, name: &str, elements: &[&str]) {
        let initial = IntegerValue::new(0, chrono::Utc::now()).to_json();
        let elements: Vec<(&str, &str)> = elements.iter().map(|e| (*e, initial.as_str())).collect();
        self.add_twin_with_values(id, name, &elements);
    }

    /// Elements as `(path, initial value JSON)`.
    fn add_twin_with_values(&self, id: Uuid, name: &str, elements: &[(&str, &str)]) {
        self.twins.lock().unwrap().insert(
            id,
            TwinInfo {
                name: name.to_owned(),
                elements: elements
                    .iter()
                    .map(|(path, json)| TwinElement {
                        path: (*path).to_owned(),
                        initial_json: (*json).to_owned(),
                    })
                    .collect(),
            },
        );
    }

    fn loads(&self) -> Vec<(Uuid, Uuid)> {
        self.state.lock().unwrap().loads.clone()
    }

    fn registered(&self) -> Vec<(Uuid, String)> {
        self.state.lock().unwrap().registered.clone()
    }
}

impl TwinBackend for FakeBackend {
    fn login(&self, _: &str, _: u16, _: &str, _: &str) -> Result<(), BackendError> {
        let mut s = self.state.lock().unwrap();
        s.login_calls += 1;
        if s.login_failures_left > 0 {
            s.login_failures_left -= 1;
            return Err(BackendError::Failed("login refused".into()));
        }
        Ok(())
    }

    fn load_twin(&self, project_id: Uuid, twin_id: Uuid) -> Result<TwinInfo, BackendError> {
        self.state.lock().unwrap().loads.push((project_id, twin_id));
        let running = self.running.fetch_add(1, Ordering::SeqCst) + 1;
        self.max_running.fetch_max(running, Ordering::SeqCst);
        let result = (|| {
            // take the receiver out so that the gate does not hold the lock while waiting
            let gate = self.gate.lock().unwrap().take();
            if let Some(gate) = gate {
                let _ = gate.recv_timeout(Duration::from_secs(10));
            }
            std::thread::sleep(*self.load_delay.lock().unwrap());
            if self.unauthorized_once.lock().unwrap().remove(&twin_id) {
                return Err(BackendError::Unauthorized("HTTP 401".into()));
            }
            if self.failing.lock().unwrap().contains(&twin_id) {
                return Err(BackendError::Failed("HTTP 500".into()));
            }
            self.twins
                .lock()
                .unwrap()
                .get(&twin_id)
                .cloned()
                .ok_or_else(|| BackendError::Failed("unknown twin".into()))
        })();
        self.running.fetch_sub(1, Ordering::SeqCst);
        result
    }

    fn register_function(&self, twin_id: Uuid, request_json: &str) -> Result<(), BackendError> {
        self.state
            .lock()
            .unwrap()
            .registered
            .push((twin_id, request_json.to_owned()));
        Ok(())
    }

    fn release_twin(&self, twin_id: Uuid) -> Result<(), BackendError> {
        self.state.lock().unwrap().released.push(twin_id);
        Ok(())
    }
}

#[derive(Default)]
struct Observed(Mutex<Vec<LoadedTwin>>);

impl TwinObserver for Observed {
    fn twin_loaded(&self, twin: &LoadedTwin) {
        self.0.lock().unwrap().push(twin.clone());
    }
}

fn sysml() -> SysmlConfig {
    dt_server::config::Config::from_args(Vec::<String>::new())
        .unwrap()
        .sysml
}

fn connect_request(twin: Uuid) -> String {
    ConnectToTwin {
        project_id: PROJECT,
        twin_id: twin,
    }
    .to_json()
}

async fn wait_until(what: &str, mut condition: impl FnMut() -> bool) {
    // 30 s of headroom for a loaded machine; the loop ends as soon as the condition holds
    for _ in 0..3000 {
        if condition() {
            return;
        }
        tokio::time::sleep(Duration::from_millis(10)).await;
    }
    panic!("timed out waiting for: {what}");
}

async fn publish(client: &Client, topic: &str, payload: impl Into<Vec<u8>>) {
    client
        .client
        .publish(topic, QoS::AtMostOnce, false, payload.into())
        .await
        .unwrap();
}

async fn subscribe(client: &mut Client, filter: &str) {
    client
        .client
        .subscribe(filter, QoS::AtMostOnce)
        .await
        .unwrap();
    client.wait_suback().await;
}

struct Fixture {
    broker: Broker,
    service: TwinService,
    observed: Arc<Observed>,
}

async fn fixture(backend: Arc<dyn TwinBackend>, options: TwinOptions) -> Fixture {
    let broker = start(BrokerLimits::default()).await;
    let observed = Arc::new(Observed::default());
    let service = TwinService::start(
        broker.hub(),
        backend,
        sysml(),
        options,
        Some(Arc::clone(&observed) as Arc<dyn TwinObserver>),
    )
    .unwrap();
    Fixture {
        broker,
        service,
        observed,
    }
}

#[tokio::test]
async fn connect_publishes_the_interface_and_forwards_registrations() {
    let backend =
        FakeBackend::with_twin(twin_id(1), "Pump", &["Motor/speed", "Motor/temp", "level"]);
    let f = fixture(backend.clone(), TwinOptions::default()).await;

    let mut watcher = Client::connect(f.broker.port(), "watcher").await;
    subscribe(&mut watcher, "Pump/#").await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;

    // one initial value per element, then the empty registration request
    let mut topics = Vec::new();
    for _ in 0..3 {
        let (topic, payload) = watcher.next_publish().await;
        let value = classify(&payload).expect("the initial value is a valid payload");
        assert_eq!(value.value, Value::Integer(0), "{topic}");
        let age = chrono::Utc::now() - value.time_point;
        assert!(age.num_seconds().abs() < 60, "time_point is now-ish: {age}");
        topics.push(topic);
    }
    topics.sort();
    assert_eq!(
        topics,
        ["Pump/Motor/speed", "Pump/Motor/temp", "Pump/level"]
    );
    let (topic, payload) = watcher.next_publish().await;
    assert_eq!(topic, "Pump/registerFunction");
    assert_eq!(
        FunctionRegistrationRequest::from_json(&payload).unwrap(),
        FunctionRegistrationRequest {
            linked_variable_return: String::new(),
            linked_variables: vec![],
            qualified_name: String::new()
        }
    );
    assert_eq!(backend.loads(), vec![(PROJECT, twin_id(1))]);
    // the observer (history registry) was told
    assert_eq!(
        f.observed.0.lock().unwrap().clone(),
        vec![LoadedTwin {
            twin_id: twin_id(1),
            project_id: PROJECT,
            name: "Pump".into()
        }]
    );
    // the server's own empty request reaches the forwarder but is not forwarded to the backend
    let stats = f.service.stats();
    wait_until("own request seen", || {
        stats.registration_messages.load(Ordering::Relaxed) == 1
    })
    .await;
    assert_eq!(stats.registrations_ignored.load(Ordering::Relaxed), 1);
    assert!(backend.registered().is_empty());

    // a client registers a function: forwarded as it came
    let request = FunctionRegistrationRequest {
        linked_variable_return: "Pump::out".into(),
        linked_variables: vec![("a".into(), "Pump::a".into())],
        qualified_name: "Pump::add".into(),
    }
    .to_json();
    publish(&requester, "Pump/registerFunction", request.clone()).await;
    wait_until("registration forwarded", || {
        !backend.registered().is_empty()
    })
    .await;
    assert_eq!(backend.registered(), vec![(twin_id(1), request)]);

    // garbage and nameless requests are ignored
    publish(&requester, "Pump/registerFunction", "not json").await;
    publish(
        &requester,
        "Pump/registerFunction",
        FunctionRegistrationRequest {
            linked_variable_return: String::new(),
            linked_variables: vec![],
            qualified_name: "  ".into(),
        }
        .to_json(),
    )
    .await;
    wait_until("both bad requests seen", || {
        stats.registrations_ignored.load(Ordering::Relaxed) == 3
    })
    .await;
    assert_eq!(stats.registration_messages.load(Ordering::Relaxed), 4);
    assert_eq!(stats.registrations_forwarded.load(Ordering::Relaxed), 1);
    assert_eq!(backend.registered().len(), 1);

    // clean shutdown: the hub closes, the twin is released
    f.broker.shutdown().await;
    tokio::time::timeout(Duration::from_secs(10), f.service.shutdown())
        .await
        .expect("twin service stops");
    assert_eq!(backend.state.lock().unwrap().released, vec![twin_id(1)]);
}

#[tokio::test]
async fn duplicate_request_while_a_download_runs_is_skipped() {
    let backend = FakeBackend::with_twin(twin_id(1), "Pump", &["x"]);
    let (gate_tx, gate_rx) = std_mpsc::channel();
    *backend.gate.lock().unwrap() = Some(gate_rx);
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    let mut watcher = Client::connect(f.broker.port(), "watcher").await;
    subscribe(&mut watcher, "Pump/#").await;
    let requester = Client::connect(f.broker.port(), "requester").await;

    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    wait_until("download started", || backend.loads().len() == 1).await;
    // the same twin again, twice, while the first download is blocked
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    wait_until("duplicates seen", || {
        f.service.stats().skipped.load(Ordering::Relaxed) == 2
    })
    .await;
    assert_eq!(backend.loads().len(), 1, "no second download");

    gate_tx.send(()).unwrap();
    let (topic, _) = watcher.next_publish().await;
    assert_eq!(topic, "Pump/x");
    assert_eq!(watcher.next_publish().await.0, "Pump/registerFunction");
    // exactly one interface was published: there was exactly one download
    assert_eq!(f.service.stats().loaded.load(Ordering::Relaxed), 1);
    assert_eq!(backend.loads().len(), 1);

    // after the download a new request is accepted again (reload)
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    wait_until("second download", || backend.loads().len() == 2).await;
    assert_eq!(watcher.next_publish().await.0, "Pump/x");
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn reload_keeps_a_single_registration_forwarder() {
    let backend = FakeBackend::with_twin(twin_id(1), "Pump", &["x"]);
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    for expected in 1..=2usize {
        publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
        wait_until("loaded", || {
            f.service.stats().loaded.load(Ordering::Relaxed) == expected as u64
        })
        .await;
    }
    let request =
        r#"{"linked_variable_return":"","linked_variables":[],"qualified_name":"Pump::f"}"#;
    publish(&requester, "Pump/registerFunction", request).await;
    let stats = f.service.stats();
    wait_until("forwarded", || !backend.registered().is_empty()).await;
    // two own requests (one per load) and the client's: a second forwarder would see the client's message too
    wait_until("messages seen", || {
        stats.registration_messages.load(Ordering::Relaxed) >= 3
    })
    .await;
    assert_eq!(stats.registration_messages.load(Ordering::Relaxed), 3);
    assert_eq!(backend.registered().len(), 1, "forwarded exactly once");
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn backend_error_does_not_stop_the_service() {
    let backend = FakeBackend::with_twin(twin_id(2), "Valve", &["open"]);
    backend.failing.lock().unwrap().insert(twin_id(1));
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    let mut watcher = Client::connect(f.broker.port(), "watcher").await;
    subscribe(&mut watcher, "#").await;
    let requester = Client::connect(f.broker.port(), "requester").await;

    // an HTTP error of the backend, and a twin the backend does not know
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    publish(&requester, "connectToTwin", connect_request(twin_id(3))).await;
    wait_until("both failed", || {
        f.service.stats().failed.load(Ordering::Relaxed) == 2
    })
    .await;
    // the service is still there: a good twin loads
    publish(&requester, "connectToTwin", connect_request(twin_id(2))).await;
    loop {
        let (topic, _) = watcher.next_publish().await;
        if topic == "Valve/open" {
            break;
        }
    }
    assert_eq!(f.service.stats().loaded.load(Ordering::Relaxed), 1);
    // a failed twin can be requested again (the in-progress mark was removed)
    backend.failing.lock().unwrap().remove(&twin_id(1));
    backend.add_twin(twin_id(1), "Pump", &["x"]);
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    loop {
        if watcher.next_publish().await.0 == "Pump/x" {
            break;
        }
    }
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn malformed_connect_payloads_are_ignored() {
    let backend = FakeBackend::with_twin(twin_id(1), "Pump", &["x"]);
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    let mut watcher = Client::connect(f.broker.port(), "watcher").await;
    subscribe(&mut watcher, "Pump/#").await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    for bad in [
        &b"garbage"[..],
        b"",
        br#"{"twin_id":"not-a-uuid","project_id":"also-not"}"#,
        br#"{"twin_id":"123e4567-e89b-12d3-a456-426614174000"}"#,
        &[0xff, 0xfe, 0x00],
        b"[]",
    ] {
        publish(&requester, "connectToTwin", bad).await;
    }
    wait_until("malformed counted", || {
        f.service.stats().malformed.load(Ordering::Relaxed) == 6
    })
    .await;
    assert!(backend.loads().is_empty());
    assert_eq!(f.service.stats().requests.load(Ordering::Relaxed), 0);
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    assert_eq!(watcher.next_publish().await.0, "Pump/x");
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn failed_login_is_retried_with_the_next_request() {
    let backend = FakeBackend::with_twin(twin_id(1), "Pump", &["x"]);
    backend.state.lock().unwrap().login_failures_left = 1;
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    // the login at startup fails, the server keeps running
    wait_until("startup login", || {
        backend.state.lock().unwrap().login_calls == 1
    })
    .await;
    let mut watcher = Client::connect(f.broker.port(), "watcher").await;
    subscribe(&mut watcher, "Pump/#").await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    assert_eq!(watcher.next_publish().await.0, "Pump/x");
    assert_eq!(backend.state.lock().unwrap().login_calls, 2);
    // logged in now: no further login for the next twin
    backend.add_twin(twin_id(2), "Valve", &["y"]);
    subscribe(&mut watcher, "Valve/#").await;
    publish(&requester, "connectToTwin", connect_request(twin_id(2))).await;
    loop {
        if watcher.next_publish().await.0 == "Valve/y" {
            break;
        }
    }
    assert_eq!(backend.state.lock().unwrap().login_calls, 2);
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn unavailable_backend_answers_every_request_with_an_error_and_keeps_running() {
    let f = fixture(Arc::new(UnavailableBackend), TwinOptions::default()).await;
    let mut watcher = Client::connect(f.broker.port(), "watcher").await;
    subscribe(&mut watcher, "#").await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    publish(&requester, "connectToTwin", connect_request(twin_id(2))).await;
    wait_until("both failed", || {
        f.service.stats().failed.load(Ordering::Relaxed) == 2
    })
    .await;
    assert_eq!(f.service.stats().loaded.load(Ordering::Relaxed), 0);
    // nothing but the two requests themselves were published, and the broker still works
    assert_eq!(watcher.next_publish().await.0, "connectToTwin");
    assert_eq!(watcher.next_publish().await.0, "connectToTwin");
    publish(&requester, "plain/topic", "x").await;
    assert_eq!(watcher.next_publish().await.0, "plain/topic");
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn downloads_run_on_a_bounded_pool() {
    let backend = Arc::new(FakeBackend::default());
    for n in 1..=6u128 {
        backend.add_twin(twin_id(n), &format!("T{n}"), &["x"]);
    }
    *backend.load_delay.lock().unwrap() = Duration::from_millis(150);
    let f = fixture(
        backend.clone(),
        TwinOptions {
            pool_size: 2,
            ..TwinOptions::default()
        },
    )
    .await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    for n in 1..=6u128 {
        publish(&requester, "connectToTwin", connect_request(twin_id(n))).await;
    }
    wait_until("all loaded", || {
        f.service.stats().loaded.load(Ordering::Relaxed) == 6
    })
    .await;
    assert_eq!(backend.max_running.load(Ordering::SeqCst), 2);
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn too_many_pending_twins_are_skipped_not_queued_forever() {
    let backend = Arc::new(FakeBackend::default());
    let (gate_tx, gate_rx) = std_mpsc::channel();
    *backend.gate.lock().unwrap() = Some(gate_rx);
    let f = fixture(
        backend.clone(),
        TwinOptions {
            pool_size: 1,
            max_pending: 3,
            ..TwinOptions::default()
        },
    )
    .await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    for n in 1..=10u128 {
        publish(&requester, "connectToTwin", connect_request(twin_id(n))).await;
    }
    wait_until("limit reached", || {
        f.service.stats().skipped.load(Ordering::Relaxed) == 7
    })
    .await;
    gate_tx.send(()).unwrap();
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn service_ends_when_the_hub_closes() {
    let backend = FakeBackend::with_twin(twin_id(1), "Pump", &["x"]);
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    wait_until("loaded", || {
        f.service.stats().loaded.load(Ordering::Relaxed) == 1
    })
    .await;
    f.broker.shutdown().await;
    // no explicit shutdown call: the end of the hub ends the service (and releases the twin)
    tokio::time::timeout(Duration::from_secs(10), f.service.join())
        .await
        .expect("the twin logic ends with the hub");
    assert_eq!(backend.state.lock().unwrap().released, vec![twin_id(1)]);
}

#[tokio::test]
async fn twin_names_that_would_become_wildcards_are_rejected() {
    let backend = Arc::new(FakeBackend::default());
    for (n, name) in [(1u128, "+"), (2, "#"), (3, "a/b"), (4, ""), (5, "$SYS")] {
        backend.add_twin(twin_id(n), name, &["x"]);
    }
    backend.add_twin(twin_id(6), "Pump", &["x"]);
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    // an innocent twin whose registrations a wildcard name would intercept
    let requester = Client::connect(f.broker.port(), "requester").await;
    for n in 1..=5u128 {
        publish(&requester, "connectToTwin", connect_request(twin_id(n))).await;
    }
    wait_until("all rejected", || {
        f.service.stats().failed.load(Ordering::Relaxed) == 5
    })
    .await;
    assert!(
        f.observed.0.lock().unwrap().is_empty(),
        "no twin was registered"
    );
    publish(&requester, "connectToTwin", connect_request(twin_id(6))).await;
    wait_until("good twin loaded", || {
        f.service.stats().loaded.load(Ordering::Relaxed) == 1
    })
    .await;
    // a registration for Pump reaches the backend exactly once, with the id of Pump
    let request =
        r#"{"linked_variable_return":"","linked_variables":[],"qualified_name":"Pump::f"}"#;
    publish(&requester, "Pump/registerFunction", request).await;
    wait_until("forwarded", || !backend.registered().is_empty()).await;
    assert_eq!(backend.registered(), vec![(twin_id(6), request.to_owned())]);
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn invalid_element_paths_are_skipped_and_counted_once_per_load() {
    let backend = FakeBackend::with_twin(
        twin_id(1),
        "Pump",
        &["ok", "bad/+", "#", "a//b", "", "Motor/speed", "x/#"],
    );
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    let mut watcher = Client::connect(f.broker.port(), "watcher").await;
    subscribe(&mut watcher, "Pump/#").await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    let mut topics = Vec::new();
    loop {
        let (topic, _) = watcher.next_publish().await;
        if topic == "Pump/registerFunction" {
            break;
        }
        topics.push(topic);
    }
    topics.sort();
    assert_eq!(topics, ["Pump/Motor/speed", "Pump/ok"]);
    assert_eq!(
        f.service.stats().elements_skipped.load(Ordering::Relaxed),
        5
    );
    assert_eq!(f.service.stats().loaded.load(Ordering::Relaxed), 1);
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn typed_initial_values_are_published_as_given_and_invalid_ones_skipped() {
    let tp = r#""time_point":"2026-11-17T09:40:12.345Z""#;
    let real = format!("{{{tp},\"value\":0.0}}");
    let flag = format!("{{{tp},\"value\":false}}");
    let backend = Arc::new(FakeBackend::default());
    backend.add_twin_with_values(
        twin_id(1),
        "Pump",
        &[
            ("speed", real.as_str()),
            ("on", flag.as_str()),
            // C++ placeholder of the old interface: neither a valid payload nor accepted
            ("broken", "{value=0, timepoint=\"\"}"),
        ],
    );
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    let mut watcher = Client::connect(f.broker.port(), "watcher").await;
    subscribe(&mut watcher, "Pump/#").await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    let mut published = Vec::new();
    loop {
        let (topic, payload) = watcher.next_publish().await;
        if topic == "Pump/registerFunction" {
            break;
        }
        published.push((topic, String::from_utf8(payload.to_vec()).unwrap()));
    }
    published.sort();
    assert_eq!(
        published,
        [
            ("Pump/on".to_owned(), flag),
            ("Pump/speed".to_owned(), real)
        ]
    );
    assert_eq!(
        f.service.stats().elements_skipped.load(Ordering::Relaxed),
        1
    );
    f.broker.shutdown().await;
    f.service.shutdown().await;
}

#[tokio::test]
async fn an_expired_session_is_refreshed_once_and_the_download_retried() {
    let backend = FakeBackend::with_twin(twin_id(1), "Pump", &["x"]);
    backend.unauthorized_once.lock().unwrap().insert(twin_id(1));
    let f = fixture(backend.clone(), TwinOptions::default()).await;
    wait_until("startup login", || {
        backend.state.lock().unwrap().login_calls == 1
    })
    .await;
    let requester = Client::connect(f.broker.port(), "requester").await;
    publish(&requester, "connectToTwin", connect_request(twin_id(1))).await;
    wait_until("loaded", || {
        f.service.stats().loaded.load(Ordering::Relaxed) == 1
    })
    .await;
    assert_eq!(
        backend.state.lock().unwrap().login_calls,
        2,
        "logged in again"
    );
    assert_eq!(backend.loads().len(), 2, "one refused, one successful load");
    assert_eq!(f.service.stats().relogins.load(Ordering::Relaxed), 1);
    assert_eq!(f.service.stats().failed.load(Ordering::Relaxed), 0);

    f.broker.shutdown().await;
    f.service.shutdown().await;
}
