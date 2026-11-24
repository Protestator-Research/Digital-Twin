//! Verification of credentials against the SysMLv2 backend (`POST <base>/login`, as
//! `SysMLAPIImplementation::loginUserWithPassword` of the C++ library does it).

use std::collections::HashMap;
use std::future::Future;
use std::pin::Pin;
use std::sync::Mutex;
use std::time::Duration;

use sha2::{Digest, Sha256};
use tokio::sync::Semaphore;
use tokio::time::Instant;
use tracing::debug;

use super::{AuthOutcome, Authenticator, DEFAULT_CACHE_TTL_S, DEFAULT_MAX_CONCURRENT};

/// Largest login response that is read; a real one is a few hundred bytes.
const MAX_RESPONSE_BYTES: usize = 64 * 1024;
/// Largest number of remembered logins.
const CACHE_CAPACITY: usize = 1024;

/// Tuning of a [`SysmlAuthenticator`].
#[derive(Debug, Clone)]
pub struct SysmlAuthOptions {
    /// How long a successful login is remembered; zero disables the cache.
    pub cache_ttl: Duration,
    /// Maximum number of simultaneous requests to the backend.
    pub max_concurrent: usize,
    /// Time allowed to establish the connection to the backend.
    pub connect_timeout: Duration,
    /// Time allowed for one login request in total.
    pub timeout: Duration,
}

impl Default for SysmlAuthOptions {
    fn default() -> Self {
        Self {
            cache_ttl: Duration::from_secs(DEFAULT_CACHE_TTL_S),
            max_concurrent: DEFAULT_MAX_CONCURRENT,
            connect_timeout: Duration::from_secs(10),
            timeout: Duration::from_secs(15),
        }
    }
}

/// The login URL for a backend address and port, built like `CommunicationService` does for its base URL: an
/// address that already contains a scheme is used as it is (the port is ignored then), otherwise port 443 means
/// `https://<url>`, port 80 `http://<url>` and any other port `http://<url>:<port>/`.
pub fn login_url(address: &str, port: u16) -> String {
    let base = if address.contains("://") {
        address.to_owned()
    } else {
        match port {
            443 => format!("https://{address}"),
            80 => format!("http://{address}"),
            _ => format!("http://{address}:{port}/"),
        }
    };
    format!("{}/login", base.trim_end_matches('/'))
}

/// The URL for log messages: a `user:password@` part is replaced by `***@`.
pub fn redact_url(url: &str) -> String {
    let Some((scheme, rest)) = url.split_once("://") else {
        return url.to_owned();
    };
    let authority_end = rest.find(['/', '?', '#']).unwrap_or(rest.len());
    match rest[..authority_end].rfind('@') {
        Some(at) => format!("{scheme}://***@{}", &rest[at + 1..]),
        None => url.to_owned(),
    }
}

type CacheKey = (String, [u8; 32]);

/// Remembers successful logins for a while. The password is never stored: the key is the user name and a
/// SHA-256 of a random per-process salt and the password.
struct LoginCache {
    ttl: Duration,
    capacity: usize,
    salt: [u8; 32],
    entries: Mutex<HashMap<CacheKey, Instant>>,
}

impl LoginCache {
    fn new(ttl: Duration, capacity: usize) -> Self {
        // uuid v4 is filled from the operating system's random number generator
        let mut salt = [0u8; 32];
        salt[..16].copy_from_slice(uuid::Uuid::new_v4().as_bytes());
        salt[16..].copy_from_slice(uuid::Uuid::new_v4().as_bytes());
        Self {
            ttl,
            capacity,
            salt,
            entries: Mutex::new(HashMap::new()),
        }
    }

    fn key(&self, username: &str, password: &[u8]) -> CacheKey {
        let mut hasher = Sha256::new();
        hasher.update(self.salt);
        hasher.update(password);
        (username.to_owned(), hasher.finalize().into())
    }

    fn enabled(&self) -> bool {
        !self.ttl.is_zero()
    }

    fn contains(&self, username: &str, password: &[u8]) -> bool {
        if !self.enabled() {
            return false;
        }
        let key = self.key(username, password);
        let mut entries = self.entries.lock().unwrap();
        match entries.get(&key) {
            Some(expires) if *expires > Instant::now() => true,
            Some(_) => {
                entries.remove(&key);
                false
            }
            None => false,
        }
    }

