cmake_minimum_required(VERSION 3.15)

add_library(PhysicalTwinCommunication
        src/PTCommunicationService.cpp
        src/PTCommunicationService.h
        src/Services/AuthenticationService.cpp
        src/Services/AuthenticationService.h
        src/Services/MqttClientService.cpp
        src/Services/MqttClientService.h
        src/MQTT/entities/DigitalTwinEntity.cpp
        src/MQTT/entities/DigitalTwinEntity.h
        src/MQTT/entities/JsonEntities.h
        src/MQTT/entities/JsonEntities.cpp
        src/cpp_physical_twin_communication_global.h
        src/MQTT/Topics.h
        src/MQTT/entities/ValueEntity.cpp
        src/MQTT/entities/ValueEntity.h
        src/Services/IMqttClientService.h
        src/MQTT/entities/IntegerValueEntity.cpp
        src/MQTT/entities/IntegerValueEntity.h
        src/MQTT/entities/BooleanValueEntity.cpp
        src/MQTT/entities/BooleanValueEntity.h
        src/MQTT/entities/StringValueEntity.cpp
        src/MQTT/entities/StringValueEntity.h
        src/MQTT/entities/ComplexValueEntity.cpp
        src/MQTT/entities/ComplexValueEntity.h
        src/MQTT/entities/RealValueEntity.cpp
        src/MQTT/entities/RealValueEntity.h
        src/MQTT/entities/RationalValueEntity.cpp
        src/MQTT/entities/RationalValueEntity.h
        src/MQTT/entities/NaturalValueEntity.cpp
        src/MQTT/entities/NaturalValueEntity.h
        src/MQTT/entities/PositiveValueEnitity.cpp
        src/MQTT/entities/PositiveValueEnitity.h)

target_link_libraries(PhysicalTwinCommunication
        mqtt_cpp::mqtt_cpp
        nlohmann_json::nlohmann_json
        Threads::Threads
        boost::boost
        CPSBaseLib)

target_include_directories(PhysicalTwinCommunication PUBLIC
        ../CpsBaseLib/src)

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
    target_compile_options(PhysicalTwinCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror -fcoroutines)
else ()
    target_compile_options(PhysicalTwinCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()