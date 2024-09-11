cmake_minimum_required(VERSION 3.15)

project(CppDigitalTwinServer CXX)

find_package(mqtt_cpp REQUIRED CONFIG)

add_executable(CppDigitalTwinServer
        src/main.cpp
        src/DigitalTwinServerInstanceManager.cpp
        src/DigitalTwinServerInstanceManager.h)

target_include_directories(CppDigitalTwinServer PUBLIC
        ../BackendCommunicationService/src/
        ../DigitalTwinLib/src/
        ../PhysicalTwinCommunicationService/src/)

target_link_libraries(CppDigitalTwinServer
        CppBackendCommunication
        CppDigitalTwinLib
        CppPhysicalTwinCommunication
        CPSBaseLib
        mqtt_cpp::mqtt_cpp)

if(MSVC)
    target_compile_options(CppDigitalTwinServer PRIVATE /W4 /WX-)
else()
    target_compile_options(CppDigitalTwinServer PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()