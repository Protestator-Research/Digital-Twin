cmake_minimum_required(VERSION 3.15)

project(DigitalTwin CXX)

include(CMakeDependentOption)

option(RUN_TESTS "Build the tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "~/Qt/6.7.1/macos/")
endif(APPLE)

add_subdirectory(./CppDigitalTwin/CpsBaseLib)
add_subdirectory(./CppDigitalTwin/BackendCommunicationService)
add_subdirectory(./CppDigitalTwin/DigitalTwinLib)
add_subdirectory(./CppDigitalTwin/DTServer)
add_subdirectory(./CppDigitalTwin/PhysicalTwinCommunicationService)
add_subdirectory(./CppDigitalTwin/DTClient)