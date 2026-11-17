//! Rows of `measurement`, and the column vectors of a batch insert.

use chrono::{DateTime, Utc};
use uuid::Uuid;

use crate::payload::values::{Value, ValuePayload};

/// `value_type` codes of the table.
pub mod value_type {
    pub const REAL: i16 = 1;
    pub const INTEGER: i16 = 2;
    pub const BOOLEAN: i16 = 3;
    pub const RATIONAL: i16 = 4;
    pub const COMPLEX: i16 = 5;
    pub const STRING: i16 = 6;
}

/// Integers up to this magnitude are exact in a double.
const MAX_EXACT_F64_INT: u64 = 1 << 53;

#[derive(Debug, Clone, PartialEq)]
pub struct Row {
    pub twin_id: Uuid,
    pub project_id: Uuid,
    pub element_path: String,
    /// `time_point` of the payload.
    pub ts: DateTime<Utc>,
    /// Arrival at the broker.
    pub received_at: DateTime<Utc>,
    pub value_type: i16,
    pub value_num: Option<f64>,
    pub value_text: Option<String>,
    /// JSON text, cast to `jsonb` by the insert statement.
    pub value_json: Option<String>,
}

impl Row {
    pub fn new(
        twin_id: Uuid,
        project_id: Uuid,
        element_path: &str,
        payload: &ValuePayload,
        received_at: DateTime<Utc>,
    ) -> Row {
        let (value_type, value_num, value_text, value_json) = value_columns(&payload.value);
        Row {
            twin_id,
            project_id,
            element_path: element_path.to_owned(),
            ts: payload.time_point,
            received_at,
            value_type,
            value_num,
            value_text,
            value_json,
        }
    }
}

/// `(value_type, value_num, value_text, value_json)` per plan 6.1.
pub fn value_columns(value: &Value) -> (i16, Option<f64>, Option<String>, Option<String>) {
    match value {
        Value::Real(x) => (value_type::REAL, Some(*x), None, None),
        Value::Integer(i) => {
            // the exact digits are kept in value_text when a double cannot hold them
            let text = (i.unsigned_abs() > MAX_EXACT_F64_INT).then(|| i.to_string());
            (value_type::INTEGER, Some(*i as f64), text, None)
        }
        // beyond i64: the exact digits in value_text, the nearest double in value_num
        Value::BigInteger { digits, approx } => (
            value_type::INTEGER,
            Some(*approx),
            Some(digits.clone()),
            None,
        ),
        Value::Text(text) => (value_type::STRING, None, Some(text.clone()), None),
        Value::Boolean(b) => (
            value_type::BOOLEAN,
            Some(if *b { 1.0 } else { 0.0 }),
            None,
            None,
        ),
        Value::Rational(r) => (value_type::RATIONAL, None, Some(r.to_string()), None),
        Value::Complex { real, imaginary } => {
            let json = serde_json::json!({ "imaginaryValue": imaginary, "realValue": real });
            (value_type::COMPLEX, None, None, Some(json.to_string()))
        }
    }
}

/// The rows of a batch as one vector per column, ready to be bound as arrays of one `UNNEST` statement.
#[derive(Debug, Default, Clone, PartialEq)]
pub struct BatchColumns {
    pub twin_id: Vec<Uuid>,
    pub project_id: Vec<Uuid>,
    pub element_path: Vec<String>,
    pub ts: Vec<DateTime<Utc>>,
    pub received_at: Vec<DateTime<Utc>>,
    pub value_type: Vec<i16>,
    pub value_num: Vec<Option<f64>>,
    pub value_text: Vec<Option<String>>,
    pub value_json: Vec<Option<String>>,
}

impl BatchColumns {
    pub fn from_rows(rows: &[Row]) -> BatchColumns {
        let mut c = BatchColumns::default();
        for row in rows {
            c.twin_id.push(row.twin_id);
            c.project_id.push(row.project_id);
            c.element_path.push(row.element_path.clone());
            c.ts.push(row.ts);
            c.received_at.push(row.received_at);
            c.value_type.push(row.value_type);
            c.value_num.push(row.value_num);
            c.value_text.push(row.value_text.clone());
            c.value_json.push(row.value_json.clone());
        }
        c
    }

    pub fn len(&self) -> usize {
        self.twin_id.len()
    }

