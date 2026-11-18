cmake_minimum_required(VERSION 3.15)

project(DigitalTwin CXX C)

option(DT_BUILD_CLIENT "Build the Qt client" ON)
option(DT_BUILD_TESTS "Build the C++ tests" ON)
option(RUN_TESTS "Build the tests" ON)

if(DT_BUILD_CLIENT)
    if(APPLE)
        list(APPEND CMAKE_PREFIX_PATH "~/Qt/6.11.1/macos/")
    endif(APPLE)
    if(WIN32)
       list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64")
    endif(WIN32)
endif()

# The library CMakeLists add their tests/ subdirectory on RUN_TESTS, so DT_BUILD_TESTS=OFF switches it off here.
if(NOT DT_BUILD_TESTS)
    set(RUN_TESTS OFF)
endif()

find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)
find_package(date REQUIRED)
find_package(sysmllib REQUIRED)
if(DT_BUILD_TESTS)
    find_package(GTest REQUIRED)
endif()
find_package(CURL REQUIRED)
find_package(antlr4-runtime REQUIRED)
find_package(mqtt_cpp REQUIRED)
find_package(PahoMqttCpp REQUIRED)
if(DT_BUILD_CLIENT)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Charts)
    find_package(md4c REQUIRED)
endif()
find_package(Threads REQUIRED)

include(CMakeDependentOption)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out) 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out) 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)


# Top level, so that `ctest --test-dir build/Debug` finds the tests of all subdirectories.
if(RUN_TESTS)
    enable_testing()
endif()

add_subdirectory(./CpsBaseLib)
add_subdirectory(./BackendCommunicationService)
add_subdirectory(./DigitalTwinLib)
# add_subdirectory(./DTServer) C++ Server Version is not maintained anymore, since this would require Drogon on windows, which is not possible.
add_subdirectory(./PhysicalTwinCommunicationService)
if(DT_BUILD_CLIENT)
    add_subdirectory(./DTClient)
endif()
