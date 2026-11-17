# dt-server (Rust port of the Digital Twin Server)

Port described in `Markdownfiles/Rust_Server_Portierung_Implementierungsplan.md`: configuration/CLI, embedded
MQTT v5 broker, in-process hub and payload types (first increment), twin logic with the FFI bridge to the C++
libraries (phase 3) and the history in PostgreSQL (phase 5). Not included: REST API, TLS, authentication, removal
of the C++ server.

## Build and test

```
cargo build
cargo test
cargo clippy --all-targets -- -D warnings
cargo fmt
```

Environment overrides (beat file and command line): `DT_SYSML_PASSWORD` (Structura password) and
`DT_HISTORY_DATABASE_URL` (history database URL, may contain a password). Empty values are ignored.

Run: `cargo run -- --instance.config config.cfg --instance.mqtt.port 1883` (same keys as the C++ server).
Log level via `RUST_LOG` (default `info`). Output goes to `target/` (ignored by `.gitignore`).

## Features, build and run

Without features nothing but Rust and crates.io is needed. `cargo build` produces a server whose twin backend
reports "twin backend not available" for every `connectToTwin` (logged, the server keeps running); broker and
history work.

### With the C++ model libraries (`--features ffi`)

```
export DT_CPP_BUILD_DIR=<repo>/build/Debug      # the existing CMake build tree, read only
cargo build --features ffi
cargo test  --features ffi
```

`build.rs` compiles the cxx bridge and the facade `ffi/dt_facade.cpp` (C++20, uses `DigitalTwinLib`,
`BackendCommunicationService` and `sysmllib` unmodified) against that tree. It never writes there and never runs
cmake or conan:

- libraries: `$DT_CPP_BUILD_DIR/out/lib{DigitalTwinLib,BackendCommunication,PhysicalTwinCommunication,CPSBaseLib}.so`
  plus the shared libraries of the Conan package `sysmllib` (the facade uses `SysMLv2::REST` classes directly);
- source directory: `CMAKE_HOME_DIRECTORY` from `CMakeCache.txt` (header directories of the four C++ libraries);
- Conan include and library directories: read from `$DT_CPP_BUILD_DIR/generators/*-debug-x86_64-data.cmake`
  (`DT_CPP_CONAN_CONFIG` changes the suffix `debug-x86_64`); no store path is hard-coded;
- `DT_CPP_EXTRA_LIB_DIRS` (optional, `:` separated) is searched first for libraries.

Link search order and rpath: extra directories, Conan packages, then `out`. **The order matters**: `out` also holds
older copies of `libsysml*.so` (built against a different Boost whose store directory is gone) that must not win
over the Conan `sysmllib` package; if `out` came first the link would succeed but the binary would fail to start
or to load a library. `build.rs` stops with a message that names the generator file or variable when a package
it needs is missing (for example `sysmllib-debug-x86_64-data.cmake`, or `<pkg>_INCLUDE_DIRS_DEBUG` naming a
directory that no longer exists); the variable suffix (`DEBUG`) is derived from `DT_CPP_CONAN_CONFIG`. The
rpath is DT_RPATH (not RUNPATH) so it also covers dependencies of the libraries; a library that itself carries a
RUNPATH (the C++ project libraries do) ignores it, so the run environment of the C++ build is still needed:

```
source "$DT_CPP_BUILD_DIR/generators/conanrun.sh"
```

`build.rs` prints a `cargo:warning` for every library directory the generator files name that does not exist
(stale build tree). A failing `--features ffi` build or a missing library at run time is a problem of the C++
build tree: rebuild it with conan/cmake there, this crate does not touch it.

### Environment variables

| Variable | Used by | Meaning |
|---|---|---|
| `DT_SYSML_PASSWORD` | server | Structura password (beats file and command line) |
| `DT_HISTORY_DATABASE_URL` | server | PostgreSQL URL of the history (beats the file); may contain a password, never logged |
| `RUST_LOG` | server | log level, default `info` |
| `DT_CPP_BUILD_DIR` | build, `ffi` | CMake build directory of the C++ project (required for `--features ffi`) |
| `DT_CPP_CONAN_CONFIG`, `DT_CPP_EXTRA_LIB_DIRS` | build, `ffi` | see above |
| `DT_TEST_DATABASE_URL` | tests | enables the PostgreSQL tests (see below) |
| `DT_TEST_SERVER_URL`, `DT_TEST_USER`, `DT_TEST_PASSWORD`, `DT_TEST_PROJECT_ID`, `DT_TEST_TWIN_ID` | tests, `ffi` | opt-in live test against a Structura server (same names as `tools/run_live_test.py`); never started by the tests |