    pub fn is_empty(&self) -> bool {
        self.twin_id.is_empty()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::payload::values::{classify, Rational};
    use chrono::TimeZone;

    fn tp() -> DateTime<Utc> {
        Utc.with_ymd_and_hms(2026, 11, 17, 9, 40, 12).unwrap()
    }

    #[test]
    fn value_columns_per_type() {
        assert_eq!(
            value_columns(&Value::Real(1.5)),
            (value_type::REAL, Some(1.5), None, None)
        );
        assert_eq!(
            value_columns(&Value::Integer(-7)),
            (value_type::INTEGER, Some(-7.0), None, None)
        );
        assert_eq!(
            value_columns(&Value::Boolean(true)),
            (value_type::BOOLEAN, Some(1.0), None, None)
        );
        assert_eq!(
            value_columns(&Value::Boolean(false)),
            (value_type::BOOLEAN, Some(0.0), None, None)
        );
        assert_eq!(
            value_columns(&Value::Rational(Rational::new(2, 4).unwrap())),
            (value_type::RATIONAL, None, Some("1/2".to_owned()), None)
        );
        let (ty, num, text, json) = value_columns(&Value::Complex {
            real: 1.0,
            imaginary: -2.5,
        });
        assert_eq!((ty, num, text), (value_type::COMPLEX, None, None));
        let json: serde_json::Value = serde_json::from_str(&json.unwrap()).unwrap();
        assert_eq!(json["realValue"], 1.0);
        assert_eq!(json["imaginaryValue"], -2.5);
    }

    #[test]
    fn strings_and_huge_integers() {
        assert_eq!(
            value_columns(&Value::Text("hello".into())),
            (value_type::STRING, None, Some("hello".to_owned()), None)
        );
        assert_eq!(
            value_columns(&Value::BigInteger {
                digits: "18446744073709551615".into(),
                approx: 1.8446744073709552e19
            }),
            (
                value_type::INTEGER,
                Some(1.8446744073709552e19),
                Some("18446744073709551615".to_owned()),
                None
            )
        );
    }

    #[test]
    fn large_integers_keep_their_exact_digits() {
        let big = (1i64 << 53) + 1;
        let (_, num, text, _) = value_columns(&Value::Integer(big));
        assert_eq!(num, Some(big as f64));
        assert_eq!(text.as_deref(), Some("9007199254740993"));
        // exactly 2^53 is still exact in a double
        assert_eq!(value_columns(&Value::Integer(1 << 53)).2, None);
        assert!(value_columns(&Value::Integer(i64::MIN)).2.is_some());
    }

    #[test]
    fn row_from_classified_payload() {
        let payload =
            classify(br#"{"time_point":"2026-11-17T09:40:12.345Z","value":21.5}"#).unwrap();
        let received = tp();
        let row = Row::new(
            Uuid::from_u128(1),
            Uuid::from_u128(2),
            "Motor/temp",
            &payload,
            received,
        );
        assert_eq!(row.element_path, "Motor/temp");
        assert_eq!(row.ts, payload.time_point);
        assert_eq!(row.received_at, received);
        assert_eq!(row.value_num, Some(21.5));
    }

    #[test]
    fn batch_columns_keep_row_order_and_alignment() {
        let p1 = classify(br#"{"time_point":"2026-11-17T09:40:12.000Z","value":1}"#).unwrap();
        let p2 = classify(br#"{"time_point":"2026-11-17T09:40:13.000Z","value":"3/4"}"#).unwrap();
        let rows = vec![
            Row::new(Uuid::from_u128(1), Uuid::from_u128(9), "a", &p1, tp()),
            Row::new(Uuid::from_u128(2), Uuid::from_u128(9), "b/c", &p2, tp()),
        ];
        let c = BatchColumns::from_rows(&rows);
        assert_eq!(c.len(), 2);
        assert_eq!(c.twin_id, vec![Uuid::from_u128(1), Uuid::from_u128(2)]);
        assert_eq!(c.element_path, vec!["a".to_owned(), "b/c".to_owned()]);
        assert_eq!(
            c.value_type,
            vec![value_type::INTEGER, value_type::RATIONAL]
        );
        assert_eq!(c.value_num, vec![Some(1.0), None]);
        assert_eq!(c.value_text, vec![None, Some("3/4".to_owned())]);
        assert_eq!(c.value_json, vec![None, None]);
        for len in [
            c.project_id.len(),
            c.ts.len(),
            c.received_at.len(),
            c.value_num.len(),
        ] {
            assert_eq!(len, 2);
        }
        assert!(BatchColumns::from_rows(&[]).is_empty());
    }
}
