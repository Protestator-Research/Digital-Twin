//! Protection of the login backend against guessing and floods: after too many failed logins of one user name or
//! from one IP address within a window, further attempts are refused for a short time without asking the backend,
//! and the number of simultaneously running checks per IP address is limited.

use std::collections::HashMap;
use std::hash::Hash;
use std::net::IpAddr;
use std::sync::{Arc, Mutex};
use std::time::Duration;

use tokio::time::Instant;

/// Largest number of tracked user names / IP addresses each; the entry with the oldest window goes first.
const MAX_TRACKED: usize = 10_000;

/// Settings of the throttle (`run.mqtt.auth_*`).
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ThrottleConfig {
    /// Failed logins within `window` after which the user name / IP address is blocked; 0 disables the blocking.
    pub max_failures: u32,
    /// Length of the window in which failures are counted.
    pub window: Duration,
    /// How long a blocked user name / IP address is refused immediately.
    pub block: Duration,
    /// Simultaneously running checks per IP address; 0 means no limit.
    pub max_pending_per_ip: usize,
}

impl Default for ThrottleConfig {
    fn default() -> Self {
        Self {
            max_failures: 5,
            window: Duration::from_secs(60),
            block: Duration::from_secs(10),
            max_pending_per_ip: 4,
        }
    }
}

/// Why a login attempt is not even checked.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Denied {
    /// Too many failures of this user name or from this IP address: answered like a wrong password.
    Blocked,
    /// Too many checks of this IP address are running: answered like an unavailable backend.
    TooManyPending,
}

#[derive(Debug, Clone, Copy)]
struct Entry {
    failures: u32,
    window_start: Instant,
    blocked_until: Option<Instant>,
}

struct Inner {
    users: HashMap<String, Entry>,
    ips: HashMap<IpAddr, Entry>,
    pending: HashMap<IpAddr, usize>,
}

/// Shared by all sessions of a broker.
pub struct LoginThrottle {
    config: ThrottleConfig,
    inner: Mutex<Inner>,
}

/// A running check of an IP address; ends the count when dropped.
pub struct PendingGuard {
    throttle: Arc<LoginThrottle>,
    ip: IpAddr,
    counted: bool,
}

impl Drop for PendingGuard {
    fn drop(&mut self) {
        if !self.counted {
            return;
        }
        let mut inner = self.throttle.inner.lock().unwrap();
        if let Some(n) = inner.pending.get_mut(&self.ip) {
            *n -= 1;
            if *n == 0 {
                inner.pending.remove(&self.ip);
            }
        }
    }
}

fn blocked<K: Eq + Hash>(map: &HashMap<K, Entry>, key: &K, now: Instant) -> bool {
    map.get(key)
        .and_then(|e| e.blocked_until)
        .is_some_and(|until| until > now)
}

fn fail<K: Eq + Hash + Clone>(map: &mut HashMap<K, Entry>, key: &K, config: &ThrottleConfig) {
    let now = Instant::now();
    if map.len() >= MAX_TRACKED && !map.contains_key(key) {
        map.retain(|_, e| {
            e.blocked_until.is_some_and(|u| u > now) || e.window_start + config.window > now
        });
        if map.len() >= MAX_TRACKED {
            let oldest = map
                .iter()
                .min_by_key(|(_, e)| e.window_start)
                .map(|(k, _)| k.clone());
            if let Some(oldest) = oldest {
                map.remove(&oldest);
            }
        }
    }
    let entry = map.entry(key.clone()).or_insert(Entry {
        failures: 0,
        window_start: now,
        blocked_until: None,
    });
    if entry.window_start + config.window <= now {
        entry.failures = 0;
        entry.window_start = now;
    }
    entry.failures += 1;
    if entry.failures >= config.max_failures {
        entry.blocked_until = Some(now + config.block);
        entry.failures = 0;
        entry.window_start = now;
    }
}

impl LoginThrottle {
    pub fn new(config: ThrottleConfig) -> Arc<Self> {
        Arc::new(Self {
            config,
            inner: Mutex::new(Inner {
                users: HashMap::new(),
                ips: HashMap::new(),
                pending: HashMap::new(),
            }),
        })
    }

