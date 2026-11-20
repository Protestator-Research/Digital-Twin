# dt-server (Rust port of the Digital Twin Server)

Port described in `Markdownfiles/Rust_Server_Portierung_Implementierungsplan.md`: configuration/CLI, embedded
MQTT broker (3.1.1 and 5, selectable), in-process hub and payload types (first increment), twin logic with the FFI bridge to the C++
libraries (phase 3) and the history in PostgreSQL (phase 5). Not included: REST API, TLS, removal
of the C++ server. MQTT clients log in with their SysML credentials, see "Authentication".

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

MQTT protocol version of the broker: `--instance.mqtt.protocol_version <value>` or `run.mqtt.protocol_version` in the
configuration file (the file wins, like for the port). Values:

| Value | Accepts |
|---|---|
| `"3.1.1"` (default) | MQTT 3.1.1 clients only (the behaviour before MQTT 5 was added) |
| `"5"` | MQTT 5 clients only |
| `"both"` | MQTT 3.1.1 and MQTT 5, decided per connection by the protocol level of the CONNECT |

A client with a version that is not enabled is refused with a CONNACK in its own format (3.1.1: return code `0x01`,
MQTT 5: reason code `0x84`, then the connection is closed). The old MQTT 3.1 (`MQIsdp`, level 3) is not supported;
`"3.1.1"` is the protocol with level 4. An invalid value is a startup error.

### Authentication

Every MQTT client has to log in: the user name and password of its CONNECT packet (3.1.1 and MQTT 5) are checked
against the SysMLv2 backend (`sysml.url` / `sysml.port`) with `POST <base>/login`, JSON body
`{"username": "...", "password": "..."}`. A 2xx answer with a `{"bearer": "<token>"}` body means the credentials are
valid (the token is not used), 401 and 403 mean they are wrong. The base URL is built like in the C++ code: port 443 is
`https://<url>`, port 80 `http://<url>`, any other port `http://<url>:<port>/`; a `sysml.url` that already contains
`://` is used as it is and `sysml.port` is then ignored (the server logs a warning at start if it is not 80 or 443; the login URL is logged without any `user:password@` part). No redirects are followed; connect timeout 10 s, request timeout 15 s. HTTPS certificates are checked against the operating system's root store (and the bundled webpki roots), so a backend behind a corporate CA works like with the C++ client.

Setting: `--instance.mqtt.authentication <value>` or `run.mqtt.authentication` in the configuration file (the file wins).

| Value | Effect |
|---|---|
| `"sysml"` (default) | CONNECT needs user name and password, verified at the backend. An empty `sysml.url` is a startup error |
| `"none"` | every client is accepted, user name and password are ignored |

Further keys under `run.mqtt`: `auth_cache_s` (default 300; seconds a successful login is remembered, 0 = no cache; only
successes are cached, the key is the user name plus a salted SHA-256 of the password, never the password itself) and
`auth_max_concurrent` (default 8; simultaneous `/login` requests, further CONNECTs wait). The whole check, including that
wait, is bounded by the CONNECT timeout (`run.mqtt.limits.connect_timeout_s`) minus 0.5 s: a backend that is too slow
gives CONNACK `0x03` / `0x88` (below) before the connection would be closed for timing out.

Throttling of failed logins (`run.mqtt`): after `auth_max_failures` (default 5, 0 = off) wrong logins of one user name or
from one IP address within `auth_failure_window_s` (60) that user name / address is refused for `auth_block_s` (10)
seconds without asking the backend, with the same CONNACK as a wrong password (`0x04` / `0x86`). A valid login clears
the failures of the user name. At most `auth_max_pending_per_ip` (4, 0 = no limit) checks of one address run at the same
time; more are answered with `0x03` / `0x88`. Note that a blocked user name is also blocked for its legitimate owner
until the block ends.

