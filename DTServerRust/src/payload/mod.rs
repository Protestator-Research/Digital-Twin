//! Payload formats of the MQTT topics (byte compatible with the C++ entities in
//! `PhysicalTwinCommunicationService/src/MQTT`).
//!
//! The C++ side uses nlohmann::json, which writes object keys sorted alphabetically. The structs
//! below declare their fields in that order, so serializing yields the same bytes.

pub mod time;
pub mod twin;
pub mod values;

/// Topic of the connect request (`Topics.h`).
pub const CONNECT_TO_TWIN: &str = "connectToTwin";
/// Last topic level of the function registration, used as `<TwinName>/registerFunction`.
pub const REGISTER_FUNCTION: &str = "registerFunction";
