//! Subscription store: filter to sessions with a reverse index session to filters.
//! Port of `SubscriptionStorage`. The store only knows session ids, delivery is done by the caller.

use std::collections::{HashMap, HashSet};

use super::topic::match_filter;

/// Identity of a session inside the broker.
pub type SessionId = u64;

/// Result of [`SubscriptionStore::add`].
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AddResult {
    Added,
    Updated,
    QuotaExceeded,
}

/// Not thread safe on its own; the broker keeps it behind a mutex.
#[derive(Debug, Default)]
pub struct SubscriptionStore {
    by_filter: HashMap<String, Vec<SessionId>>,
    by_session: HashMap<SessionId, HashSet<String>>,
}

impl SubscriptionStore {
    pub fn new() -> Self {
        Self::default()
    }

    /// Adds or updates a subscription. Updating an existing filter never counts against the quota.
    pub fn add(&mut self, session: SessionId, filter: &str, max_subscriptions: usize) -> AddResult {
        if self
            .by_filter
            .get(filter)
            .is_some_and(|sessions| sessions.contains(&session))
        {
            return AddResult::Updated;
        }
        let owned = self.by_session.entry(session).or_default();
        if owned.len() >= max_subscriptions {
            if owned.is_empty() {
                self.by_session.remove(&session);
            }
            return AddResult::QuotaExceeded;
        }
        owned.insert(filter.to_owned());
        self.by_filter
            .entry(filter.to_owned())
            .or_default()
            .push(session);
        AddResult::Added
    }

    /// Removes one subscription. Returns true if the session had it.
    pub fn remove(&mut self, session: SessionId, filter: &str) -> bool {
        let mut removed = false;
        if let Some(sessions) = self.by_filter.get_mut(filter) {
            let before = sessions.len();
            sessions.retain(|s| *s != session);
            removed = sessions.len() != before;
            if sessions.is_empty() {
                self.by_filter.remove(filter);
            }
        }
        if let Some(owned) = self.by_session.get_mut(&session) {
            owned.remove(filter);
            if owned.is_empty() {
                self.by_session.remove(&session);
            }
        }
        removed
    }

    /// Removes every subscription of the session.
    pub fn remove_all(&mut self, session: SessionId) {
        let Some(filters) = self.by_session.remove(&session) else {
            return;
        };
        for filter in filters {
            if let Some(sessions) = self.by_filter.get_mut(&filter) {
                sessions.retain(|s| *s != session);
                if sessions.is_empty() {
                    self.by_filter.remove(&filter);
                }
            }
        }
    }

    /// Number of distinct filters of a session.
    pub fn filter_count(&self, session: SessionId) -> usize {
        self.by_session.get(&session).map_or(0, HashSet::len)
    }

    /// Total number of (session, filter) subscriptions.
    pub fn subscription_count(&self) -> usize {
        self.by_session.values().map(HashSet::len).sum()
    }

    /// True if no subscription is stored at all.
    pub fn is_empty(&self) -> bool {
        self.by_filter.is_empty() && self.by_session.is_empty()
    }

    /// Sessions that receive a message on `topic`, each session once, even if several of its filters match.
    /// The publisher is not excluded (MQTT 3.1.1 has no no-local option).
    pub fn matching(&self, topic: &str) -> Vec<SessionId> {
        let mut seen = HashSet::new();
        let mut out = Vec::new();
        for (filter, sessions) in &self.by_filter {
            if !match_filter(filter, topic) {
                continue;
            }
            for &session in sessions {
                if seen.insert(session) {
                    out.push(session);
                }
            }
        }
        out
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn sorted(mut v: Vec<SessionId>) -> Vec<SessionId> {
        v.sort_unstable();
        v
    }

    #[test]
    fn add_update_and_quota() {
        let mut s = SubscriptionStore::new();
        assert_eq!(s.add(1, "a", 2), AddResult::Added);
        assert_eq!(s.add(1, "b", 2), AddResult::Added);
        assert_eq!(s.add(1, "c", 2), AddResult::QuotaExceeded);
        // updating an existing filter never counts, even at the limit
        assert_eq!(s.add(1, "a", 2), AddResult::Updated);
        assert_eq!(s.filter_count(1), 2);
        // other sessions have their own quota
        assert_eq!(s.add(2, "c", 2), AddResult::Added);
        // a rejected add leaves no trace
        assert!(s.matching("c") == vec![2]);
    }

    #[test]
    fn quota_zero_leaves_no_state() {
        let mut s = SubscriptionStore::new();
        assert_eq!(s.add(1, "a", 0), AddResult::QuotaExceeded);
        assert!(s.is_empty());
    }

    #[test]
    fn remove_and_remove_all() {
        let mut s = SubscriptionStore::new();
        s.add(1, "a", 10);
        s.add(1, "b/#", 10);
        s.add(2, "a", 10);
        assert!(s.remove(1, "a"));
        assert!(!s.remove(1, "a"));
        assert_eq!(s.matching("a"), vec![2]);
        assert_eq!(s.filter_count(1), 1);
        s.remove_all(1);
        assert_eq!(s.filter_count(1), 0);
        assert!(s.matching("b/x").is_empty());
        s.remove_all(2);
        assert!(s.is_empty());
        // unknown session is fine
        s.remove_all(99);
        assert!(!s.remove(99, "x"));
    }

    #[test]
    fn dedup_one_delivery_per_session() {
        let mut s = SubscriptionStore::new();
        s.add(1, "a/b", 10);
        s.add(1, "a/+", 10);
        s.add(1, "#", 10);
        s.add(2, "a/#", 10);
        assert_eq!(sorted(s.matching("a/b")), vec![1, 2]);
    }
}
