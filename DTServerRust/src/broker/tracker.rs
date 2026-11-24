//! Connection counting in total and per remote IP. Port of `ConnectionTracker`.

use std::collections::HashMap;
use std::net::{IpAddr, SocketAddr};
use std::sync::{Arc, Mutex};

/// Normalizes the key: IPv4-mapped IPv6 addresses (`::ffff:a.b.c.d`) count like the plain IPv4 address.
pub fn normalize_ip(addr: IpAddr) -> IpAddr {
    match addr {
        IpAddr::V6(v6) => v6.to_ipv4_mapped().map_or(IpAddr::V6(v6), IpAddr::V4),
        v4 => v4,
    }
}

/// Convenience for [`normalize_ip`] on a socket address.
pub fn peer_key(addr: SocketAddr) -> IpAddr {
    normalize_ip(addr.ip())
}

#[derive(Debug, Default)]
struct State {
    total: usize,
    per_ip: HashMap<IpAddr, usize>,
}

/// Thread safe connection counter.
#[derive(Debug)]
pub struct ConnectionTracker {
    max_total: usize,
    max_per_ip: usize,
    state: Mutex<State>,
}

impl ConnectionTracker {
    pub fn new(max_total: usize, max_per_ip: usize) -> Arc<Self> {
        Arc::new(Self {
            max_total,
            max_per_ip,
            state: Mutex::new(State::default()),
        })
    }

    /// Reserves a slot. The key has to be normalized (see [`normalize_ip`]).
    pub fn try_acquire(&self, ip: IpAddr) -> bool {
        let mut st = self.state.lock().unwrap();
        if st.total >= self.max_total {
            return false;
        }
        let count = st.per_ip.get(&ip).copied().unwrap_or(0);
        if count >= self.max_per_ip {
            return false;
        }
        st.per_ip.insert(ip, count + 1);
        st.total += 1;
        true
    }

    /// Releases a slot. Releasing an IP without slot does nothing (as in the C++ implementation).
    pub fn release(&self, ip: IpAddr) {
        let mut st = self.state.lock().unwrap();
        let Some(count) = st.per_ip.get_mut(&ip) else {
            return;
        };
        *count -= 1;
        if *count == 0 {
            st.per_ip.remove(&ip);
        }
        st.total = st.total.saturating_sub(1);
    }

    pub fn total(&self) -> usize {
        self.state.lock().unwrap().total
    }

    /// Acquires a slot that is released when the permit is dropped.
    pub fn acquire_permit(self: &Arc<Self>, ip: IpAddr) -> Option<ConnectionPermit> {
        self.try_acquire(ip).then(|| ConnectionPermit {
            tracker: Arc::clone(self),
            ip,
        })
    }
}

/// RAII guard for one connection slot.
#[derive(Debug)]
pub struct ConnectionPermit {
    tracker: Arc<ConnectionTracker>,
    ip: IpAddr,
}

impl Drop for ConnectionPermit {
    fn drop(&mut self) {
        self.tracker.release(self.ip);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn ip(s: &str) -> IpAddr {
        s.parse().unwrap()
    }

    #[test]
    fn per_ip_limit() {
        let t = ConnectionTracker::new(10, 2);
        assert!(t.try_acquire(ip("10.0.0.1")));
        assert!(t.try_acquire(ip("10.0.0.1")));
        assert!(!t.try_acquire(ip("10.0.0.1")));
        assert!(t.try_acquire(ip("10.0.0.2")));
        assert_eq!(t.total(), 3);
        t.release(ip("10.0.0.1"));
        assert!(t.try_acquire(ip("10.0.0.1")));
    }

    #[test]
    fn total_limit() {
        let t = ConnectionTracker::new(2, 10);
        assert!(t.try_acquire(ip("10.0.0.1")));
        assert!(t.try_acquire(ip("10.0.0.2")));
        assert!(!t.try_acquire(ip("10.0.0.3")));
        t.release(ip("10.0.0.2"));
        assert!(t.try_acquire(ip("10.0.0.3")));
    }

    #[test]
    fn release_unknown_is_noop() {
        let t = ConnectionTracker::new(2, 2);
        assert!(t.try_acquire(ip("10.0.0.1")));
        t.release(ip("10.0.0.9"));
        assert_eq!(t.total(), 1);
        t.release(ip("10.0.0.1"));
        t.release(ip("10.0.0.1"));
        assert_eq!(t.total(), 0);
    }

    #[test]
    fn zero_limits_reject_everything() {
        assert!(!ConnectionTracker::new(0, 5).try_acquire(ip("10.0.0.1")));
        assert!(!ConnectionTracker::new(5, 0).try_acquire(ip("10.0.0.1")));
    }

    #[test]
    fn ipv4_mapped_counts_as_ipv4() {
        assert_eq!(normalize_ip(ip("::ffff:10.0.0.1")), ip("10.0.0.1"));
        assert_eq!(normalize_ip(ip("2001:db8::1")), ip("2001:db8::1"));
        assert_eq!(normalize_ip(ip("::1")), ip("::1"));
        let t = ConnectionTracker::new(10, 1);
        assert!(t.try_acquire(normalize_ip(ip("10.0.0.1"))));
        assert!(!t.try_acquire(normalize_ip(ip("::ffff:10.0.0.1"))));
    }

    #[test]
    fn permit_releases_on_drop() {
        let t = ConnectionTracker::new(1, 1);
        let p = t.acquire_permit(ip("10.0.0.1")).unwrap();
        assert!(t.acquire_permit(ip("10.0.0.1")).is_none());
        drop(p);
        assert_eq!(t.total(), 0);
        assert!(t.acquire_permit(ip("10.0.0.1")).is_some());
    }
}
