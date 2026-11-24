//! The pure part of the REST API: the words for the numeric codes of the schema, the decoding of the value
//! columns and the parsing of the query parameters. Nothing here touches the database.

use std::collections::HashMap;

use chrono::{DateTime, SecondsFormat, Utc};
use serde_json::{Number, Value};
use uuid::Uuid;

use super::ApiError;

/// `limit` of the series when the request names none.
pub const DEFAULT_LIMIT: i64 = 1000;
/// Largest accepted `limit` of the series.
pub const MAX_LIMIT: i64 = 10_000;

/// The word for `element.kind` (see the schema). Unknown codes (a newer schema) are reported as "unknown".
pub fn kind_word(kind: i16) -> &'static str {
    match kind {
        1 => "package",
        2 => "component",
        3 => "port",
        4 => "measurable",
        5 => "controllable",
        6 => "attribute",
        _ => "unknown",
    }
}

/// The word for `element.value_type`; `None` for elements without one.
pub fn value_type_word(value_type: Option<i16>) -> Option<&'static str> {
    value_type.map(|code| match code {
        1 => "real",
        2 => "integer",
        3 => "boolean",
        4 => "rational",
        5 => "complex",
        6 => "string",
        _ => "unknown",
    })
}

/// The word for `measurement.source`.
pub fn source_word(source: i16) -> &'static str {
    match source {
        1 => "client",
        2 => "derived",
        3 => "checkpoint",
        _ => "unknown",
    }
}

/// A timestamp as RFC 3339 in UTC (`2026-01-10T10:05:00Z`, fractional seconds only when present).
pub fn format_time(time: DateTime<Utc>) -> String {
    time.to_rfc3339_opts(SecondsFormat::AutoSi, true)
}

/// Decodes the three value columns into the natural JSON form, by the `value_type` of the element:
///
/// - real: number (JSON has no NaN or infinity: those become `null`)
/// - integer: number, or the string of digits when it was stored as text (magnitude above 2^53)
/// - boolean: `true` / `false`
/// - rational: the string `"n/d"`
/// - complex: the stored object
/// - string: string
///
/// A column that does not fit the type (or an unknown type) gives the first value present, in the order
/// object, text, number; nothing at all gives `null`.
pub fn decode_value(
    value_type: Option<i16>,
    num: Option<f64>,
    text: Option<&str>,
    json: Option<Value>,
) -> Value {
    let number = |n: f64| Number::from_f64(n).map_or(Value::Null, Value::Number);
    match (value_type, num, text, json) {
        (Some(1), Some(n), _, _) => number(n),
        (Some(2), _, Some(digits), _) => Value::String(digits.to_owned()),
        (Some(2), Some(n), None, _) => {
            // an integer column holds whole numbers up to 2^53, which i64 represents exactly
            if n.fract() == 0.0 && n.abs() < 9_007_199_254_740_992.0 {
                Value::Number(Number::from(n as i64))
            } else {
                number(n)
            }
        }
        (Some(3), Some(n), _, _) => Value::Bool(n != 0.0),
        (Some(4), _, Some(fraction), _) => Value::String(fraction.to_owned()),
        (Some(5), _, _, Some(object)) => object,
        (Some(6), _, Some(string), _) => Value::String(string.to_owned()),
        (_, num, text, json) => json
            .or_else(|| text.map(|t| Value::String(t.to_owned())))
            .or_else(|| num.map(number))
            .unwrap_or(Value::Null),
    }
}

/// The query string as a map. Rejects what is not understood instead of ignoring it, so a misspelled parameter
/// (`form=` for `from=`) is an error and not a silently unfiltered answer.
fn query_map(raw: Option<&str>, allowed: &[&str]) -> Result<HashMap<String, String>, ApiError> {
    let pairs: Vec<(String, String)> = serde_urlencoded::from_str(raw.unwrap_or(""))
        .map_err(|_| ApiError::bad_request("the query string is not valid"))?;
    let mut map = HashMap::new();
    for (key, value) in pairs {
        if !allowed.contains(&key.as_str()) {
            return Err(ApiError::bad_request(format!(
                "unknown parameter {key:?} (expected: {})",
                allowed.join(", ")
            )));
        }
        if map.insert(key.clone(), value).is_some() {
            return Err(ApiError::bad_request(format!(
                "parameter {key:?} is given more than once"
            )));
        }
    }
    Ok(map)
}

/// A UUID from a path segment or parameter. `what` names it in the error message.
pub fn parse_uuid(text: &str, what: &str) -> Result<Uuid, ApiError> {
    Uuid::try_parse(text.trim())
        .map_err(|_| ApiError::bad_request(format!("{what} is not a valid UUID")))
}