CONNACK of a refused login: wrong or missing user name or password: 3.1.1 return code `0x04`, MQTT 5 reason `0x86`;
backend not reachable, throttled (429), server error or an answer that is not a login response: 3.1.1 `0x03`, MQTT 5
`0x88` (this includes a check that does not finish in time). The connection is closed after the CONNACK. Packets that follow the CONNECT in the same TCP segment are only
processed after the login was accepted. Passwords and tokens are never logged; the user name and the result are
(`info` for accepted, `warn` otherwise).

The C++ client (`MqttClientService`, Paho) sends the SysML REST user name and password of its settings. It does the
reconnect itself (pauses of 1 s growing to 60 s) instead of using Paho's automatic reconnect, and stops for good on
CONNACK `0x04` / `0x05` (3.1.1) or `0x86` / `0x87` (MQTT 5), so a wrong password does not make it ask the backend again
and again.

Notes: the broker has no TLS yet, so **passwords travel in clear text over MQTT**. Physical twin devices and test
tools (`mosquitto_pub`, ...) now need SysML credentials in their CONNECT as well, or the broker has to run with
`authentication: "none"`. The SysML credentials of the broker itself (`sysml.username` / `sysml.password`) are not
related to the login of the clients.
Log level via `RUST_LOG` (default `info`). Output goes to `target/` (ignored by `.gitignore`).

## Features, build and run

Without features nothing but Rust and crates.io is needed. `cargo build` produces a server whose twin backend
reports "twin backend not available" for every `connectToTwin` (logged, the server keeps running); broker and
history work.

### With the C++ model libraries (`--features ffi`)

The feature links the real twin backend: `build.rs` compiles the cxx bridge and the facade `ffi/dt_facade.cpp`
(C++20, uses `DigitalTwinLib`, `BackendCommunicationService` and `sysmllib` unmodified) and links it against an
existing CMake/Conan build tree of the C++ project. `build.rs` never writes into that tree and never runs cmake or
conan; a failing `--features ffi` build or a missing library at run time is a problem of the C++ tree, rebuild it
there.

One-time C++ steps, from the repository root (they write only into `build/`). The location of the build tree
depends on the generator: multi-config generators (Visual Studio, Xcode) use plain `build/`, single-config
generators (Makefiles, Ninja; the Linux default) use `build/<build_type>` (`cmake_layout`), here `build/Release`.

```
# Linux / macOS with Makefiles or Ninja (tree: build/Release)
conan install . -s build_type=Release --build missing --format=json --out-file build/Release/conan-graph.json
conan build . -s build_type=Release
```

```
# Windows with Visual Studio (tree: build)
conan install . -s build_type=Release --build missing --format=json --out-file build/conan-graph.json
conan build . -s build_type=Release
```

Add `-o with_client=False` to both commands to skip the Qt client and the `md4c`/Qt requirements; the four libraries
and, with the default `DT_BUILD_TESTS=ON`, their C++ tests (which need GTest) are still built. The libraries must be
built shared (the Conan default, `shared=True`).
`conan-graph.json` is the list of the Conan packages with their include, library and binary directories; it is
written by `conan install` (use `--format=json`) and is read by `build.rs`.

Then build and test the server:

```
# Linux / macOS (sh); with Xcode use <repo>/build
export DT_CPP_BUILD_DIR=<repo>/build/Release
cargo build --features ffi
cargo test  --features ffi
```

```
# Windows (PowerShell)
$env:DT_CPP_BUILD_DIR = "<repo>\build"
cargo build --features ffi
cargo test  --features ffi
```

Where the inputs come from:

- Conan packages (include, library and binary directories, the library names of `sysmllib`): from the graph JSON
  (`DT_CPP_CONAN_GRAPH`, default `$DT_CPP_BUILD_DIR/conan-graph.json`, else `$DT_CPP_BUILD_DIR/../conan-graph.json`). Only `host` packages that are not
  `test_requires` are used; the same file layout works on every platform. The facade links the four project
  libraries and exactly the libraries `sysmllib` lists in its `cpp_info` (names are not hard-coded). Conan `defines`
  are passed to the C++ compiler.
