cmake_minimum_required(VERSION 3.15)

add_library(PhysicalTwinCommunication
        src/PTCommunicationService.cpp
        src/PTCommunicationService.h
        src/Services/AuthenticationService.cpp
        src/Services/AuthenticationService.h
        src/Services/MqttClientService.cpp
        src/Services/MqttClientService.h
        src/Services/MqttTlsOptions.cpp
        src/Services/MqttTlsOptions.h
        src/MQTT/entities/DigitalTwinEntity.cpp
        src/MQTT/entities/DigitalTwinEntity.h
        src/MQTT/entities/JsonEntities.h
        src/MQTT/entities/JsonEntities.cpp
        src/cpp_physical_twin_communication_global.h
        src/MQTT/Topics.h
        src/MQTT/entities/values/ValueEntity.cpp
        src/MQTT/entities/values/ValueEntity.h
        src/Services/IMqttClientService.h
        src/MQTT/entities/values/IntegerValueEntity.cpp
        src/MQTT/entities/values/IntegerValueEntity.h
        src/MQTT/entities/values/BooleanValueEntity.cpp
        src/MQTT/entities/values/BooleanValueEntity.h
        src/MQTT/entities/values/StringValueEntity.cpp
        src/MQTT/entities/values/StringValueEntity.h
        src/MQTT/entities/values/ComplexValueEntity.cpp
        src/MQTT/entities/values/ComplexValueEntity.h
        src/MQTT/entities/values/RealValueEntity.cpp
        src/MQTT/entities/values/RealValueEntity.h
        src/MQTT/entities/values/RationalValueEntity.cpp
        src/MQTT/entities/values/RationalValueEntity.h
        src/MQTT/entities/values/NaturalValueEntity.cpp
        src/MQTT/entities/values/NaturalValueEntity.h
        src/MQTT/entities/values/PositiveValueEntity.cpp
        src/MQTT/entities/values/PositiveValueEntity.h
        src/MQTT/entities/FunctionValueEntity.h
        src/MQTT/entities/FunctionValueEntity.cpp
        src/MQTT/entities/JsonEntities.cpp
        src/MQTT/entities/FunctionRegistrationRequest.hpp)

# Used directly (checkMqttCaFile loads the CA file like Paho does); the same OpenSSL as Paho.
find_package(OpenSSL REQUIRED)

# Paho is PUBLIC: MqttClientService.h exposes mqtt::async_client.
target_link_libraries(PhysicalTwinCommunication
        PUBLIC
        paho-mqtt-cpp::paho-mqtt-cpp
        nlohmann_json::nlohmann_json
        Threads::Threads
        boost::boost
        CPSBaseLib)

target_include_directories(PhysicalTwinCommunication PUBLIC
        ../CpsBaseLib/src)

# Only the implementation uses it (checkMqttCaFile); no public header includes OpenSSL.
target_link_libraries(PhysicalTwinCommunication PRIVATE OpenSSL::SSL OpenSSL::Crypto)

install(TARGETS PhysicalTwinCommunication DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

if(MSVC)
    target_compile_options(PhysicalTwinCommunication PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(PhysicalTwinCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror)
else ()
    target_compile_options(PhysicalTwinCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()