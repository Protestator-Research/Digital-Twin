//! Value payloads published on `<TwinName>/<element path>`.
//!
//! | Type | JSON |
//! |---|---|
//! | Real, Integer, Natural, Positive, Boolean | `{"time_point": "...", "value": <number or bool>}` |
//! | Rational | `{"time_point": "...", "value": "n/d"}` |
//! | Complex | `{"imaginary_value": x, "real_value": y, "time_point": "..."}` |
//!
//! [`classify`] determines the type of an arbitrary payload, see there for the limits.

use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use thiserror::Error;

use super::time::{parse_time_point, serde_time_point};

macro_rules! simple_value {
    ($(#[$doc:meta])* $name:ident, $ty:ty) => {
        $(#[$doc])*
        #[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
        pub struct $name {
            #[serde(with = "serde_time_point")]
            pub time_point: DateTime<Utc>,
            pub value: $ty,
        }

        impl $name {
            pub fn new(value: $ty, time_point: DateTime<Utc>) -> Self {
                Self { time_point, value }
            }

            pub fn from_json(payload: &[u8]) -> Result<Self, serde_json::Error> {
                serde_json::from_slice(payload)
            }

            pub fn to_json(&self) -> String {
                serde_json::to_string(self).expect("plain struct serializes")
            }
        }
    };
}

simple_value!(
    /// `RealValueEntity` (C++ `double`).
    RealValue,
    f64
);
simple_value!(
    /// `IntegerValueEntity` (C++ `int`).
    IntegerValue,
    i32
);
simple_value!(
    /// `NaturalValueEntity` (C++ `unsigned int`).
    NaturalValue,
    u32
);
simple_value!(
    /// `PositiveValueEntity` (C++ `unsigned int`; the C++ code does not reject 0 either).
    PositiveValue,
    u32
);
simple_value!(
    /// `BooleanValueEntity`.
    BooleanValue,
    bool
);

/// A reduced fraction with a positive denominator (like `boost::rational<long>`).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Rational {
    numerator: i64,
    denominator: i64,
}

#[derive(Debug, Clone, PartialEq, Eq, Error)]
#[error("invalid rational {0:?}, expected \"numerator/denominator\" with a non-zero denominator")]
pub struct RationalError(String);

fn gcd(a: u64, b: u64) -> u64 {
    if b == 0 {
        a
    } else {
        gcd(b, a % b)
    }
}

impl Rational {
    /// Reduces the fraction. The denominator must not be 0.
    pub fn new(numerator: i64, denominator: i64) -> Option<Self> {
        if denominator == 0 {
            return None;
        }
        let g = gcd(numerator.unsigned_abs(), denominator.unsigned_abs()).max(1);
        let sign: i128 = if denominator < 0 { -1 } else { 1 };
        let n = i64::try_from(i128::from(numerator) / i128::from(g) * sign).ok()?;
        let d = i64::try_from(i128::from(denominator).abs() / i128::from(g)).ok()?;
        Some(Self {
            numerator: n,
            denominator: d,
        })
    }

    pub fn numerator(&self) -> i64 {
        self.numerator
    }

    pub fn denominator(&self) -> i64 {
        self.denominator
    }

    /// Parses `"n/d"`.
    pub fn parse(s: &str) -> Result<Self, RationalError> {
        let bad = || RationalError(s.to_owned());
        let (n, d) = s.split_once('/').ok_or_else(bad)?;
        let n: i64 = n.trim().parse().map_err(|_| bad())?;
        let d: i64 = d.trim().parse().map_err(|_| bad())?;
        Rational::new(n, d).ok_or_else(bad)
    }

    pub fn as_f64(&self) -> f64 {
        self.numerator as f64 / self.denominator as f64
    }
}

impl std::fmt::Display for Rational {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}/{}", self.numerator, self.denominator)
    }
}

/// `RationalValueEntity`: `value` is the string `"n/d"`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct RationalValue {
    pub time_point: DateTime<Utc>,
    pub value: Rational,
}