- Project libraries and configuration: from `$DT_CPP_BUILD_DIR/CMakeCache.txt` (if it is missing but
  `$DT_CPP_BUILD_DIR/<config>/CMakeCache.txt` exists, the error names that directory; `build.rs` does not switch to
  it by itself). A tree with
  `CMAKE_CONFIGURATION_TYPES` (Visual Studio, Xcode, Ninja Multi-Config) is multi-config: the libraries are in
  `out/<config>`. Otherwise (Makefiles, Ninja) the tree is single-config and they are in `out`. The configuration is
  `DT_CPP_CONFIG`, else `CMAKE_BUILD_TYPE` if set, else `Release`; it must be one of `CMAKE_CONFIGURATION_TYPES`
  (multi-config) or equal to `CMAKE_BUILD_TYPE` (single-config). `BUILD_SHARED_LIBS=OFF` in the cache is an error.
- Headers of the four C++ libraries: `<source dir>/{CpsBaseLib,BackendCommunicationService,
  PhysicalTwinCommunicationService,DigitalTwinLib}/src`; the source directory is `DT_CPP_SOURCE_DIR`, default the
  parent of `DTServerRust/`.

Library search order: `DT_CPP_EXTRA_LIB_DIRS`, then the Conan packages, then the project `out` directory.
**The order matters**: the `out` directory can hold older copies of the `sysml*` libraries (built against other
dependencies) that must not win over the Conan package `sysmllib`; with `out` first the link could succeed and the
binary would still fail to start or to load a library.

Finding the libraries at run time:

- Linux: `-Wl,--disable-new-dtags` and one `-Wl,-rpath,<dir>` per search directory, so the rpath is DT_RPATH (not
  RUNPATH) and also covers the dependencies of the libraries. A library that itself carries a RUNPATH (the C++
  project libraries do) ignores it; if a library is still not found, run in the environment of the C++ build:
  `source "$DT_CPP_BUILD_DIR/generators/conanrun.sh"` (`$DT_CPP_BUILD_DIR` is `build/Release` for a single-config tree).
- macOS: one `-Wl,-rpath,<dir>` per search directory.
- Windows: there is no rpath. `build.rs` copies the `*.dll` files of `DT_CPP_EXTRA_LIB_DIRS` (they come first), of
  the Conan `bin` directories and of the project library directory into `<OUT_DIR>/dll`, which is passed to the
  linker as a search directory; cargo puts such directories on `PATH` for `cargo run` and `cargo test`. The DLLs are
  also copied next to the final artifacts (`target\<profile>`) so `dt-server.exe` starts directly; a deleted copy
  is restored by the next `cargo build`. The first DLL of a name wins, in the order of the search directories.
  `DT_CPP_NO_DLL_COPY=1` switches both copies off; then put the Conan run environment (`generators\conanrun.bat`)
  and `build\out\<config>` on `PATH` yourself.

C++ compiler and runtime on Linux and macOS (not verified on a real Linux or macOS machine; the decisions are unit
tested in `tests/link_plan.rs`): unless `CXX` is set, the facade is compiled with `CMAKE_CXX_COMPILER` from
`CMakeCache.txt`. `compiler.libcxx` of the Conan graph (root node, else the `sysmllib` node) selects the C++
runtime: `libstdc++` (old ABI) defines `_GLIBCXX_USE_CXX11_ABI=0`, `libstdc++11` changes nothing, `libc++` on Linux
adds `-stdlib=libc++` and links `c++`; on macOS the default libc++ is kept. Conan `defines` are taken only from
`sysmllib` and the packages it requires.

Rules and limits:

- **MSVC needs a Release-like C++ tree** (`Release`, `RelWithDebInfo` or `MinSizeRel`). Rust always links the release
  C runtime (`/MD`); Debug DLLs use `/MDd`, and `std::string` is not ABI compatible across that boundary. `build.rs`
  stops with a message if the configuration is `Debug` or if `sysmllib` in the graph is a Debug build. Run
  `conan install` with `-s build_type=Release` and recreate the graph. Because a Visual Studio tree holds all
  configurations side by side in `out/<config>`, an existing Debug tree stays usable for the C++ client.
