cmake_minimum_required(VERSION 3.15)

project(CppBackendCommunication CXX)

find_package(curl REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(libuuid REQUIRED CONFIG)

add_library(CppBackendCommunication
        src/CommunicationService.h
        src/CommunicationService.cpp
        src/Entities/Commit.hpp
        src/Entities/Project.hpp
        src/Entities/Element.hpp
        src/Entities/DigitalTwin.hpp
        src/Entities/Branch.hpp
        src/APIImplementations/SysMLAPIImplementation.cpp
        src/APIImplementations/SysMLAPIImplementation.h
        src/Entities/User.hpp
        src/Entities/IEntitie.h)

target_link_libraries(CppBackendCommunication
        CURL::libcurl
        nlohmann_json::nlohmann_json
        libuuid::libuuid)

install(TARGETS ${PROJECT_NAME} DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
