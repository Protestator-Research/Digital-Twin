# dt-server (Rust port of the Digital Twin Server)

Port described in `Markdownfiles/Rust_Server_Portierung_Implementierungsplan.md`: configuration/CLI, embedded
MQTT broker (3.1.1 and 5, selectable; QoS 0, 1 and 2; resumable sessions that can be kept in PostgreSQL), in-process hub and payload types (first increment), twin logic with the FFI bridge to the C++
libraries (phase 3), and a read-only REST API over the history database. The history itself is written by the
C++ core (feature `ffi`), see "History and REST API". Not included: TLS for the REST API, removal of the C++ server. The MQTT broker can run with TLS, see "TLS and Let's Encrypt". MQTT clients log in with their SysML credentials, see "Authentication".

## Build and test

```
cargo build
cargo test
cargo clippy --all-targets -- -D warnings
cargo fmt
```

Environment overrides (beat file and command line): `DT_SYSML_PASSWORD` (Structura password),
`DT_SYSML_APPLICATION_KEY` (Structura application key, see below), `DT_HISTORY_DATABASE_URL` (history database URL,
may contain a password), `DT_MQTT_SESSION_DATABASE_URL` (database of the MQTT sessions, may contain a password, see
"Session persistence (PostgreSQL)"), `DT_API_LISTEN` (address of the REST API),
`DT_API_DATABASE_URL` (its database URL, may contain a password) and `DT_API_BEHIND_PROXY` (`true` / `false`; an
invalid value is a startup error). Empty values are ignored.

Run: `cargo run -- --instance.config config.cfg --instance.mqtt.port 1883` (same keys as the C++ server).

### Session at the SysMLv2 server: user name and password, or an application key

The server's own session at the SysMLv2 (Structura) server, with which it downloads the twins, is set up in one of two
ways:

| Setting | Meaning |
|---|---|
| `sysml.username`, `sysml.password` (default `admin` / `admin`; env `DT_SYSML_PASSWORD`) | A login request is made and the returned token is used. If a download is refused as unauthorized (HTTP 401), the session expired: the server logs in again and retries once. |
| `sysml.application_key` (command line `--sysml.application_key`, file key `sysml.application_key`, env `DT_SYSML_APPLICATION_KEY`; default empty) | **If it is not empty it is used instead of user name and password.** The key is sent directly as the bearer token (`Authorization: Bearer <key>`) with every request; there is no login request. |

Notes on the application key:

- It is a secret: it is redacted in the log of the configuration (`<redacted>`), never logged and never part of an
  error message. Prefer the environment variable or the file over the command line (a command line is visible to
  other users of the machine). Startup logs one line that names the method but not the key.
- Whitespace around the key (a newline at the end of a file or a variable) is removed. Whatever remains has to consist
  of visible ASCII characters only (no spaces, no control characters, no non-ASCII text), because it is sent in a header;
  otherwise the configuration is rejected at startup with an error that names the setting but does not show the key.
- A key that consists only of whitespace is **not** "no key": from the command line or the file it is a configuration
  error (otherwise the server would quietly log in with the default user name and password). In the environment
  variable such a value (for example the newline of an empty mounted secret) is ignored like an empty one, so it does
  not replace a key from the file or the command line. An explicitly empty value means "no key".
- **The key is sent with every request, and in plain text unless the connection uses TLS.** The server talks TLS
  (`https://`) only if `sysml.port` is 443; every other port, 80 included, is plain HTTP. Use port 443, or a network you
  trust. The server logs a warning at startup if a key is configured and the port is not 443.
- It only concerns the server's own session at the SysMLv2 server. **MQTT clients are not affected**: they log in with
  their own user name and password, checked against the SysMLv2 server by `run.mqtt.authentication = "sysml"` (see
  "Authentication"), whichever way the server's own session is set up.
- **A wrong key is only noticed at the first twin load.** Setting the key makes no request, so nothing is checked at
  startup, and this server does not invent a validation request. If the SysMLv2 server answers a download with HTTP 401,
  the key is wrong, revoked or lacks the rights for the project: the error in the log says that the SysMLv2 server
  rejected the application key (followed by the server's own error detail, if it sent one; not the advice to log in
  again that a password session gets). Whatever text the SysMLv2 client library puts into an error, the key itself is
  replaced there before the message is logged. There is nothing to renew, so the server does **not** log in again and does not retry
  (unlike with a password, where a 401 means an expired session).

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

Notes: without TLS (see "TLS and Let's Encrypt") **passwords travel in clear text over MQTT**. Physical twin devices and test
tools (`mosquitto_pub`, ...) now need SysML credentials in their CONNECT as well, or the broker has to run with
`authentication: "none"`. The SysML credentials of the broker itself (`sysml.username` / `sysml.password`) are not
related to the login of the clients. With authentication a session (see "QoS and sessions") belongs to the user name
that created it.
Log level via `RUST_LOG` (default `info`). Output goes to `target/` (ignored by `.gitignore`).

### TLS and Let's Encrypt

The MQTT broker speaks TLS if both options are set: `--instance.mqtt.cert_chain <file>` and
`--instance.mqtt.cert_private_key <file>`, or `run.mqtt.cert_chain` and `run.mqtt.cert_private_key` in the
configuration file (the file wins). Both are PEM files. The listener on `instance.mqtt.port` then accepts **only TLS**
(TLS 1.2 and 1.3, no client certificates): a client that connects in plain text is disconnected. Neither option set:
plain TCP as before. The server logs at startup whether the broker runs with TLS or in plain text. Only one option set,
a file that cannot be read, or a file without certificate or key stops the start with an error that names the option
and the path (key material is never logged). The key may be PKCS#8 (`BEGIN PRIVATE KEY`), RSA PKCS#1 (`BEGIN RSA PRIVATE
KEY`) or SEC1 EC (`BEGIN EC PRIVATE KEY`); the whole chain of the certificate file is sent to the client.

The TLS handshake runs in the task of the connection, not in the accept loop, and counts against the CONNECT timeout
(`run.mqtt.limits.connect_timeout_s`, default 10 s): a client that does not finish the handshake and its CONNECT in this
time is disconnected. A failed handshake is logged at `debug` level and releases its slot of the connection limits.

With Let's Encrypt (certbot) the files are in `/etc/letsencrypt/live/<host name>/`:

| Option | File |
|---|---|
| `instance.mqtt.cert_chain` | `fullchain.pem` (certificate and intermediate certificates; not `cert.pem`) |
| `instance.mqtt.cert_private_key` | `privkey.pem` |

```
dt-server --instance.mqtt.port 8883 \
  --instance.mqtt.cert_chain /etc/letsencrypt/live/mqtt.example.org/fullchain.pem \
  --instance.mqtt.cert_private_key /etc/letsencrypt/live/mqtt.example.org/privkey.pem