- Windows targets other than `*-pc-windows-msvc` (for example `*-windows-gnu`) are not supported: the C++ libraries
  are built with MSVC.
- Only linux, windows and macos are supported targets.
- `build.rs` prints a `cargo:warning` for every include or library directory of the plan that does not exist (stale
  build tree or graph). It reruns when the variables below, the graph, `CMakeCache.txt`, the project and `sysmllib`
  headers or the project library files change.
- The planning logic is in `ffi/link_plan.rs` (no I/O, shared with `tests/link_plan.rs`), so the Linux and macOS
  rules are tested on every host.

### Environment variables

| Variable | Used by | Meaning |
|---|---|---|
| `DT_SYSML_PASSWORD` | server | Structura password (beats file and command line) |
| `DT_HISTORY_DATABASE_URL` | server | PostgreSQL URL of the history (beats the file); may contain a password, never logged |
| `RUST_LOG` | server | log level, default `info` |
| `DT_CPP_BUILD_DIR` | build, `ffi` | CMake build directory of the C++ project (required for `--features ffi`) |
| `DT_CPP_CONFIG` | build, `ffi` | configuration of the C++ tree; default `CMAKE_BUILD_TYPE`, else `Release` |
| `DT_CPP_CONAN_GRAPH` | build, `ffi` | Conan graph JSON; default `$DT_CPP_BUILD_DIR/conan-graph.json` |
| `DT_CPP_SOURCE_DIR` | build, `ffi` | source directory with the four library directories; default the parent of `DTServerRust/` |
| `DT_CPP_EXTRA_LIB_DIRS` | build, `ffi` | extra library directories searched first, separated like `PATH` (`:` on Linux/macOS, `;` on Windows) |
| `DT_CPP_NO_DLL_COPY` | build, `ffi`, Windows | any value except empty, `0`, `false`, `off`: do not copy the DLLs (see above) |
| `DT_TEST_DATABASE_URL` | tests | enables the PostgreSQL tests (see below) |
| `DT_TEST_SERVER_URL`, `DT_TEST_USER`, `DT_TEST_PASSWORD`, `DT_TEST_PROJECT_ID`, `DT_TEST_TWIN_ID` | tests, `ffi` | opt-in live test against a Structura server (same names as `tools/run_live_test.py`); never started by the tests |

### Tests

- `cargo test`: unit tests plus the integration tests. `tests/twin_integration.rs` drives the whole twin flow
  (`connectToTwin` -> interface topics -> `registerFunction` forwarding, duplicate request, backend error,
  malformed payload, login retry, bounded pool, shutdown) over a real broker with a fake `TwinBackend`.
- `cargo test --test link_plan`: the link planning of `build.rs` for linux, macos and windows with synthetic
  CMake caches and Conan graphs (no C++ tree needed).
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
- after the load it publishes, through the hub, the typed initial value of the backend (`ValueEntity::getJson()`,
  `{"time_point": ..., "value": ...}`, not the C++ placeholder) on `<TwinName>/<element>` for every variable (a value
  that is not a valid payload is skipped and counted, like an invalid path) and the empty
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

The facade returns, per variable, the path and its initial value as JSON (`DigitalTwinModel::getElementValueEntities`,
`ValueEntity::getJson()`), so the initial value has the type of the variable (ScalarQuantities are Real, unknown types
are provisionally Integer 0). Functions are not part of it: their MQTT interface exists only after initialization.
The recorder classifies the type from the payload. The facade keeps its global state in a leaked
singleton, so a blocking call that is still running when the process exits never races with static destruction.

