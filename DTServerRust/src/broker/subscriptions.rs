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

#[derive(Debug)]
struct Entry {
    session: SessionId,
    no_local: bool,
}

/// Not thread safe on its own; the broker keeps it behind a mutex.
#[derive(Debug, Default)]
pub struct SubscriptionStore {
    by_filter: HashMap<String, Vec<Entry>>,
    by_session: HashMap<SessionId, HashSet<String>>,
}

impl SubscriptionStore {
    pub fn new() -> Self {
        Self::default()
    }

    /// Adds or updates a subscription. Updating an existing filter never counts against the quota.
    pub fn add(
        &mut self,
        session: SessionId,
        filter: &str,
        no_local: bool,
        max_subscriptions: usize,
    ) -> AddResult {
        if let Some(entry) = self
            .by_filter
            .get_mut(filter)
            .and_then(|entries| entries.iter_mut().find(|e| e.session == session))
        {
            entry.no_local = no_local;
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
            .push(Entry { session, no_local });
        AddResult::Added
    }

    /// Removes one subscription. Returns true if the session had it.
    pub fn remove(&mut self, session: SessionId, filter: &str) -> bool {
        let mut removed = false;
        if let Some(entries) = self.by_filter.get_mut(filter) {
            let before = entries.len();
            entries.retain(|e| e.session != session);
            removed = entries.len() != before;
            if entries.is_empty() {
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
            if let Some(entries) = self.by_filter.get_mut(&filter) {
                entries.retain(|e| e.session != session);
                if entries.is_empty() {
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
    /// An entry with `no_local` is skipped if the session is the publisher (another matching filter of
    /// the same session without `no_local` still delivers).
    pub fn matching(&self, topic: &str, publisher: Option<SessionId>) -> Vec<SessionId> {
        let mut seen = HashSet::new();
        let mut out = Vec::new();
        for (filter, entries) in &self.by_filter {
            if !match_filter(filter, topic) {
                continue;
            }
            for entry in entries {
                if entry.no_local && Some(entry.session) == publisher {
                    continue;
                }
                if seen.insert(entry.session) {
                    out.push(entry.session);
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
        assert_eq!(s.add(1, "a", false, 2), AddResult::Added);
        assert_eq!(s.add(1, "b", false, 2), AddResult::Added);
        assert_eq!(s.add(1, "c", false, 2), AddResult::QuotaExceeded);
        // updating an existing filter never counts, even at the limit
        assert_eq!(s.add(1, "a", true, 2), AddResult::Updated);
        assert_eq!(s.filter_count(1), 2);
        // other sessions have their own quota
        assert_eq!(s.add(2, "c", false, 2), AddResult::Added);
        // a rejected add leaves no trace
        assert!(s.matching("c", None) == vec![2]);
    }

    #[test]
    fn quota_zero_leaves_no_state() {
        let mut s = SubscriptionStore::new();
        assert_eq!(s.add(1, "a", false, 0), AddResult::QuotaExceeded);
        assert!(s.is_empty());
    }

    #[test]
    fn remove_and_remove_all() {
        let mut s = SubscriptionStore::new();
        s.add(1, "a", false, 10);
        s.add(1, "b/#", false, 10);
        s.add(2, "a", false, 10);
        assert!(s.remove(1, "a"));
        assert!(!s.remove(1, "a"));
        assert_eq!(s.matching("a", None), vec![2]);
        assert_eq!(s.filter_count(1), 1);
        s.remove_all(1);
        assert_eq!(s.filter_count(1), 0);
        assert!(s.matching("b/x", None).is_empty());
        s.remove_all(2);
        assert!(s.is_empty());
        // unknown session is fine
        s.remove_all(99);
        assert!(!s.remove(99, "x"));
    }

    #[test]
    fn dedup_one_delivery_per_session() {
        let mut s = SubscriptionStore::new();
        s.add(1, "a/b", false, 10);
        s.add(1, "a/+", false, 10);
        s.add(1, "#", false, 10);
        s.add(2, "a/#", false, 10);
        assert_eq!(sorted(s.matching("a/b", None)), vec![1, 2]);
    }

    #[test]
    fn no_local() {
        let mut s = SubscriptionStore::new();
        s.add(1, "a/b", true, 10);
        s.add(2, "a/b", true, 10);
        s.add(3, "a/b", false, 10);
        // publisher 1 does not get its own message back
        assert_eq!(sorted(s.matching("a/b", Some(1))), vec![2, 3]);
        // without publisher (internal publish) everybody gets it
        assert_eq!(sorted(s.matching("a/b", None)), vec![1, 2, 3]);
        // a second filter without no_local still delivers to the publisher
        s.add(1, "a/+", false, 10);
        assert_eq!(sorted(s.matching("a/b", Some(1))), vec![1, 2, 3]);
        // updating the option
        s.add(3, "a/b", true, 10);
        assert_eq!(sorted(s.matching("a/b", Some(3))), vec![1, 2]);
    }
}