```

- The server process must be able to read these files, and by default it cannot: `/etc/letsencrypt/live` and
  `/etc/letsencrypt/archive` are directories of root only (mode 0700), and `privkey.pem` is a link into `archive/`.
  The usual way is a certbot deploy hook that copies the files to a directory that belongs to the user of the server
  (`/etc/dt-server/tls/`, mode 0750, `privkey.pem` mode 0640 or 0600) and restarts the server, see below. The
  direct way is to give the server's group execute permission on `live/` and `archive/` and read permission on
  the key file in `archive/` (the link in `live/` points there); that also lets that group read all other keys that
  certbot manages, so the copy is the safer choice.
- Clients must connect by the host name that is in the certificate (`mqtt.example.org`), not by IP address or another
  name, or the verification of the certificate fails. The usual port for MQTT over TLS is 8883.
- The files are read at startup only. After a renewal the server has to be restarted to use the new certificate, for
  example with a certbot deploy hook (`--deploy-hook "systemctl restart dt-server"`).
- This is for the MQTT broker only. The REST API has no TLS, `rest.cert_chain` and `rest.cert_private_key` are still
  not used, see "REST API".

## Features, build and run

Without features nothing but Rust and crates.io is needed. `cargo build` produces a server whose twin backend
reports "twin backend not available" for every `connectToTwin` (logged, the server keeps running); the broker and
the REST API work, but no history is recorded (the C++ core writes it).

### With the C++ model libraries (`--features ffi`)

The feature links the real twin backend: `build.rs` compiles the cxx bridge and the facade `ffi/dt_facade.cpp`
(C++20, uses `DigitalTwinLib`, `DigitalTwinStore` (the history writer), `BackendCommunicationService` and
`sysmllib`) and links it against an
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

Add `-o with_client=False` to both commands to skip the Qt client and the `md4c`/Qt requirements; the five libraries
(`DigitalTwinLib`, `BackendCommunication`, `PhysicalTwinCommunication`, `CPSBaseLib`, `DigitalTwinStore`) and, with the default `DT_BUILD_TESTS=ON`, their C++ tests (which need GTest) are still built. The libraries must be
built shared (the Conan default, `shared=True`).
**`DigitalTwinStore` has to be built before the Rust build.** It is one of the project libraries the facade links; if
`DigitalTwinStore.lib` (`libDigitalTwinStore` elsewhere) is missing from the output directory, the link of
`cargo build --features ffi` fails with a missing library. `conan build` builds it with everything else. It needs
`libpqxx` (and with it `libpq`), which the Conan graph has to list: run `conan install` again (with `--format=json`)
whenever the dependencies of the C++ project changed.
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
  `test_requires` are used; the same file layout works on every platform. The facade links the five project
  libraries and exactly the libraries `sysmllib` lists in its `cpp_info` (names are not hard-coded). Conan `defines`
  are passed to the C++ compiler.
- Project libraries and configuration: from `$DT_CPP_BUILD_DIR/CMakeCache.txt` (if it is missing but
  `$DT_CPP_BUILD_DIR/<config>/CMakeCache.txt` exists, the error names that directory; `build.rs` does not switch to
  it by itself). A tree with
  `CMAKE_CONFIGURATION_TYPES` (Visual Studio, Xcode, Ninja Multi-Config) is multi-config: the libraries are in
  `out/<config>`. Otherwise (Makefiles, Ninja) the tree is single-config and they are in `out`. The configuration is
  `DT_CPP_CONFIG`, else `CMAKE_BUILD_TYPE` if set, else `Release`; it must be one of `CMAKE_CONFIGURATION_TYPES`
  (multi-config) or equal to `CMAKE_BUILD_TYPE` (single-config). `BUILD_SHARED_LIBS=OFF` in the cache is an error.
- Headers of the five C++ libraries: `<source dir>/{CpsBaseLib,BackendCommunicationService,
  PhysicalTwinCommunicationService,DigitalTwinLib,DigitalTwinStore}/src`; the source directory is `DT_CPP_SOURCE_DIR`, default the
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
| `DT_SYSML_APPLICATION_KEY` | server | Structura application key (beats file and command line); if set it replaces user name and password for the server's own session, see "Session at the SysMLv2 server" |
| `DT_HISTORY_DATABASE_URL` | server | PostgreSQL URL of the history (beats the file); may contain a password, never logged |
| `DT_MQTT_SESSION_DATABASE_URL` | server | PostgreSQL URL of the MQTT sessions (beats the file; not set: sessions only in memory, there is no fallback to the history URL); may contain a password, never logged |
| `DT_API_LISTEN` | server | listen address of the REST API, e.g. `127.0.0.1:8080` (beats the file); empty = disabled |
| `DT_API_DATABASE_URL` | server | PostgreSQL URL of the REST API (beats the file, default the history URL); may contain a password, never logged |
| `DT_API_BEHIND_PROXY` | server | `true` / `false`: take the client address of the API from the last `X-Forwarded-For` entry (beats the file) |
| `RUST_LOG` | server | log level, default `info` |
| `DT_CPP_BUILD_DIR` | build, `ffi` | CMake build directory of the C++ project (required for `--features ffi`) |
| `DT_CPP_CONFIG` | build, `ffi` | configuration of the C++ tree; default `CMAKE_BUILD_TYPE`, else `Release` |
| `DT_CPP_CONAN_GRAPH` | build, `ffi` | Conan graph JSON; default `$DT_CPP_BUILD_DIR/conan-graph.json` |
| `DT_CPP_SOURCE_DIR` | build, `ffi` | source directory with the five library directories; default the parent of `DTServerRust/` |
| `DT_CPP_EXTRA_LIB_DIRS` | build, `ffi` | extra library directories searched first, separated like `PATH` (`:` on Linux/macOS, `;` on Windows) |
| `DT_CPP_NO_DLL_COPY` | build, `ffi`, Windows | any value except empty, `0`, `false`, `off`: do not copy the DLLs (see above) |
| `DT_TEST_DATABASE_URL` | tests | enables the PostgreSQL tests (see below) |
| `DT_TEST_SERVER_URL`, `DT_TEST_USER`, `DT_TEST_PASSWORD`, `DT_TEST_PROJECT_ID`, `DT_TEST_TWIN_ID` | tests, `ffi` | opt-in live test against a Structura server (same names as `tools/run_live_test.py`); never started by the tests |

### Tests

- `cargo test`: unit tests plus the integration tests. `tests/twin_integration.rs` drives the whole twin flow
  (`connectToTwin` -> interface topics -> `registerFunction` forwarding, duplicate request, backend error,
  malformed payload, login retry, bounded pool, shutdown) over a real broker with a fake `TwinBackend`.
- `tests/qos_integration.rs`: QoS 1 and 2 in both directions (raw clients pin the bytes, rumqttc in both versions),
  delivery QoS, redelivery with DUP, offline queues, take-over, session expiry and quota, the owner of a session,
  Receive Maximum, Maximum Packet Size, limits, shutdown with open handshakes. The unit tests of `src/broker/` include
  whole connections against a test store that holds its barriers (`broker::session::tests`): they pin when CONNACK,
  SUBACK, UNSUBACK and the acknowledgements of a connection wait for the database, the order in which they leave,
  the stop of the read loop and a take-over during CONNECT.
- MQTT session database (needs PostgreSQL, see below): `tests/session_db.rs` (the whole broker: restart, expiry, clean
  sessions) and `src/broker/persist/db_tests.rs` (every operation of the store, group commit, bad rows, the lock of
  the schema). Run only when `DT_TEST_DATABASE_URL` is set, e.g.
  `DT_TEST_DATABASE_URL=postgres://user:pw@localhost/postgres cargo test --test session_db` and
  `DT_TEST_DATABASE_URL=... cargo test --lib db_tests` (or plain `cargo test` with the variable set). The user needs
  the right to create schemas. Every test creates its own schema `dt_test_<uuid>`, the broker (or the store) creates the
  tables in it from `sql/0001_mqtt_session.sql`, and the schema is dropped afterwards (also after a panic). Without the
  variable each test prints `SKIPPED ...` and passes, so **the database code paths are only tested when the variable is
  set**. These tests were written on a machine without PostgreSQL and have not been run against one yet; the
  statements are only checked for syntax (below). The tests of `session_db.rs` that need no database (unreachable or
  invalid URL) always run.
- `cargo test --test link_plan`: the link planning of `build.rs` for linux, macos and windows with synthetic
  CMake caches and Conan graphs (no C++ tree needed).
- `cargo test --features ffi`: additionally `tests/ffi_backend.rs`: calls without login, login against a closed
  port and against a server that answers 500 return errors (no abort, no password in the message). The live test
  needs `DT_TEST_SERVER_URL`, `DT_TEST_PROJECT_ID` and `DT_TEST_TWIN_ID` and is skipped otherwise.
  `tests/ffi_application_key.rs` (own test binary, because the facade has one session per process) uses a fake
  SysMLv2 server that records requests: after the application key login nothing was sent, the twin load sends
  `Authorization: Bearer <key>`, and a 401 is the unauthorized error; the password login makes `POST /login`.
- `tests/api_auth.rs`: the REST API without a database: Basic authentication (same `Authenticator` and throttle as
  MQTT, with a stub), routing, JSON errors and the parameter validation that happens before any query.
