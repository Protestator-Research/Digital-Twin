//! Subscription store: filter to sessions with a reverse index session to filters.
//! Port of `SubscriptionStorage`. The store only knows session ids, delivery is done by the caller.

use std::collections::{HashMap, HashSet};

use super::protocol::QoS;
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

/// Options of one subscription.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
struct Options {
    /// QoS granted to the subscriber: the upper bound of the QoS of the messages delivered through it.
    qos: QoS,
    /// MQTT 5 No Local.
    no_local: bool,
}

/// Not thread safe on its own; the broker keeps it behind a mutex.
#[derive(Debug, Default)]
pub struct SubscriptionStore {
    by_filter: HashMap<String, Vec<(SessionId, Options)>>,
    by_session: HashMap<SessionId, HashSet<String>>,
}

impl SubscriptionStore {
    pub fn new() -> Self {
        Self::default()
    }

    #[cfg(test)]
    /// Adds or updates a subscription with granted QoS 0. Updating an existing filter never counts against
    /// the quota.
    pub fn add(&mut self, session: SessionId, filter: &str, max_subscriptions: usize) -> AddResult {
        self.add_with_options(session, filter, max_subscriptions, false)
    }

    #[cfg(test)]
    /// Like [`SubscriptionStore::add`] with the MQTT 5 No Local option: with `no_local` the session does not
    /// get the messages it published itself through this subscription. An update replaces the option.
    pub fn add_with_options(
        &mut self,
        session: SessionId,
        filter: &str,
        max_subscriptions: usize,
        no_local: bool,
    ) -> AddResult {
        self.add_subscription(
            session,
            filter,
            max_subscriptions,
            QoS::AtMostOnce,
            no_local,
        )
    }

    /// Adds or updates a subscription that is granted `qos`. An update replaces QoS and No Local.
    pub fn add_subscription(
        &mut self,
        session: SessionId,
        filter: &str,
        max_subscriptions: usize,
        qos: QoS,
        no_local: bool,
    ) -> AddResult {
        let options = Options { qos, no_local };
        if let Some(existing) = self
            .by_filter
            .get_mut(filter)
            .and_then(|sessions| sessions.iter_mut().find(|(s, _)| *s == session))
        {
            existing.1 = options;
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
            .push((session, options));
        AddResult::Added
    }

    /// Removes one subscription. Returns true if the session had it.
    pub fn remove(&mut self, session: SessionId, filter: &str) -> bool {
        let mut removed = false;
        if let Some(sessions) = self.by_filter.get_mut(filter) {
            let before = sessions.len();
            sessions.retain(|(s, _)| *s != session);
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
                sessions.retain(|(s, _)| *s != session);
                if sessions.is_empty() {
                    self.by_filter.remove(&filter);
                }
            }
        }
    }

    #[cfg(test)]
    /// Number of distinct filters of a session.
    pub fn filter_count(&self, session: SessionId) -> usize {
        self.by_session.get(&session).map_or(0, HashSet::len)
    }

    /// Total number of (session, filter) subscriptions.
    pub fn subscription_count(&self) -> usize {
        self.by_session.values().map(HashSet::len).sum()
    }

    #[cfg(test)]
    /// True if no subscription is stored at all.
    pub fn is_empty(&self) -> bool {
        self.by_filter.is_empty() && self.by_session.is_empty()
    }

    /// The subscriptions of a session as (filter, granted QoS, No Local), in no particular order.
    pub fn subscriptions_of(&self, session: SessionId) -> Vec<(String, QoS, bool)> {
        let Some(filters) = self.by_session.get(&session) else {
            return Vec::new();
        };
        filters
            .iter()
            .filter_map(|filter| {
                let (_, o) = self
                    .by_filter
                    .get(filter)?
                    .iter()
                    .find(|(s, _)| *s == session)?;
                Some((filter.clone(), o.qos, o.no_local))
            })
            .collect()
    }

    #[cfg(test)]
    /// Sessions that receive a message on `topic`, each session once, even if several of its filters match.
    /// The publisher is not excluded.
    pub fn matching(&self, topic: &str) -> Vec<SessionId> {
        self.matching_from(topic, None)
    }

    #[cfg(test)]
    /// Like [`SubscriptionStore::matching`] for a message published by session `publisher`: a subscription
    /// with the No Local option does not count for the publisher, so the publisher only receives the message
    /// if one of its other matching subscriptions allows it.
    pub fn matching_from(&self, topic: &str, publisher: Option<SessionId>) -> Vec<SessionId> {
        self.targets_from(topic, publisher)
            .into_iter()
            .map(|(session, _)| session)
            .collect()
    }

