//! Configuration and command line, compatible with `DigitalTwinServerInstanceManager`.
//!
//! Precedence (as in the C++ server): default < command line < configuration file. A key that is present
//! in the file overrides the command line. Missing keys keep their previous value.

use std::collections::HashMap;
use std::fmt;
use std::path::Path;

use serde::Deserialize;
use thiserror::Error;

use crate::auth::{AuthMode, ThrottleConfig};
use crate::broker::{BrokerLimits, ProtocolMode};

/// The command line keys, in the order of the C++ `ARGUMENTS` enum.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Argument {
    SysmlUrl,
    SysmlPort,
    SysmlUsername,
    SysmlPassword,
    InstanceMqttPort,
    InstanceMqttProtocolVersion,
    InstanceMqttAuthentication,
    InstanceMqttCertChain,
    InstanceMqttCertPrivateKey,
    InstanceConfig,
}

impl Argument {
    pub const ALL: [Argument; 10] = [
        Argument::SysmlUrl,
        Argument::SysmlPort,
        Argument::SysmlUsername,
        Argument::SysmlPassword,
        Argument::InstanceMqttPort,
        Argument::InstanceMqttProtocolVersion,
        Argument::InstanceMqttAuthentication,
        Argument::InstanceMqttCertChain,
        Argument::InstanceMqttCertPrivateKey,
        Argument::InstanceConfig,
    ];

    /// The key as it appears on the command line (`Arguments` in C++).
    pub fn key(self) -> &'static str {
        match self {
            Argument::SysmlUrl => "sysml.url",
            Argument::SysmlPort => "sysml.port",
            Argument::SysmlUsername => "sysml.username",
            Argument::SysmlPassword => "sysml.password",
            Argument::InstanceMqttPort => "instance.mqtt.port",
            Argument::InstanceMqttProtocolVersion => "instance.mqtt.protocol_version",
            Argument::InstanceMqttAuthentication => "instance.mqtt.authentication",
            Argument::InstanceMqttCertChain => "instance.mqtt.cert_chain",
            Argument::InstanceMqttCertPrivateKey => "instance.mqtt.cert_private_key",
            Argument::InstanceConfig => "instance.config",
        }
    }

    /// The default value (`DefaultValueForArgument` in C++).
    pub fn default_value(self) -> &'static str {
        match self {
            Argument::SysmlUrl => "localhost",
            Argument::SysmlPort => "8088",
            Argument::SysmlUsername | Argument::SysmlPassword => "admin",
            Argument::InstanceMqttPort => "1883",
            Argument::InstanceMqttProtocolVersion => "3.1.1",
            Argument::InstanceMqttAuthentication => "sysml",
            Argument::InstanceMqttCertChain
            | Argument::InstanceMqttCertPrivateKey
            | Argument::InstanceConfig => "",
        }
    }
}

#[derive(Debug, Error)]
pub enum ConfigError {
    #[error("cannot read configuration file {path}: {source}")]
    Read {
        path: String,
        source: std::io::Error,
    },
    #[error("invalid configuration file: {0}")]
    Parse(#[from] serde_json::Error),
    #[error("invalid value for {key}: {message}")]
    Invalid { key: String, message: String },
}

fn invalid(key: &str, message: impl Into<String>) -> ConfigError {
    ConfigError::Invalid {
        key: key.to_owned(),
        message: message.into(),
    }
}

/// Maps command line arguments like the C++ server: for every key, an argument that contains the key as
/// substring provides the next argument as value; the first match wins. The program name is skipped.
pub fn parse_cli<I, S>(args: I) -> HashMap<Argument, String>
where
    I: IntoIterator<Item = S>,
    S: AsRef<str>,
{
    let args: Vec<String> = args.into_iter().map(|a| a.as_ref().to_owned()).collect();
    let mut map = HashMap::new();
    for key in Argument::ALL {
        for j in 1..args.len() {
            if args[j].contains(key.key()) && j + 1 < args.len() {
                map.entry(key).or_insert_with(|| args[j + 1].clone());
            }
        }
    }
    map
}

#[derive(Clone, PartialEq, Eq)]
pub struct SysmlConfig {
    pub url: String,
    pub port: u16,
    pub username: String,
    pub password: String,
}

impl fmt::Debug for SysmlConfig {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("SysmlConfig")
            .field("url", &self.url)
            .field("port", &self.port)
            .field("username", &self.username)
            .field("password", &"<redacted>")
            .finish()
    }
}

/// Smallest accepted `run.mqtt.limits.max_packet_size` in bytes.
pub const MIN_MAX_PACKET_SIZE: u32 = 64;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct MqttConfig {
    pub port: u16,
    /// MQTT versions the broker accepts (`run.mqtt.protocol_version`: "3.1.1", "5" or "both").
    pub protocol: ProtocolMode,
    /// How MQTT clients are authenticated (`run.mqtt.authentication`: "none" or "sysml", default "sysml").
    pub authentication: AuthMode,
    /// Seconds a successful login is remembered (`run.mqtt.auth_cache_s`, default 300, 0 = no cache).
    pub auth_cache_s: u64,
    /// Maximum number of simultaneous login requests to the SysML backend (`run.mqtt.auth_max_concurrent`,
    /// default 8, at least 1).
    pub auth_max_concurrent: usize,
    /// Blocking of repeated failed logins (`run.mqtt.auth_max_failures` 5, `auth_failure_window_s` 60,
    /// `auth_block_s` 10; 0 failures disables it) and the limit of simultaneous checks per IP address
    /// (`auth_max_pending_per_ip`, 4; 0 = no limit).
    pub auth_throttle: ThrottleConfig,
    pub cert_chain: String,
    pub cert_private_key: String,
    pub limits: BrokerLimits,
}

/// `run.rest.*`. Parsed but not used yet.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct RestConfig {
    pub port: u16,
    pub cert_chain: String,
    pub cert_private_key: String,
}