/// An RFC 3339 timestamp. A `+` in a query string is decoded as a space, so a space in front of the offset
/// digits (`2026-01-10T10:00:00 01:00`) is read as the `+` it was meant to be; clients should send `%2B`.
pub fn parse_time(text: &str, what: &str) -> Result<DateTime<Utc>, ApiError> {
    let text = text.trim();
    DateTime::parse_from_rfc3339(text)
        .or_else(|_| DateTime::parse_from_rfc3339(&text.replacen(' ', "+", 1)))
        .map(|t| t.with_timezone(&Utc))
        .map_err(|_| {
            ApiError::bad_request(format!(
                "{what} is not a valid RFC 3339 timestamp (for example 2026-01-10T10:00:00Z)"
            ))
        })
}

fn parse_commit(map: &HashMap<String, String>) -> Result<Option<Uuid>, ApiError> {
    map.get("commit")
        .map(|c| parse_uuid(c, "commit"))
        .transpose()
}

/// A subtree path: no NUL (PostgreSQL text cannot hold it), no trailing separators.
fn clean_path(text: &str) -> Result<&str, ApiError> {
    if text.contains('\0') {
        return Err(ApiError::bad_request("path contains a NUL character"));
    }
    Ok(text.trim_end_matches('/'))
}

/// `GET /twins/{id}/elements`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ElementsParams {
    pub commit: Option<Uuid>,
}

impl ElementsParams {
    pub fn parse(raw: Option<&str>) -> Result<Self, ApiError> {
        let map = query_map(raw, &["commit"])?;
        Ok(Self {
            commit: parse_commit(&map)?,
        })
    }
}

/// `GET /twins/{id}/state`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StateParams {
    pub at: DateTime<Utc>,
    pub commit: Option<Uuid>,
    /// Subtree filter: the element with this path and everything below it. `None`: the whole tree.
    pub path: Option<String>,
}

impl StateParams {
    /// `now` is the default of `at`.
    pub fn parse(raw: Option<&str>, now: DateTime<Utc>) -> Result<Self, ApiError> {
        let map = query_map(raw, &["at", "commit", "path"])?;
        let path = match map.get("path") {
            Some(path) => Some(clean_path(path)?).filter(|p| !p.is_empty()),
            None => None,
        };
        Ok(Self {
            at: map.get("at").map_or(Ok(now), |t| parse_time(t, "at"))?,
            commit: parse_commit(&map)?,
            path: path.map(str::to_owned),
        })
    }
}

/// Position in a series: the sort key `(received_at, change_no, element_no, source)` of the last row of a page.
/// Written `received_at,change_no,element_no,source`; a client treats it as opaque and passes the `next` of a
/// response as `after`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Cursor {
    pub received_at: DateTime<Utc>,
    pub change_no: i64,
    pub element_no: i32,
    pub source: i16,
}

impl Cursor {
    pub fn format(&self) -> String {
        format!(
            "{},{},{},{}",
            format_time(self.received_at),
            self.change_no,
            self.element_no,
            self.source
        )
    }

    pub fn parse(text: &str) -> Result<Self, ApiError> {
        let invalid = || ApiError::bad_request("after is not a cursor from a previous response");
        let parts: Vec<&str> = text.trim().split(',').collect();
        let [received_at, change_no, element_no, source] = parts[..] else {
            return Err(invalid());
        };
        Ok(Self {
            received_at: parse_time(received_at, "after").map_err(|_| invalid())?,
            change_no: change_no.parse().map_err(|_| invalid())?,
            element_no: element_no.parse().map_err(|_| invalid())?,
            source: source.parse().map_err(|_| invalid())?,
        })
    }
}

/// `GET /twins/{id}/series`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SeriesParams {
    /// Exact path of the variable.
    pub path: String,
    pub from: Option<DateTime<Utc>>,
    pub to: Option<DateTime<Utc>>,
    pub limit: i64,
    pub commit: Option<Uuid>,
    /// Continue after this row (the `next` of the previous page).
    pub after: Option<Cursor>,
}