The facade (`ffi/dt_facade.cpp`) does the equivalent of `DigitalTwinManager::downloadDigitalTwin` without
`generateMQTTInterface` (which `dynamic_cast`s the client to `MqttClientService*`): it lists the twins of the
project, builds `DigitalTwinModel` with a manager that has a no-op `IMqttClientService`, keeps the model per twin id
(mutex protected, one mutex per model), and returns the name and the elements (path and initial value JSON). Every
call catches all C++ exceptions and returns an error code and message. An empty qualified name in `register_function`
is rejected (`DigitalTwinModel::registerFunction` would index `parts.size() - 1`).

A single element whose initial value cannot be serialized does not fail the load: the facade passes it on with an
empty initial value, which the server rejects, counts in `elements_skipped` and logs at debug level. Non-finite Real
and Complex initial values are replaced by 0 in C++ (JSON has no NaN/inf). A String variable whose text looks like a
fraction (`"5/0"`) is rejected as an invalid Rational, not published as text.

The Rust build links the **Release** C++ libraries (`build/out/Release`) by default (see `DT_CPP_CONFIG`): after
every change in the C++ sources, rebuild that configuration (`cmake --build build --config Release --target
DigitalTwinLib`) before `cargo test --features ffi`.
`facade_sample_initial_values` is test/diagnostic support (one value entity per type, no server) used by the offline
C++ `getJson()` <-> `classify` test in `tests/ffi_backend.rs`.

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
| `src/auth/` | MQTT login: `Authenticator` trait, `SysmlAuthenticator` (`POST /login`, cache, concurrency limit) |
| `src/config.rs` | CLI keys, defaults and JSON file layout, compatible with `DigitalTwinServerInstanceManager` |
| `src/broker/` | `codec` (MQTT 3.1.1 and 5 packets), `protocol` (protocol versions and the configurable mode), `topic` (matching, limits), `subscriptions` (store), `tracker` (connection limits), `session` (includes the login check of the CONNECT), `limits` |
| `src/hub.rs` | in-process hub: `subscribe(filter, capacity)` and `publish(topic, payload)`; messages carry their `Origin` |
| `src/twin/` | `TwinBackend` trait, twin service (`mod.rs`), `ffi.rs` (feature `ffi`, cxx bridge) |
| `ffi/`, `build.rs` | C++ facade (`dt_facade.h/.cpp`); `build.rs` compiles it against the C++ build tree (feature `ffi`) and emits the link and run-time settings; `ffi/link_plan.rs` is its pure planning logic (paths, libraries, rpath, DLLs), also included by `tests/link_plan.rs` (runs without the feature, covers all three target systems) |
| `src/history/` | `partition`, `registry`, `row`, `store` (trait), `pg` (sqlx), `recorder` (ingest, stats), `writer` (batches, maintenance) |
| `migrations/` | embedded SQL migrations |
| `src/payload/` | `time` (`time_point`), `twin` (`connectToTwin`, `registerFunction`), `values` (value types and `classify`) |
| `tests/` | integration tests against the real broker (rumqttc MQTT 3.1.1 and MQTT 5 clients and a raw socket client; MQTT 5 and the protocol modes in `mqtt5_integration.rs`) |

## Decision: MQTT codec (phase 0)

Decision: **own small codec** (`src/broker/codec.rs`, QoS 0 only, MQTT 3.1.1 and 5, plus tests), no codec crate.

Candidates, checked against the crate sources (downloaded and read, not run against Paho or Python clients):

- `mqttbytes` 0.6.0 (originally evaluated for its MQTT 5 module; the 3.1.1 `v4` module has the same structure).
  Rejected because: the last release is from 2021-10 (the repository is `rumqtt`, which moved on to its own internal
  copy), and its `read` rejects packets whose payload is required but missing with `Error::PayloadRequired`
  (checked for the MQTT 5 module). `max_size` is checked in the same call that parses the whole packet; our decoder
  checks the size from the fixed header alone and never buffers an oversize body. The evaluation was done for MQTT 5
  and not repeated for 3.1.1.
- `mqttbytes-ng` is a fork of the above; not evaluated further (small, young, same code base).
- `ntex-mqtt` 10.x is a full server/client framework with its own service and codec stack (and a Rust 1.97 MSRV).
  It would take over session handling, which the plan wants to keep under our control (limits, return codes,
  later per-topic authorization).

