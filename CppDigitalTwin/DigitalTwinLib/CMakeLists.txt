cmake_minimum_required(VERSION 3.15)

project(CppDigitalTwinLib CXX)

find_package(curl REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(libuuid REQUIRED CONFIG)

add_library(CppDigitalTwinLib
        src/BaseDTClasses/BaseDigitalTwinInstanceManager.cpp
        src/BaseDTClasses/BaseDigitalTwinInstanceManager.h
        src/DigitalTwinManager.cpp
        src/DigitalTwinManager.h)

#target_link_libraries(DigitalTwinLib)

install(TARGETS CppDigitalTwinLib DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)