    fn insert(&self, username: &str, password: &[u8]) {
        if !self.enabled() {
            return;
        }
        let key = self.key(username, password);
        let now = Instant::now();
        let mut entries = self.entries.lock().unwrap();
        if entries.len() >= self.capacity && !entries.contains_key(&key) {
            entries.retain(|_, expires| *expires > now);
            if entries.len() >= self.capacity {
                // all entries have the same lifetime, so the one that expires first is the oldest
                let oldest = entries
                    .iter()
                    .min_by_key(|(_, expires)| **expires)
                    .map(|(k, _)| k.clone());
                if let Some(oldest) = oldest {
                    entries.remove(&oldest);
                }
            }
        }
        entries.insert(key, now + self.ttl);
    }

    #[cfg(test)]
    fn len(&self) -> usize {
        self.entries.lock().unwrap().len()
    }
}

/// Checks credentials with the login endpoint of the SysMLv2 backend. 2xx with a `bearer` token means valid,
/// 401 and 403 mean rejected, everything else (throttling, server errors, network problems, an answer that
/// does not look like a login response) means the check could not be made. The token is not used.
pub struct SysmlAuthenticator {
    client: reqwest::Client,
    url: String,
    cache: LoginCache,
    limiter: Semaphore,
}

impl SysmlAuthenticator {
    /// For the backend at `address` and `port` (see [`login_url`]).
    pub fn new(address: &str, port: u16, options: SysmlAuthOptions) -> Result<Self, String> {
        Self::with_url(login_url(address, port), options)
    }

    /// For an explicit login URL.
    pub fn with_url(url: String, options: SysmlAuthOptions) -> Result<Self, String> {
        let client = reqwest::Client::builder()
            .connect_timeout(options.connect_timeout)
            .timeout(options.timeout)
            // like the C++ client: no redirects
            .redirect(reqwest::redirect::Policy::none())
            .user_agent("dt-server")
            .build()
            .map_err(|e| format!("cannot create the HTTP client: {e}"))?;
        Ok(Self {
            client,
            url,
            cache: LoginCache::new(options.cache_ttl, CACHE_CAPACITY),
            limiter: Semaphore::new(options.max_concurrent.max(1)),
        })
    }

    /// The login URL requests go to.
    pub fn url(&self) -> &str {
        &self.url
    }

    async fn request(&self, username: &str, password: &str) -> AuthOutcome {
        let Ok(_permit) = self.limiter.acquire().await else {
            return AuthOutcome::Unavailable("login limiter closed".into());
        };
        let body = serde_json::json!({ "username": username, "password": password }).to_string();
        let sent = self
            .client
            .post(&self.url)
            .header(reqwest::header::CONTENT_TYPE, "application/json")
            .header(reqwest::header::ACCEPT, "application/json")
            .body(body)
            .send()
            .await;
        // error texts of reqwest can contain the URL, so only the kind of the failure is reported
        let mut response = match sent {
            Ok(response) => response,
            Err(e) => return AuthOutcome::Unavailable(failure_kind(&e).into()),
        };
        let status = response.status();
        if status == reqwest::StatusCode::UNAUTHORIZED || status == reqwest::StatusCode::FORBIDDEN {
            return AuthOutcome::Rejected;
        }
        if !status.is_success() {
            return AuthOutcome::Unavailable(format!("login answered with status {status}"));
        }
        let mut body = Vec::new();
        loop {
            match response.chunk().await {
                Ok(Some(chunk)) => {
                    if body.len() + chunk.len() > MAX_RESPONSE_BYTES {
                        return AuthOutcome::Unavailable("login response too large".into());
                    }
                    body.extend_from_slice(&chunk);
                }
                Ok(None) => break,
                Err(e) => return AuthOutcome::Unavailable(failure_kind(&e).into()),
            }
        }
        match serde_json::from_slice::<serde_json::Value>(&body) {
            Ok(value)
                if value
                    .get("bearer")
                    .is_some_and(serde_json::Value::is_string) =>
            {
                AuthOutcome::Accepted
            }
            _ => AuthOutcome::Unavailable("login response without bearer token".into()),
        }
    }
}

