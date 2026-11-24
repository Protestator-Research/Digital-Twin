//! Payloads of the control topics: `connectToTwin` and `<TwinName>/registerFunction`.

use serde::{Deserialize, Serialize};
use uuid::Uuid;

/// `{"project_id": "<uuid>", "twin_id": "<uuid>"}` (`DigitalTwinEntity`).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub struct ConnectToTwin {
    #[serde(rename = "project_id")]
    pub project_id: Uuid,
    #[serde(rename = "twin_id")]
    pub twin_id: Uuid,
}

impl ConnectToTwin {
    pub fn from_json(payload: &[u8]) -> Result<Self, serde_json::Error> {
        serde_json::from_slice(payload)
    }

    pub fn to_json(&self) -> String {
        serde_json::to_string(self).expect("plain struct serializes")
    }
}

/// `FunctionRegistrationRequest`. Fields are declared in the key order nlohmann::json writes.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct FunctionRegistrationRequest {
    pub linked_variable_return: String,
    /// Pairs of (variable, qualified name of the linked variable).
    pub linked_variables: Vec<(String, String)>,
    pub qualified_name: String,
}

impl FunctionRegistrationRequest {
    pub fn from_json(payload: &[u8]) -> Result<Self, serde_json::Error> {
        serde_json::from_slice(payload)
    }

    pub fn to_json(&self) -> String {
        serde_json::to_string(self).expect("plain struct serializes")
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const TWIN: &str = "123e4567-e89b-12d3-a456-426614174000";
    const PROJECT: &str = "9b2c1d9e-7a1f-4c63-8f64-0d6a5b1e2f33";

    #[test]
    fn connect_to_twin_is_byte_compatible() {
        let c = ConnectToTwin {
            project_id: PROJECT.parse().unwrap(),
            twin_id: TWIN.parse().unwrap(),
        };
        let expected = format!(r#"{{"project_id":"{PROJECT}","twin_id":"{TWIN}"}}"#);
        assert_eq!(c.to_json(), expected);
        assert_eq!(ConnectToTwin::from_json(expected.as_bytes()).unwrap(), c);
    }

    #[test]
    fn connect_to_twin_parsing() {
        // key order and whitespace do not matter, uppercase and unknown keys are accepted
        let json = format!(
            r#"{{ "twin_id": "{}", "extra": 1, "project_id": "{}" }}"#,
            TWIN.to_uppercase(),
            PROJECT
        );
        let c = ConnectToTwin::from_json(json.as_bytes()).unwrap();
        assert_eq!(c.twin_id.to_string(), TWIN);
        assert!(ConnectToTwin::from_json(br#"{"twin_id":"x","project_id":"y"}"#).is_err());
        assert!(
            ConnectToTwin::from_json(br#"{"twin_id":"123e4567-e89b-12d3-a456-426614174000"}"#)
                .is_err()
        );
        assert!(ConnectToTwin::from_json(b"").is_err());
    }

    #[test]
    fn registration_request_is_byte_compatible() {
        let r = FunctionRegistrationRequest {
            linked_variable_return: "Twin::result".into(),
            linked_variables: vec![
                ("a".into(), "Twin::a".into()),
                ("b".into(), "Twin::b".into()),
            ],
            qualified_name: "Twin::add".into(),
        };
        let expected = r#"{"linked_variable_return":"Twin::result","linked_variables":[["a","Twin::a"],["b","Twin::b"]],"qualified_name":"Twin::add"}"#;
        assert_eq!(r.to_json(), expected);
        assert_eq!(
            FunctionRegistrationRequest::from_json(expected.as_bytes()).unwrap(),
            r
        );
    }

    #[test]
    fn registration_request_without_variables() {
        let json = br#"{"qualified_name":"f","linked_variables":[],"linked_variable_return":"r"}"#;
        let r = FunctionRegistrationRequest::from_json(json).unwrap();
        assert!(r.linked_variables.is_empty());
        assert_eq!(
            r.to_json(),
            r#"{"linked_variable_return":"r","linked_variables":[],"qualified_name":"f"}"#
        );
        assert!(FunctionRegistrationRequest::from_json(br#"{"qualified_name":"f"}"#).is_err());
    }
}