/// `run.history.*`: the history store of the C++ core (only with the `ffi` feature). The values are passed to
/// the backend as they are.
#[derive(Clone, PartialEq, Eq)]
pub struct HistoryConfig {
    /// May contain a password, never logged. Empty: no history.
    pub database_url: String,
    /// 0 keeps the data forever.
    pub retention_days: u32,
    /// Seconds between two checkpoints of the full state (default 600). 0 switches the interval checkpoints off;
    /// the checkpoints at load, in a new month, after data loss and at shutdown remain.
    pub checkpoint_interval_s: u32,
    /// Rows (not change sets) that may wait for the writer.
    pub queue_capacity: u32,
    /// Rows per insert.
    pub batch_size: u32,
    pub flush_interval_ms: u32,
}

impl fmt::Debug for HistoryConfig {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("HistoryConfig")
            .field(
                "database_url",
                &if self.database_url.is_empty() {
                    ""
                } else {
                    "<redacted>"
                },
            )
            .field("retention_days", &self.retention_days)
            .field("checkpoint_interval_s", &self.checkpoint_interval_s)
            .field("queue_capacity", &self.queue_capacity)
            .field("batch_size", &self.batch_size)
            .field("flush_interval_ms", &self.flush_interval_ms)
            .finish()
    }
}

/// `run.api.*`: the read-only REST API over the history database.
#[derive(Clone, PartialEq, Eq)]
pub struct ApiConfig {
    /// Socket address to listen on (`127.0.0.1:8080`). Empty (default): the API is disabled.
    pub listen: String,
    /// May contain a password, never logged. Empty: the history database URL is used. Should name a database
    /// role that may only `SELECT`.
    pub database_url: String,
    /// The API is only reachable through a reverse proxy that sets or appends `X-Forwarded-For` (default false).
    /// Then the client address for the login throttling is the last entry of that header instead of the peer
    /// address. Switch it on only if nothing but the proxy can reach the API port: otherwise a client could send
    /// any header it likes.
    pub behind_proxy: bool,
}

impl ApiConfig {
    /// The database URL the API connects with: its own, or else the history URL.
    pub fn effective_database_url<'a>(&'a self, history: &'a HistoryConfig) -> &'a str {
        if self.database_url.is_empty() {
            &history.database_url
        } else {
            &self.database_url
        }
    }
}

