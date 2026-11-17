//! Monthly partitions of `measurement`: names, bounds, which months to keep, and the accepted time window.
//!
//! Policy for timestamps without a partition (plan 6.1): there is no default partition. A row whose `ts` is
//! inside the accepted window gets its partition created on demand; a row outside the window is dropped and
//! counted. The window is `[max(2000-01-01, retention cutoff), end of next month)`, so at most about 320
//! partitions can ever be created by stray timestamps, and no row is accepted whose partition the retention
//! would drop right away.

use chrono::{DateTime, Datelike, Duration, NaiveDate, TimeZone, Utc};

/// Earliest accepted `ts`.
pub fn min_accepted_ts() -> DateTime<Utc> {
    Utc.with_ymd_and_hms(2000, 1, 1, 0, 0, 0).unwrap()
}

/// A calendar month (UTC) = one partition.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Month {
    year: i32,
    month: u32,
}

impl Month {
    pub fn new(year: i32, month: u32) -> Option<Month> {
        ((1..=9998).contains(&year) && (1..=12).contains(&month)).then_some(Month { year, month })
    }

    pub fn of(ts: DateTime<Utc>) -> Month {
        Month {
            year: ts.year(),
            month: ts.month(),
        }
    }

    pub fn year(self) -> i32 {
        self.year
    }

    pub fn month(self) -> u32 {
        self.month
    }

    pub fn next(self) -> Month {
        if self.month == 12 {
            Month {
                year: self.year + 1,
                month: 1,
            }
        } else {
            Month {
                year: self.year,
                month: self.month + 1,
            }
        }
    }

    /// First instant of the month (inclusive lower bound of the partition).
    pub fn start(self) -> DateTime<Utc> {
        let date = NaiveDate::from_ymd_opt(self.year, self.month, 1).expect("valid month");
        Utc.from_utc_datetime(&date.and_hms_opt(0, 0, 0).unwrap())
    }

    /// First instant of the next month (exclusive upper bound of the partition).
    pub fn end(self) -> DateTime<Utc> {
        self.next().start()
    }

    /// Name of the partition table, e.g. `measurement_y2026m11`.
    pub fn partition_name(self) -> String {
        format!("measurement_y{:04}m{:02}", self.year, self.month)
    }

    /// Inverse of [`Month::partition_name`]; other names (e.g. a partition somebody else created) give `None`.
    pub fn from_partition_name(name: &str) -> Option<Month> {
        let rest = name.strip_prefix("measurement_y")?;
        let (year, month) = rest.split_once('m')?;
        if year.len() != 4 || month.len() != 2 {
            return None;
        }
        if !year
            .bytes()
            .chain(month.bytes())
            .all(|b| b.is_ascii_digit())
        {
            return None;
        }
        Month::new(year.parse().ok()?, month.parse().ok()?)
    }

    /// DDL for the partition. Contains only numbers computed here, never external text.
    pub fn create_sql(self) -> String {
        let next = self.next();
        format!(
            "CREATE TABLE IF NOT EXISTS {} PARTITION OF measurement \
             FOR VALUES FROM ('{:04}-{:02}-01 00:00:00+00') TO ('{:04}-{:02}-01 00:00:00+00')",
            self.partition_name(),
            self.year,
            self.month,
            next.year,
            next.month
        )
    }

    pub fn drop_sql(self) -> String {
        format!("DROP TABLE IF EXISTS {}", self.partition_name())
    }
}

/// The partitions that must exist: the current and the next month.
pub fn months_to_ensure(now: DateTime<Utc>) -> [Month; 2] {
    let current = Month::of(now);
    [current, current.next()]
}

/// Rows with `ts` before this are not worth storing. `None` if everything is kept (`retention_days == 0`).
pub fn retention_cutoff(now: DateTime<Utc>, retention_days: u32) -> Option<DateTime<Utc>> {
    (retention_days > 0).then(|| {
        now.checked_sub_signed(Duration::days(i64::from(retention_days)))
            .unwrap_or(DateTime::<Utc>::MIN_UTC)
    })
}

/// Partitions that lie entirely before the cutoff (their exclusive upper bound is not after it).
pub fn partitions_to_drop(existing: &[Month], cutoff: DateTime<Utc>) -> Vec<Month> {
    let mut months: Vec<Month> = existing
        .iter()
        .copied()
        .filter(|m| m.end() <= cutoff)
        .collect();
    months.sort();
    months.dedup();
    months
}

/// The accepted range of `ts`: `[lower, upper)`.
pub fn accepted_window(now: DateTime<Utc>, retention_days: u32) -> (DateTime<Utc>, DateTime<Utc>) {
    let lower = match retention_cutoff(now, retention_days) {
        Some(cutoff) => cutoff.max(min_accepted_ts()),
        None => min_accepted_ts(),
    };
    let upper = Month::of(now).next().next().start();
    (lower, upper)
}

pub fn in_window(ts: DateTime<Utc>, now: DateTime<Utc>, retention_days: u32) -> bool {
    let (lower, upper) = accepted_window(now, retention_days);
    ts >= lower && ts < upper
}

#[cfg(test)]
mod tests {
    use super::*;

    fn t(y: i32, m: u32, d: u32, h: u32) -> DateTime<Utc> {
        Utc.with_ymd_and_hms(y, m, d, h, 0, 0).unwrap()
    }

