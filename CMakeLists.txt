cmake_minimum_required(VERSION 3.15)

project(DigitalTwin CXX C)

find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)
find_package(date REQUIRED)
find_package(sysmllib REQUIRED)
find_package(GTest REQUIRED)
find_package(CURL REQUIRED)
find_package(antlr4-runtime REQUIRED)
find_package(mqtt_cpp REQUIRED)
find_package(Qt6 REQUIRED)
find_package(md4c REQUIRED)
find_package(Threads REQUIRED)


include(CMakeDependentOption)

option(RUN_TESTS "Build the tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out) 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out) 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)


if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "~/Qt/6.7.3/macos/")
endif(APPLE)

add_subdirectory(./CpsBaseLib)
add_subdirectory(./BackendCommunicationService)
add_subdirectory(./DigitalTwinLib)
add_subdirectory(./DTServer)
add_subdirectory(./PhysicalTwinCommunicationService)
add_subdirectory(./DTClient)