### Tests

- `cargo test`: unit tests plus the integration tests. `tests/twin_integration.rs` drives the whole twin flow
  (`connectToTwin` -> interface topics -> `registerFunction` forwarding, duplicate request, backend error,
  malformed payload, login retry, bounded pool, shutdown) over a real broker with a fake `TwinBackend`.
- `cargo test --features ffi`: additionally `tests/ffi_backend.rs`: calls without login, login against a closed
  port and against a server that answers 500 return errors (no abort, no password in the message). The live test
  needs `DT_TEST_SERVER_URL`, `DT_TEST_PROJECT_ID` and `DT_TEST_TWIN_ID` and is skipped otherwise.
- PostgreSQL tests (`tests/history_db.rs`): run only when `DT_TEST_DATABASE_URL` is set, e.g.
  `DT_TEST_DATABASE_URL=postgres://user:pw@localhost/postgres cargo test --test history_db`. The user needs the
  right to create schemas. Every test creates its own schema `dt_test_<uuid>`, uses it as `search_path`, and
  drops it afterwards (also after a panic); nothing else is touched. Without the variable each test prints
  `SKIPPED ...` and passes.
- `cargo test --features sql-syntax-check`: checks the syntax of every SQL statement (migration, insert, upsert,
  partition DDL) with PostgreSQL's own parser (`pg_query`, libpg_query, no server). Off by default because the
  build of `pg_query` needs libclang.

## Twin logic (`src/twin/`)

`TwinBackend` (`backend.rs`) is the interface to the model library: `login`, `load_twin`, `register_function`,
`release_twin`. `TwinService` implements everything on top of it:

- subscribes to `connectToTwin` on the hub; each valid payload starts a download on a pool of 4 (a semaphore, the
  calls run in `spawn_blocking`); at most one download per twin id (a repeated request while one is queued or
  running is skipped and logged, like `DownloadsInProgress`); at most 64 distinct twins may be pending;
- after the load it publishes, through the hub, a valid initial value (`{"time_point": ..., "value": 0}`, the
  existing `IntegerValue` payload, not the C++ placeholder) on `<TwinName>/<element>` for every element and the empty
  registration request on `<TwinName>/registerFunction`, and forwards client payloads on that topic to
  `register_function` (malformed or nameless requests are ignored);
- errors of the backend are logged and counted, never fatal; malformed `connectToTwin` payloads are ignored;
- a download refused as unauthorized (HTTP 401 / not logged in) logs in again and is retried once;
- a twin name that is empty, contains `+`, `#`, `/` or NUL, or starts with `$`, is rejected before anything is
  subscribed or published (a twin named `+` would otherwise subscribe to `+/registerFunction`); element paths that
  are no valid topic (empty levels, wildcards, NUL) are skipped, with one warning and one count per load;
- at shutdown the loaded twins are released (at most 5 s).

Login: done in the background at startup and retried before every download until it works. A failed login (or the
missing backend) is logged and the server keeps running. The C++ server lets the exception end `runInstance`;
that would make a binary without `ffi`, or a Structura that starts later than the server, unusable.

The facade returns element paths WITHOUT their type (plan 5.3 asked for the type): `DigitalTwinModel::getElementStrings`
only yields strings, and the type would need a change of the C++ library. The initial value is therefore always
`Integer 0`, and the recorder classifies the type from the payload. The facade keeps its global state in a leaked
singleton, so a blocking call that is still running when the process exits never races with static destruction.

The facade (`ffi/dt_facade.cpp`) does the equivalent of `DigitalTwinManager::downloadDigitalTwin` without
`generateMQTTInterface` (which `dynamic_cast`s the client to `MqttClientService*`): it lists the twins of the
project, builds `DigitalTwinModel` with a manager that has a no-op `IMqttClientService`, keeps the model per twin id
(mutex protected, one mutex per model), and returns the name and the element strings. Every call catches all C++
exceptions and returns an error code and message. An empty qualified name in `register_function` is rejected
(`DigitalTwinModel::registerFunction` would index `parts.size() - 1`).

