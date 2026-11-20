//! HTTP Basic authentication of the REST API with the same check as the MQTT clients: the user name and
//! password go to the [`Authenticator`] of the broker (the SysMLv2 backend, with its cache), behind the broker's
//! own [`LoginThrottle`] used the same way as the MQTT side uses it: a user name or a client address with too many
//! failures is refused without a check, and only a few checks of one address run at the same time. It is one
//! instance for both protocols, so the failure budget is not doubled and a user name blocked over MQTT is blocked
//! here.
//!
//! The client address is the peer address of the connection. Behind a reverse proxy that is the proxy for every
//! request, so all users would share one address (and one failure budget); with `run.api.behind_proxy` the address is
//! taken from the `X-Forwarded-For` header instead, see [`client_ip`].
//!
//! The pending-check guard of the throttle is held only while the credentials are verified, not while the request
//! is served. Requests without a usable `Authorization` header are answered 401 and counted nowhere.
//!
//! It is only set up when MQTT authentication is enabled. There is no authorization beyond that: every user who
//! may log in can read the whole history, as every MQTT client can read every topic.
//!
//! Basic authentication sends the password with every request. Without TLS (which this server does not
//! provide) it is readable on the network: bind the API to localhost or put it behind a TLS terminating
//! reverse proxy.

use std::net::{IpAddr, Ipv4Addr, SocketAddr};
use std::sync::Arc;
use std::time::Duration;

use axum::extract::{ConnectInfo, Request, State};
use axum::http::HeaderMap;
use axum::middleware::Next;
use axum::response::{IntoResponse, Response};
use base64::engine::general_purpose::STANDARD;
use base64::Engine as _;
use tracing::{debug, warn};

use super::ApiError;
use crate::auth::{AuthOutcome, Authenticator, Denied, LoginThrottle};

/// A check that takes longer is answered as "unavailable".
const CHECK_TIMEOUT: Duration = Duration::from_secs(10);

/// The check of the credentials of the API requests.
pub struct BasicAuth {
    authenticator: Arc<dyn Authenticator>,
    throttle: Arc<LoginThrottle>,
    behind_proxy: bool,
}

impl BasicAuth {
    /// `throttle` should be the one of the broker ([`crate::broker::Broker::throttle`]). `behind_proxy`: see
    /// [`client_ip`].
    pub fn new(
        authenticator: Arc<dyn Authenticator>,
        throttle: Arc<LoginThrottle>,
        behind_proxy: bool,
    ) -> Self {
        Self {
            authenticator,
            throttle,
            behind_proxy,
        }
    }
}

/// User name and password of an `Authorization: Basic ...` header value. `None` if it is something else or
/// malformed. The user name has to be UTF-8, the password is taken as bytes (like the MQTT password field).
pub fn parse_basic(header: &str) -> Option<(String, Vec<u8>)> {
    let (scheme, encoded) = header.trim().split_once(' ')?;
    if !scheme.eq_ignore_ascii_case("basic") {
        return None;
    }
    let decoded = STANDARD.decode(encoded.trim()).ok()?;
    let colon = decoded.iter().position(|&b| b == b':')?;
    let username = String::from_utf8(decoded[..colon].to_vec()).ok()?;
    Some((username, decoded[colon + 1..].to_vec()))
}

/// An address as it can appear in `X-Forwarded-For`: `1.2.3.4`, `1.2.3.4:5678`, `2001:db8::1`, `[2001:db8::1]` or
/// `[2001:db8::1]:5678`.
fn parse_forwarded_address(text: &str) -> Option<IpAddr> {
    let text = text.trim();
    text.parse::<IpAddr>()
        .ok()
        .or_else(|| text.parse::<SocketAddr>().ok().map(|addr| addr.ip()))
        .or_else(|| {
            text.strip_prefix('[')
                .and_then(|rest| rest.strip_suffix(']'))
                .and_then(|inner| inner.parse::<IpAddr>().ok())
        })
}

/// The address a request is throttled by. With `behind_proxy` off it is `peer`, and the header is ignored (a client
/// can send any header). With it on it is the **last** (rightmost) entry of `X-Forwarded-For`: the one the trusted
/// proxy itself appended, which the client cannot influence; entries to its left are whatever the client or earlier
/// hops claimed. Several header lines count as one comma separated list. If the header is missing, or that entry is
/// not an address, it falls back to `peer`.
pub fn client_ip(headers: &HeaderMap, peer: IpAddr, behind_proxy: bool) -> IpAddr {
    if !behind_proxy {
        return peer;
    }
    let joined = headers
        .get_all("x-forwarded-for")
        .iter()
        .filter_map(|value| value.to_str().ok())
        .collect::<Vec<_>>()
        .join(",");
    joined
        .rsplit(',')
        .next()
        .and_then(parse_forwarded_address)
        .unwrap_or(peer)
}