The needed subset is small (QoS 0): decode CONNECT, PUBLISH, SUBSCRIBE, UNSUBSCRIBE, PINGREQ, DISCONNECT, encode
CONNACK, PUBLISH, SUBACK, UNSUBACK, PINGRESP. MQTT 5 adds property blocks (incoming ones are length-checked and
skipped, outgoing ones are empty except in CONNACK), reason codes in SUBACK/UNSUBACK and the server DISCONNECT; the
codec functions take the protocol version that the CONNECT of the connection negotiated. The codec is covered by unit
tests with hand computed bytes, and by integration tests with `rumqttc` (a real, independent client with a 3.1.1 API,
`rumqttc::{AsyncClient, MqttOptions, ...}`, and an MQTT 5 API, `rumqttc::v5`) and a raw socket client. `rumqttc` 0.25
is used as dev-dependency only (default features off are enough for plain TCP).

## Behaviour and deviations from the C++ server

MQTT 3.1.1 (protocol name `MQTT`, level 4) and MQTT 5 (level 5), selected by `run.mqtt.protocol_version` (see above; default
3.1.1 only). A level that is neither 4 nor 5 gets CONNACK return code 0x01 and is closed [MQTT-3.1.2-2]; a level that
is valid but not enabled gets the refusal CONNACK of its own version (0x01 for 3.1.1, 0x84 for MQTT 5); another
protocol name, e.g. 3.1 `MQIsdp`, is closed without a reply,
QoS 0 only (a QoS > 0 PUBLISH closes the connection; SUBSCRIBE with QoS 1/2 is granted with QoS 0), first packet must
be CONNECT, CONNECT timeout, CONNACK with `session_present = 0` and return code 0, subscription quota (updates never
count; over the quota and invalid filters get SUBACK return code 0x80), topic length/level limits (SUBSCRIBE: 0x80;
PUBLISH: dropped silently), `matchFilter` semantics (also `a/#` matches `a`, `#` matches `$SYS/...`),
one delivery per session (the publishing session included, unless an MQTT 5 subscription uses No Local), pending-send limit with drop and disconnect after 3 consecutive overflows (accounting
topic + payload bytes, like C++), dual-stack listener with IPv4 fallback, IPv4-mapped IPv6 counted as IPv4, CLI
matching by substring with first match winning, file values overriding the command line.

Deviations:

- Keep alive: the keep alive of the client from its CONNECT is enforced (3.1.1, 3.1.2.10): after 1.5 x that interval of
  silence the connection is closed. Keep alive 0 disables it. There is no server-side override any more (MQTT 3.1.1
  has no Server Keep Alive), so the former limit `server_keep_alive_s` is gone; a configuration file that still
  contains the key is accepted and the key is ignored.
- MQTT 3.1.1 has no server DISCONNECT: whenever the broker ends a connection (shutdown, keep alive timeout, protocol
  error, malformed or oversize packet, second CONNECT, invalid topic name, slow client) it just closes the TCP
  connection; the reason is only logged. The DISCONNECT of a client is accepted. MQTT 5 clients get a DISCONNECT with a
  reason code first (see "MQTT 5").
- Empty client identifier: in 3.1.1 accepted only with clean session = 1 (the broker generates an id for its own
  bookkeeping and does not announce it). Without clean session the broker answers CONNACK return code 0x02 (identifier
  rejected) and closes (3.1.1, 3.1.3.1). In MQTT 5 an empty id is always accepted and the generated one is returned in
  the CONNACK property Assigned Client Identifier. Sessions are never persisted, so `session_present` is always 0.
- Control packets (CONNACK, SUBACK, UNSUBACK, PINGRESP) count against `max_pending_send_bytes` like publishes. A client
  that sends requests without reading the answers is disconnected when the limit is exceeded, so the memory of a
  session is bounded. (C++ had no such limit.)