impl SeriesParams {
    pub fn parse(raw: Option<&str>) -> Result<Self, ApiError> {
        let map = query_map(raw, &["path", "from", "to", "limit", "commit", "after"])?;
        let path = clean_path(map.get("path").map_or("", String::as_str))?;
        if path.is_empty() {
            return Err(ApiError::bad_request("the parameter path is required"));
        }
        let from = map.get("from").map(|t| parse_time(t, "from")).transpose()?;
        let to = map.get("to").map(|t| parse_time(t, "to")).transpose()?;
        if let (Some(from), Some(to)) = (from, to) {
            if from > to {
                return Err(ApiError::bad_request("from is later than to"));
            }
        }
        let limit = match map.get("limit") {
            None => DEFAULT_LIMIT,
            Some(text) => match text.trim().parse::<i64>() {
                Ok(limit) if (1..=MAX_LIMIT).contains(&limit) => limit,
                _ => {
                    return Err(ApiError::bad_request(format!(
                        "limit must be a number from 1 to {MAX_LIMIT}"
                    )))
                }
            },
        };
        Ok(Self {
            path: path.to_owned(),
            from,
            to,
            limit,
            commit: parse_commit(&map)?,
            after: map.get("after").map(|c| Cursor::parse(c)).transpose()?,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use chrono::TimeZone;
    use serde_json::json;

    fn now() -> DateTime<Utc> {
        Utc.with_ymd_and_hms(2026, 1, 20, 12, 0, 0).unwrap()
    }

    #[test]
    fn words_for_the_codes() {
        let kinds: Vec<_> = (1..=6).map(kind_word).collect();
        assert_eq!(
            kinds,
            [
                "package",
                "component",
                "port",
                "measurable",
                "controllable",
                "attribute"
            ]
        );
        let types: Vec<_> = (1..=6).map(|t| value_type_word(Some(t)).unwrap()).collect();
        assert_eq!(
            types,
            ["real", "integer", "boolean", "rational", "complex", "string"]
        );
        assert_eq!(value_type_word(None), None);
        assert_eq!(
            [source_word(1), source_word(2), source_word(3)],
            ["client", "derived", "checkpoint"]
        );
        assert_eq!(kind_word(0), "unknown");
        assert_eq!(kind_word(7), "unknown");
        assert_eq!(value_type_word(Some(9)), Some("unknown"));
        assert_eq!(source_word(0), "unknown");
    }

    #[test]
    fn decodes_every_value_type() {
        // real
        assert_eq!(decode_value(Some(1), Some(2.5), None, None), json!(2.5));
        assert_eq!(
            decode_value(Some(1), Some(f64::NAN), None, None),
            Value::Null
        );
        assert_eq!(
            decode_value(Some(1), Some(f64::INFINITY), None, None),
            Value::Null
        );
        // integer, as number and as text
        assert_eq!(decode_value(Some(2), Some(42.0), None, None), json!(42));
        assert_eq!(decode_value(Some(2), Some(-7.0), None, None), json!(-7));
        assert_eq!(
            decode_value(Some(2), Some(9_007_199_254_740_991.0), None, None),
            json!(9_007_199_254_740_991_i64)
        );
        assert_eq!(
            decode_value(Some(2), None, Some("9223372036854775807"), None),
            json!("9223372036854775807")
        );
        // boolean
        assert_eq!(decode_value(Some(3), Some(1.0), None, None), json!(true));
        assert_eq!(decode_value(Some(3), Some(0.0), None, None), json!(false));
        // rational
        assert_eq!(decode_value(Some(4), None, Some("3/4"), None), json!("3/4"));
        // complex
        let complex = json!({"imaginaryValue": 2.0, "realValue": 1.0});
        assert_eq!(
            decode_value(Some(5), None, None, Some(complex.clone())),
            complex
        );
        // string, including the empty one
        assert_eq!(decode_value(Some(6), None, Some("on"), None), json!("on"));
        assert_eq!(decode_value(Some(6), None, Some(""), None), json!(""));
    }

    #[test]
    fn mismatching_columns_fall_back_or_give_null() {
        assert_eq!(decode_value(Some(1), None, None, None), Value::Null);
        assert_eq!(decode_value(Some(4), Some(1.0), None, None), json!(1.0));
        assert_eq!(decode_value(None, Some(1.5), None, None), json!(1.5));
        assert_eq!(decode_value(Some(9), None, Some("x"), None), json!("x"));
        assert_eq!(decode_value(None, None, None, None), Value::Null);
        // a whole number above 2^53 stored as a double is shown as the number it is
        assert_eq!(decode_value(Some(2), Some(1e300), None, None), json!(1e300));
    }

    #[test]
    fn times_parse_and_format() {
        let t = parse_time("2026-01-10T11:00:00+01:00", "at").unwrap();
        assert_eq!(format_time(t), "2026-01-10T10:00:00Z");
        // `+` decoded as a space by the query string parser
        assert_eq!(parse_time("2026-01-10T11:00:00 01:00", "at").unwrap(), t);
        assert_eq!(
            format_time(parse_time("2026-01-10T10:00:00.250Z", "at").unwrap()),
            "2026-01-10T10:00:00.250Z"
        );
        for bad in ["", "yesterday", "2026-01-10", "2026-13-01T00:00:00Z", "1"] {
            assert!(parse_time(bad, "at").is_err(), "{bad:?}");
        }
        let message = parse_time("x", "from").unwrap_err().message().to_owned();
        assert!(message.starts_with("from "), "{message}");
    }

    #[test]
    fn state_parameters() {
        let p = StateParams::parse(None, now()).unwrap();
        assert_eq!(
            p,
            StateParams {
                at: now(),
                commit: None,
                path: None
            }
        );
        let id = "6f9619ff-8b86-d011-b42d-00c04fc964ff";
        let query = format!("at=2026-01-10T10:00:00Z&commit={id}&path=Motor%2Fspeed%2F");
        let p = StateParams::parse(Some(&query), now()).unwrap();
        assert_eq!(p.at, Utc.with_ymd_and_hms(2026, 1, 10, 10, 0, 0).unwrap());
        assert_eq!(p.commit, Some(id.parse().unwrap()));
        assert_eq!(p.path.as_deref(), Some("Motor/speed"), "trailing / removed");
        // an empty path is no filter
        assert_eq!(StateParams::parse(Some("path="), now()).unwrap().path, None);
        for bad in [
            "at=now",
            "commit=1234",
            "commit=",
            "path=a%00b",
            "x=1",
            "at=2026-01-10T10:00:00Z&at=2026-01-10T11:00:00Z",
        ] {
            assert!(StateParams::parse(Some(bad), now()).is_err(), "{bad:?}");
        }
    }

    #[test]
    fn elements_parameters() {
        assert_eq!(
            ElementsParams::parse(None).unwrap(),
            ElementsParams { commit: None }
        );
        assert!(ElementsParams::parse(Some("commit=x")).is_err());
        assert!(ElementsParams::parse(Some("at=2026-01-10T10:00:00Z")).is_err());
    }

    #[test]
    fn cursors_round_trip_and_reject_garbage() {
        let cursor = Cursor {
            received_at: Utc.with_ymd_and_hms(2026, 1, 10, 10, 30, 0).unwrap()
                + chrono::Duration::microseconds(123_456),
            change_no: 2099,
            element_no: 17,
            source: 1,
        };
        let text = cursor.format();
        assert_eq!(text, "2026-01-10T10:30:00.123456Z,2099,17,1");
        assert_eq!(Cursor::parse(&text).unwrap(), cursor);
        let p = SeriesParams::parse(Some(&format!("path=a&after={text}"))).unwrap();
        assert_eq!(p.after, Some(cursor));
        for bad in [
            "",
            "x",
            "2026-01-10T10:30:00Z,1,2",
            "2026-01-10T10:30:00Z,1,2,x",
            "x,1,2,3",
            "2026-01-10T10:30:00Z,1,2,3,4",
        ] {
            assert!(Cursor::parse(bad).is_err(), "{bad:?}");
        }
    }

    #[test]
    fn series_parameters_and_limit() {
        let p = SeriesParams::parse(Some("path=Motor/speed")).unwrap();
        assert_eq!(p.path, "Motor/speed");
        assert_eq!((p.from, p.to, p.commit), (None, None, None));
        assert_eq!(p.limit, DEFAULT_LIMIT);
        assert_eq!(DEFAULT_LIMIT, 1000);
        assert_eq!(MAX_LIMIT, 10_000);

        let limit = |l: &str| SeriesParams::parse(Some(&format!("path=a&limit={l}")));
        assert_eq!(limit("1").unwrap().limit, 1);
        assert_eq!(limit("10000").unwrap().limit, 10_000);
        for bad in ["0", "-1", "10001", "x", "", "1.5", "99999999999999999999"] {
            assert!(limit(bad).is_err(), "limit={bad:?}");
        }
        let message = limit("10001").unwrap_err().message().to_owned();
        assert!(message.contains("10000"), "{message}");

        let p = SeriesParams::parse(Some(
            "path=a&from=2026-01-10T10:00:00Z&to=2026-01-10T10:00:00Z",
        ))
        .unwrap();
        assert_eq!(p.from, p.to, "from and to may be equal");
        for bad in [
            "",
            "path=",
            "path=%2F",
            "from=2026-01-10T10:00:00Z",
            "path=a&from=2026-01-10T11:00:00Z&to=2026-01-10T10:00:00Z",
            "path=a&to=soon",
            "path=a&since=2026-01-10T10:00:00Z",
            "path=a&path=b",
        ] {
            assert!(SeriesParams::parse(Some(bad)).is_err(), "{bad:?}");
        }
    }
}
