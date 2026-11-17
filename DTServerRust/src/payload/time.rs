//! `time_point`: ISO 8601 in UTC with milliseconds and `Z`, e.g. `2026-11-17T09:40:12.345Z`.
//! Port of `StringExtention::timepointToString` / `timepointFromString`.

use chrono::{DateTime, NaiveDate, TimeZone, Utc};
use thiserror::Error;

#[derive(Debug, Clone, PartialEq, Eq, Error)]
#[error("invalid timestamp: {0}")]
pub struct TimeError(pub String);

/// Formats with exactly three fractional digits (sub-millisecond parts are cut off).
pub fn format_time_point(t: DateTime<Utc>) -> String {
    let floored = DateTime::from_timestamp_millis(t.timestamp_millis()).unwrap_or(t);
    floored.format("%Y-%m-%dT%H:%M:%S%.3fZ").to_string()
}

/// Parses `YYYY-MM-DDTHH:MM:SS[.fff...]Z`. Like the C++ parser, fractional seconds are optional and
/// normalized to milliseconds (further digits are ignored); the suffix `Z` is required.
pub fn parse_time_point(s: &str) -> Result<DateTime<Utc>, TimeError> {
    let err = || TimeError(s.to_owned());
    let b = s.as_bytes();
    if b.len() < 20
        || b[4] != b'-'
        || b[7] != b'-'
        || b[10] != b'T'
        || b[13] != b':'
        || b[16] != b':'
    {
        return Err(err());
    }
    let num = |from: usize, to: usize| -> Result<u32, TimeError> {
        let digits = &b[from..to];
        if !digits.iter().all(u8::is_ascii_digit) {
            return Err(err());
        }
        Ok(digits
            .iter()
            .fold(0u32, |acc, d| acc * 10 + u32::from(d - b'0')))
    };
    let (year, month, day) = (num(0, 4)?, num(5, 7)?, num(8, 10)?);
    let (hour, minute, second) = (num(11, 13)?, num(14, 16)?, num(17, 19)?);

    let mut pos = 19;
    let mut millis = 0u32;
    if b[pos] == b'.' {
        pos += 1;
        let mut digits = 0;
        while pos < b.len() && b[pos].is_ascii_digit() {
            if digits < 3 {
                millis = millis * 10 + u32::from(b[pos] - b'0');
                digits += 1;
            }
            pos += 1;
        }
        while digits < 3 {
            millis *= 10;
            digits += 1;
        }
    }
    if &b[pos..] != b"Z" {
        return Err(err());
    }
    let date = NaiveDate::from_ymd_opt(year as i32, month, day).ok_or_else(err)?;
    let naive = date
        .and_hms_milli_opt(hour, minute, second, millis)
        .ok_or_else(err)?;
    Ok(Utc.from_utc_datetime(&naive))
}

/// serde adapter: `#[serde(with = "crate::payload::time::serde_time_point")]`.
pub mod serde_time_point {
    use chrono::{DateTime, Utc};
    use serde::{Deserialize, Deserializer, Serializer};

    pub fn serialize<S: Serializer>(t: &DateTime<Utc>, s: S) -> Result<S::Ok, S::Error> {
        s.serialize_str(&super::format_time_point(*t))
    }

    pub fn deserialize<'de, D: Deserializer<'de>>(d: D) -> Result<DateTime<Utc>, D::Error> {
        let text = String::deserialize(d)?;
        super::parse_time_point(&text).map_err(serde::de::Error::custom)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn at(y: i32, mo: u32, d: u32, h: u32, mi: u32, s: u32, ms: u32) -> DateTime<Utc> {
        Utc.from_utc_datetime(
            &NaiveDate::from_ymd_opt(y, mo, d)
                .unwrap()
                .and_hms_milli_opt(h, mi, s, ms)
                .unwrap(),
        )
    }

    #[test]
    fn formats_with_milliseconds_and_z() {
        assert_eq!(
            format_time_point(at(2026, 11, 17, 9, 40, 12, 345)),
            "2026-11-17T09:40:12.345Z"
        );
        assert_eq!(
            format_time_point(at(2026, 1, 2, 3, 4, 5, 0)),
            "2026-01-02T03:04:05.000Z"
        );
        assert_eq!(
            format_time_point(at(2026, 1, 2, 3, 4, 5, 7)),
            "2026-01-02T03:04:05.007Z"
        );
    }

    #[test]
    fn sub_millisecond_part_is_floored() {
        let t = Utc.from_utc_datetime(
            &NaiveDate::from_ymd_opt(2026, 1, 1)
                .unwrap()
                .and_hms_nano_opt(0, 0, 0, 999_999_999)
                .unwrap(),
        );
        assert_eq!(format_time_point(t), "2026-01-01T00:00:00.999Z");
    }

    #[test]
    fn parses_with_and_without_fraction() {
        let expected = at(2026, 11, 17, 9, 40, 12, 0);
        assert_eq!(parse_time_point("2026-11-17T09:40:12Z").unwrap(), expected);
        assert_eq!(
            parse_time_point("2026-11-17T09:40:12.345Z").unwrap(),
            at(2026, 11, 17, 9, 40, 12, 345)
        );
        assert_eq!(
            parse_time_point("2026-11-17T09:40:12.5Z").unwrap(),
            at(2026, 11, 17, 9, 40, 12, 500)
        );
        assert_eq!(
            parse_time_point("2026-11-17T09:40:12.05Z").unwrap(),
            at(2026, 11, 17, 9, 40, 12, 50)
        );
        // more than three digits are cut off, like in the C++ parser
        assert_eq!(
            parse_time_point("2026-11-17T09:40:12.123987Z").unwrap(),
            at(2026, 11, 17, 9, 40, 12, 123)
        );
        // a dot without digits is accepted by the C++ parser
        assert_eq!(parse_time_point("2026-11-17T09:40:12.Z").unwrap(), expected);
    }

    #[test]
    fn round_trip() {
        for s in [
            "2026-11-17T09:40:12.345Z",
            "1970-01-01T00:00:00.000Z",
            "2024-02-29T23:59:59.999Z",
        ] {
            assert_eq!(format_time_point(parse_time_point(s).unwrap()), s);
        }
    }

    #[test]
    fn rejects_invalid_input() {
        for s in [
            "",
            "2026-11-17",
            "2026-11-17T09:40:12",
            "2026-11-17T09:40:12+01:00",
            "2026-11-17 09:40:12Z",
            "2026-13-17T09:40:12Z",
            "2026-02-30T09:40:12Z",
            "2026-11-17T25:40:12Z",
            "2026-11-17T09:40:12.3ZZ",
            "2026-11-17T09:40:12.3z",
            "abcd-11-17T09:40:12Z",
            "2026-11-17T09:40:12Zjunk",
            "2026-11-17T09:40:12.34x5Z",
        ] {
            assert!(parse_time_point(s).is_err(), "{s:?} should be rejected");
        }
    }
}