- `max_packet_size` is only enforced for incoming packets. MQTT 5 clients are told the limit in the CONNACK (Maximum
  Packet Size); 3.1.1 cannot announce it. `Hub::publish` rejects payloads that do not fit into it (counted with the
  MQTT 5 encoding if MQTT 5 is enabled, which is one byte larger).
- Nothing but CONNACK is sent before the connection is established: an invalid first packet (e.g. QoS 1 PUBLISH,
  malformed CONNECT) just closes the connection.
- Reserved fixed header flags (SUBSCRIBE/UNSUBSCRIBE must be 0x2, CONNECT/PINGREQ/DISCONNECT 0x0), QoS 3 and
  any set reserved bit 2-7 of the requested QoS byte of SUBSCRIBE are malformed packets (connection closed). Not checked: non-minimal variable byte integers, U+0000
  in strings.
- A second CONNECT, and a PUBLISH with an empty topic or a wildcard in the topic, close the connection. C++
  would only drop or ignore (not verified). Packet type 0 and 15 are reserved in 3.1.1 and malformed; other types the broker does not act on (PUBACK, ...) are ignored like in C++.
- A SUBSCRIBE filter that is syntactically invalid (empty, `a/#/b`, `a#`) gets return code 0x80 (MQTT 5: 0x8F). C++ only checks
  the length/level limits and would store a filter that never matches.
- Oversize packets: the size is checked on the fixed header; the connection is closed. C++ just closes.
- Dropping a `Broker` starts the shutdown (client connections are closed, the hub closes) without waiting;
  `shutdown().await` waits. After either, `HubSubscription::recv()` returns `None` and `Hub::publish` / `subscribe`
  fail with `HubError::Closed`.
- A connection whose first packet has another protocol name than `MQTT` is closed without a reply.
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
- `classify()` classifies by JSON shape: `real_value`+`imaginary_value` is Complex, bool is Boolean, `"n/d"` string is
  Rational, a JSON integer is Integer, any other number is Real. Natural and Positive are not distinguishable from
  Integer by the payload and are reported as Integer; the typed structs `NaturalValue` / `PositiveValue` exist for
  callers that know the type. A string value that is not a fraction is rejected (String values are not defined yet).
  `PositiveValue` accepts 0, like the C++ entity.
- Rational values are reduced and have a positive denominator (like `boost::rational`); denominator 0 is rejected.
  The Rust side uses `i64` (the C++ `long` of Linux). On Windows the C++ `long` has 32 bits: this does not affect the
  facade path (the facade handles no values), but the C++ libraries (`IntegerVariable`, `RationalVariable`,
  `RationalValueEntity` with `std::stol`) would truncate or reject values beyond 32 bits there.

## Hub

`Broker::hub()` returns a cloneable `Hub`. `hub.subscribe(filter, capacity)` gives a bounded receiver of
`Message { origin, topic, payload }` for every accepted PUBLISH (and for hub publishes). A full channel drops the message and
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
  server; thread safety of the C++ libraries.
- The `ffi` build is verified on Windows 11 with MSVC against a Release tree (Conan prebuilt packages): `cargo
  build --features ffi`, the `ffi_backend`, `twin_integration`, `broker_integration` and `history_db` tests and a
  start of `dt-server.exe` without `PATH` additions work. Not verified: macOS (never built, only the planning logic
  is unit tested), Linux (no Linux build was run; rpath handling is unit tested only), the live test against
  Structura, and the MSVC `RelWithDebInfo`/`MinSizeRel` trees. The DLL copy next to the artifact is verified for the
  default `target` directory only; a separately configured cargo `build-dir` or `--target` is not verified.

Broker and payloads (first increment):

- No shared subscriptions: in 3.1.1 `$share/...` filters are stored like ordinary filters; MQTT 5 clients get SUBACK
  reason code 0x9E (and Shared Subscription Available = 0 in the CONNACK).
- UNSUBACK carries only the packet id in 3.1.1 (no per-filter codes), also for filters the session did not have. In
  MQTT 5 it has one reason code per filter: 0x00, or 0x11 (no subscription existed).