## History (`src/history/`)

Enabled only when `run.history.database_url` or `DT_HISTORY_DATABASE_URL` is set; otherwise one log line says
that recording is off. `run.history.{retention_days,queue_capacity,batch_size,flush_interval_ms}` as in the plan.

- Schema (`migrations/0001_init.sql`, embedded with `sqlx::migrate!`, applied at startup): `twin` and the monthly
  range partitioned `measurement` as in plan 6.1, plus a unique index on `twin.name`. There is no primary key or
  unique constraint on `measurement` (PostgreSQL would require the partition key in it) and no default partition.
  `value_type`: 1 Real, 2 Integer, 3 Boolean, 4 Rational, 5 Complex, 6 String. Integers are stored as `double precision`;
  beyond 2^53 the exact digits are also in `value_text`; integers beyond `i64` (exact digits in `value_text`, the
  nearest double in `value_num`) are never turned into a Real. A JSON string that is not a fraction is a String
  (`value_text`, at most 4096 bytes, no NUL); `"1/0"`-like strings are invalid.
- Partitions `measurement_yYYYYmMM`: current and next month at startup and hourly; created on demand for rows whose
  `ts` lies in the accepted window `[max(2000-01-01, retention cutoff), end of next month)`. Rows outside are
  dropped and counted. Chosen over a default partition because a default partition blocks the creation of any
  partition that overlaps rows it already holds.
- Retention: `retention_days > 0` drops partitions that lie entirely before `now - retention_days` (hourly);
  0 keeps everything.
- Registry: the twin logic tells the history about every loaded twin (before the interface is published); the
  history applies the collision rule in memory (a name owned by another twin id stays with the first; warning; the
  later twin is not stored) and upserts `(twin_id, project_id, name, last_seen)`. At startup the registry is loaded
  from the table, so recording works after a restart before any `connectToTwin`.
- Recorder: subscribes to `#` on the hub, ignores `connectToTwin`, `*/registerFunction` and every message with
  `Origin::Server`, maps `<TwinName>/<elementPath>`, classifies the payload (`payload::values::classify`), stores
  `ts` from `time_point` and `received_at` from the arrival. Unknown twin, invalid payload and out-of-window values
  are dropped and counted (`HistoryStats`, logged every 60 s when changed and at shutdown).
- The server's own initial values are not recorded: `Hub::publish` marks messages `Origin::Server`, MQTT clients'
  messages `Origin::Client`, and the recorder only takes `Client` messages. (Values the server computes itself
  would need their own path; there are none yet.)
- Queue and writer: bounded queue (`queue_capacity`) filled with `try_send` (full: drop and count, the broker never
  waits); the writer collects `batch_size` rows or waits `flush_interval_ms`, then ONE
  `INSERT ... SELECT FROM UNNEST(9 arrays)` per batch. Transient errors: retry with backoff 0.5 s .. 30 s (memory is
  bounded by queue + one batch). A row the database refuses (SQLSTATE 22/23) is isolated by halving the batch and
  dropped alone. A missing partition (23514) is created and the batch retried. Shutdown flushes for at most 5 s,
  the rest is counted as `lost_on_shutdown`. The connection and the migrations are also retried in the background;
  an invalid URL is a startup error (the message never contains the URL).
- Partition DDL runs in a transaction under `pg_advisory_xact_lock`, so two servers on one database serialize; if a
  `CREATE ... PARTITION OF` still loses a race (42P07 / 23505) it counts as success when the partition is attached
  (checked in `pg_inherits`). A detached table of the same name is an error: its rows are dropped and counted.
- Time limits: every connection has `statement_timeout` 30 s and `lock_timeout` 5 s, every store call is wrapped
  in a 45 s timeout (a timeout is a transient error), connecting plus migrating is limited to 60 s per attempt,
  and during shutdown no call may run past the shutdown deadline.
- Retry rules of the writer: transient errors are retried without limit (memory is bounded); a missing partition
  (23514) is re-created ONCE, a persisting 23514 is treated as bad data (the batch is split, the offending rows
  are dropped and counted); other permanent errors (permissions, missing table) drop the batch after
  `permanent_attempts` (5) tries with an error log and the `rejected` counter, so later data is not blocked.