- PostgreSQL tests (`tests/api_db.rs`): run only when `DT_TEST_DATABASE_URL` is set, e.g.
  `DT_TEST_DATABASE_URL=postgres://user:pw@localhost/postgres cargo test --test api_db`. The user needs the
  right to create schemas. Every test creates its own schema `dt_test_<uuid>`, loads
  `DigitalTwinStore/sql/0001_schema.sql` into it, uses it as `search_path`, and drops it afterwards (also after a
  panic); nothing else is touched. They write a small twin with two commits, a checkpoint and later changes and call
  the routes through the axum router. Without the variable each test prints `SKIPPED ...` and passes.
- `cargo test --features sql-syntax-check`: checks the syntax of the schema files and of every SQL statement of the
  REST API and of the session store (`cargo test --lib --features sql-syntax-check schema_and` for the latter) with
  PostgreSQL's own parser (`pg_query`, libpg_query, no server). Off by default because the build of
  `pg_query` needs libclang.

## Twin logic (`src/twin/`)

`TwinBackend` (`backend.rs`) is the interface to the model library: `login`, `load_twin`, `register_function`,
`release_twin`, `update_variable` (with the arrival time) and the history store of the core (`configure_store`,
`store_stats`, `shutdown_store`). `TwinService` implements everything on top of it:

- subscribes to `connectToTwin` on the hub; each valid payload starts a download on a pool of 4 (a semaphore, the
  calls run in `spawn_blocking`); at most one download per twin id (a repeated request while one is queued or
  running is skipped and logged, like `DownloadsInProgress`); at most 64 distinct twins may be pending;
- after the load it publishes, through the hub, the typed initial value of the backend (`ValueEntity::getJson()`,
  `{"time_point": ..., "value": ...}`, not the C++ placeholder) on `<TwinName>/<element>` for every variable (a value
  that is not a valid payload is skipped and counted, like an invalid path) and the empty
  registration request on `<TwinName>/registerFunction`, and forwards client payloads on that topic to
  `register_function` (malformed or nameless requests are ignored), and forwards client updates of the variables
  (valid single values on `<TwinName>/<element>`) with their arrival time to `update_variable`. The two forwarding
  tasks of a twin live until the twin is loaded again or the service stops;
- errors of the backend are logged and counted, never fatal; malformed `connectToTwin` payloads are ignored;
- a download refused as unauthorized (HTTP 401 / not logged in) logs in again and is retried once;
- a twin name that is empty, contains `+`, `#`, `/` or NUL, or starts with `$`, is rejected before anything is
  subscribed or published (a twin named `+` would otherwise subscribe to `+/registerFunction`); element paths that
  are no valid topic (empty levels, wildcards, NUL) are skipped, with one warning and one count per load;
- at shutdown the loaded twins are released (at most 5 s).

Login: done in the background at startup and retried before every download until it works. A failed login (or the
missing backend) is logged and the server keeps running. With an application key the "login" only hands the key to
the backend (no request), and a download refused with HTTP 401 is reported as a rejected key without a new login or
retry (see "Session at the SysMLv2 server"). The C++ server lets the exception end `runInstance`;
that would make a binary without `ffi`, or a Structura that starts later than the server, unusable.

The facade returns, per variable, the path and its initial value as JSON (`DigitalTwinModel::getElementValueEntities`,
`ValueEntity::getJson()`), so the initial value has the type of the variable (ScalarQuantities are Real, unknown types
are provisionally Integer 0). Functions are not part of it: their MQTT interface exists only after initialization.
The server classifies the type from the payload. The facade keeps its global state in a leaked
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
every change in the C++ sources, rebuild that configuration before `cargo test --features ffi`, for **both** libraries
the facade depends on that change: `cmake --build build --config Release --target DigitalTwinLib DigitalTwinStore`
(or the whole tree). Rust does not rebuild them; a stale `DigitalTwinStore` is linked as it is.
`facade_sample_initial_values` is test/diagnostic support (one value entity per type, no server) used by the offline
C++ `getJson()` <-> `classify` test in `tests/ffi_backend.rs`.

## History and REST API

### History: written by the C++ core, not by this server

The history of the twins is written to PostgreSQL by the C++ core (`DigitalTwinStore`, plan:
`Markdownfiles/Plan_Datenbank_CppKern.md`), **only in a build with `--features ffi`**. This server does not write to
the database. It passes three things to the core:

- the settings (`run.history.*`, see below), once at startup, through `TwinBackend::configure_store`;
- every client update of a variable with its arrival time, through `update_variable(twin, path, payload,
  received_at_ms)`. The time is taken when the broker has decoded the PUBLISH (`Message::received_at_ms`, Unix
  milliseconds, UTC), before any queue of a subscriber, so a slow subscriber does not shift it;
- the load of a twin (`load_twin`): the core registers project, twin, commit and model tree in the database itself,
  and the initial values it returns are already the restored state (the last stored values of the same commit, or
  the initial values of a new commit).

What is stored: a change log (one change set per update: the variable the client set and every variable the model
changed with it) plus checkpoints (the full state at load, in a fixed interval, in a new month, after data loss and at
shutdown). The schema is `DigitalTwinStore/sql/0001_schema.sql`; the core creates it and maintains the monthly
partitions and the retention. The core is the only writer and the only one that changes the schema.

Without the feature there is no history: if a database URL is configured, the server says so once at startup
(`history is NOT recorded: ...`) and goes on.

Starting the store does **not** touch the database: `facade_store_configure` creates the store and its threads and
returns, and the store connects in the background (and keeps trying). The log line `history store configured` therefore
says nothing about the database. Whether there is a connection is the `connected` field of the statistics: the server
logs `history database connected` / `history database NOT connected` as soon as it changes (checked every 2 s) and the
other counters (`pending_rows`, `written_rows`, `dropped_rows`, `retries`, `checkpoints`) every 10 s, only when they
changed. A configuration the store cannot start with at all (an invalid URL, a second configuration) is an error of
the call; the server logs it and goes on without history.

The store writes its own messages to stderr as `[DigitalTwinStore] ...`, outside the `tracing` output (no level, no
`RUST_LOG`). Look there for the reason when `connected` stays false (wrong password, no such database, schema
problems).

**Old database:** a database that still has the tables of the old Rust recorder (`twin` and `measurement` with the
old layout) must have them dropped before the first start with the C++ store. The store refuses to start on such a
schema with a clear message (in its stderr output) instead of working in it; its data is not migrated.

At shutdown the twin service is stopped first (its forwarders work off what is queued, and a running backend call
ends before the twins are released; at most 5 s, then at most 5 s for the release, 15 s as the outer limit), then the store gets 10 s to write its queue and a last checkpoint.

| Key (`run.history.`) | Default | Meaning |
|---|---|---|
| `database_url` | empty = no history | PostgreSQL URL; env `DT_HISTORY_DATABASE_URL` beats the file. May contain a password, never logged |
| `retention_days` | 0 | drop data older than this many days (whole partitions); 0 keeps everything |
| `checkpoint_interval_s` | 600 | seconds between two interval checkpoints; 0 switches them off (the checkpoints at load, in a new month, after data loss and at shutdown remain) |
| `queue_capacity` | 10000 | **rows** (not change sets; one update can write several) waiting for the writer; further ones are dropped and counted |
| `batch_size` | 500 | rows per insert |
| `flush_interval_ms` | 200 | longest wait before a batch is written |

All numbers are unsigned 32-bit values; for `queue_capacity`, `batch_size` and `flush_interval_ms` 0 selects the
default of the store. The counters `pending`, `written` and `dropped` of the store statistics are rows as well. The old Rust recorder (`src/history`, its migration and its `measurement`
table with a different layout) is gone; its data is not carried over.

### REST API (`src/api/`)

A read-only HTTP API over the same database, in this process (axum on the tokio runtime). It is **off by default**.