- QoS > 0 PUBLISH closes the connection (3.1.1 has no way to announce a maximum QoS); the retain flag of a PUBLISH is
  accepted and ignored, nothing is retained.
- Floating point formatting: serde_json writes small and large exponents like `1e-7`, nlohmann::json (C++) writes
  `1e-07`. Both parse back to the same value; the bytes differ for such values. Not compared against a C++ build.
- The will (properties, topic, message) of CONNECT is validated for length and skipped; user name and password are
  decoded and used for the login (see "Authentication"). The CONNECT flags are validated: will QoS 3, will QoS or will
  retain without will flag, and (3.1.1 only; MQTT 5 allows it) password flag without user name flag are malformed, as is data behind the payload. A CONNECT with a password but no user name is refused with `0x86` when authentication is on, accepted with `none`. A PUBLISH with DUP = 1 at QoS 0 is malformed.

## MQTT 5

Enabled with `run.mqtt.protocol_version` = `"5"` or `"both"`. Scope: the same as for 3.1.1 (QoS 0 publish/subscribe,
no sessions, no retained messages, login by user name and password only, see "Authentication"), plus the parts of MQTT 5 that a QoS 0 broker needs. Verified with
unit tests, hand written packets and `rumqttc::v5`; not verified against other client libraries (Paho and others).

- CONNECT properties are length-checked and skipped (session expiry, receive maximum, maximum packet size, topic alias
  maximum, user properties, ...). In particular the **client's Maximum Packet Size is not applied**: the broker does not
  drop publishes that exceed it. Will properties are not parsed (the will itself is not used either).
- CONNACK announces: Maximum QoS 0, Retain Available 0, Subscription Identifiers Available 0, Shared Subscription Available 0, Maximum Packet Size (the limit
  `max_packet_size`) and, for an empty client id, the Assigned Client Identifier. Topic aliases are not offered (no Topic
  Alias Maximum): a client must not use them, a PUBLISH with an empty topic closes the connection.
- Incoming PUBLISH, SUBSCRIBE, UNSUBSCRIBE and DISCONNECT properties are skipped; outgoing PUBLISH, SUBACK and UNSUBACK
  have empty properties. Retain As Published and Retain Handling of a subscription are accepted and have no effect
  (nothing is retained). **No Local is honoured**: a session does not receive its own publishes through a subscription
  that has it. A QoS above 0 in a subscription is granted as QoS 0.
- Retain flag: although Retain Available is announced as 0, a PUBLISH with the retain flag is not an error; it is
  accepted and the flag is ignored (as for 3.1.1). The specification asks for DISCONNECT 0x9A here.
- SUBACK reason codes: 0x00 (granted QoS 0), 0x8F (topic filter invalid or beyond the topic limits), 0x97 (quota
  exceeded), 0x9E (shared subscriptions not supported). Subscription options with QoS 3, Retain Handling 3 or reserved
  bits 6-7 set are malformed packets.
- DISCONNECT sent by the broker (MQTT 5 clients only, after the queued messages): 0x8B server shutting down, 0x8D keep
  alive timeout, 0x81 malformed packet, 0x82 protocol error (second CONNECT), 0x90 topic name invalid, 0x95 packet too
  large, 0x9B QoS not supported (QoS 1/2 PUBLISH). A client that is dropped because it is too slow, or whose connection
  fails, gets nothing. DISCONNECT of a client with a reason code and properties is accepted. The AUTH packet (type 15)
  is a protocol error, DISCONNECT 0x82 (no enhanced authentication).
- PUBLISH properties of a client (user properties, content type, message expiry, ...) are skipped and **not forwarded** to
  subscribers, and the RETAIN flag (=1) of a PUBLISH is accepted and ignored: nothing is retained.
- `run.mqtt.limits.max_packet_size` must be at least 64 (it is announced as Maximum Packet Size, and 0 is invalid).
- Keep alive, connect timeout, limits and the in-process hub work as for 3.1.1. Server Keep Alive is not used.