impl fmt::Debug for ApiConfig {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("ApiConfig")
            .field("listen", &self.listen)
            .field(
                "database_url",
                &if self.database_url.is_empty() {
                    ""
                } else {
                    "<redacted>"
                },
            )
            .field("behind_proxy", &self.behind_proxy)
            .finish()
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Config {
    pub sysml: SysmlConfig,
    pub mqtt: MqttConfig,
    pub rest: RestConfig,
    pub history: HistoryConfig,
    pub api: ApiConfig,
    /// Path given by `instance.config`, if any.
    pub config_file: Option<String>,
    /// Invalid command line ports. They only count as errors if the file does not override them
    /// (in C++ the file value wins), see [`Config::validate`].
    deferred: Vec<Deferred>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
struct Deferred {
    /// Key in the JSON file that overrides the command line value.
    file_key: &'static str,
    cli_key: &'static str,
    message: String,
}

// ---------------------------------------------------------------------------------------------
// File layout
// ---------------------------------------------------------------------------------------------

/// A JSON value that may be written as string or as number (ports are strings in the existing files).
#[derive(Debug, Clone, Deserialize)]
#[serde(untagged)]
enum StrNum {
    Str(String),
    Num(u64),
}

impl StrNum {
    fn to_u64(&self, key: &str) -> Result<u64, ConfigError> {
        match self {
            StrNum::Num(n) => Ok(*n),
            StrNum::Str(s) => s
                .trim()
                .parse()
                .map_err(|_| invalid(key, format!("{s:?} is not a number"))),
        }
    }
}

fn convert<T: TryFrom<u64>>(
    key: &str,
    value: &Option<StrNum>,
    target: &mut T,
) -> Result<(), ConfigError> {
    if let Some(v) = value {
        let n = v.to_u64(key)?;
        *target = T::try_from(n).map_err(|_| invalid(key, format!("{n} is out of range")))?;
    }
    Ok(())
}

fn set_string(value: &Option<String>, target: &mut String) {
    if let Some(v) = value {
        target.clone_from(v);
    }
}

#[derive(Debug, Default, Deserialize)]
struct FileConfig {
    sysml: Option<FileSysml>,
    run: Option<FileRun>,
}

#[derive(Debug, Default, Deserialize)]
struct FileSysml {
    url: Option<String>,
    port: Option<StrNum>,
    username: Option<String>,
    password: Option<String>,
}

#[derive(Debug, Default, Deserialize)]
struct FileRun {
    mqtt: Option<FileMqtt>,
    rest: Option<FileRest>,
    api: Option<FileApi>,
    history: Option<FileHistory>,
}

#[derive(Debug, Default, Deserialize)]
struct FileMqtt {
    port: Option<StrNum>,
    /// "3.1.1", "5" or "both"; a number is accepted for 5 (`"protocol_version": 5`).
    protocol_version: Option<StrNum>,
    /// "none" or "sysml".
    authentication: Option<String>,
    auth_cache_s: Option<StrNum>,
    auth_max_concurrent: Option<StrNum>,
    auth_max_failures: Option<StrNum>,
    auth_failure_window_s: Option<StrNum>,
    auth_block_s: Option<StrNum>,
    auth_max_pending_per_ip: Option<StrNum>,
    cert_chain: Option<String>,
    cert_private_key: Option<String>,
    limits: Option<FileLimits>,
}

#[derive(Debug, Default, Deserialize)]
struct FileLimits {
    max_packet_size: Option<StrNum>,
    max_connections: Option<StrNum>,
    max_connections_per_ip: Option<StrNum>,
    max_subscriptions_per_session: Option<StrNum>,
    max_pending_send_bytes: Option<StrNum>,
    connect_timeout_s: Option<StrNum>,
}

#[derive(Debug, Default, Deserialize)]
struct FileRest {
    port: Option<StrNum>,
    cert_chain: Option<String>,
    cert_private_key: Option<String>,
}

#[derive(Debug, Default, Deserialize)]
struct FileHistory {
    database_url: Option<String>,
    retention_days: Option<StrNum>,
    checkpoint_interval_s: Option<StrNum>,
    queue_capacity: Option<StrNum>,
    batch_size: Option<StrNum>,
    flush_interval_ms: Option<StrNum>,
}

#[derive(Debug, Default, Deserialize)]
struct FileApi {
    listen: Option<String>,
    database_url: Option<String>,
    behind_proxy: Option<BoolValue>,
}

/// A JSON value that may be written as boolean or as text (`true`, `"true"`, `"yes"`, `"on"`, `"1"` and the
/// opposites, case-insensitive).
#[derive(Debug, Clone, Deserialize)]
#[serde(untagged)]
enum BoolValue {
    Bool(bool),
    Str(String),
}

/// Parses the text form of a boolean setting.
fn parse_bool(text: &str) -> Option<bool> {
    match text.trim().to_ascii_lowercase().as_str() {
        "true" | "yes" | "on" | "1" => Some(true),
        "false" | "no" | "off" | "0" => Some(false),
        _ => None,
    }
}

impl BoolValue {
    fn to_bool(&self, key: &str) -> Result<bool, ConfigError> {
        match self {
            BoolValue::Bool(b) => Ok(*b),
            BoolValue::Str(s) => parse_bool(s)
                .ok_or_else(|| invalid(key, format!("{s:?} is not a boolean (true or false)"))),
        }
    }
}

impl Config {
    /// Settings from defaults and command line only (no file). Invalid ports are reported by `validate`.
    fn from_cli_map(cli: &HashMap<Argument, String>) -> Config {
        let get = |a: Argument| {
            cli.get(&a)
                .map_or_else(|| a.default_value().to_owned(), Clone::clone)
        };
        let mut deferred = Vec::new();
        let mut port = |a: Argument, file_key: &'static str| -> u16 {
            let v = get(a);
            v.trim().parse().unwrap_or_else(|_| {
                deferred.push(Deferred {
                    file_key,
                    cli_key: a.key(),
                    message: format!("{v:?} is not a valid port"),
                });
                0
            })
        };
        let sysml_port = port(Argument::SysmlPort, "sysml.port");
        let mqtt_port = port(Argument::InstanceMqttPort, "run.mqtt.port");
        let protocol = get(Argument::InstanceMqttProtocolVersion)
            .parse()
            .unwrap_or_else(|e: crate::broker::protocol::ParseProtocolModeError| {
                deferred.push(Deferred {
                    file_key: "run.mqtt.protocol_version",
                    cli_key: Argument::InstanceMqttProtocolVersion.key(),
                    message: e.to_string(),
                });
                ProtocolMode::default()
            });
        let authentication = get(Argument::InstanceMqttAuthentication)
            .parse()
            .unwrap_or_else(|e: crate::auth::ParseAuthModeError| {
                deferred.push(Deferred {
                    file_key: "run.mqtt.authentication",
                    cli_key: Argument::InstanceMqttAuthentication.key(),
                    message: e.to_string(),
                });
                AuthMode::default()
            });
        let config_file = get(Argument::InstanceConfig);
        Config {
            sysml: SysmlConfig {
                url: get(Argument::SysmlUrl),
                port: sysml_port,
                username: get(Argument::SysmlUsername),
                password: get(Argument::SysmlPassword),
            },
            mqtt: MqttConfig {
                port: mqtt_port,
                protocol,
                authentication,
                auth_cache_s: crate::auth::DEFAULT_CACHE_TTL_S,
                auth_max_concurrent: crate::auth::DEFAULT_MAX_CONCURRENT,
                auth_throttle: ThrottleConfig::default(),
                cert_chain: get(Argument::InstanceMqttCertChain),
                cert_private_key: get(Argument::InstanceMqttCertPrivateKey),
                limits: BrokerLimits::default(),
            },
            rest: RestConfig {
                port: 8090,
                cert_chain: String::new(),
                cert_private_key: String::new(),
            },
            history: HistoryConfig {
                database_url: String::new(),
                retention_days: 0,
                checkpoint_interval_s: 600,
                queue_capacity: 10_000,
                batch_size: 500,
                flush_interval_ms: 200,
            },
            api: ApiConfig {
                listen: String::new(),
                database_url: String::new(),
                behind_proxy: false,
            },
            config_file: (!config_file.is_empty()).then_some(config_file),
            deferred,
        }
    }

    /// Fails if a command line value is still invalid after the file was applied.
    pub fn validate(self) -> Result<Config, ConfigError> {
        match self.deferred.first() {
            Some(d) => Err(invalid(d.cli_key, d.message.clone())),
            None => Ok(self),
        }
    }

    /// Environment overrides, they beat the file and the command line:
    /// `DT_SYSML_PASSWORD`, `DT_HISTORY_DATABASE_URL`, `DT_API_LISTEN`, `DT_API_DATABASE_URL` and
    /// `DT_API_BEHIND_PROXY`. Empty values are ignored; an invalid boolean is reported by [`Config::validate`].
    pub fn apply_env(&mut self, get: impl Fn(&str) -> Option<String>) {
        let non_empty = |name: &str| get(name).filter(|v| !v.is_empty());
        if let Some(v) = non_empty("DT_SYSML_PASSWORD") {
            self.sysml.password = v;
        }
        if let Some(v) = non_empty("DT_HISTORY_DATABASE_URL") {
            self.history.database_url = v;
        }
        if let Some(v) = non_empty("DT_API_LISTEN") {
            self.api.listen = v;
        }
        if let Some(v) = non_empty("DT_API_DATABASE_URL") {
            self.api.database_url = v;
        }
        if let Some(v) = non_empty("DT_API_BEHIND_PROXY") {
            match parse_bool(&v) {
                Some(b) => self.api.behind_proxy = b,
                // reported by `validate`, like an invalid command line value
                None => self.deferred.push(Deferred {
                    file_key: "run.api.behind_proxy",
                    cli_key: "DT_API_BEHIND_PROXY",
                    message: format!("{v:?} is not a boolean (true or false)"),
                }),
            }
        }
    }

    /// Like [`Config::from_args`] for `std::env::args_os()`. Arguments that are not valid UTF-8 are converted
    /// lossily instead of panicking.
    pub fn from_os_args<I>(args: I) -> Result<Config, ConfigError>
    where
        I: IntoIterator<Item = std::ffi::OsString>,
    {
        Config::from_args(args.into_iter().map(|a| a.to_string_lossy().into_owned()))
    }

    /// Builds the configuration from the process arguments (first element is the program name), reads
    /// the file named by `instance.config` and applies the environment overrides.
    pub fn from_args<I, S>(args: I) -> Result<Config, ConfigError>
    where
        I: IntoIterator<Item = S>,
        S: AsRef<str>,
    {
        let mut config = Config::from_cli_map(&parse_cli(args));
        if let Some(path) = config.config_file.clone() {
            let text =
                std::fs::read_to_string(Path::new(&path)).map_err(|source| ConfigError::Read {
                    path: path.clone(),
                    source,
                })?;
            config.apply_json(&text)?;
        }
        config.apply_env(|name| std::env::var(name).ok());
        config.validate()
    }

    /// Applies the JSON configuration file on top of the current values. Missing keys keep their value.
    pub fn apply_json(&mut self, json: &str) -> Result<(), ConfigError> {
        let file: FileConfig = serde_json::from_str(json)?;

        if let Some(sysml) = &file.sysml {
            set_string(&sysml.url, &mut self.sysml.url);
            convert("sysml.port", &sysml.port, &mut self.sysml.port)?;
            if sysml.port.is_some() {
                self.deferred.retain(|d| d.file_key != "sysml.port");
            }
            set_string(&sysml.username, &mut self.sysml.username);
            set_string(&sysml.password, &mut self.sysml.password);
        }
        let Some(run) = &file.run else {
            return Ok(());
        };
        if let Some(mqtt) = &run.mqtt {
            convert("run.mqtt.port", &mqtt.port, &mut self.mqtt.port)?;
            if mqtt.port.is_some() {
                self.deferred.retain(|d| d.file_key != "run.mqtt.port");
            }
            if let Some(v) = &mqtt.protocol_version {
                let text = match v {
                    StrNum::Str(s) => s.clone(),
                    StrNum::Num(n) => n.to_string(),
                };
                self.mqtt.protocol = text.parse().map_err(
                    |e: crate::broker::protocol::ParseProtocolModeError| {
                        invalid("run.mqtt.protocol_version", e.to_string())
                    },
                )?;
                self.deferred
                    .retain(|d| d.file_key != "run.mqtt.protocol_version");
            }
            if let Some(v) = &mqtt.authentication {
                self.mqtt.authentication =
                    v.parse().map_err(|e: crate::auth::ParseAuthModeError| {
                        invalid("run.mqtt.authentication", e.to_string())
                    })?;
                self.deferred
                    .retain(|d| d.file_key != "run.mqtt.authentication");
            }
            convert(
                "run.mqtt.auth_cache_s",
                &mqtt.auth_cache_s,
                &mut self.mqtt.auth_cache_s,
            )?;
            convert(
                "run.mqtt.auth_max_concurrent",
                &mqtt.auth_max_concurrent,
                &mut self.mqtt.auth_max_concurrent,
            )?;
            if mqtt.auth_max_concurrent.is_some() && self.mqtt.auth_max_concurrent == 0 {
                return Err(invalid(
                    "run.mqtt.auth_max_concurrent",
                    "must be at least 1",
                ));
            }
            convert(
                "run.mqtt.auth_max_failures",
                &mqtt.auth_max_failures,
                &mut self.mqtt.auth_throttle.max_failures,
            )?;
            let seconds = |key: &str, value: &Option<StrNum>, target: &mut std::time::Duration| {
                let mut n = target.as_secs();
                convert(key, value, &mut n)?;
                *target = std::time::Duration::from_secs(n);
                Ok::<(), ConfigError>(())
            };
            seconds(
                "run.mqtt.auth_failure_window_s",
                &mqtt.auth_failure_window_s,
                &mut self.mqtt.auth_throttle.window,
            )?;
            seconds(
                "run.mqtt.auth_block_s",
                &mqtt.auth_block_s,
                &mut self.mqtt.auth_throttle.block,
            )?;
            convert(
                "run.mqtt.auth_max_pending_per_ip",
                &mqtt.auth_max_pending_per_ip,
                &mut self.mqtt.auth_throttle.max_pending_per_ip,
            )?;
            set_string(&mqtt.cert_chain, &mut self.mqtt.cert_chain);
            set_string(&mqtt.cert_private_key, &mut self.mqtt.cert_private_key);
            if let Some(l) = &mqtt.limits {
                let limits = &mut self.mqtt.limits;
                convert(
                    "run.mqtt.limits.max_packet_size",
                    &l.max_packet_size,
                    &mut limits.max_packet_size,
                )?;
                // announced to MQTT 5 clients as Maximum Packet Size, where 0 is a protocol error; and a CONNECT
                // has to fit
                if l.max_packet_size.is_some() && limits.max_packet_size < MIN_MAX_PACKET_SIZE {
                    return Err(invalid(
                        "run.mqtt.limits.max_packet_size",
                        format!("must be at least {MIN_MAX_PACKET_SIZE}"),
                    ));
                }
                convert(
                    "run.mqtt.limits.max_connections",
                    &l.max_connections,
                    &mut limits.max_connections,
                )?;
                convert(
                    "run.mqtt.limits.max_connections_per_ip",
                    &l.max_connections_per_ip,
                    &mut limits.max_connections_per_ip,
                )?;
                convert(
                    "run.mqtt.limits.max_subscriptions_per_session",
                    &l.max_subscriptions_per_session,
                    &mut limits.max_subscriptions_per_session,
                )?;
                convert(
                    "run.mqtt.limits.max_pending_send_bytes",
                    &l.max_pending_send_bytes,
                    &mut limits.max_pending_send_bytes,
                )?;
                convert(
                    "run.mqtt.limits.connect_timeout_s",
                    &l.connect_timeout_s,
                    &mut limits.connect_timeout_s,
                )?;
            }
        }
        if let Some(rest) = &run.rest {
            convert("run.rest.port", &rest.port, &mut self.rest.port)?;
            set_string(&rest.cert_chain, &mut self.rest.cert_chain);
            set_string(&rest.cert_private_key, &mut self.rest.cert_private_key);
        }
        if let Some(h) = &run.history {
            set_string(&h.database_url, &mut self.history.database_url);
            convert(
                "run.history.retention_days",
                &h.retention_days,
                &mut self.history.retention_days,
            )?;
            convert(
                "run.history.checkpoint_interval_s",
                &h.checkpoint_interval_s,
                &mut self.history.checkpoint_interval_s,
            )?;
            convert(
                "run.history.queue_capacity",
                &h.queue_capacity,
                &mut self.history.queue_capacity,
            )?;
            convert(
                "run.history.batch_size",
                &h.batch_size,
                &mut self.history.batch_size,
            )?;
            convert(
                "run.history.flush_interval_ms",
                &h.flush_interval_ms,
                &mut self.history.flush_interval_ms,
            )?;
        }
        if let Some(api) = &run.api {
            set_string(&api.listen, &mut self.api.listen);
            set_string(&api.database_url, &mut self.api.database_url);
            if let Some(v) = &api.behind_proxy {
                self.api.behind_proxy = v.to_bool("run.api.behind_proxy")?;
            }
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn args(list: &[&str]) -> Vec<String> {
        std::iter::once("dt-server")
            .chain(list.iter().copied())
            .map(str::to_owned)
            .collect()
    }

    fn config(list: &[&str]) -> Config {
        Config::from_cli_map(&parse_cli(args(list)))
            .validate()
            .unwrap()
    }

    #[test]
    fn defaults_without_arguments() {
        let c = config(&[]);
        assert_eq!(c.sysml.url, "localhost");
        assert_eq!(c.sysml.port, 8088);
        assert_eq!(c.sysml.username, "admin");
        assert_eq!(c.sysml.password, "admin");
        assert_eq!(c.mqtt.port, 1883);
        assert_eq!(c.mqtt.protocol, ProtocolMode::V311);
        assert_eq!(c.mqtt.authentication, AuthMode::Sysml);
        assert_eq!(c.mqtt.auth_cache_s, 300);
        assert_eq!(c.mqtt.auth_max_concurrent, 8);
        assert_eq!(c.mqtt.cert_chain, "");
        assert_eq!(c.mqtt.cert_private_key, "");
        assert_eq!(c.config_file, None);
        assert_eq!(c.mqtt.limits, BrokerLimits::default());
        assert_eq!(c.rest.port, 8090);
        assert_eq!(c.history.queue_capacity, 10_000);
        assert_eq!(c.history.batch_size, 500);
        assert_eq!(c.history.flush_interval_ms, 200);
        assert_eq!(c.history.retention_days, 0);
        assert_eq!(c.history.checkpoint_interval_s, 600);
        assert_eq!(c.history.database_url, "");
        // the API is disabled by default
        assert_eq!(c.api.listen, "");
        assert_eq!(c.api.database_url, "");
        assert!(!c.api.behind_proxy);
    }

    #[test]
    fn behind_proxy_is_parsed_from_file_and_environment() {
        let mut c = config(&[]);
        for (json, expected) in [
            (r#"{"run":{"api":{"behind_proxy":true}}}"#, true),
            (r#"{"run":{"api":{"behind_proxy":"false"}}}"#, false),
            (r#"{"run":{"api":{"behind_proxy":"Yes"}}}"#, true),
            (r#"{"run":{"api":{"behind_proxy":"0"}}}"#, false),
            (r#"{"run":{"api":{}}}"#, false),
        ] {
            c.apply_json(json).unwrap();
            assert_eq!(c.api.behind_proxy, expected, "{json}");
        }
        for bad in [
            r#"{"run":{"api":{"behind_proxy":"maybe"}}}"#,
            r#"{"run":{"api":{"behind_proxy":2}}}"#,
        ] {
            assert!(c.apply_json(bad).is_err(), "{bad}");
        }
        // the environment beats the file
        let mut c = config(&[]);
        c.apply_json(r#"{"run":{"api":{"behind_proxy":false}}}"#)
            .unwrap();
        c.apply_env(|name| (name == "DT_API_BEHIND_PROXY").then(|| "true".to_owned()));
        assert!(c.api.behind_proxy);
        c.apply_env(|name| (name == "DT_API_BEHIND_PROXY").then(String::new));
        assert!(c.api.behind_proxy, "empty is ignored");
        assert!(c.validate().is_ok());
        // an invalid value is an error, not silently false
        let mut c = config(&[]);
        c.apply_env(|name| (name == "DT_API_BEHIND_PROXY").then(|| "perhaps".to_owned()));
        assert!(!c.api.behind_proxy);
        assert!(c.validate().is_err());
    }

    #[test]
    fn default_limits_match_the_cpp_header() {
        let l = BrokerLimits::default();
        assert_eq!(l.max_packet_size, 262_144);
        assert_eq!(l.max_connections, 1000);
        assert_eq!(l.max_connections_per_ip, 20);
        assert_eq!(l.max_subscriptions_per_session, 100);
        assert_eq!(l.max_pending_send_bytes, 4 * 1024 * 1024);
        assert_eq!(l.max_topic_length, 512);
        assert_eq!(l.max_topic_levels, 16);
        assert_eq!(l.max_consecutive_send_overflows, 3);
        assert_eq!(l.connect_timeout_s, 10);
    }

    #[test]
    fn cli_keys() {
        let c = config(&[
            "--sysml.url",
            "structura.example",
            "--sysml.port",
            "9000",
            "--sysml.username",
            "bob",
            "--sysml.password",
            "pw",
            "--instance.mqtt.port",
            "1884",
            "--instance.mqtt.protocol_version",
            "both",
            "--instance.mqtt.cert_chain",
            "chain.pem",
            "--instance.mqtt.cert_private_key",
            "key.pem",
            "--instance.config",
            "x.cfg",
        ]);
        assert_eq!(c.sysml.url, "structura.example");
        assert_eq!(c.sysml.port, 9000);
        assert_eq!(c.sysml.username, "bob");
        assert_eq!(c.sysml.password, "pw");
        assert_eq!(c.mqtt.port, 1884);
        assert_eq!(c.mqtt.protocol, ProtocolMode::Both);
        assert_eq!(c.mqtt.cert_chain, "chain.pem");
        assert_eq!(c.mqtt.cert_private_key, "key.pem");
        assert_eq!(c.config_file.as_deref(), Some("x.cfg"));
    }

    #[test]
    fn cli_matching_is_substring_based_first_wins_and_needs_a_value() {
        let m = parse_cli(args(&[
            "sysml.url=ignored",
            "first",
            "--sysml.url",
            "second",
        ]));
        assert_eq!(m[&Argument::SysmlUrl], "first");
        // key as last argument has no value
        let m = parse_cli(args(&["--sysml.url"]));
        assert!(!m.contains_key(&Argument::SysmlUrl));
        // the program name is not interpreted
        let m = parse_cli(["sysml.url", "x"]);
        assert!(!m.contains_key(&Argument::SysmlUrl));
    }

    #[test]
    fn invalid_cli_port_is_an_error() {
        let cli = parse_cli(args(&["--instance.mqtt.port", "abc"]));
        assert!(matches!(
            Config::from_cli_map(&cli).validate(),
            Err(ConfigError::Invalid { .. })
        ));
        let cli = parse_cli(args(&["--instance.mqtt.port", "70000"]));
        assert!(Config::from_cli_map(&cli).validate().is_err());
    }

    #[test]
    fn file_value_wins_over_an_invalid_cli_port() {
        let cli = parse_cli(args(&["--instance.mqtt.port", "abc", "--sysml.port", "x"]));
        let mut c = Config::from_cli_map(&cli);
        c.apply_json(r#"{"run":{"mqtt":{"port":"1999"}}}"#).unwrap();
        // the sysml port is still invalid
        assert!(c.clone().validate().is_err());
        c.apply_json(r#"{"sysml":{"port":8000}}"#).unwrap();
        let c = c.validate().unwrap();
        assert_eq!((c.mqtt.port, c.sysml.port), (1999, 8000));
    }

    #[test]
    fn protocol_version_from_cli() {
        for (value, mode) in [
            ("3.1.1", ProtocolMode::V311),
            ("5", ProtocolMode::V5),
            ("both", ProtocolMode::Both),
        ] {
            let c = config(&["--instance.mqtt.protocol_version", value]);
            assert_eq!(c.mqtt.protocol, mode, "{value}");
        }
        // does not collide with the port key, which is a substring match
        let c = config(&["--instance.mqtt.port", "1884"]);
        assert_eq!(c.mqtt.protocol, ProtocolMode::V311);
    }

    #[test]
    fn protocol_version_from_file_wins_over_cli() {
        let mut c = config(&["--instance.mqtt.protocol_version", "5"]);
        c.apply_json(r#"{"run":{"mqtt":{"protocol_version":"both"}}}"#)
            .unwrap();
        assert_eq!(c.mqtt.protocol, ProtocolMode::Both);
        // a file without the key keeps the command line value
        let mut c = config(&["--instance.mqtt.protocol_version", "5"]);
        c.apply_json(r#"{"run":{"mqtt":{"port":1999}}}"#).unwrap();
        assert_eq!(c.mqtt.protocol, ProtocolMode::V5);
        // a number is accepted for 5
        let mut c = config(&[]);
        c.apply_json(r#"{"run":{"mqtt":{"protocol_version":5}}}"#)
            .unwrap();
        assert_eq!(c.mqtt.protocol, ProtocolMode::V5);
    }

    #[test]
    fn too_small_max_packet_size_is_an_error() {
        for bad in ["0", "63"] {
            let mut c = config(&[]);
            let json = format!(r#"{{"run":{{"mqtt":{{"limits":{{"max_packet_size":{bad}}}}}}}}}"#);
            assert!(
                matches!(c.apply_json(&json), Err(ConfigError::Invalid { .. })),
                "{bad}"
            );
        }
        let mut c = config(&[]);
        c.apply_json(r#"{"run":{"mqtt":{"limits":{"max_packet_size":64}}}}"#)
            .unwrap();
        assert_eq!(c.mqtt.limits.max_packet_size, 64);
    }

    #[test]
    fn invalid_protocol_version_is_an_error() {
        let cli = parse_cli(args(&["--instance.mqtt.protocol_version", "4"]));
        assert!(matches!(
            Config::from_cli_map(&cli).validate(),
            Err(ConfigError::Invalid { .. })
        ));
        // ... unless the file overrides it
        let mut c = Config::from_cli_map(&cli);
        c.apply_json(r#"{"run":{"mqtt":{"protocol_version":"both"}}}"#)
            .unwrap();
        assert_eq!(c.validate().unwrap().mqtt.protocol, ProtocolMode::Both);
        // an invalid value in the file is an error right away
        let mut c = config(&[]);
        for bad in [r#""3.1""#, r#""""#, "4", "true"] {
            let json = format!(r#"{{"run":{{"mqtt":{{"protocol_version":{bad}}}}}}}"#);
            assert!(c.apply_json(&json).is_err(), "{bad}");
        }
    }

    #[test]
    fn authentication_from_cli_and_file() {
        for (value, mode) in [("none", AuthMode::None), ("SysML", AuthMode::Sysml)] {
            let c = config(&["--instance.mqtt.authentication", value]);
            assert_eq!(c.mqtt.authentication, mode, "{value}");
        }
        // the file wins over the command line
        let mut c = config(&["--instance.mqtt.authentication", "sysml"]);
        c.apply_json(r#"{"run":{"mqtt":{"authentication":"none"}}}"#)
            .unwrap();
        assert_eq!(c.mqtt.authentication, AuthMode::None);
        // missing key keeps the value
        c.apply_json(r#"{"run":{"mqtt":{}}}"#).unwrap();
        assert_eq!(c.mqtt.authentication, AuthMode::None);
    }

    #[test]
    fn invalid_authentication_is_an_error() {
        let cli = parse_cli(args(&["--instance.mqtt.authentication", "ldap"]));
        assert!(matches!(
            Config::from_cli_map(&cli).validate(),
            Err(ConfigError::Invalid { .. })
        ));
        // ... unless the file overrides it
        let mut c = Config::from_cli_map(&cli);
        c.apply_json(r#"{"run":{"mqtt":{"authentication":"none"}}}"#)
            .unwrap();
        assert_eq!(c.validate().unwrap().mqtt.authentication, AuthMode::None);
        // an invalid value in the file is an error right away
        let mut c = config(&[]);
        for bad in [r#""ldap""#, r#""""#, "true", "1"] {
            let json = format!(r#"{{"run":{{"mqtt":{{"authentication":{bad}}}}}}}"#);
            assert!(c.apply_json(&json).is_err(), "{bad}");
        }
    }

    #[test]
    fn auth_throttle_defaults_and_file() {
        let mut c = config(&[]);
        let t = &c.mqtt.auth_throttle;
        assert_eq!(
            (
                t.max_failures,
                t.window.as_secs(),
                t.block.as_secs(),
                t.max_pending_per_ip
            ),
            (5, 60, 10, 4)
        );
        c.apply_json(
            r#"{"run":{"mqtt":{"auth_max_failures":"3","auth_failure_window_s":30,"auth_block_s":2,"auth_max_pending_per_ip":0}}}"#,
        )
        .unwrap();
        let t = &c.mqtt.auth_throttle;
        assert_eq!(
            (
                t.max_failures,
                t.window.as_secs(),
                t.block.as_secs(),
                t.max_pending_per_ip
            ),
            (3, 30, 2, 0)
        );
        assert!(c
            .apply_json(r#"{"run":{"mqtt":{"auth_block_s":"x"}}}"#)
            .is_err());
    }

    #[test]
    fn auth_cache_and_concurrency_from_file() {
        let mut c = config(&[]);
        c.apply_json(r#"{"run":{"mqtt":{"auth_cache_s":"60","auth_max_concurrent":2}}}"#)
            .unwrap();
        assert_eq!((c.mqtt.auth_cache_s, c.mqtt.auth_max_concurrent), (60, 2));
        assert!(c
            .apply_json(r#"{"run":{"mqtt":{"auth_max_concurrent":0}}}"#)
            .is_err());
        assert!(c
            .apply_json(r#"{"run":{"mqtt":{"auth_cache_s":"x"}}}"#)
            .is_err());
    }

    #[test]
    fn environment_beats_file_and_cli() {
        let mut c = config(&["--sysml.password", "cli"]);
        c.apply_json(
            r#"{"sysml":{"password":"file"},"run":{"history":{"database_url":"postgres://file"}}}"#,
        )
        .unwrap();
        let env = |name: &str| match name {
            "DT_SYSML_PASSWORD" => Some("env-secret".to_owned()),
            "DT_HISTORY_DATABASE_URL" => Some("postgres://env".to_owned()),
            _ => None,
        };
        c.apply_env(env);
        assert_eq!(c.sysml.password, "env-secret");
        assert_eq!(c.history.database_url, "postgres://env");
        let mut c = config(&[]);
        c.apply_json(r#"{"run":{"api":{"listen":"file:1","database_url":"postgres://file"}}}"#)
            .unwrap();
        c.apply_env(|name| match name {
            "DT_API_LISTEN" => Some("127.0.0.1:9000".to_owned()),
            "DT_API_DATABASE_URL" => Some("postgres://api-env".to_owned()),
            _ => None,
        });
        assert_eq!(c.api.listen, "127.0.0.1:9000");
        assert_eq!(c.api.database_url, "postgres://api-env");
        // empty variables change nothing
        c.apply_env(|_| Some(String::new()));
        assert_eq!(c.api.listen, "127.0.0.1:9000");
        // unset or empty variables change nothing
        let mut c = config(&["--sysml.password", "cli"]);
        c.apply_env(|name| (name == "DT_SYSML_PASSWORD").then(String::new));
        assert_eq!(c.sysml.password, "cli");
        assert!(!format!("{c:?}").contains("env-secret"));
    }

    #[cfg(unix)]
    #[test]
    fn non_utf8_arguments_do_not_panic() {
        use std::ffi::OsString;
        use std::os::unix::ffi::OsStringExt;
        let args = vec![
            OsString::from("dt-server"),
            OsString::from_vec(vec![0x66, 0xFF, 0xFE]),
            OsString::from("--sysml.url"),
            OsString::from_vec(vec![b'h', 0xFF]),
        ];
        let c = Config::from_os_args(args).unwrap();
        assert_eq!(c.sysml.url, "h\u{FFFD}");
    }

    const FILE: &str = r#"{
        "sysml": { "url": "backend", "port": "8089", "username": "u", "password": "secret" },
        "run": {
            "mqtt": {
                "port": "2883",
                "protocol_version": "5",
                "authentication": "none",
                "auth_cache_s": 0,
                "auth_max_concurrent": "3",
                "cert_chain": "c.pem",
                "cert_private_key": "k.pem",
                "limits": {
                    "max_packet_size": 1000,
                    "max_connections": 5,
                    "max_connections_per_ip": 2,
                    "max_subscriptions_per_session": 3,
                    "max_pending_send_bytes": 4096,
                    "connect_timeout_s": 1
                }
            },
            "rest": { "port": "8091", "cert_chain": "rc", "cert_private_key": "rk" },
            "history": {
                "database_url": "postgres://u:p@h/db",
                "retention_days": 30,
                "checkpoint_interval_s": "60",
                "queue_capacity": 11,
                "batch_size": 12,
                "flush_interval_ms": 13
            },
            "api": { "listen": "127.0.0.1:8081", "database_url": "postgres://ro:pw@h/db" }
        }
    }"#;

    #[test]
    fn full_file_overrides_everything() {
        let mut c = config(&["--sysml.url", "cli-url"]);
        c.apply_json(FILE).unwrap();
        // the file wins over the command line, as in C++
        assert_eq!(c.sysml.url, "backend");
        assert_eq!(c.sysml.port, 8089);
        assert_eq!(c.sysml.username, "u");
        assert_eq!(c.sysml.password, "secret");
        assert_eq!(c.mqtt.port, 2883);
        assert_eq!(c.mqtt.protocol, ProtocolMode::V5);
        assert_eq!(c.mqtt.authentication, AuthMode::None);
        assert_eq!(c.mqtt.auth_cache_s, 0);
        assert_eq!(c.mqtt.auth_max_concurrent, 3);
        assert_eq!(c.mqtt.cert_chain, "c.pem");
        assert_eq!(c.mqtt.cert_private_key, "k.pem");
        let l = &c.mqtt.limits;
        assert_eq!(l.max_packet_size, 1000);
        assert_eq!(l.max_connections, 5);
        assert_eq!(l.max_connections_per_ip, 2);
        assert_eq!(l.max_subscriptions_per_session, 3);
        assert_eq!(l.max_pending_send_bytes, 4096);
        assert_eq!(l.connect_timeout_s, 1);
        // not configurable in C++, stay at their defaults
        assert_eq!(l.max_topic_length, 512);
        assert_eq!(c.rest.port, 8091);
        assert_eq!(c.rest.cert_chain, "rc");
        assert_eq!(c.rest.cert_private_key, "rk");
        assert_eq!(c.history.database_url, "postgres://u:p@h/db");
        assert_eq!(c.history.retention_days, 30);
        assert_eq!(c.history.queue_capacity, 11);
        assert_eq!(c.history.batch_size, 12);
        assert_eq!(c.history.flush_interval_ms, 13);
        assert_eq!(c.history.checkpoint_interval_s, 60);
        assert_eq!(c.api.listen, "127.0.0.1:8081");
        assert_eq!(c.api.database_url, "postgres://ro:pw@h/db");
    }

    #[test]
    fn api_database_url_defaults_to_the_history_url() {
        let mut c = config(&[]);
        c.apply_json(r#"{"run":{"history":{"database_url":"postgres://h"}}}"#)
            .unwrap();
        assert_eq!(c.api.effective_database_url(&c.history), "postgres://h");
        c.apply_json(r#"{"run":{"api":{"database_url":"postgres://ro"}}}"#)
            .unwrap();
        assert_eq!(c.api.effective_database_url(&c.history), "postgres://ro");
    }

    #[test]
    fn history_values_must_fit_the_bridge() {
        let mut c = config(&[]);
        for key in [
            "checkpoint_interval_s",
            "queue_capacity",
            "batch_size",
            "flush_interval_ms",
            "retention_days",
        ] {
            let json = format!(r#"{{"run":{{"history":{{"{key}":4294967296}}}}}}"#);
            assert!(
                matches!(c.apply_json(&json), Err(ConfigError::Invalid { .. })),
                "{key}"
            );
            let json = format!(r#"{{"run":{{"history":{{"{key}":"x"}}}}}}"#);
            assert!(
                matches!(c.apply_json(&json), Err(ConfigError::Invalid { .. })),
                "{key}"
            );
        }
    }

    #[test]
    fn ports_accept_string_and_number() {
        let mut c = config(&[]);
        c.apply_json(
            r#"{"sysml":{"port":8000},"run":{"mqtt":{"port":1999},"rest":{"port":"8100"}}}"#,
        )
        .unwrap();
        assert_eq!(c.sysml.port, 8000);
        assert_eq!(c.mqtt.port, 1999);
        assert_eq!(c.rest.port, 8100);
    }

    #[test]
    fn missing_sections_and_keys_keep_defaults() {
        for json in [
            "{}",
            r#"{"sysml":{}}"#,
            r#"{"run":{}}"#,
            r#"{"run":{"mqtt":{}}}"#,
            r#"{"run":{"mqtt":{"limits":{}}}}"#,
        ] {
            let mut c = config(&[]);
            c.apply_json(json).unwrap();
            assert_eq!(c, config(&[]), "{json}");
        }
        let mut c = config(&["--instance.mqtt.port", "1884"]);
        c.apply_json(r#"{"run":{"mqtt":{"limits":{"max_connections":7}}}}"#)
            .unwrap();
        assert_eq!(c.mqtt.port, 1884);
        assert_eq!(c.mqtt.limits.max_connections, 7);
        assert_eq!(c.mqtt.limits.max_connections_per_ip, 20);
    }

    #[test]
    fn unknown_keys_are_ignored() {
        let mut c = config(&[]);
        c.apply_json(r#"{"other":1,"run":{"mqtt":{"future":true}}}"#)
            .unwrap();
        assert_eq!(c, config(&[]));
    }

    #[test]
    fn invalid_files_are_errors() {
        let mut c = config(&[]);
        assert!(matches!(
            c.apply_json("not json"),
            Err(ConfigError::Parse(_))
        ));
        assert!(matches!(c.apply_json(""), Err(ConfigError::Parse(_))));
        assert!(matches!(
            c.apply_json(r#"{"run":{"mqtt":{"port":"x"}}}"#),
            Err(ConfigError::Invalid { .. })
        ));
        assert!(matches!(
            c.apply_json(r#"{"run":{"mqtt":{"port":70000}}}"#),
            Err(ConfigError::Invalid { .. })
        ));
    }

    #[test]
    fn missing_config_file_is_an_error() {
        let r = Config::from_args(args(&["--instance.config", "/nonexistent/dir/config.cfg"]));
        assert!(matches!(r, Err(ConfigError::Read { .. })));
    }

    #[test]
    fn debug_output_hides_secrets() {
        let mut c = config(&["--sysml.password", "hunter2"]);
        c.apply_json(FILE).unwrap();
        let shown = format!("{c:?}");
        assert!(!shown.contains("secret"));
        assert!(!shown.contains("hunter2"));
        assert!(!shown.contains("u:p@h"));
        assert!(!shown.contains("ro:pw@h"));
        assert!(shown.contains("<redacted>"));
    }
}