    #[test]
    fn names_and_bounds() {
        let m = Month::of(t(2026, 11, 17, 9));
        assert_eq!(m.partition_name(), "measurement_y2026m11");
        assert_eq!(m.start(), t(2026, 11, 1, 0));
        assert_eq!(m.end(), t(2026, 12, 1, 0));
        assert_eq!(
            Month::new(2026, 3).unwrap().partition_name(),
            "measurement_y2026m03"
        );
        assert_eq!(
            Month::new(900, 3).unwrap().partition_name(),
            "measurement_y0900m03"
        );
    }

    #[test]
    fn year_rollover() {
        let dec = Month::new(2026, 12).unwrap();
        assert_eq!(dec.next(), Month::new(2027, 1).unwrap());
        assert_eq!(dec.end(), t(2027, 1, 1, 0));
        assert_eq!(
            months_to_ensure(t(2026, 12, 31, 23)),
            [Month::new(2026, 12).unwrap(), Month::new(2027, 1).unwrap()]
        );
        assert_eq!(
            dec.create_sql(),
            "CREATE TABLE IF NOT EXISTS measurement_y2026m12 PARTITION OF measurement \
             FOR VALUES FROM ('2026-12-01 00:00:00+00') TO ('2027-01-01 00:00:00+00')"
        );
        // the boundary instant belongs to the next month
        assert_eq!(Month::of(t(2027, 1, 1, 0)), Month::new(2027, 1).unwrap());
        assert_eq!(
            Month::of(t(2026, 12, 31, 23) + Duration::minutes(59)),
            Month::new(2026, 12).unwrap()
        );
    }

    #[test]
    fn leap_february_has_no_gap() {
        let feb = Month::new(2028, 2).unwrap();
        assert_eq!(feb.end(), t(2028, 3, 1, 0));
        assert_eq!(Month::of(t(2028, 2, 29, 12)), feb);
    }

    #[test]
    fn partition_name_round_trip_and_foreign_names() {
        for (y, m) in [(2026, 1), (2026, 12), (2000, 6), (9998, 12)] {
            let month = Month::new(y, m).unwrap();
            assert_eq!(
                Month::from_partition_name(&month.partition_name()),
                Some(month)
            );
        }
        for bad in [
            "measurement",
            "measurement_default",
            "measurement_y2026m13",
            "measurement_y2026m00",
            "measurement_y26m11",
            "measurement_y2026m1",
            "measurement_y2026m11_old",
            "other_y2026m11",
            "measurement_y+026m11",
        ] {
            assert_eq!(Month::from_partition_name(bad), None, "{bad}");
        }
        assert_eq!(Month::new(2026, 13), None);
        assert_eq!(Month::new(2026, 0), None);
    }

    #[test]
    fn drop_sql_names_only_the_partition() {
        assert_eq!(
            Month::new(2026, 1).unwrap().drop_sql(),
            "DROP TABLE IF EXISTS measurement_y2026m01"
        );
    }

    #[test]
    fn retention_selects_only_entirely_older_partitions() {
        let now = t(2026, 11, 17, 9);
        assert_eq!(retention_cutoff(now, 0), None);
        let cutoff = retention_cutoff(now, 30).unwrap();
        assert_eq!(cutoff, t(2026, 10, 18, 9));
        let all: Vec<Month> = (1..=12).map(|m| Month::new(2026, m).unwrap()).collect();
        // October ends 2026-11-01 > cutoff: it still holds rows that are younger than 30 days
        assert_eq!(
            partitions_to_drop(&all, cutoff),
            (1..=9)
                .map(|m| Month::new(2026, m).unwrap())
                .collect::<Vec<_>>()
        );
        // a cutoff exactly at a month boundary drops the month before it
        assert_eq!(
            partitions_to_drop(&all, t(2026, 10, 1, 0)),
            (1..=9)
                .map(|m| Month::new(2026, m).unwrap())
                .collect::<Vec<_>>()
        );
        assert!(partitions_to_drop(&all, t(2026, 1, 1, 0)).is_empty());
        // across the year boundary, unsorted input with duplicates
        let mixed = [
            Month::new(2027, 1).unwrap(),
            Month::new(2026, 12).unwrap(),
            Month::new(2026, 12).unwrap(),
        ];
        assert_eq!(
            partitions_to_drop(&mixed, t(2027, 1, 20, 0)),
            vec![Month::new(2026, 12).unwrap()]
        );
    }

    #[test]
    fn accepted_window_bounds() {
        let now = t(2026, 11, 17, 9);
        // keep everything: from 2000-01-01 to the end of next month
        assert!(in_window(t(2000, 1, 1, 0), now, 0));
        assert!(!in_window(t(1999, 12, 31, 23), now, 0));
        assert!(in_window(t(2026, 12, 31, 23), now, 0));
        assert!(!in_window(t(2027, 1, 1, 0), now, 0));
        // with retention the lower bound is the cutoff
        assert!(in_window(t(2026, 10, 18, 9), now, 30));
        assert!(!in_window(t(2026, 10, 18, 8), now, 30));
        // year rollover of the upper bound
        let dec = t(2026, 12, 15, 0);
        assert!(in_window(t(2027, 1, 31, 23), dec, 0));
        assert!(!in_window(t(2027, 2, 1, 0), dec, 0));
        // huge retention does not go below the minimum
        assert_eq!(accepted_window(now, 100_000).0, min_accepted_ts());
    }
}
