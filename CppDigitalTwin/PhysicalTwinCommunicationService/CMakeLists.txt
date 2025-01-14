cmake_minimum_required(VERSION 3.15)

project(CppPhysicalTwinCommunication CXX)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(mqtt_cpp REQUIRED CONFIG)
find_package(Threads REQUIRED)

add_library(CppPhysicalTwinCommunication
        src/PTCommunicationService.cpp
        src/PTCommunicationService.h
        src/Services/AuthenticationService.cpp
        src/Services/AuthenticationService.h
        src/Services/MqttBrokerService.cpp
        src/Services/MqttBrokerService.h
        src/Services/MqttClientService.cpp
        src/Services/MqttClientService.h
        src/MQTT/entities/DigitalTwinEntity.cpp
        src/MQTT/entities/DigitalTwinEntity.h
        src/MQTT/entities/JsonEntities.hpp
        src/cpp_physical_twin_communication_global.h
        src/broker_includes/external/picosha2.h
        src/broker_includes/broker.hpp
        src/broker_includes/constant.hpp
        src/broker_includes/endpoint_variant.hpp
        src/broker_includes/fixed_core_map.hpp
        src/broker_includes/inflight_message.hpp
        src/broker_includes/mutex.hpp
        src/broker_includes/offline_message.hpp
        src/broker_includes/retain_type.hpp
        src/broker_includes/retained_messages.hpp
        src/broker_includes/retained_topic_map.hpp
        src/broker_includes/security.hpp
        src/broker_includes/session_state.hpp
        src/broker_includes/session_state_fwd.hpp
        src/broker_includes/shared_target.hpp
        src/broker_includes/shared_target_impl.hpp
        src/broker_includes/sub_con_map.hpp
        src/broker_includes/subscription.hpp
        src/broker_includes/subscription_map.hpp
        src/broker_includes/tags.hpp
        src/broker_includes/topic_filter.hpp
        src/broker_includes/uuid.hpp
        src/MQTT/Topics.h)

target_link_libraries(CppPhysicalTwinCommunication
        mqtt_cpp::mqtt_cpp
        nlohmann_json::nlohmann_json
        Threads::Threads)

install(TARGETS CppPhysicalTwinCommunication DESTINATION "."
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
    target_compile_options(CppPhysicalTwinCommunication PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(CppPhysicalTwinCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror -fcoroutines)
else ()
    target_compile_options(CppPhysicalTwinCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()