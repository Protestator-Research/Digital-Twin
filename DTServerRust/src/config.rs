//! Configuration and command line, compatible with `DigitalTwinServerInstanceManager`.
//!
//! Precedence (as in the C++ server): default < command line < configuration file. A key that is present
//! in the file overrides the command line. Missing keys keep their previous value.

use std::collections::HashMap;
use std::fmt;
use std::path::Path;

use serde::Deserialize;
use thiserror::Error;

use crate::broker::BrokerLimits;

/// The command line keys, in the order of the C++ `ARGUMENTS` enum.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum Argument {
    SysmlUrl,
    SysmlPort,
    SysmlUsername,
    SysmlPassword,
    InstanceMqttPort,
    InstanceMqttCertChain,
    InstanceMqttCertPrivateKey,
    InstanceConfig,
}

impl Argument {
    pub const ALL: [Argument; 8] = [
        Argument::SysmlUrl,
        Argument::SysmlPort,
        Argument::SysmlUsername,
        Argument::SysmlPassword,
        Argument::InstanceMqttPort,
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

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct MqttConfig {
    pub port: u16,
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

/// `run.history.*`. Parsed but not used yet.
#[derive(Clone, PartialEq, Eq)]
pub struct HistoryConfig {
    /// May contain a password, never logged.
    pub database_url: String,
    /// 0 keeps the data forever.
    pub retention_days: u32,
    pub queue_capacity: usize,
    pub batch_size: usize,
    pub flush_interval_ms: u64,
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
            .field("queue_capacity", &self.queue_capacity)
            .field("batch_size", &self.batch_size)
            .field("flush_interval_ms", &self.flush_interval_ms)
            .finish()
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Config {
    pub sysml: SysmlConfig,
    pub mqtt: MqttConfig,
    pub rest: RestConfig,
    pub history: HistoryConfig,
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
    history: Option<FileHistory>,
}

#[derive(Debug, Default, Deserialize)]
struct FileMqtt {
    port: Option<StrNum>,
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
    server_keep_alive_s: Option<StrNum>,
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
    queue_capacity: Option<StrNum>,
    batch_size: Option<StrNum>,
    flush_interval_ms: Option<StrNum>,
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
                queue_capacity: 10_000,
                batch_size: 500,
                flush_interval_ms: 200,
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
    /// `DT_SYSML_PASSWORD` and `DT_HISTORY_DATABASE_URL`. Empty values are ignored.
    pub fn apply_env(&mut self, get: impl Fn(&str) -> Option<String>) {
        let non_empty = |name: &str| get(name).filter(|v| !v.is_empty());
        if let Some(v) = non_empty("DT_SYSML_PASSWORD") {
            self.sysml.password = v;
        }
        if let Some(v) = non_empty("DT_HISTORY_DATABASE_URL") {
            self.history.database_url = v;
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
            set_string(&mqtt.cert_chain, &mut self.mqtt.cert_chain);
            set_string(&mqtt.cert_private_key, &mut self.mqtt.cert_private_key);
            if let Some(l) = &mqtt.limits {
                let limits = &mut self.mqtt.limits;
                convert(
                    "run.mqtt.limits.max_packet_size",
                    &l.max_packet_size,
                    &mut limits.max_packet_size,
                )?;
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
                convert(
                    "run.mqtt.limits.server_keep_alive_s",
                    &l.server_keep_alive_s,
                    &mut limits.server_keep_alive_s,
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
        assert_eq!(c.mqtt.cert_chain, "");
        assert_eq!(c.mqtt.cert_private_key, "");
        assert_eq!(c.config_file, None);
        assert_eq!(c.mqtt.limits, BrokerLimits::default());
        assert_eq!(c.rest.port, 8090);
        assert_eq!(c.history.queue_capacity, 10_000);
        assert_eq!(c.history.batch_size, 500);
        assert_eq!(c.history.flush_interval_ms, 200);
        assert_eq!(c.history.retention_days, 0);
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
        assert_eq!(l.server_keep_alive_s, 60);
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
                "cert_chain": "c.pem",
                "cert_private_key": "k.pem",
                "limits": {
                    "max_packet_size": 1000,
                    "max_connections": 5,
                    "max_connections_per_ip": 2,
                    "max_subscriptions_per_session": 3,
                    "max_pending_send_bytes": 4096,
                    "connect_timeout_s": 1,
                    "server_keep_alive_s": 7
                }
            },
            "rest": { "port": "8091", "cert_chain": "rc", "cert_private_key": "rk" },
            "history": {
                "database_url": "postgres://u:p@h/db",
                "retention_days": 30,
                "queue_capacity": 11,
                "batch_size": 12,
                "flush_interval_ms": 13
            }
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
        assert_eq!(c.mqtt.cert_chain, "c.pem");
        assert_eq!(c.mqtt.cert_private_key, "k.pem");
        let l = &c.mqtt.limits;
        assert_eq!(l.max_packet_size, 1000);
        assert_eq!(l.max_connections, 5);
        assert_eq!(l.max_connections_per_ip, 2);
        assert_eq!(l.max_subscriptions_per_session, 3);
        assert_eq!(l.max_pending_send_bytes, 4096);
        assert_eq!(l.connect_timeout_s, 1);
        assert_eq!(l.server_keep_alive_s, 7);
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
        assert!(matches!(
            c.apply_json(r#"{"run":{"mqtt":{"limits":{"server_keep_alive_s":70000}}}}"#),
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
        assert!(shown.contains("<redacted>"));
    }
}