- Registry registrations (`PendingUpserts`, at most `registry_queue` = 1024 twins, newest version per twin) stay
  pending until the database accepts them and are retried with backoff; at shutdown they are drained for at most
  5 s. The registry load at startup merges with the memory by one rule: the database owner of a name wins; a twin
  id already known in memory keeps its newer in-memory name; an in-memory twin that holds a name of a database
  twin loses it (warning) and nothing is recorded for it.
- Maintenance (partitions, retention) runs every step even if another failed and repeats after 60 s when it had
  errors.
- Memory bound of the recorder: the hub subscription holds at most 256 messages (`HUB_CAPACITY`), i.e. at most
  256 x `max_packet_size` (256 KiB) = 64 MiB of payloads; messages beyond that are dropped and counted
  (`hub_dropped`). Classification happens before queueing, so queue and writer batch only hold compact rows
  (topic path at most 512 bytes, text at most 4096 bytes): at most about `(queue_capacity + batch_size) x 5 KiB`
  (default 10000 + 500: roughly 52 MiB, in practice far less). Worst case with the defaults: about 120 MiB.
- Rows dropped because the database was never reached before shutdown are counted in `lost_on_shutdown`.

## Layout

| Path | Content |
|---|---|
| `src/config.rs` | CLI keys, defaults and JSON file layout, compatible with `DigitalTwinServerInstanceManager` |
| `src/broker/` | `codec` (MQTT v5 packets), `topic` (matching, limits), `subscriptions` (store), `tracker` (connection limits), `session`, `limits` |
| `src/hub.rs` | in-process hub: `subscribe(filter, capacity)` and `publish(topic, payload)`; messages carry their `Origin` |
| `src/twin/` | `TwinBackend` trait, twin service (`mod.rs`), `ffi.rs` (feature `ffi`, cxx bridge) |
| `ffi/`, `build.rs` | C++ facade (`dt_facade.h/.cpp`); `build.rs` compiles it against the C++ build tree (feature `ffi`) |
| `src/history/` | `partition`, `registry`, `row`, `store` (trait), `pg` (sqlx), `recorder` (ingest, stats), `writer` (batches, maintenance) |
| `migrations/` | embedded SQL migrations |
| `src/payload/` | `time` (`time_point`), `twin` (`connectToTwin`, `registerFunction`), `values` (value types and `classify`) |
| `tests/` | integration tests against the real broker (rumqttc v5 client and a raw socket client) |

## Decision: MQTT codec (phase 0)

Decision: **own small codec** (`src/broker/codec.rs`, QoS 0 only, about 300 lines plus tests), no codec crate.

Candidates, checked against the crate sources (downloaded and read, not run against Paho or Python clients):

- `mqttbytes` 0.6.0 has a `v5` module and covers the fields we need (CONNACK `max_packet_size` and
  `server_keep_alive`, subscribe `nolocal`, SUBACK `QuotaExceeded` / `TopicFilterInvalid`, DISCONNECT `ProtocolError`).
  Rejected because: the last release is from 2021-10 (the repository is `rumqtt`, which moved on to its own internal
  copy), and `v5::read` returns `Error::PayloadRequired` for every packet with remaining length 0 except
  PINGREQ/PINGRESP. A DISCONNECT without reason code (remaining length 0) is the normal way for clients to say
  goodbye in MQTT 5, so it would be treated as an error. `max_size` is checked in the same call that parses the
  whole packet; our decoder checks the size from the fixed header alone and never buffers an oversize body.
- `mqttbytes-ng` is a fork of the above; not evaluated further (small, young, same code base).
- `ntex-mqtt` 10.x is a full server/client framework with its own service and codec stack (and a Rust 1.97 MSRV).
  It would take over session handling, which the plan wants to keep under our control (limits, reason codes,
  later per-topic authorization).

The needed subset is small (QoS 0): decode CONNECT, PUBLISH, SUBSCRIBE, UNSUBSCRIBE, PINGREQ, DISCONNECT, encode
CONNACK, PUBLISH, SUBACK, UNSUBACK, PINGRESP, DISCONNECT. Properties of incoming packets are skipped with bounds
checks, not interpreted. The codec is covered by unit tests with hand computed bytes, and by integration tests with
`rumqttc` (a real, independent MQTT v5 client) and a raw socket client.
`rumqttc` 0.25 is used as dev-dependency only.