    /// Like [`SubscriptionStore::matching_from`], with the QoS each session is granted for the message: the
    /// highest QoS of its matching subscriptions (that count for the publisher), because a session gets
    /// the message once, with the best level it asked for.
    pub fn targets_from(&self, topic: &str, publisher: Option<SessionId>) -> Vec<(SessionId, QoS)> {
        let mut best: HashMap<SessionId, QoS> = HashMap::new();
        for (filter, sessions) in &self.by_filter {
            if !match_filter(filter, topic) {
                continue;
            }
            for (session, options) in sessions {
                if publisher == Some(*session) && options.no_local {
                    continue;
                }
                best.entry(*session)
                    .and_modify(|q| *q = (*q).max(options.qos))
                    .or_insert(options.qos);
            }
        }
        best.into_iter().collect()
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

    #[test]
    fn no_local_excludes_only_the_publisher() {
        let mut s = SubscriptionStore::new();
        s.add_with_options(1, "a/#", 10, true);
        s.add(2, "a/#", 10);
        assert_eq!(sorted(s.matching_from("a/b", Some(1))), vec![2]);
        assert_eq!(sorted(s.matching_from("a/b", Some(2))), vec![1, 2]);
        assert_eq!(sorted(s.matching_from("a/b", None)), vec![1, 2]);
        assert_eq!(sorted(s.matching("a/b")), vec![1, 2]);
    }

    #[test]
    fn no_local_needs_every_matching_subscription_to_have_it() {
        let mut s = SubscriptionStore::new();
        s.add_with_options(1, "a/#", 10, true);
        s.add(1, "a/b", 10);
        // the second subscription has no no-local: the publisher still gets the message once
        assert_eq!(s.matching_from("a/b", Some(1)), vec![1]);
        assert!(s.matching_from("a/c", Some(1)).is_empty());
    }

    #[test]
    fn no_local_is_replaced_by_an_update_and_cleaned_up() {
        let mut s = SubscriptionStore::new();
        s.add_with_options(1, "a", 10, true);
        assert_eq!(s.add_with_options(1, "a", 10, false), AddResult::Updated);
        assert_eq!(s.matching_from("a", Some(1)), vec![1]);
        s.add_with_options(1, "a", 10, true);
        assert!(s.matching_from("a", Some(1)).is_empty());
        s.remove(1, "a");
        assert!(s.is_empty());
        s.add_with_options(1, "b", 10, true);
        s.remove_all(1);
        assert!(s.is_empty());
    }

    fn targets(
        s: &SubscriptionStore,
        topic: &str,
        publisher: Option<SessionId>,
    ) -> Vec<(SessionId, QoS)> {
        let mut v = s.targets_from(topic, publisher);
        v.sort_unstable();
        v
    }

    #[test]
    fn highest_granted_qos_of_the_matching_filters_counts() {
        let mut s = SubscriptionStore::new();
        s.add_subscription(1, "a/b", 10, QoS::AtLeastOnce, false);
        s.add_subscription(1, "a/+", 10, QoS::ExactlyOnce, false);
        s.add_subscription(1, "#", 10, QoS::AtMostOnce, false);
        s.add_subscription(2, "a/#", 10, QoS::AtLeastOnce, false);
        s.add(3, "a/b", 10);
        // one entry per session, with the best QoS of its matching filters
        assert_eq!(
            targets(&s, "a/b", None),
            vec![
                (1, QoS::ExactlyOnce),
                (2, QoS::AtLeastOnce),
                (3, QoS::AtMostOnce)
            ]
        );
        // only "#" matches
        assert_eq!(targets(&s, "x", None), vec![(1, QoS::AtMostOnce)]);
    }

    #[test]
    fn update_replaces_the_qos_and_no_local_stays_per_subscription() {
        let mut s = SubscriptionStore::new();
        assert_eq!(
            s.add_subscription(1, "t", 10, QoS::ExactlyOnce, true),
            AddResult::Added
        );
        assert!(targets(&s, "t", Some(1)).is_empty());
        assert_eq!(
            s.add_subscription(1, "t", 10, QoS::AtLeastOnce, false),
            AddResult::Updated
        );
        assert_eq!(targets(&s, "t", Some(1)), vec![(1, QoS::AtLeastOnce)]);
        assert_eq!(
            s.subscriptions_of(1),
            vec![("t".to_owned(), QoS::AtLeastOnce, false)]
        );
        // a no local subscription with a higher QoS does not raise the QoS of the publisher's other match
        s.add_subscription(1, "#", 10, QoS::ExactlyOnce, true);
        assert_eq!(targets(&s, "t", Some(1)), vec![(1, QoS::AtLeastOnce)]);
        assert_eq!(targets(&s, "t", Some(2)), vec![(1, QoS::ExactlyOnce)]);
        assert!(s.subscriptions_of(9).is_empty());
    }
}
