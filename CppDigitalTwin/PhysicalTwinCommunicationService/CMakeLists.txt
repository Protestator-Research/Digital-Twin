cmake_minimum_required(VERSION 3.15)

project(CppPhysicalTwinCommunication CXX)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(mqtt_cpp REQUIRED CONFIG)

add_library(CppPhysicalTwinCommunication
        src/PTCommunicationService.cpp
        src/PTCommunicationService.h
        src/Services/AuthenticationService.cpp
        src/Services/AuthenticationService.h
        src/Services/MqttBrokerService.cpp
        src/Services/MqttBrokerService.h
        src/Services/MqttClientService.cpp
        src/Services/MqttClientService.h
        src/entities/DigitalTwinEntity.cpp
        src/entities/DigitalTwinEntity.h
        src/entities/JsonEntities.hpp
        src/cpp_physical_twin_communication_global.h)

target_link_libraries(CppPhysicalTwinCommunication
        mqtt_cpp::mqtt_cpp
        nlohmann_json::nlohmann_json)

install(TARGETS CppPhysicalTwinCommunication DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

if(MSVC)
    target_compile_options(CppPhysicalTwinCommunication PRIVATE /W4 /WX-)
else()
    target_compile_options(CppPhysicalTwinCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()