/// Middleware: lets a request pass if its credentials are accepted.
pub async fn require_basic(
    State(auth): State<Arc<BasicAuth>>,
    request: Request,
    next: Next,
) -> Response {
    let Some((username, password)) = request
        .headers()
        .get(axum::http::header::AUTHORIZATION)
        .and_then(|value| value.to_str().ok())
        .and_then(parse_basic)
    else {
        return ApiError::Unauthorized.into_response();
    };
    // without connection information (a router called directly in a test) all requests have one address
    let peer = request
        .extensions()
        .get::<ConnectInfo<SocketAddr>>()
        .map_or(IpAddr::V4(Ipv4Addr::UNSPECIFIED), |info| info.0.ip());
    let ip = client_ip(request.headers(), peer, auth.behind_proxy);

    // Like the CONNECT of an MQTT client: refused without a check after too many failures of the user name or the
    // address, and only a few checks of one address at a time.
    let pending = match auth.throttle.begin(&username, ip) {
        Ok(guard) => guard,
        Err(Denied::Blocked) => {
            warn!(username = ?username, %ip, "API login refused: too many failed logins");
            return ApiError::Unauthorized.into_response();
        }
        Err(Denied::TooManyPending) => {
            warn!(%ip, "API login refused: too many pending checks");
            return ApiError::AuthUnavailable.into_response();
        }
    };
    let verdict = tokio::time::timeout(
        CHECK_TIMEOUT,
        auth.authenticator.verify(&username, &password),
    )
    .await
    .unwrap_or_else(|_| AuthOutcome::Unavailable("login check timed out".into()));
    // the guard only covers the verification, not the handler
    drop(pending);
    match verdict {
        AuthOutcome::Accepted => {
            auth.throttle.record_success(&username);
            debug!(username = ?username, path = %request.uri().path(), "API request");
            next.run(request).await
        }
        AuthOutcome::Rejected => {
            auth.throttle.record_failure(&username, ip);
            warn!(username = ?username, %ip, "API login rejected");
            ApiError::Unauthorized.into_response()
        }
        AuthOutcome::Unavailable(why) => {
            warn!(username = ?username, reason = %why, "API login check unavailable");
            ApiError::AuthUnavailable.into_response()
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use axum::http::HeaderValue;

    fn basic(user: &[u8], password: &[u8]) -> String {
        let mut raw = user.to_vec();
        raw.push(b':');
        raw.extend_from_slice(password);
        format!("Basic {}", STANDARD.encode(raw))
    }

    #[test]
    fn parses_basic_headers() {
        assert_eq!(
            parse_basic(&basic(b"admin", b"s3cret")),
            Some(("admin".into(), b"s3cret".to_vec()))
        );
        // the password may contain colons and may be empty; the scheme is case-insensitive
        assert_eq!(
            parse_basic(&basic(b"u", b"a:b:c")),
            Some(("u".into(), b"a:b:c".to_vec()))
        );
        assert_eq!(
            parse_basic(&basic(b"u", b"").replace("Basic", "basic")),
            Some(("u".into(), Vec::new()))
        );
        // a password that is not UTF-8 is passed on as bytes
        assert_eq!(
            parse_basic(&basic(b"u", &[0xff, 0xfe])),
            Some(("u".into(), vec![0xff, 0xfe]))
        );
        for bad in [
            "",
            "Basic",
            "Basic !!!",
            "Bearer abc",
            "Basic dXNlcg==",      // "user": no colon
            &basic(&[0xff], b"p"), // user name is no UTF-8
        ] {
            assert_eq!(parse_basic(bad), None, "{bad:?}");
        }
    }

    fn headers(lines: &[&str]) -> HeaderMap {
        let mut map = HeaderMap::new();
        for line in lines {
            map.append("x-forwarded-for", HeaderValue::from_str(line).unwrap());
        }
        map
    }

    fn ip(text: &str) -> IpAddr {
        text.parse().unwrap()
    }

    #[test]
    fn the_client_address_is_the_last_forwarded_entry_when_behind_a_proxy() {
        let peer = ip("10.0.0.1");
        let from = |lines: &[&str]| client_ip(&headers(lines), peer, true);
        // one entry, several entries (the client can have put anything to the left), spaces
        assert_eq!(from(&["203.0.113.7"]), ip("203.0.113.7"));
        assert_eq!(
            from(&["1.1.1.1, 2.2.2.2,   203.0.113.7"]),
            ip("203.0.113.7")
        );
        assert_eq!(from(&["  203.0.113.7  "]), ip("203.0.113.7"));
        // several header lines are one list: the last line's last entry wins
        assert_eq!(
            from(&["1.1.1.1", "2.2.2.2, 203.0.113.7"]),
            ip("203.0.113.7")
        );
        // IPv6, with brackets and with a port
        assert_eq!(from(&["2001:db8::7"]), ip("2001:db8::7"));
        assert_eq!(from(&["9.9.9.9, [2001:db8::7]"]), ip("2001:db8::7"));
        assert_eq!(from(&["[2001:db8::7]:4711"]), ip("2001:db8::7"));
        assert_eq!(from(&["203.0.113.7:4711"]), ip("203.0.113.7"));
        // garbage, an empty last entry, an unknown token: the peer address
        for bad in [
            "",
            "garbage",
            "1.1.1.1, unknown",
            "1.1.1.1,",
            "1.1.1.1, ",
            "999.1.1.1",
            "[::1",
        ] {
            assert_eq!(from(&[bad]), peer, "{bad:?}");
        }
        // no header at all
        assert_eq!(from(&[]), peer);
        // a header that is not text
        let mut map = HeaderMap::new();
        map.insert(
            "x-forwarded-for",
            HeaderValue::from_bytes(&[0xff, 0xfe]).unwrap(),
        );
        assert_eq!(client_ip(&map, peer, true), peer);
    }

    #[test]
    fn without_the_setting_the_header_is_ignored() {
        let peer = ip("10.0.0.1");
        assert_eq!(client_ip(&headers(&["203.0.113.7"]), peer, false), peer);
        assert_eq!(client_ip(&headers(&[]), peer, false), peer);
    }
}