## Behaviour and deviations from the C++ server

Same as C++: MQTT v5 only, QoS 0 only (QoS > 0 PUBLISH gives DISCONNECT `protocol_error` and close), first packet must
be CONNECT, CONNECT timeout, CONNACK with `session_present = false`, `maximum_packet_size` and `server_keep_alive`,
subscription quota (updates never count), topic length/level limits (SUBSCRIBE: `topic_filter_invalid`; PUBLISH:
dropped silently), `matchFilter` semantics (also `a/#` matches `a`, `#` matches `$SYS/...`), no-local,
one delivery per session, pending-send limit with drop and disconnect after 3 consecutive overflows (accounting
topic + payload bytes, like C++), dual-stack listener with IPv4 fallback, IPv4-mapped IPv6 counted as IPv4, CLI
matching by substring with first match winning, file values overriding the command line.

Deviations:

- Keep alive: enforced as 1.5 x `server_keep_alive_s` of silence (plan 4.1/5.1), then DISCONNECT 0x8D and close. The
  C++ code has no explicit timer (it relies on the async_mqtt endpoint). `server_keep_alive_s = 0` disables it.
- Control packets (CONNACK, SUBACK, UNSUBACK, PINGRESP) count against `max_pending_send_bytes` like publishes. A client
  that sends requests without reading the answers is disconnected when the limit is exceeded, so the memory of a
  session is bounded. (C++ had no such limit.)
- PUBLISH properties (response topic, correlation data, user properties, content type, ...) are forwarded to
  subscribers and to the hub (`Message::properties`, raw bytes); Topic Alias and Subscription Identifier are removed.
  A PUBLISH with a Topic Alias is a protocol error (no Topic Alias Maximum is announced).
- The Maximum Packet Size of the client (CONNECT property) is honoured: larger PUBLISHes are not sent to that client,
  they are counted (`Hub::oversize_dropped_total`) and are not a slow-consumer overflow. `Hub::publish` rejects
  payloads that do not fit into the broker's maximum packet size.
- An empty client identifier gets a generated one (Assigned Client Identifier in CONNACK).
- Nothing but CONNACK is sent before the connection is established: an invalid first packet (e.g. QoS 1 PUBLISH,
  malformed CONNECT) just closes the connection.
- Reserved fixed header flags (SUBSCRIBE/UNSUBSCRIBE must be 0x2, CONNECT/PINGREQ/DISCONNECT 0x0), QoS 3 and
  Retain Handling 3 are malformed packets (DISCONNECT 0x81). Not checked: non-minimal variable byte integers, U+0000
  in strings.
- A second CONNECT, and a PUBLISH with an empty topic or a wildcard in the topic, end in DISCONNECT (0x82 / 0x90). C++
  would only drop or ignore (not verified). Packets of other types (PUBACK, AUTH, ...) are ignored like in C++.
- A SUBSCRIBE filter that is syntactically invalid (empty, `a/#/b`, `a#`) gets `topic_filter_invalid`. C++ only checks
  the length/level limits and would store a filter that never matches.
- Oversize packets: the size is checked on the fixed header; the client gets DISCONNECT 0x95 (`packet_too_large`)
  before the close. C++ just closes.
- Dropping a `Broker` starts the shutdown (clients get DISCONNECT 0x8B, the hub closes) without waiting;
  `shutdown().await` waits. After either, `HubSubscription::recv()` returns `None` and `Hub::publish` / `subscribe`
  fail with `HubError::Closed`.
- A connection that does not speak MQTT v5 is closed without a reply.
- The file is applied before the command line ports are validated: an invalid command line port is only an error if
  the file does not provide the value (C++: the file value wins).
- Arguments are read with `args_os` and converted lossily (no panic on non-UTF-8).
- Ports in the file may be strings or numbers (C++ would throw on numbers). Numeric limits may also be strings.
- A configuration file that is named but cannot be read or parsed is a startup error (exit code 1). In C++ the
  parse exception is thrown in the constructor, outside of the `try` blocks of `main`, so the process terminates too.