fn failure_kind(e: &reqwest::Error) -> &'static str {
    if e.is_timeout() {
        "login request timed out"
    } else if e.is_connect() {
        "cannot connect to the login endpoint"
    } else {
        "login request failed"
    }
}

impl Authenticator for SysmlAuthenticator {
    fn verify<'a>(
        &'a self,
        username: &'a str,
        password: &'a [u8],
    ) -> Pin<Box<dyn Future<Output = AuthOutcome> + Send + 'a>> {
        Box::pin(async move {
            // the backend takes the password as a JSON string
            let Ok(text) = std::str::from_utf8(password) else {
                return AuthOutcome::Rejected;
            };
            if username.is_empty() {
                return AuthOutcome::Rejected;
            }
            if self.cache.contains(username, password) {
                debug!("login served from the cache");
                return AuthOutcome::Accepted;
            }
            let outcome = self.request(username, text).await;
            if outcome == AuthOutcome::Accepted {
                self.cache.insert(username, password);
            }
            outcome
        })
    }
}

#[cfg(test)]
mod tests {
    use std::sync::atomic::{AtomicUsize, Ordering};
    use std::sync::Arc;

    use tokio::io::{AsyncReadExt, AsyncWriteExt};
    use tokio::net::TcpListener;

    use super::*;

    #[test]
    fn url_building_follows_the_cpp_client() {
        assert_eq!(
            login_url("structura.example", 443),
            "https://structura.example/login"
        );
        assert_eq!(
            login_url("structura.example", 80),
            "http://structura.example/login"
        );
        assert_eq!(login_url("localhost", 8088), "http://localhost:8088/login");
        // an address with a scheme is used as it is, the port is ignored
        assert_eq!(
            login_url("https://h.example", 8088),
            "https://h.example/login"
        );
        assert_eq!(
            login_url("http://h.example:9000/", 1),
            "http://h.example:9000/login"
        );
        // with a path
        assert_eq!(
            login_url("https://h.example/api/", 443),
            "https://h.example/api/login"
        );
    }

    #[test]
    fn urls_are_redacted_for_logs() {
        assert_eq!(
            redact_url("https://u:p@h.example/api"),
            "https://***@h.example/api"
        );
        assert_eq!(redact_url("http://u@h:8/x@y"), "http://***@h:8/x@y");
        assert_eq!(redact_url("http://h.example/a@b"), "http://h.example/a@b");
        assert_eq!(redact_url("h.example"), "h.example");
    }

    #[tokio::test(start_paused = true)]
    async fn cache_hit_expiry_and_password_change() {
        let cache = LoginCache::new(Duration::from_secs(10), 16);
        assert!(!cache.contains("u", b"pw"));
        cache.insert("u", b"pw");
        assert!(cache.contains("u", b"pw"));
        // another password or user is no hit
        assert!(!cache.contains("u", b"other"));
        assert!(!cache.contains("v", b"pw"));
        tokio::time::advance(Duration::from_secs(9)).await;
        assert!(cache.contains("u", b"pw"));
        tokio::time::advance(Duration::from_secs(2)).await;
        assert!(!cache.contains("u", b"pw"));
        assert_eq!(cache.len(), 0);
    }

    #[tokio::test(start_paused = true)]
    async fn cache_is_bounded_and_can_be_disabled() {
        let cache = LoginCache::new(Duration::from_secs(100), 3);
        for i in 0..3u8 {
            cache.insert("u", &[i]);
            tokio::time::advance(Duration::from_secs(1)).await;
        }
        cache.insert("u", &[9]);
        assert_eq!(cache.len(), 3);
        // the oldest one was dropped
        assert!(!cache.contains("u", &[0]));
        assert!(cache.contains("u", &[1]) && cache.contains("u", &[9]));
        let off = LoginCache::new(Duration::ZERO, 3);
        off.insert("u", b"pw");
        assert!(!off.contains("u", b"pw"));
        assert_eq!(off.len(), 0);
    }

    #[test]
    fn cache_key_does_not_hold_the_password() {
        let cache = LoginCache::new(Duration::from_secs(10), 4);
        let (user, hash) = cache.key("u", b"secret-password");
        assert_eq!(user, "u");
        assert!(!hash.windows(6).any(|w| w == b"secret"));
        // the salt differs between caches
        let other = LoginCache::new(Duration::from_secs(10), 4);
        assert_ne!(other.key("u", b"secret-password").1, hash);
    }