#[derive(Serialize, Deserialize)]
struct RationalWire {
    #[serde(with = "serde_time_point")]
    time_point: DateTime<Utc>,
    value: String,
}

impl RationalValue {
    pub fn new(value: Rational, time_point: DateTime<Utc>) -> Self {
        Self { time_point, value }
    }

    pub fn from_json(payload: &[u8]) -> Result<Self, ValueError> {
        let wire: RationalWire = serde_json::from_slice(payload)?;
        Ok(Self {
            time_point: wire.time_point,
            value: Rational::parse(&wire.value)?,
        })
    }

    pub fn to_json(&self) -> String {
        let wire = RationalWire {
            time_point: self.time_point,
            value: self.value.to_string(),
        };
        serde_json::to_string(&wire).expect("plain struct serializes")
    }
}

/// `ComplexValueEntity`.
#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub struct ComplexValue {
    #[serde(rename = "imaginary_value")]
    pub imaginary_value: f64,
    #[serde(rename = "real_value")]
    pub real_value: f64,
    #[serde(with = "serde_time_point")]
    pub time_point: DateTime<Utc>,
}

impl ComplexValue {
    pub fn new(real_value: f64, imaginary_value: f64, time_point: DateTime<Utc>) -> Self {
        Self {
            imaginary_value,
            real_value,
            time_point,
        }
    }

    pub fn from_json(payload: &[u8]) -> Result<Self, serde_json::Error> {
        serde_json::from_slice(payload)
    }

    pub fn to_json(&self) -> String {
        serde_json::to_string(self).expect("plain struct serializes")
    }
}