| Key (`run.api.`) | Default | Meaning |
|---|---|---|
| `listen` | empty = disabled | socket address, e.g. `127.0.0.1:8080`; env `DT_API_LISTEN` beats the file |
| `database_url` | the history URL | database of the API; env `DT_API_DATABASE_URL` beats the file. May contain a password, never logged |
| `behind_proxy` | `false` | the API is only reachable through a reverse proxy that appends `X-Forwarded-For`: throttle by its last entry instead of the peer address (see below); env `DT_API_BEHIND_PROXY` beats the file |

The API works without the `ffi` feature (it only reads). It connects lazily, so it starts and answers `503` while the
database is down. Reads are runtime-checked `sqlx` queries (the build needs no database). The API never writes: give
it a database role that may only `SELECT` (its own `run.api.database_url`); as a second line of defence every
connection is opened with `default_transaction_read_only = on` and a statement timeout of 15 s.

Access control: **if MQTT authentication is enabled (`run.mqtt.authentication = "sysml"`), every request needs HTTP
Basic credentials, checked exactly like the CONNECT of an MQTT client** (same SysML login, same cache; the answer is
`401` with `WWW-Authenticate: Basic`, or `503` if the SysML backend cannot decide). Failed logins are throttled
by the **same `LoginThrottle` instance as the MQTT logins**, used the same way (`run.mqtt.auth_max_failures`,
`auth_failure_window_s`, `auth_block_s`, `auth_max_pending_per_ip`): a user name or a client address with too many
failures is refused without a check (answered `401`), and only a few checks of one address run at the same time
(further ones get `503`). A user name or address blocked over MQTT is blocked for the API and the other way round, so
the failure budget is not doubled. The slot of the pending checks is only held while the credentials are verified, not
while the request is served. Requests without a usable `Authorization` header get `401` and are counted nowhere. As
with MQTT, anybody who knows a user name can block that user name for a while by failing logins.

The client address is the peer address of the connection. **With a reverse proxy in front and `behind_proxy` off, all
users share the proxy's address**, so a few failed logins of one person block every API user for a while, and the
pending-check limit applies to all of them together. Switch `run.api.behind_proxy` (`DT_API_BEHIND_PROXY`, `true` or
`false`, default `false`) on only if the API port is reachable **solely through a proxy that sets or appends the
`X-Forwarded-For` header** (and that you trust to do so): the client address is then the last (rightmost) entry of that
header, the one the proxy itself added; entries to its left are whatever the client claimed and are ignored. If the
header is missing, or the last entry is not an address, the peer address is used. With the setting on and the port
reachable directly, a client could choose its own address and would escape the throttling. With it off the header is
ignored.

There is no
authorization beyond the login: every user who can log in can read all projects and twins, as
every MQTT client can read every topic. **With `authentication = "none"` the API is open.** In both cases: Basic
sends the password with every request and this server has no TLS, so bind the API to localhost
(`127.0.0.1:...`) or put it behind a reverse proxy that terminates TLS (and, if you need it, restricts access
further). The server warns at startup if the API is open on a non-local address.

Responses are JSON. Errors are `{"error": "..."}` with a fitting status: `400` for bad parameters (also unknown or
repeated parameters), `401`, `404` for unknown ids (and for a state without a checkpoint), `405` for anything but GET,
`503` for an unreachable database. Messages never contain SQL, driver errors or the database URL (those are only
logged). Ids are UUIDs, times are RFC 3339 in UTC; in a query string write `+` of an offset as `%2B`.

| Route | Answer |
|---|---|
| `GET /projects` | `[{project_id, name, first_seen}]` |
| `GET /projects/{project_id}/twins` | `[{twin_id, name, first_seen, last_seen}]`; `404` for an unknown project |
| `GET /twins/{twin_id}/versions` | the loaded commits, newest first: `[{commit, first_loaded, last_loaded}]` |
| `GET /twins/{twin_id}/elements?commit=` | `{twin_id, commit, elements: [{element_no, parent_no, kind, name, path, value_type}]}`, the model tree of one commit as a flat list. `kind` is `package`, `component`, `port`, `measurable`, `controllable` or `attribute`; `value_type` is `real`, `integer`, `boolean`, `rational`, `complex`, `string`, or `null` for non-variables. Default commit: the one with the latest `last_loaded`, i.e. the one loaded last (a reload of an older commit makes it the default again) |
| `GET /twins/{twin_id}/state?at=&commit=&path=` | the complete state at a time (default now), see below |
| `GET /twins/{twin_id}/series?path=&from=&to=&limit=&commit=` | the time series of one variable, see below |

`state`: the newest checkpoint at or before `at` (ties: the later one) decides the version and the base; the state is
that checkpoint plus the changes of the same version after it up to `at` (inclusive). Without `commit` it is the
newest checkpoint of **any** version of the twin, so `at` in the time commit B ran gives B even if commit A was loaded
again later (`last_loaded` is not used). With `commit` only the checkpoints of that commit count. `commit` and
`checkpoint_at` of the answer say which version and which checkpoint were used; version, checkpoint and values come
from one statement. `path` restricts the answer to that element and everything below it (a whole path level: `Motor`
does not match `Motors`). If there is no checkpoint at or before `at` (the twin was loaded later, or the data was
removed by the retention), the answer is `404`, not an empty state.

```json
{
  "twin_id": "00000000-0000-0000-0000-000000000020",
  "commit": "00000000-0000-0000-0000-000000000031",
  "at": "2026-01-10T10:07:00Z",
  "checkpoint_at": "2026-01-10T10:00:00Z",
  "variables": [
    {"path": "Motor/enabled", "kind": "controllable", "value_type": "boolean", "value": false,
     "received_at": "2026-01-10T10:00:00Z", "ts": "2026-01-10T10:00:00Z", "source": "checkpoint"},
    {"path": "Motor/speed", "kind": "measurable", "value_type": "real", "value": 2.5,
     "received_at": "2026-01-10T10:05:00Z", "ts": "2026-01-10T10:04:59Z", "source": "client"}
  ]
}
```

`received_at` is when the broker accepted the update, `ts` the `time_point` of the client's payload. `source` is
`client` (set by a client), `derived` (changed by the model because of a client update) or `checkpoint` (taken
over from the checkpoint).

`series`: the rows of the variable with exactly this `path`, ordered by `received_at` ascending (`from` and `to`
bound it, both inclusive). Without `commit` it spans all versions of the twin and joins them by path; every point
says which `commit` it is from and carries its `change_no`. `limit` is 1 to 10000 (default 1000; anything else is
`400`). Rows are ordered by `(received_at, change_no, element, source)`, which is total, so rows that arrived in the
same instant (millisecond stamps, a checkpoint row next to the update that caused it) have a fixed order. `more` tells
whether there are more rows than `limit`; then `next` is a cursor: pass it unchanged as `after=<next>` (together
with the same other parameters) to get the following page without skipping or repeating a row. Do not use `from`
for paging. Checkpoint rows are part of the series
(`source` `checkpoint`); filter them out if only changes matter. `404` if the twin has no variable with that path.

A page of a series (`GET /twins/{twin_id}/series?path=Motor/speed&limit=2`); the next page is
`...&limit=2&after=2026-01-10T10:05:00Z,2001,12,1`:

```json
{
  "twin_id": "00000000-0000-0000-0000-000000000020",
  "path": "Motor/speed",
  "limit": 2,
  "more": true,
  "next": "2026-01-10T10:05:00Z,2001,12,1",
  "points": [
    {"commit": "00000000-0000-0000-0000-000000000031", "received_at": "2026-01-10T10:00:00Z", "change_no": 2000,
     "ts": "2026-01-10T10:00:00Z", "source": "checkpoint", "value_type": "real", "value": 1.0},
    {"commit": "00000000-0000-0000-0000-000000000031", "received_at": "2026-01-10T10:05:00Z", "change_no": 2001,
     "ts": "2026-01-10T10:04:59Z", "source": "client", "value_type": "real", "value": 2.5}
  ]
}
```

`value` is decoded by the element's type: real is a number (NaN and infinity, which JSON cannot hold, become `null`),
integer a number (or a string of digits when the magnitude exceeds 2^53 and was stored as text), boolean `true` /
`false`, rational the string `"n/d"`, complex the stored object (`{"imaginaryValue": 2.0, "realValue": 1.0}`), string
a string.