    /// What the fake backend answers.
    #[derive(Clone)]
    enum Reply {
        Raw(String),
        /// Accepts the connection and never answers.
        Hang,
    }

    struct Fake {
        url: String,
        requests: Arc<Mutex<Vec<String>>>,
        hits: Arc<AtomicUsize>,
    }

    /// A tiny HTTP server that answers every request with the same canned response and records the requests.
    async fn fake_backend(reply: Reply) -> Fake {
        let listener = TcpListener::bind("127.0.0.1:0").await.unwrap();
        let url = format!("http://{}/login", listener.local_addr().unwrap());
        let requests = Arc::new(Mutex::new(Vec::new()));
        let hits = Arc::new(AtomicUsize::new(0));
        let (r, h) = (Arc::clone(&requests), Arc::clone(&hits));
        tokio::spawn(async move {
            loop {
                let Ok((mut socket, _)) = listener.accept().await else {
                    return;
                };
                let (reply, r, h) = (reply.clone(), Arc::clone(&r), Arc::clone(&h));
                tokio::spawn(async move {
                    let mut data = Vec::new();
                    let mut chunk = [0u8; 4096];
                    // read until the headers and the announced body are complete
                    loop {
                        let Ok(n) = socket.read(&mut chunk).await else {
                            return;
                        };
                        if n == 0 {
                            return;
                        }
                        data.extend_from_slice(&chunk[..n]);
                        if let Some(end) = data.windows(4).position(|w| w == b"\r\n\r\n") {
                            let head = String::from_utf8_lossy(&data[..end]).to_lowercase();
                            let len = head
                                .lines()
                                .find_map(|l| l.strip_prefix("content-length:"))
                                .and_then(|v| v.trim().parse::<usize>().ok())
                                .unwrap_or(0);
                            if data.len() >= end + 4 + len {
                                break;
                            }
                        }
                    }
                    r.lock()
                        .unwrap()
                        .push(String::from_utf8_lossy(&data).into_owned());
                    h.fetch_add(1, Ordering::SeqCst);
                    match reply {
                        Reply::Raw(text) => {
                            let _ = socket.write_all(text.as_bytes()).await;
                            let _ = socket.shutdown().await;
                        }
                        Reply::Hang => tokio::time::sleep(Duration::from_secs(60)).await,
                    }
                });
            }
        });
        Fake {
            url,
            requests,
            hits,
        }
    }

    fn json_reply(status: &str, body: &str) -> Reply {
        Reply::Raw(format!(
            "HTTP/1.1 {status}\r\nContent-Type: application/json\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{body}",
            body.len()
        ))
    }

    fn options() -> SysmlAuthOptions {
        SysmlAuthOptions {
            cache_ttl: Duration::from_secs(60),
            ..SysmlAuthOptions::default()
        }
    }

    async fn outcome_for(reply: Reply) -> AuthOutcome {
        let fake = fake_backend(reply).await;
        let auth = SysmlAuthenticator::with_url(fake.url, options()).unwrap();
        auth.verify("alice", b"pw").await
    }

