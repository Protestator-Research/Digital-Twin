cmake_minimum_required(VERSION 3.15)

project(CppDigitalTwinLib CXX)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED CONFIG)

add_library(CppDigitalTwinLib
        src/DigitalTwinManager.cpp
        src/DigitalTwinManager.h
        src/Model/DigitalTwinModel.cpp
        src/Model/DigitalTwinModel.h)

target_include_directories(CppDigitalTwinLib PUBLIC
        ../PhysicalTwinCommunicationService/src/
        ../BackendCommunicationService/src/
        ../CpsBaseLib/src)

target_link_libraries(CppDigitalTwinLib
        CppPhysicalTwinCommunication
        CppBackendCommunication
        CPSBaseLib)

if(MSVC)
    target_compile_options(CppDigitalTwinLib PRIVATE /W4 /WX)
else()
    target_compile_options(CppDigitalTwinLib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(TARGETS CppDigitalTwinLib DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