The API documents itself: `/docs/` is a Swagger UI page for it (`/docs` redirects there, temporarily) and
`/openapi.yaml` is the OpenAPI 3.1 document, also at `/docs/openapi.yaml`. Both are embedded in the binary, the UI
assets are vendored (`swagger-ui-dist`, version in `docs/api/swagger-ui/VERSION`), so the page makes no external
request. They are behind the same authentication as the data routes: with MQTT authentication enabled they need Basic
credentials too. The files live in `docs/api/` and are served by `src/api/docs.rs`; keep `docs/api/openapi.yaml` in
step with the routes (tests check that every documented path is a route of the router and that the documented paths
are the expected six).

## Layout

| Path | Content |
|---|---|
| `src/auth/` | MQTT login: `Authenticator` trait, `SysmlAuthenticator` (`POST /login`, cache, concurrency limit) |
| `src/config.rs` | CLI keys, defaults and JSON file layout, compatible with `DigitalTwinServerInstanceManager` |
| `src/broker/` | `tls` (certificate and key of the TLS listener), `codec` (MQTT 3.1.1 and 5 packets), `protocol` (protocol versions, QoS and the configurable mode), `topic` (matching, limits), `subscriptions` (store, with the granted QoS), `tracker` (connection limits), `session` (one connection: reader, writer, the login check of the CONNECT, ordered and deferred acknowledgements), `client` (one client session: queue of QoS 1/2 messages, handshakes, packet identifiers), `core` (registry of the sessions, delivery, take-over, expiry), `persist` (interface and PostgreSQL implementation of the session store), `limits` |
| `sql/` | `0001_mqtt_session.sql`: the tables of the MQTT sessions (applied by the broker if they are missing, see "Session persistence (PostgreSQL)") |
| `src/hub.rs` | in-process hub: `subscribe(filter, capacity)` and `publish(topic, payload)`; messages carry their `Origin` |
| `src/twin/` | `TwinBackend` trait, twin service (`mod.rs`), `ffi.rs` (feature `ffi`, cxx bridge) |
| `ffi/`, `build.rs` | C++ facade (`dt_facade.h/.cpp`); `build.rs` compiles it against the C++ build tree (feature `ffi`) and emits the link and run-time settings; `ffi/link_plan.rs` is its pure planning logic (paths, libraries, rpath, DLLs), also included by `tests/link_plan.rs` (runs without the feature, covers all three target systems) |
| `src/api/` | read-only REST API: `mod.rs` (routes, errors, server), `model.rs` (words, value decoding, parameters), `sql.rs` (the statements), `auth.rs` (HTTP Basic with the MQTT authenticator), `docs.rs` (serves `docs/api/`) |
| `docs/api/` | OpenAPI document of the REST API (`openapi.yaml`), the page `index.html` and the vendored Swagger UI (`swagger-ui/`), embedded by `src/api/docs.rs` |
| `src/payload/` | `time` (`time_point`), `twin` (`connectToTwin`, `registerFunction`), `values` (value types and `classify`) |
| `tests/` | integration tests against the real broker (rumqttc MQTT 3.1.1 and MQTT 5 clients and a raw socket client; MQTT 5 and the protocol modes in `mqtt5_integration.rs`) |

## Decision: MQTT codec (phase 0)

Decision: **own small codec** (`src/broker/codec.rs`, QoS 0 to 2, MQTT 3.1.1 and 5, plus tests), no codec crate.

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

The needed subset is small: decode CONNECT, PUBLISH (any QoS, packet identifier, DUP), PUBACK, PUBREC, PUBREL, PUBCOMP,
SUBSCRIBE, UNSUBSCRIBE, PINGREQ, DISCONNECT, encode CONNACK, PUBLISH, PUBACK, PUBREC, PUBREL, PUBCOMP, SUBACK, UNSUBACK,
PINGRESP. MQTT 5 adds property blocks (incoming ones are length-checked and skipped except the few the broker acts on,
see "MQTT 5"; outgoing ones are empty except in CONNACK), reason codes in the acknowledgements, SUBACK/UNSUBACK and the
server DISCONNECT; the
codec functions take the protocol version that the CONNECT of the connection negotiated. The codec is covered by unit
tests with hand computed bytes, and by integration tests with `rumqttc` (a real, independent client with a 3.1.1 API,
`rumqttc::{AsyncClient, MqttOptions, ...}`, and an MQTT 5 API, `rumqttc::v5`) and a raw socket client. `rumqttc` 0.25
is used as dev-dependency only (default features off are enough for plain TCP).

## Behaviour and deviations from the C++ server

MQTT 3.1.1 (protocol name `MQTT`, level 4) and MQTT 5 (level 5), selected by `run.mqtt.protocol_version` (see above; default
3.1.1 only). A level that is neither 4 nor 5 gets CONNACK return code 0x01 and is closed [MQTT-3.1.2-2]; a level that
is valid but not enabled gets the refusal CONNACK of its own version (0x01 for 3.1.1, 0x84 for MQTT 5); another
protocol name, e.g. 3.1 `MQIsdp`, is closed without a reply,
QoS 0, 1 and 2 in both directions and resumable sessions (see "QoS and sessions"; SUBSCRIBE is granted with the QoS
it asks for), first packet must be CONNECT, CONNECT timeout, CONNACK with `session_present` (set when a session is
resumed) and return code 0, subscription quota (updates never
count; over the quota and invalid filters get SUBACK return code 0x80), topic length/level limits (SUBSCRIBE: 0x80;
PUBLISH: a QoS 0 PUBLISH is dropped silently; QoS 1 and 2 are acknowledged and not delivered, see "QoS and sessions": MQTT 3.1.1 plain PUBACK / PUBREC (the identifier of a QoS 2 PUBLISH is remembered until its PUBREL), MQTT 5 reason 0x97 and, for QoS 2, no state), `matchFilter` semantics (also `a/#` matches `a`, `#` matches `$SYS/...`),
one delivery per session (the publishing session included, unless an MQTT 5 subscription uses No Local), pending-send limit with drop and disconnect after 3 consecutive overflows (accounting
topic + payload bytes, like C++; for QoS 0 and control packets: QoS 1 and 2 messages have their own queue limits, see "QoS and sessions"), dual-stack listener with IPv4 fallback, IPv4-mapped IPv6 counted as IPv4, CLI
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
  the CONNACK property Assigned Client Identifier (a client that asks for a Session Expiry Interval can resume that
  session with it). A client identifier longer than 1024 bytes is refused (see "QoS and sessions").
- Control packets (CONNACK, SUBACK, UNSUBACK, PINGRESP) count against `max_pending_send_bytes` like publishes. A client
  that sends requests without reading the answers is disconnected when the limit is exceeded, so the memory of a
  session is bounded. (C++ had no such limit.)
- `max_packet_size` is enforced for incoming packets. MQTT 5 clients are told the limit in the CONNACK (Maximum
  Packet Size); 3.1.1 cannot announce it. The Maximum Packet Size an MQTT 5 client announces in its CONNECT is applied
  to what is sent to it: a larger PUBLISH is not sent to that client, whatever its QoS. `Hub::publish` rejects payloads that do not fit into it (counted with the
  MQTT 5 encoding if MQTT 5 is enabled, which is one byte larger, and with the 2-byte packet identifier of the QoS 1
  delivery).
- Nothing but CONNACK is sent before the connection is established: an invalid first packet (e.g. a PUBLISH,
  malformed CONNECT) just closes the connection.
- Reserved fixed header flags (SUBSCRIBE/UNSUBSCRIBE must be 0x2, CONNECT/PINGREQ/DISCONNECT 0x0), QoS 3 and
  any set reserved bit 2-7 of the requested QoS byte of SUBSCRIBE are malformed packets (connection closed). Not checked: non-minimal variable byte integers. A U+0000
  in a string is a malformed packet (MQTT-1.5.4-2; the PostgreSQL session store could not hold it either).