    /// Decides whether a login of `username` from `ip` may be checked now. On success the returned guard has to
    /// be kept while the check runs.
    pub fn begin(self: &Arc<Self>, username: &str, ip: IpAddr) -> Result<PendingGuard, Denied> {
        let now = Instant::now();
        let mut inner = self.inner.lock().unwrap();
        if self.config.max_failures > 0
            && (blocked(&inner.ips, &ip, now) || blocked(&inner.users, &username.to_owned(), now))
        {
            return Err(Denied::Blocked);
        }
        let limit = self.config.max_pending_per_ip;
        let counted = limit > 0;
        if counted {
            let n = inner.pending.entry(ip).or_insert(0);
            if *n >= limit {
                return Err(Denied::TooManyPending);
            }
            *n += 1;
        }
        Ok(PendingGuard {
            throttle: Arc::clone(self),
            ip,
            counted,
        })
    }

    /// A check ended with "wrong credentials".
    pub fn record_failure(&self, username: &str, ip: IpAddr) {
        if self.config.max_failures == 0 {
            return;
        }
        let mut inner = self.inner.lock().unwrap();
        fail(&mut inner.users, &username.to_owned(), &self.config);
        fail(&mut inner.ips, &ip, &self.config);
    }

    /// A check ended with "valid": the failures of the user name are forgotten.
    pub fn record_success(&self, username: &str) {
        self.inner.lock().unwrap().users.remove(username);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn ip(last: u8) -> IpAddr {
        IpAddr::from([10, 0, 0, last])
    }

    fn config() -> ThrottleConfig {
        ThrottleConfig {
            max_failures: 3,
            window: Duration::from_secs(60),
            block: Duration::from_secs(10),
            max_pending_per_ip: 2,
        }
    }

    #[tokio::test(start_paused = true)]
    async fn blocks_a_user_after_repeated_failures_and_releases_later() {
        let t = LoginThrottle::new(config());
        for _ in 0..3 {
            assert!(t.begin("alice", ip(1)).is_ok());
            t.record_failure("alice", ip(1));
        }
        // blocked, also from another address, but other users are fine
        assert_eq!(t.begin("alice", ip(2)).err(), Some(Denied::Blocked));
        assert!(t.begin("bob", ip(2)).is_ok());
        tokio::time::advance(Duration::from_secs(11)).await;
        assert!(t.begin("alice", ip(2)).is_ok());
    }

    #[tokio::test(start_paused = true)]
    async fn blocks_an_ip_over_many_user_names() {
        let t = LoginThrottle::new(config());
        for name in ["a", "b", "c"] {
            t.record_failure(name, ip(1));
        }
        assert_eq!(t.begin("d", ip(1)).err(), Some(Denied::Blocked));
        assert!(t.begin("d", ip(2)).is_ok());
    }

    #[tokio::test(start_paused = true)]
    async fn failures_outside_the_window_do_not_add_up_and_success_resets() {
        let t = LoginThrottle::new(config());
        t.record_failure("alice", ip(1));
        t.record_failure("alice", ip(1));
        tokio::time::advance(Duration::from_secs(61)).await;
        t.record_failure("alice", ip(1));
        assert!(t.begin("alice", ip(1)).is_ok());
        t.record_failure("alice", ip(2));
        t.record_success("alice");
        t.record_failure("alice", ip(3));
        t.record_failure("alice", ip(4));
        assert!(t.begin("alice", ip(5)).is_ok());
    }

    #[tokio::test(start_paused = true)]
    async fn limits_pending_checks_per_ip() {
        let t = LoginThrottle::new(config());
        let g1 = t.begin("a", ip(1)).unwrap();
        let _g2 = t.begin("b", ip(1)).unwrap();
        assert_eq!(t.begin("c", ip(1)).err(), Some(Denied::TooManyPending));
        assert!(t.begin("c", ip(2)).is_ok());
        drop(g1);
        assert!(t.begin("c", ip(1)).is_ok());
    }

    #[tokio::test(start_paused = true)]
    async fn zero_disables() {
        let t = LoginThrottle::new(ThrottleConfig {
            max_failures: 0,
            max_pending_per_ip: 0,
            ..config()
        });
        for _ in 0..20 {
            t.record_failure("alice", ip(1));
            assert!(t.begin("alice", ip(1)).is_ok());
        }
    }
}