    #[tokio::test]
    async fn accepts_2xx_with_bearer_and_sends_the_documented_request() {
        let fake = fake_backend(json_reply("200 OK", r#"{"bearer":"tok-123"}"#)).await;
        let auth = SysmlAuthenticator::with_url(fake.url.clone(), options()).unwrap();
        assert_eq!(auth.verify("al\"ice", b"p\\w").await, AuthOutcome::Accepted);
        let requests = fake.requests.lock().unwrap();
        assert_eq!(requests.len(), 1);
        let request = &requests[0];
        assert!(request.starts_with("POST /login HTTP/1.1\r\n"), "{request}");
        assert!(request
            .to_lowercase()
            .contains("content-type: application/json"));
        let body = request.split("\r\n\r\n").nth(1).unwrap();
        let value: serde_json::Value = serde_json::from_str(body).unwrap();
        assert_eq!(
            value,
            serde_json::json!({"username": "al\"ice", "password": "p\\w"})
        );
    }

    #[tokio::test]
    async fn status_and_body_mapping() {
        assert_eq!(
            outcome_for(json_reply("201 Created", r#"{"bearer":"t","x":1}"#)).await,
            AuthOutcome::Accepted
        );
        for status in ["401 Unauthorized", "403 Forbidden"] {
            assert_eq!(
                outcome_for(json_reply(status, r#"{"message":"no"}"#)).await,
                AuthOutcome::Rejected,
                "{status}"
            );
        }
        for status in [
            "429 Too Many Requests",
            "500 Internal Server Error",
            "503 Service Unavailable",
            "404 Not Found",
            "302 Found",
        ] {
            assert!(
                matches!(
                    outcome_for(json_reply(status, "{}")).await,
                    AuthOutcome::Unavailable(_)
                ),
                "{status}"
            );
        }
        // 2xx that does not carry a bearer string
        for body in [
            "{}",
            r#"{"bearer":null}"#,
            r#"{"bearer":5}"#,
            "not json",
            "",
        ] {
            assert!(
                matches!(
                    outcome_for(json_reply("200 OK", body)).await,
                    AuthOutcome::Unavailable(_)
                ),
                "{body:?}"
            );
        }
    }

    #[tokio::test]
    async fn unavailable_messages_never_contain_secrets() {
        let fake = fake_backend(json_reply(
            "500 Internal Server Error",
            r#"{"bearer":"tok-xyz"}"#,
        ))
        .await;
        let auth = SysmlAuthenticator::with_url(fake.url, options()).unwrap();
        let AuthOutcome::Unavailable(message) = auth.verify("alice", b"hunter2").await else {
            panic!("expected Unavailable");
        };
        assert!(!message.contains("hunter2") && !message.contains("tok-xyz"));
    }

    #[tokio::test]
    async fn unreachable_backend_is_unavailable() {
        // a port nobody listens on
        let listener = std::net::TcpListener::bind("127.0.0.1:0").unwrap();
        let url = format!("http://{}/login", listener.local_addr().unwrap());
        drop(listener);
        let auth = SysmlAuthenticator::with_url(url, options()).unwrap();
        assert!(matches!(
            auth.verify("alice", b"pw").await,
            AuthOutcome::Unavailable(_)
        ));
    }

    #[tokio::test]
    async fn hanging_backend_times_out() {
        let fake = fake_backend(Reply::Hang).await;
        let options = SysmlAuthOptions {
            timeout: Duration::from_millis(200),
            ..options()
        };
        let auth = SysmlAuthenticator::with_url(fake.url, options).unwrap();
        let AuthOutcome::Unavailable(message) = auth.verify("alice", b"pw").await else {
            panic!("expected Unavailable");
        };
        assert!(message.contains("timed out"), "{message}");
    }

    #[tokio::test]
    async fn success_is_cached_failure_is_not() {
        let fake = fake_backend(json_reply("200 OK", r#"{"bearer":"t"}"#)).await;
        let auth = SysmlAuthenticator::with_url(fake.url, options()).unwrap();
        assert_eq!(auth.verify("alice", b"pw").await, AuthOutcome::Accepted);
        assert_eq!(auth.verify("alice", b"pw").await, AuthOutcome::Accepted);
        assert_eq!(fake.hits.load(Ordering::SeqCst), 1);
        // another password is checked again
        auth.verify("alice", b"other").await;
        assert_eq!(fake.hits.load(Ordering::SeqCst), 2);

        let fake = fake_backend(json_reply("401 Unauthorized", "{}")).await;
        let auth = SysmlAuthenticator::with_url(fake.url, options()).unwrap();
        for _ in 0..2 {
            assert_eq!(auth.verify("alice", b"bad").await, AuthOutcome::Rejected);
        }
        assert_eq!(fake.hits.load(Ordering::SeqCst), 2);
    }

    #[tokio::test]
    async fn unusable_credentials_are_rejected_without_a_request() {
        let fake = fake_backend(json_reply("200 OK", r#"{"bearer":"t"}"#)).await;
        let auth = SysmlAuthenticator::with_url(fake.url, options()).unwrap();
        assert_eq!(
            auth.verify("alice", &[0xFF, 0xFE]).await,
            AuthOutcome::Rejected
        );
        assert_eq!(auth.verify("", b"pw").await, AuthOutcome::Rejected);
        assert_eq!(fake.hits.load(Ordering::SeqCst), 0);
    }
}