- A second CONNECT, and a PUBLISH with an empty topic or a wildcard in the topic, close the connection. C++
  would only drop or ignore (not verified). Packet type 0 and 15 are reserved in 3.1.1 and malformed; other types the broker does not act on (CONNACK, SUBACK, UNSUBACK, PINGRESP) are ignored like in C++.
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

## QoS and sessions

QoS 0, 1 and 2 work in both directions in MQTT 3.1.1 and MQTT 5. One connection is `src/broker/session.rs`, the state
of one client identifier `src/broker/client.rs`, the registry of the sessions `src/broker/core.rs`.

### QoS

- **Delivery QoS** is the lower of the QoS of the PUBLISH and the QoS granted to the subscription. A SUBSCRIBE is
  granted as requested (SUBACK 0x00, 0x01, 0x02); there is no downgrade and no Maximum QoS (MQTT 5 CONNACK does not
  announce one, so it is 2). A session with several matching subscriptions gets the message once, at the highest QoS
  of them (a subscription with No Local does not count for the session's own messages). The server's own publishes
  (`Hub::publish`) are made at QoS 1 and are only queued for sessions that have a connection (see "Known remaining
  deviations and limits").
- **From a client** (it publishes): QoS 1 is answered with PUBACK, QoS 2 with PUBREC, then PUBREL gets PUBCOMP. The
  message is delivered when the PUBLISH arrives. The packet identifier of a QoS 2 PUBLISH is remembered until its PUBREL,
  so a repeated PUBLISH with the same identifier is only acknowledged again, never delivered twice. A PUBREL for an
  unknown identifier gets PUBCOMP (MQTT 5: reason 0x92). MQTT 5: more QoS 2 PUBLISH packets waiting for their PUBREL
  than the Receive Maximum announced in the CONNACK (`run.mqtt.limits.receive_maximum`, default 100) are answered with
  DISCONNECT 0x93; MQTT 3.1.1 has no such limit. A PUBLISH with a topic beyond the topic limits is acknowledged and not
  delivered (MQTT 5: reason 0x97).
- **To a client** (it subscribed): QoS 1 and 2 messages are queued in the session (also while the client is offline, except the server's own publishes) in
  order, each with a packet identifier of its own (1 to 65535, counting upwards and skipping identifiers that are still
  in use, never 0). The broker sends them as long as fewer than the client's **Receive Maximum** (MQTT 5 CONNECT
  property; 65535 in 3.1.1) are unacknowledged; each acknowledgement lets the next one out. QoS 1 ends with PUBACK. QoS 2:
  PUBREC is answered with PUBREL, PUBCOMP ends it. An MQTT 5 PUBREC with a reason code of 0x80 or more ends the flow
  without PUBREL; a PUBREC for an unknown identifier gets a PUBREL (MQTT 5: 0x92); a PUBACK or PUBCOMP for an unknown
  identifier is ignored.
- **No retransmission on a live connection.** A PUBLISH or PUBREL is never repeated while the connection stays up. After
  a reconnect that resumes the session the broker first sends the PUBREL of every QoS 2 message whose PUBREC had arrived,
  then every PUBLISH that was not acknowledged with DUP = 1 (same packet identifier), then the rest of the queue with
  DUP = 0. What was in flight on the old connection counts against the Receive Maximum of the new one.
- A message larger than the Maximum Packet Size an MQTT 5 client announced in its CONNECT is not sent to that client
  (QoS 0, 1 and 2; a queued QoS 1/2 entry is dropped at the moment it would be sent).
- Acknowledgements (PUBACK, PUBREC, PUBREL, PUBCOMP) of one connection leave in the order of the packets that caused
  them (MQTT-4.6.0-2 to 4), also when one of them has to wait for the session database.
- **Queue limits per session** (QoS 1 and 2): `max_queued_messages` entries (default 1000, 1 to 65535: every entry owns a
  packet identifier) and `max_pending_send_bytes` bytes of topic and payload (default 4 MiB, like for QoS 0 but counted
  separately); messages in flight count. A message that does not fit is **dropped for that session**, counted and logged
  at debug level; the publisher still gets its acknowledgement and the subscriber is **not** disconnected. (QoS 0 keeps
  its rule: drop, and disconnect after 3 consecutive overflows. QoS 0 messages are not kept for an offline session.)

### Sessions

- The session is keyed by the **client identifier**. It holds the subscriptions (with granted QoS and No Local), the
  queue of QoS 1/2 messages with the state of their handshakes, and the QoS 2 identifiers received from the client that
  wait for their PUBREL. It lives in memory; with a session database it is also stored (see below).
- **Clean session** (3.1.1) / **Clean Start** (MQTT 5) = 1: an existing session of that client identifier is destroyed
  and the client starts with an empty one (`session_present` 0).
- **Session expiry.** MQTT 3.1.1: clean session 1 ends the session with the connection; clean session 0 keeps it for
  `run.mqtt.limits.session_expiry_max_s` (default 86400, i.e. 24 hours) after the connection ended. MQTT 5: the Session
  Expiry Interval of the CONNECT (absent = 0 = ends with the connection), capped at `session_expiry_max_s`; if the broker
  uses another value than the client asked for it announces it in the CONNACK (property 0x11). The DISCONNECT may
  carry a new interval (capped); raising it from 0 is a protocol error (DISCONNECT 0x82, the session ends with the
  connection). A session that ends with its connection is never resumed. A sweeper removes expired offline sessions once
  a second; an expired session is not resumed even before the sweeper ran. `session_expiry_max_s = 0` makes every session
  end with its connection.
- **`session_present`** is 1 when the client did not ask for a clean start and a persistent session of its identifier
  existed and has not expired. Subscriptions, queued messages and QoS 2 state are then continued.
- **Take-over.** A CONNECT with the client identifier of a session that has a live connection takes the session over:
  the old connection is closed (MQTT 5: DISCONNECT 0x8E, MQTT 3.1.1: just closed), and the new one resumes the session or,
  with a clean start, replaces it. **This is a behaviour change: two connections with the same client identifier no longer
  coexist** (before sessions existed they did). Compatibility: the C++ `DTClient` connects with an empty client identifier since this
  change (the client service then makes up a unique one; it connects with a clean session); older `DTClient` builds with
  the fixed identifier `digital-twin-client` still disconnect each other in a reconnect loop when two instances run
  against one broker. What the old connection sends afterwards is ignored; the same holds when
  the take-over arrives while the first connection is still in its CONNECT waiting for the session database.
- **Owner.** With authentication (`run.mqtt.authentication = "sysml"`) a session belongs to the user name that created
  it: a CONNECT with the identifier of a live or offline session of another user is refused (CONNACK 0x02 / 0x85, resume
  and clean start alike) and destroys nothing; only an expired session is replaced whoever asks. **Without authentication
  the user name is not checked**: any peer can take over, resume or destroy any session by its client identifier.
- **Quota.** `run.mqtt.limits.max_persistent_sessions` (default 1000) limits the sessions that outlive their connection
  (expiry above 0), online and offline together. A CONNECT that would create one more is refused (3.1.1 CONNACK 0x03,
  MQTT 5 0x97); resuming a session needs no extra room, and replacing a persistent one takes its place; replacing a session that ends with its connection by a persistent one counts against the quota. **Without authentication any peer can fill the quota**
  with sessions of its choice. Worst case with the defaults: 1000 sessions of up to 4 MiB of queued topics and payloads
  each, about 4 GiB of memory (and the same in the database; a payload is stored once per receiving session) plus the
  bookkeeping of up to 1000 messages per session. Lower `max_persistent_sessions`, `max_pending_send_bytes` or
  `max_queued_messages`, or keep authentication on, where that is not acceptable.
- **Limits of the identity.** A client identifier or user name longer than 1024 bytes is refused (client identifier:
  CONNACK 0x02 / 0x85, user name: 0x04 / 0x86) before the authentication backend is asked. A U+0000 in any string of a
  packet is malformed.

| Key (`run.mqtt.limits.`) | Default | Range | Meaning |
|---|---|---|---|
| `receive_maximum` | 100 | 1 to 65535 | announced to MQTT 5 clients: QoS 2 PUBLISH packets that may wait for their PUBREL (also bounds the acknowledgements held back for the database, see below) |
| `max_queued_messages` | 1000 | 1 to 65535 | QoS 1/2 messages queued or in flight per session |
| `session_expiry_max_s` | 86400 | 0 to 4294967295 | longest time an offline session is kept; 0 = no session outlives its connection |
| `max_persistent_sessions` | 1000 | 0 and up | sessions with expiry above 0, online or offline; 0 = none |

## Session persistence (PostgreSQL)

Without a database the sessions live in memory: they survive reconnects of the clients but not a restart of the server.
With one, the sessions that outlive their connection are mirrored in PostgreSQL and come back after a restart.

- **Configuration.** `run.mqtt.session_database_url` (environment `DT_MQTT_SESSION_DATABASE_URL`, which beats the
  file; there is no command line key). May contain a password, never logged. Persistence is on only when it is set; there is **no
  fallback to the history database** (a deployment that only sets the history URL keeps its sessions in memory). If it is
  empty the sessions are memory only and the server warns about that at start, naming the key and the variable. If
  `session_expiry_max_s` is 0 persistence is off (the server logs that at info level and does not open the store).
- **Schema.** `sql/0001_mqtt_session.sql` (embedded in the binary): `mqtt_session` (one row per session: id, unique client
  identifier, user name, protocol level, expiry, disconnect time), `mqtt_subscription`, `mqtt_outbound` (the queue: sequence,
  packet identifier unique per session, QoS, PUBREC received, origin, topic, payload, arrival time) and `mqtt_inbound_qos2`
  (identifiers waiting for their PUBREL); the last three reference the session with `ON DELETE CASCADE`. Table names are
  unqualified, so the tables are in the first schema of the `search_path` of the database role. The broker creates them
  at start **only if one of the four is missing** (`CREATE TABLE IF NOT EXISTS`, in one transaction, serialized by an
  advisory lock). A role that may only read and write needs `SELECT`, `INSERT`, `UPDATE` and `DELETE` on the four tables and
  no DDL right, provided someone applied `sql/0001_mqtt_session.sql` before; without the tables and without the right
  to create them the start fails with a message that says so. The tables are separate from the history schema of the C++
  core and may live in the same database.
- **Which sessions are stored.** Those with an expiry above 0 (every 3.1.1 session with clean session 0, every MQTT 5
  session with a Session Expiry Interval): while a client is connected the row has no disconnect time. Sessions that end
  with their connection are never written. A session whose expiry becomes 0 (new CONNECT, DISCONNECT) is deleted; one that
  becomes persistent is written with its full state. A stored payload is a copy per receiving session.
- **What is committed before which packet leaves.** Changes are queued and applied by one writer in transactions of up to
  500 changes (group commit); a barrier submitted after them runs once they are committed, in submission order. The packet
  that promises durable state waits for such a barrier: the **CONNACK** (the session row was created or updated), **SUBACK** and
  **UNSUBACK** (a stored subscription changed), the **PUBACK** or **PUBREC** to a publisher (the message was queued for at
  least one stored session; a PUBREC also waits for the stored identifier), the **PUBREL** the broker sends after the
  subscriber's PUBREC (the stored entry is marked), and the **PUBCOMP** answering a PUBREL (the identifier is removed).
  Acknowledgements that need nothing durable still leave in order behind waiting ones. **Write-behind** (queued, no wait):
  the removal of an acknowledged message, disconnect times, deletion of sessions, expiry, and the messages of
  `Hub::publish` (the server's own publishes, only queued for connected sessions): after a crash a server message of the
  last commit window may be missing. QoS 0 and sessions that end with their connection never touch the database.
- **Start.** The store connects and prepares within 10 seconds, otherwise the broker does not start (the error never contains
  the URL or the password). Sessions whose expiry has passed (disconnect time plus the expiry, capped at
  `session_expiry_max_s`) are deleted; sessions that were still marked connected (the previous run was stopped or
  killed) get the start time as disconnect time and expire from then on; all others are loaded as offline sessions with
  their subscriptions, queues and QoS 2 identifiers. Rows that cannot be understood are skipped with a warning. When the
  client comes back, every message that was not acknowledged is sent again with DUP = 1 and the PUBREL of every QoS 2
  message past its PUBREC first.
- **Availability at start.** A configured session database must be reachable at start,
  otherwise the server does not start (unlike the history store, which only logs and retries). It needs the four tables or,
  once, the right to create them; the role needs `USAGE` on the schema, and a missing or unusable schema is reported as a
  generic refusal with its SQLSTATE (e.g. 3F000, 42501). `run.mqtt.limits.session_expiry_max_s = 0` turns persistence off
  without opening the store (no connection, no tables).
- **Loaded sessions are not cut back.** Sessions loaded at start keep their queues and are counted as they are, even if
  `max_persistent_sessions` or `max_queued_messages` were lowered since; only the expiry is capped at the current
  `session_expiry_max_s`.
- **One broker per schema.** The store holds a session-level advisory lock for as long as it lives (a connection of its own,
  checked every 30 seconds); a second broker on the same schema does not start ("another broker uses this MQTT session
  database"). If the lock connection is lost the server logs a warning and runs on.
- **When the database fails later.** The writer retries a change that fails with a connection, privilege, missing table or
  similar error with backoff (200 ms doubling to 5 s) for as long as it lasts, and logs the stall (repeated every minute).
  Meanwhile everything that waits for a barrier waits: CONNACK of a persistent session (until the CONNECT timeout, then
  the connection is closed), SUBACK, and the acknowledgements of QoS 1/2 PUBLISH packets that were queued for stored
  sessions. A connection stops reading when more than `2 x receive_maximum + 16` of its acknowledgements wait (216 with
  the defaults), so a pipelining client cannot pile them up; shutdown and a take-over still end such a connection. QoS 0
  and clean sessions continue. A change the database refuses because of its data (SQLSTATE class 22, 23 or 54, e.g. a
  value it cannot store) is skipped and logged once the batch was split into single changes. A statement running longer than 30
  seconds is cancelled and retried.
- **Backlog cap.** The queue between the broker and the writer is bounded: from 100,000 queued changes, or 64 MiB of topics
  and payloads of queued messages, new `Enqueue` changes are **dropped** (a warning says so once per episode, an `info`
  line when the database caught up). No other change is ever dropped, and barriers neither. The consequence: a QoS 1/2
  message that was acknowledged to its publisher can be missing in the stored copy, so an offline or reconnecting session
  loses it if the broker restarts before the database is back, and a QoS 2 flow that was already past PUBREC can be left
  half open at the client (it waits for a PUBLISH or PUBREL that the restarted broker does not know). The message is
  still delivered from memory as long as the broker runs.
- **Shutdown.** `Broker::shutdown` waits up to 5 seconds for the sessions to end, then up to 5 seconds for the writer to
  store what is queued (a warning says if something is lost), then closes the connections to the database and releases
  the lock. Dropping a `Broker` without `shutdown` does not wait: queued changes are lost.

## Hub

`Broker::hub()` returns a cloneable `Hub`. `hub.subscribe(filter, capacity)` gives a bounded receiver of
`Message { origin, topic, payload, received_at_ms }` for every accepted PUBLISH (and for hub publishes). A full channel drops the message and
counts it (`HubSubscription::dropped()`, `Hub::dropped_total()`); the broker never waits. `hub.publish(topic, payload)`
delivers to all matching client sessions (at QoS 1, lowered to the QoS of the subscription; a session whose client is not connected does not get it) and all matching internal subscribers (there is no no-local for internal
code, so a subscriber on `#` also sees what the server itself published; such messages have `Origin::Server`, those of
MQTT clients `Origin::Client`). `received_at_ms` is the time the broker accepted the message (Unix milliseconds, UTC),
stamped before any queueing; it follows the system clock and is not monotonic.

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
- Without a loaded model nothing is recorded: the history starts with the load of a twin (the C++ core), and only
  paths of the model are stored.
- Not verified: the SQL of the REST API against a real PostgreSQL (none was available; the database tests in
  `tests/api_db.rs` compile but were not executed, and the syntax check with libpg_query needs libclang); the
  facade against a real Structura server; thread safety of the C++ libraries.
- The `ffi` build is verified on Windows 11 with MSVC against a Release tree (Conan prebuilt packages): `cargo
  build --features ffi`, the `ffi_backend`, `twin_integration` and `broker_integration` tests and a
  start of `dt-server.exe` without `PATH` additions work. Not verified: macOS (never built, only the planning logic
  is unit tested), Linux (no Linux build was run; rpath handling is unit tested only), the live test against
  Structura, and the MSVC `RelWithDebInfo`/`MinSizeRel` trees. The DLL copy next to the artifact is verified for the
  default `target` directory only; a separately configured cargo `build-dir` or `--target` is not verified.

Broker and payloads (first increment):

- No shared subscriptions: in 3.1.1 `$share/...` filters are stored like ordinary filters; MQTT 5 clients get SUBACK
  reason code 0x9E (and Shared Subscription Available = 0 in the CONNACK).
- UNSUBACK carries only the packet id in 3.1.1 (no per-filter codes), also for filters the session did not have. In
  MQTT 5 it has one reason code per filter: 0x00, or 0x11 (no subscription existed).
- No retained messages: the retain flag of a PUBLISH is accepted and ignored. No will: the will of a CONNECT is
  validated and skipped. No topic aliases (not offered; a PUBLISH with an empty topic closes the connection). The
  properties of an MQTT 5 PUBLISH (message expiry, content type, user properties, correlation data, response topic, ...)
  are not forwarded to subscribers.
- Exactly-once holds across reconnects of the clients (a repeated PUBLISH is recognised, a resumed QoS 2 flow continues
  with the PUBREL). It does **not** hold across a crash of the broker inside the commit window: a QoS 2 message whose
  PUBREC from the subscriber had arrived but was not yet committed is sent again after the restart, and a client that does
  not recognise the repeat delivers it twice; a QoS 1 message whose PUBACK had arrived but whose removal was not yet
  committed is sent again as well (allowed for QoS 1). Messages dropped from the write queue (see "Session persistence
  (PostgreSQL)") are lost for a restart.
- Without authentication any peer can create persistent sessions up to the quota and take over or destroy sessions of
  others (see "QoS and sessions" for the worst-case memory with the defaults).
- The database code paths (`src/broker/persist.rs`, `tests/session_db.rs`) are only tested when `DT_TEST_DATABASE_URL`
  is set, and have not been run against a real PostgreSQL yet (see "Tests").
- The server's own publishes (initial values, registration requests, `Origin::Server`) at QoS 1 or 2 are queued for a
  session only while it has a connection (one that is still waiting for its CONNACK included: the message is sent at its
  start). For an offline session they are skipped: not queued, not stored, not counted as a drop, no warning. Server messages queued or in
  flight when the client disconnects stay in the queue and are redelivered on resume like any other entry. There are no
  retained messages, so a client that was away does not see what the server published meanwhile. Publishes of MQTT
  clients are queued for offline sessions.
- A connected QoS 1/2 subscriber that never acknowledges stops receiving once its queue is full (the drops are counted
  and logged as a warning once per episode, then at debug level).
- Redelivery after a reconnect only; there is no timer that resends a PUBLISH on a live connection (MQTT-4.4).
- Floating point formatting: serde_json writes small and large exponents like `1e-7`, nlohmann::json (C++) writes
  `1e-07`. Both parse back to the same value; the bytes differ for such values. Not compared against a C++ build.
- The will (properties, topic, message) of CONNECT is validated for length and skipped; user name and password are
  decoded and used for the login (see "Authentication"). The CONNECT flags are validated: will QoS 3, will QoS or will
  retain without will flag, and (3.1.1 only; MQTT 5 allows it) password flag without user name flag are malformed, as is data behind the payload. A CONNECT with a password but no user name is refused with `0x86` when authentication is on, accepted with `none`. A PUBLISH with DUP = 1 at QoS 0 is malformed.

## MQTT 5

Enabled with `run.mqtt.protocol_version` = `"5"` or `"both"`. Scope: the same as for 3.1.1 (QoS 0 to 2, sessions, no retained messages, no will, login by user name and
password only, see "Authentication"), plus the parts of MQTT 5 the broker needs for that. Verified with
unit tests, hand written packets and `rumqttc::v5`; not verified against other client libraries (Paho and others).

- CONNECT properties are length-checked; the **Session Expiry Interval, Receive Maximum and Maximum Packet Size** are
  applied (see "QoS and sessions"), the others (topic alias maximum, user properties, ...) are skipped. Will properties
  are not parsed (the will itself is not used either).
- CONNACK announces: Receive Maximum (`run.mqtt.limits.receive_maximum`), Retain Available 0, Subscription Identifiers
  Available 0, Shared Subscription Available 0, Maximum Packet Size (the limit `max_packet_size`), the Session Expiry
  Interval when the broker uses another one than the client asked for and, for an empty client id, the Assigned Client
  Identifier. Maximum QoS is not sent (absent means 2). Topic aliases are not offered (no Topic
  Alias Maximum): a client must not use them, a PUBLISH with an empty topic closes the connection.
- Incoming PUBLISH, SUBSCRIBE and UNSUBSCRIBE properties are skipped (of a DISCONNECT only the Session Expiry
  Interval is used); outgoing PUBLISH, SUBACK, UNSUBACK and acknowledgements have empty properties. Retain As Published
  and Retain Handling of a subscription are accepted and have no effect (nothing is retained). **No Local is honoured**:
  a session does not receive its own publishes through a subscription that has it (also at QoS 1 and 2, and it is stored
  with the session). The requested QoS of a subscription is granted.
- Retain flag: although Retain Available is announced as 0, a PUBLISH with the retain flag is not an error; it is
  accepted and the flag is ignored (as for 3.1.1). The specification asks for DISCONNECT 0x9A here.
- SUBACK reason codes: 0x00, 0x01, 0x02 (granted QoS), 0x8F (topic filter invalid or beyond the topic limits), 0x97 (quota
  exceeded), 0x9E (shared subscriptions not supported). Subscription options with QoS 3, Retain Handling 3 or reserved
  bits 6-7 set are malformed packets.
- DISCONNECT sent by the broker (MQTT 5 clients only, after the queued messages): 0x8B server shutting down, 0x8D keep
  alive timeout, 0x81 malformed packet, 0x82 protocol error (second CONNECT), 0x90 topic name invalid, 0x95 packet too
  large, 0x8E session taken over (also sent when the session is destroyed under the connection), 0x93 receive maximum
  exceeded. A client that is dropped because it is too slow, or whose connection
  fails, gets nothing. DISCONNECT of a client with a reason code and properties is accepted. The AUTH packet (type 15)
  is a protocol error, DISCONNECT 0x82 (no enhanced authentication).
- PUBLISH properties of a client (user properties, content type, message expiry, ...) are skipped and **not forwarded** to
  subscribers, and the RETAIN flag (=1) of a PUBLISH is accepted and ignored: nothing is retained. Acknowledgements
  carry a reason code only where one is needed: 0x92 (packet identifier not found) and 0x97 (quota exceeded, a topic
  beyond the limits).
- CONNACK refusals: 0x84 (version not enabled), 0x85 (client identifier too long, or the identifier belongs to another
  user), 0x86 (bad user name or password), 0x88 (server unavailable: the authentication backend cannot decide, or the session store refuses because it is shutting down and drops the barrier a CONNACK waits for; with the database merely down the writer retries, the client gets no CONNACK and is closed at the CONNECT timeout), 0x97 (too many persistent sessions).
- `run.mqtt.limits.max_packet_size` must be at least 64 (it is announced as Maximum Packet Size, and 0 is invalid).
- Keep alive, connect timeout, limits and the in-process hub work as for 3.1.1. Server Keep Alive is not used.
