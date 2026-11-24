//! Authentication of MQTT clients: the user name and password of the CONNECT packet are verified before the
//! broker accepts the connection. The only real implementation checks them against the SysMLv2 backend
//! ([`SysmlAuthenticator`], `POST <base>/login`).

mod sysml;
pub mod throttle;

use std::fmt;
use std::future::Future;
use std::pin::Pin;
use std::str::FromStr;

pub use sysml::{login_url, redact_url, SysmlAuthOptions, SysmlAuthenticator};
pub use throttle::{Denied, LoginThrottle, ThrottleConfig};

/// Default `run.mqtt.auth_cache_s`: seconds a successful login is remembered.
pub const DEFAULT_CACHE_TTL_S: u64 = 300;
/// Default `run.mqtt.auth_max_concurrent`: simultaneous login requests to the backend.
pub const DEFAULT_MAX_CONCURRENT: usize = 8;

/// Result of checking one set of credentials.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum AuthOutcome {
    /// The credentials are valid.
    Accepted,
    /// The credentials are wrong (or unusable).
    Rejected,
    /// The check could not be done (backend down, throttled, unexpected answer). The text is for the log and
    /// never contains the password or a token.
    Unavailable(String),
}

/// Checks the credentials of a connecting MQTT client.
pub trait Authenticator: Send + Sync + 'static {
    /// Verifies the credentials. `password` is the binary password field of the CONNECT. The future may be
    /// dropped at any time (client gone, shutdown, timeout).
    fn verify<'a>(
        &'a self,
        username: &'a str,
        password: &'a [u8],
    ) -> Pin<Box<dyn Future<Output = AuthOutcome> + Send + 'a>>;
}

/// How MQTT clients are authenticated (`run.mqtt.authentication`).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum AuthMode {
    /// Every client is accepted.
    None,
    /// User name and password are checked against the SysMLv2 backend (default).
    #[default]
    Sysml,
}

impl AuthMode {
    /// The configuration value of this mode.
    pub fn as_str(self) -> &'static str {
        match self {
            AuthMode::None => "none",
            AuthMode::Sysml => "sysml",
        }
    }
}

impl fmt::Display for AuthMode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.as_str())
    }
}

/// The text is not a valid authentication mode.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ParseAuthModeError(String);

impl fmt::Display for ParseAuthModeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{:?} is not a valid MQTT authentication mode (expected \"none\" or \"sysml\")",
            self.0
        )
    }
}

impl std::error::Error for ParseAuthModeError {}

impl FromStr for AuthMode {
    type Err = ParseAuthModeError;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s.trim().to_ascii_lowercase().as_str() {
            "none" => Ok(AuthMode::None),
            "sysml" => Ok(AuthMode::Sysml),
            _ => Err(ParseAuthModeError(s.to_owned())),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parses_the_modes() {
        assert_eq!("none".parse(), Ok(AuthMode::None));
        assert_eq!(" SysML ".parse(), Ok(AuthMode::Sysml));
        for bad in ["", "ldap", "true", "off"] {
            assert!(bad.parse::<AuthMode>().is_err(), "{bad:?}");
        }
        assert_eq!(AuthMode::default(), AuthMode::Sysml);
        for mode in [AuthMode::None, AuthMode::Sysml] {
            assert_eq!(mode.to_string().parse(), Ok(mode));
        }
    }
}
