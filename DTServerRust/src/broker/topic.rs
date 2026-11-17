//! Topic name and filter helpers. Port of `SubscriptionStorage::matchFilter` / `split` and
//! `Session::topicWithinLimits`.

use super::limits::BrokerLimits;

/// Splits a topic or filter into its levels. An empty string has one (empty) level.
pub fn split(s: &str) -> Vec<&str> {
    s.split('/').collect()
}

/// Returns true if `topic` matches `filter`. Behaves exactly like the C++ implementation:
/// `+` matches exactly one level (also an empty one), `#` matches the rest and only counts as a
/// wildcard when it is the last level of the filter (otherwise nothing matches). `a/#` also matches `a`.
pub fn match_filter(filter: &str, topic: &str) -> bool {
    let f = split(filter);
    let t = split(topic);
    for (i, level) in f.iter().enumerate() {
        if *level == "#" {
            return i + 1 == f.len();
        }
        if i >= t.len() {
            return false;
        }
        if *level == "+" {
            continue;
        }
        if *level != t[i] {
            return false;
        }
    }
    f.len() == t.len()
}

/// Length (bytes) and level limits for topic names and filters.
pub fn within_limits(topic: &str, limits: &BrokerLimits) -> bool {
    if topic.len() > limits.max_topic_length {
        return false;
    }
    let levels = topic.bytes().filter(|b| *b == b'/').count() + 1;
    levels <= limits.max_topic_levels
}

/// Syntax check for a subscription filter (MQTT 5, 4.7): not empty, `+` and `#` only as a whole
/// level, `#` only as last level.
pub fn is_valid_filter(filter: &str) -> bool {
    if filter.is_empty() {
        return false;
    }
    let levels = split(filter);
    let last = levels.len() - 1;
    levels.iter().enumerate().all(|(i, level)| match *level {
        "#" => i == last,
        "+" => true,
        other => !other.contains(['+', '#']),
    })
}

/// Syntax check for the topic name of a PUBLISH: not empty and without wildcards.
pub fn is_valid_topic_name(topic: &str) -> bool {
    !topic.is_empty() && !topic.contains(['+', '#'])
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn matching_table() {
        let cases: &[(&str, &str, bool)] = &[
            ("a/b", "a/b", true),
            ("a/b", "a/c", false),
            ("a/b", "a", false),
            ("a", "a/b", false),
            ("+", "a", true),
            ("+", "a/b", false),
            ("a/+", "a/b", true),
            ("a/+", "a/b/c", false),
            ("a/+/c", "a/b/c", true),
            ("a/+/c", "a/b/d", false),
            ("+/+", "a/b", true),
            ("#", "a", true),
            ("#", "a/b/c", true),
            ("#", "", true),
            ("a/#", "a/b/c", true),
            ("a/#", "a", true),
            ("a/#", "b/c", false),
            ("a/#/b", "a/x/b", false),
            ("a/#/b", "a/b", false),
            ("a#", "a#", true),
            // empty levels
            ("", "", true),
            ("", "a", false),
            ("+", "", true),
            ("a//b", "a//b", true),
            ("a/+/b", "a//b", true),
            ("a//b", "a/x/b", false),
            ("/a", "/a", true),
            ("+/a", "/a", true),
            ("a/", "a/", true),
            ("a/+", "a/", true),
            ("a/", "a", false),
            // matching is case sensitive and does not special case `$`
            ("A", "a", false),
            ("#", "$SYS/x", true),
        ];
        for (filter, topic, expected) in cases {
            assert_eq!(
                match_filter(filter, topic),
                *expected,
                "filter {filter:?} topic {topic:?}"
            );
        }
    }

    #[test]
    fn limits() {
        let limits = BrokerLimits {
            max_topic_length: 10,
            max_topic_levels: 3,
            ..BrokerLimits::default()
        };
        assert!(within_limits("a/b/c", &limits));
        assert!(!within_limits("a/b/c/d", &limits));
        assert!(within_limits("0123456789", &limits));
        assert!(!within_limits("01234567890", &limits));
        assert!(within_limits("", &limits));
    }

    #[test]
    fn filter_syntax() {
        for ok in ["a", "a/b", "+", "#", "a/+/b", "a/#", "+/#", "/", "a//b"] {
            assert!(is_valid_filter(ok), "{ok:?}");
        }
        for bad in ["", "a/#/b", "a#", "a+", "a/b#", "#/a", "++"] {
            assert!(!is_valid_filter(bad), "{bad:?}");
        }
        assert!(is_valid_topic_name("a/b"));
        assert!(!is_valid_topic_name(""));
        assert!(!is_valid_topic_name("a/+"));
        assert!(!is_valid_topic_name("a/#"));
    }
}
