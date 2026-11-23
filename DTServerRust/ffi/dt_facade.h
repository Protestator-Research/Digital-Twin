// Narrow C++ facade between the Rust server and the existing Digital Twin libraries (cxx bridge, see
// src/twin/ffi.rs). It lives in DTServerRust/ffi and uses DigitalTwinLib / BackendCommunicationService
// unmodified.
//
// No C++ exception crosses this boundary: every function catches everything and reports an error code with a
// message (the code is one of the values below, the message never contains credentials).
//
// History: if the store was configured (facade_store_configure), the facade registers every loaded twin in the
// DigitalTwinStore library, which writes the change log and the checkpoints to PostgreSQL (see
// Markdownfiles/Plan_Datenbank_CppKern.md). Without a configured store everything works as before.
#pragma once

#include <cstdint>

#include "rust/cxx.h"

namespace dtrs {
    // Defined by the cxx bridge in src/twin/ffi.rs (the generated header is included by dt_facade.cpp).
    struct CallResult;
    struct TwinLoad;
    struct TwinElement;
    struct StoreConfig;
    struct StoreStats;

    constexpr int32_t CODE_OK = 0;
    constexpr int32_t CODE_INVALID_ARGUMENT = 1;
    constexpr int32_t CODE_NOT_LOGGED_IN = 2;
    constexpr int32_t CODE_NOT_FOUND = 3;
    constexpr int32_t CODE_BACKEND_ERROR = 4;
    constexpr int32_t CODE_UNKNOWN_ERROR = 5;
    // The backend refused the session (HTTP 401): the caller logs in again and retries once.
    constexpr int32_t CODE_UNAUTHORIZED = 6;

    // Logs in to the Structura server and replaces the session used by load_twin.
    CallResult facade_login(rust::Str url, uint16_t port, rust::Str username, rust::Str password) noexcept;

    // Uses an application key of the SysMLv2 server as the session, instead of a login: the key is the bearer token
    // of every later request ("Authorization: Bearer <key>"). No request is made, so nothing is checked here; a wrong,
    // revoked or unauthorized key is reported by the first load_twin as CODE_UNAUTHORIZED (HTTP 401), and there is no
    // point in trying again. Replaces the current session like facade_login. The key is never logged and is not part
    // of any message; an empty key or one with characters that cannot be sent in a header is CODE_INVALID_ARGUMENT.
    CallResult facade_login_application_key(rust::Str url, uint16_t port, rust::Str application_key) noexcept;

    // Downloads the twin and builds its model, as DigitalTwinManager::downloadDigitalTwin does, but without
    // generateMQTTInterface (the Rust side publishes the interface). The model is kept for register_function.
    // With a store: the twin and its commit are registered, for a commit that was loaded before the last stored
    // state is set in the model (so the initial values returned are the restored ones), and a checkpoint of the
    // load is written. If the registration fails the twin is loaded without history and registered again later.
    TwinLoad facade_load_twin(rust::Str project_id, rust::Str twin_id) noexcept;

    // Test/diagnostic support only (no server, no session): the getJson() of one value entity per type.
    rust::Vec<TwinElement> facade_sample_initial_values() noexcept;

    // Hands a registerFunction payload to the model of the twin.
    CallResult facade_register_function(rust::Str twin_id, rust::Str request_json) noexcept;

    // Frees the model of the twin (a final checkpoint is written first if it changed). Unknown twins are not an
    // error.
    CallResult facade_release_twin(rust::Str twin_id) noexcept;

    // Gibt ein Variablen-Update (topic path relativ zum Twin-Namen) an das Modell des Twins.
    // received_at_ms: Unix time in milliseconds (UTC) at which the broker accepted the message; it is the
    // received_at of the history rows. 0 or less means "now".
    CallResult facade_update_variable(rust::Str twin_id, rust::Str path, rust::Str payload_json,
                                      int64_t received_at_ms) noexcept;

    // Creates the history store, once (a second call is an error), and the housekeeping thread. Never fails
    // because the database is down: the store connects in the background. 0 in queue_capacity, batch_size or
    // flush_interval_ms selects the default of the store. The URL is never logged.
    CallResult facade_store_configure(const StoreConfig& config) noexcept;

    // Counters of the store; enabled is false and everything else zero if it was never configured.
    StoreStats facade_store_stats() noexcept;

    // Stops the housekeeping thread, writes a final checkpoint of every twin that changed since its last one, and
    // shuts the store down within timeout_ms (the queue is written out as far as the time allows). Not an error if
    // there is no store.
    CallResult facade_store_shutdown(uint32_t timeout_ms) noexcept;
}
