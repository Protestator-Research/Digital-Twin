cmake_minimum_required(VERSION 3.15)

project(CppDigitalTwinLib CXX)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED CONFIG)

add_library(CppDigitalTwinLib
        src/BaseDTClasses/BaseDigitalTwinInstanceManager.cpp
        src/BaseDTClasses/BaseDigitalTwinInstanceManager.h
        src/DigitalTwinManager.cpp
        src/DigitalTwinManager.h)

target_include_directories(CppDigitalTwinLib PUBLIC
        ../PhysicalTwinCommunicationService/src/)

target_link_libraries(CppDigitalTwinLib
        CppPhysicalTwinCommunication)

if(MSVC)
    target_compile_options(CppDigitalTwinLib PRIVATE /W4 /WX)
else()
    target_compile_options(CppDigitalTwinLib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(TARGETS CppDigitalTwinLib DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
