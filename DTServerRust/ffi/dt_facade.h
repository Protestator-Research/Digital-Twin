// Narrow C++ facade between the Rust server and the existing Digital Twin libraries (cxx bridge, see
// src/twin/ffi.rs). It lives in DTServerRust/ffi and uses DigitalTwinLib / BackendCommunicationService
// unmodified.
//
// No C++ exception crosses this boundary: every function catches everything and reports an error code with a
// message (the code is one of the values below, the message never contains credentials).
#pragma once

#include <cstdint>

#include "rust/cxx.h"

namespace dtrs {
    // Defined by the cxx bridge in src/twin/ffi.rs (the generated header is included by dt_facade.cpp).
    struct CallResult;
    struct TwinLoad;
    struct TwinElement;

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

    // Downloads the twin and builds its model, as DigitalTwinManager::downloadDigitalTwin does, but without
    // generateMQTTInterface (the Rust side publishes the interface). The model is kept for register_function.
    TwinLoad facade_load_twin(rust::Str project_id, rust::Str twin_id) noexcept;

    // Test/diagnostic support only (no server, no session): the getJson() of one value entity per type.
    rust::Vec<TwinElement> facade_sample_initial_values() noexcept;

    // Hands a registerFunction payload to the model of the twin.
    CallResult facade_register_function(rust::Str twin_id, rust::Str request_json) noexcept;

    // Frees the model of the twin. Unknown twins are not an error.
    CallResult facade_release_twin(rust::Str twin_id) noexcept;

    // Gibt ein Variablen-Update (topic path relativ zum Twin-Namen) an das Modell des Twins.
    CallResult facade_update_variable(rust::Str twin_id, rust::Str path, rust::Str payload_json) noexcept;
}
