cmake_minimum_required(VERSION 3.15)

project(DigitalTwinLib CXX)

find_package(curl REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(libuuid REQUIRED CONFIG)

add_library(DigitalTwinLib
                src/BaseDigitalTwinInstanceManager.cpp
                src/BaseDigitalTwinInstanceManager.h)

target_link_libraries(DigitalTwinLib)

install(TARGETS ${PROJECT_NAME} DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)