- The CLI parser is hand written (not `clap`) to keep the C++ substring semantics, `argv[0]` is not interpreted.
- No tracing of PUBLISH payloads; each PUBLISH is logged at `debug` level with topic and size only. Passwords and the
  database URL are redacted in the `Debug` output of the configuration.
- Not configurable, as in C++: topic length (512) and levels (16), consecutive overflows (3).

Payload types:

- Struct fields are declared in alphabetical key order, so the JSON bytes equal nlohmann::json output (which sorts
  keys). This is asserted in unit tests against literal strings; it was not compared against a build of the C++ code.
- `time_point` is written with exactly three fractional digits and `Z` (sub-millisecond parts are floored). Parsing
  accepts the fixed layout `YYYY-MM-DDTHH:MM:SS` with an optional fraction (digits after the third are ignored) and
  requires `Z`. It is stricter than the C++ `sscanf` parser (no short fields like `2026-1-2T3:4:5Z`, ranges are
  validated, e.g. hour 25 is rejected).
- `classify()` classifies by JSON shape: `realValue`+`imaginaryValue` is Complex, bool is Boolean, `"n/d"` string is
  Rational, a JSON integer is Integer, any other number is Real. Natural and Positive are not distinguishable from
  Integer by the payload and are reported as Integer; the typed structs `NaturalValue` / `PositiveValue` exist for
  callers that know the type. A string value that is not a fraction is rejected (String values are not defined yet).
  `PositiveValue` accepts 0, like the C++ entity.
- Rational values are reduced and have a positive denominator (like `boost::rational`); denominator 0 is rejected.
  `long` is treated as 64 bit (Linux).

## Hub

`Broker::hub()` returns a cloneable `Hub`. `hub.subscribe(filter, capacity)` gives a bounded receiver of
`Message { origin, topic, payload, properties }` for every accepted PUBLISH (and for hub publishes). A full channel drops the message and
counts it (`HubSubscription::dropped()`, `Hub::dropped_total()`); the broker never waits. `hub.publish(topic, payload)`
delivers to all matching client sessions and all matching internal subscribers (there is no no-local for internal
code, so a recorder on `#` also sees what the server itself published; such messages have `Origin::Server`, those of
MQTT clients `Origin::Client`).

## Known remaining deviations and limits

Phase 3 and 5:

- The facade lives in `DTServerRust/ffi/` and not in `DigitalTwinLib/src/ffi/` (nothing outside `DTServerRust/` may
  change); `IMqttClientService` was not extended with `subscribe`: the subscription lives in Rust.
- `load_twin` of an already loaded twin replaces its model; the C++ `DigitalTwinManager` would keep the old model
  and subscribe again (duplicate callbacks).
- The up to 4 parallel downloads share one `CommunicationService` like in C++. Its thread safety (plan risk 11) has
  not been checked with a sanitizer. The model library leaks the `Component` objects of a model on release (its
  destructor only frees the instance manager); not changed.
- Twin names containing `/` are not supported (the first topic level is the name, as in the C++ topic building).
- Login: see "Twin logic". An expired token is refreshed only when a download is refused (once per download).
- The history writes the registry row of a twin asynchronously; if the queue of registrations (1024) is full the
  row is not written until the next `connectToTwin` of that twin (the in-memory registry is always right).
- Until the history database is reachable the registry is empty: values are counted as `unknown_twin`.
- Not verified: SQL against a real PostgreSQL (none available here; the statements are syntax checked with
  libpg_query and the database tests compile, but they were not executed); the facade against a real Structura
  server; thread safety of the C++ libraries; the `ffi` build on anything but this machine's tree.

Broker and payloads (first increment):

- No shared subscriptions: `$share/...` filters are stored like ordinary filters.
- UNSUBACK always reports success (also for filters the session did not have), like C++.
- CONNACK does not announce Maximum QoS or Retain Available (QoS > 0 is a protocol error at PUBLISH time; retain
  flags of PUBLISH are accepted and ignored, nothing is retained).
- Floating point formatting: serde_json writes small and large exponents like `1e-7`, nlohmann::json (C++) writes
  `1e-07`. Both parse back to the same value; the bytes differ for such values. Not compared against a C++ build.
- Will messages, user name and password of CONNECT are parsed past but not used (no authentication yet).
