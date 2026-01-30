cmake_minimum_required(VERSION 3.15)

add_executable(DigitalTwinServer
        src/main.cpp
        src/DigitalTwinServerInstanceManager.cpp
        src/DigitalTwinServerInstanceManager.h
        src/MQTTBrokerLegacy/external/picosha2.h
        src/MQTTBrokerLegacy/broker.hpp
        src/MQTTBrokerLegacy/constant.hpp
        src/MQTTBrokerLegacy/endpoint_variant.hpp
        src/MQTTBrokerLegacy/fixed_core_map.hpp
        src/MQTTBrokerLegacy/inflight_message.hpp
        src/MQTTBrokerLegacy/mutex.hpp
        src/MQTTBrokerLegacy/offline_message.hpp
        src/MQTTBrokerLegacy/retain_type.hpp
        src/MQTTBrokerLegacy/retained_messages.hpp
        src/MQTTBrokerLegacy/retained_topic_map.hpp
        src/MQTTBrokerLegacy/security.hpp
        src/MQTTBrokerLegacy/session_state.hpp
        src/MQTTBrokerLegacy/session_state_fwd.hpp
        src/MQTTBrokerLegacy/shared_target.hpp
        src/MQTTBrokerLegacy/shared_target_impl.hpp
        src/MQTTBrokerLegacy/sub_con_map.hpp
        src/MQTTBrokerLegacy/subscription.hpp
        src/MQTTBrokerLegacy/subscription_map.hpp
        src/MQTTBrokerLegacy/tags.hpp
        src/MQTTBrokerLegacy/topic_filter.hpp
        src/MQTTBrokerLegacy/uuid.hpp
        src/MQTTBrokerService.h
        src/MQTTBrokerService.cpp
        src/AuthenticationService.h
        src/AuthenticationService.cpp)

target_include_directories(DigitalTwinServer PUBLIC
        ../BackendCommunicationService/src/
        ../DigitalTwinLib/src/)

target_link_libraries(DigitalTwinServer
        BackendCommunication
        DigitalTwinLib
        CPSBaseLib
        sysmllib::sysmllib
        mqtt_cpp::mqtt_cpp)


file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

if(MSVC)
    target_compile_options(DigitalTwinServer PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(DigitalTwinServer PRIVATE -Wall -Wextra -Wpedantic -Werror -fcoroutines)
else ()
    target_compile_options(DigitalTwinServer PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()