#[derive(Debug, Error)]
pub enum ValueError {
    #[error("not valid JSON: {0}")]
    Json(#[from] serde_json::Error),
    #[error("payload is not a JSON object")]
    NotAnObject,
    #[error("missing field {0:?}")]
    MissingField(&'static str),
    #[error("field {0:?} has an unsupported type")]
    WrongType(&'static str),
    #[error(transparent)]
    Time(#[from] super::time::TimeError),
    #[error(transparent)]
    Rational(#[from] RationalError),
}

/// The value of a classified payload.
#[derive(Debug, Clone, PartialEq)]
pub enum Value {
    Real(f64),
    Integer(i64),
    /// A JSON integer that does not fit into an `i64`: the exact digits and the nearest double.
    BigInteger {
        digits: String,
        approx: f64,
    },
    /// Any JSON string that is not a fraction (`"n/d"`).
    Text(String),
    Boolean(bool),
    Rational(Rational),
    Complex {
        real: f64,
        imaginary: f64,
    },
}

impl Value {
    /// Name of the type, for logs and the history table.
    pub fn type_name(&self) -> &'static str {
        match self {
            Value::Real(_) => "Real",
            Value::Integer(_) | Value::BigInteger { .. } => "Integer",
            Value::Text(_) => "String",
            Value::Boolean(_) => "Boolean",
            Value::Rational(_) => "Rational",
            Value::Complex { .. } => "Complex",
        }
    }
}

/// A classified value payload.
#[derive(Debug, Clone, PartialEq)]
pub struct ValuePayload {
    pub time_point: DateTime<Utc>,
    pub value: Value,
}

/// Classifies a value payload by its JSON shape. Invalid JSON, a missing or malformed `time_point`, and
/// shapes that match no value type are errors.
///
/// - `real_value` and `imaginary_value` present: Complex
/// - `value` is a boolean: Boolean
/// - `value` is a string `"n/d"`: Rational; a string of the form integer/integer with denominator 0 is rejected
/// - `value` is any other string: Text (the String type). A String variable whose text looks like a fraction
///   (`"5/0"`) is therefore rejected, not published as Text
/// - `value` is a JSON integer: Integer; beyond `i64`: BigInteger (exact digits kept, never silently a Real)
/// - `value` is any other number (fraction or exponent): Real
///
/// Natural and Positive are integers on the wire and cannot be told apart from Integer by the payload
/// alone; they are classified as Integer (their values are valid Integers). Use [`NaturalValue`] or
/// [`PositiveValue`] when the type is known from the model.
pub fn classify(payload: &[u8]) -> Result<ValuePayload, ValueError> {
    let json: serde_json::Value = serde_json::from_slice(payload)?;
    let object = json.as_object().ok_or(ValueError::NotAnObject)?;
    let time_point = match object.get("time_point") {
        None => return Err(ValueError::MissingField("time_point")),
        Some(serde_json::Value::String(s)) => parse_time_point(s)?,
        Some(_) => return Err(ValueError::WrongType("time_point")),
    };

    if object.contains_key("real_value") || object.contains_key("imaginary_value") {
        let number = |key: &'static str| -> Result<f64, ValueError> {
            object
                .get(key)
                .ok_or(ValueError::MissingField(key))?
                .as_f64()
                .ok_or(ValueError::WrongType(key))
        };
        let value = Value::Complex {
            real: number("real_value")?,
            imaginary: number("imaginary_value")?,
        };
        return Ok(ValuePayload { time_point, value });
    }

    let value = match object
        .get("value")
        .ok_or(ValueError::MissingField("value"))?
    {
        serde_json::Value::Bool(b) => Value::Boolean(*b),
        serde_json::Value::String(s) => match Rational::parse(s) {
            Ok(r) => Value::Rational(r),
            // looks like a fraction but is not a valid one (denominator 0, out of range): an error, not text
            Err(e) if looks_like_fraction(s) => return Err(e.into()),
            Err(_) => Value::Text(s.clone()),
        },
        serde_json::Value::Number(n) => match n.as_i64() {
            Some(i) => Value::Integer(i),
            None => {
                let approx = n.as_f64().ok_or(ValueError::WrongType("value"))?;
                match raw_integer_token(payload) {
                    Some(digits) => Value::BigInteger { digits, approx },
                    None => Value::Real(approx),
                }
            }
        },
        _ => return Err(ValueError::WrongType("value")),
    };
    Ok(ValuePayload { time_point, value })
}

/// `-?digits/-?digits`
fn looks_like_fraction(s: &str) -> bool {
    let digits = |part: &str| {
        let part = part.trim().strip_prefix('-').unwrap_or(part.trim());
        !part.is_empty() && part.bytes().all(|b| b.is_ascii_digit())
    };
    s.split_once('/')
        .is_some_and(|(n, d)| digits(n) && digits(d))
}

/// The text of the `value` member if it is an integer literal (digits only). serde_json turns integers beyond
/// u64 into doubles, so the digits are read from the raw JSON.
fn raw_integer_token(payload: &[u8]) -> Option<String> {
    #[derive(serde::Deserialize)]
    struct Raw<'a> {
        #[serde(borrow)]
        value: Option<&'a serde_json::value::RawValue>,
    }
    let raw: Raw = serde_json::from_slice(payload).ok()?;
    let token = raw.value?.get().trim();
    let digits = token.strip_prefix('-').unwrap_or(token);
    (!digits.is_empty() && digits.bytes().all(|b| b.is_ascii_digit())).then(|| token.to_owned())
}

#[cfg(test)]
mod tests {
    use super::*;
    use chrono::TimeZone;

    fn t() -> DateTime<Utc> {
        Utc.with_ymd_and_hms(2026, 11, 17, 9, 40, 12).unwrap() + chrono::Duration::milliseconds(345)
    }

    const TP: &str = "2026-11-17T09:40:12.345Z";

    #[test]
    fn simple_values_are_byte_compatible() {
        assert_eq!(
            RealValue::new(1.5, t()).to_json(),
            format!(r#"{{"time_point":"{TP}","value":1.5}}"#)
        );
        assert_eq!(
            RealValue::new(2.0, t()).to_json(),
            format!(r#"{{"time_point":"{TP}","value":2.0}}"#)
        );
        assert_eq!(
            IntegerValue::new(-7, t()).to_json(),
            format!(r#"{{"time_point":"{TP}","value":-7}}"#)
        );
        assert_eq!(
            NaturalValue::new(0, t()).to_json(),
            format!(r#"{{"time_point":"{TP}","value":0}}"#)
        );
        assert_eq!(
            PositiveValue::new(42, t()).to_json(),
            format!(r#"{{"time_point":"{TP}","value":42}}"#)
        );
        assert_eq!(
            BooleanValue::new(true, t()).to_json(),
            format!(r#"{{"time_point":"{TP}","value":true}}"#)
        );
    }

    #[test]
    fn simple_values_round_trip() {
        let r = RealValue::new(-0.125, t());
        assert_eq!(RealValue::from_json(r.to_json().as_bytes()).unwrap(), r);
        let i = IntegerValue::new(i32::MIN, t());
        assert_eq!(IntegerValue::from_json(i.to_json().as_bytes()).unwrap(), i);
        let n = NaturalValue::new(u32::MAX, t());
        assert_eq!(NaturalValue::from_json(n.to_json().as_bytes()).unwrap(), n);
        let b = BooleanValue::new(false, t());
        assert_eq!(BooleanValue::from_json(b.to_json().as_bytes()).unwrap(), b);
    }

    #[test]
    fn real_accepts_integer_json_and_time_without_fraction() {
        let v =
            RealValue::from_json(br#"{"value": 5, "time_point": "2026-11-17T09:40:12Z"}"#).unwrap();
        assert_eq!(v.value, 5.0);
        assert_eq!(
            v.time_point,
            Utc.with_ymd_and_hms(2026, 11, 17, 9, 40, 12).unwrap()
        );
    }

    #[test]
    fn typed_parsing_rejects_out_of_range_and_missing_fields() {
        assert!(NaturalValue::from_json(
            format!(r#"{{"time_point":"{TP}","value":-1}}"#).as_bytes()
        )
        .is_err());
        assert!(IntegerValue::from_json(
            format!(r#"{{"time_point":"{TP}","value":3000000000}}"#).as_bytes()
        )
        .is_err());
        assert!(RealValue::from_json(br#"{"value":1.0}"#).is_err());
        assert!(RealValue::from_json(br#"{"timepoint":"x","value":0}"#).is_err());
    }

    #[test]
    fn rational_is_byte_compatible_and_reduced() {
        let v = RationalValue::new(Rational::new(1, 3).unwrap(), t());
        assert_eq!(
            v.to_json(),
            format!(r#"{{"time_point":"{TP}","value":"1/3"}}"#)
        );
        assert_eq!(RationalValue::from_json(v.to_json().as_bytes()).unwrap(), v);
        // boost::rational normalizes like Rational::new
        let r = Rational::parse("2/-4").unwrap();
        assert_eq!((r.numerator(), r.denominator()), (-1, 2));
        assert_eq!(Rational::parse("0/5").unwrap().to_string(), "0/1");
        assert_eq!(Rational::parse("10/5").unwrap().to_string(), "2/1");
    }

    #[test]
    fn rational_rejects_invalid_values() {
        for s in ["", "1", "1/0", "a/b", "1/", "/2", "1/2/3", "1.5/2"] {
            assert!(Rational::parse(s).is_err(), "{s:?}");
        }
        assert!(Rational::new(i64::MIN, -1).is_none());
        assert!(matches!(
            RationalValue::from_json(
                format!(r#"{{"time_point":"{TP}","value":"1/0"}}"#).as_bytes()
            ),
            Err(ValueError::Rational(_))
        ));
    }

    #[test]
    fn complex_is_byte_compatible() {
        let v = ComplexValue::new(1.5, -2.0, t());
        assert_eq!(
            v.to_json(),
            format!(r#"{{"imaginary_value":-2.0,"real_value":1.5,"time_point":"{TP}"}}"#)
        );
        assert_eq!(ComplexValue::from_json(v.to_json().as_bytes()).unwrap(), v);
    }

    #[test]
    fn classify_all_shapes() {
        let c = |json: String| classify(json.as_bytes()).unwrap().value;
        assert_eq!(c(RealValue::new(1.5, t()).to_json()), Value::Real(1.5));
        assert_eq!(c(RealValue::new(2.0, t()).to_json()), Value::Real(2.0));
        assert_eq!(c(IntegerValue::new(-7, t()).to_json()), Value::Integer(-7));
        assert_eq!(c(NaturalValue::new(3, t()).to_json()), Value::Integer(3));
        assert_eq!(c(PositiveValue::new(3, t()).to_json()), Value::Integer(3));
        assert_eq!(
            c(BooleanValue::new(true, t()).to_json()),
            Value::Boolean(true)
        );
        assert_eq!(
            c(RationalValue::new(Rational::new(2, 6).unwrap(), t()).to_json()),
            Value::Rational(Rational::new(1, 3).unwrap())
        );
        assert_eq!(
            c(ComplexValue::new(1.0, 2.0, t()).to_json()),
            Value::Complex {
                real: 1.0,
                imaginary: 2.0
            }
        );
        let p = classify(RealValue::new(1.0, t()).to_json().as_bytes()).unwrap();
        assert_eq!(p.time_point, t());
        assert_eq!(p.value.type_name(), "Real");
        // exponent notation is Real
        assert_eq!(
            c(format!(r#"{{"time_point":"{TP}","value":1e3}}"#)),
            Value::Real(1000.0)
        );
        // integers beyond i64 keep their digits
        assert_eq!(
            c(format!(
                r#"{{"time_point":"{TP}","value":18446744073709551615}}"#
            )),
            Value::BigInteger {
                digits: "18446744073709551615".into(),
                approx: 18446744073709551615.0
            }
        );
        assert_eq!(
            c(format!(
                r#"{{"time_point":"{TP}","value":-123456789012345678901234567890}}"#
            )),
            Value::BigInteger {
                digits: "-123456789012345678901234567890".into(),
                approx: -1.2345678901234568e29
            }
        );
        // a float literal with a huge integer part stays Real
        assert!(matches!(
            c(format!(
                r#"{{"time_point":"{TP}","value":123456789012345678901234567890.5}}"#
            )),
            Value::Real(_)
        ));
        // strings that are not fractions are String values
        assert_eq!(
            c(format!(r#"{{"time_point":"{TP}","value":"hello"}}"#)),
            Value::Text("hello".into())
        );
        assert_eq!(
            c(format!(r#"{{"time_point":"{TP}","value":""}}"#)),
            Value::Text(String::new())
        );
        assert_eq!(
            c(format!(r#"{{"time_point":"{TP}","value":"a/b"}}"#)),
            Value::Text("a/b".into())
        );
        assert_eq!(Value::Text(String::new()).type_name(), "String");
    }

    #[test]
    fn classify_rejects_invalid_payloads() {
        let bad: &[&str] = &[
            "",
            "not json",
            "{",
            "[]",
            "42",
            "null",
            "{}",
            // the old initial value of DigitalTwinManager.cpp:70 (neither valid JSON nor time_point)
            "{value=0, timepoint=\"\"}",
            r#"{"value":0,"timepoint":""}"#,
            r#"{"value":1}"#,
            r#"{"value":1,"time_point":5}"#,
            r#"{"value":1,"time_point":"yesterday"}"#,
            r#"{"time_point":"2026-11-17T09:40:12.345Z"}"#,
            r#"{"time_point":"2026-11-17T09:40:12.345Z","value":null}"#,
            r#"{"time_point":"2026-11-17T09:40:12.345Z","value":[1]}"#,
            r#"{"time_point":"2026-11-17T09:40:12.345Z","value":"1/0"}"#,
            r#"{"time_point":"2026-11-17T09:40:12.345Z","real_value":1}"#,
            r#"{"time_point":"2026-11-17T09:40:12.345Z","real_value":"1","imaginary_value":2}"#,
        ];
        for payload in bad {
            assert!(
                classify(payload.as_bytes()).is_err(),
                "{payload:?} should be rejected"
            );
        }
        assert!(classify(&[0xFF, 0xFE]).is_err());
    }
}
