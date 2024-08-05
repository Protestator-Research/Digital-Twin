cmake_minimum_required(VERSION 3.15)

project(CppBackendCommunication CXX)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED CONFIG)

add_library(CppBackendCommunication
        src/BECommunicationService.h
        src/BECommunicationService.cpp
        src/APIImplementations/SysMLAPIImplementation.cpp
        src/APIImplementations/SysMLAPIImplementation.h
        src/Exeptions/ConnectionError.cpp
        src/Exeptions/ConnectionError.h
        src/Exeptions/NotEnoughMemoryError.cpp
        src/Exeptions/NotEnoughMemoryError.h
        ../CpsBaseLib/src/BaseFuctions/StringExtention.hpp
        src/APIImplementations/Standards/HttpReturnCodes.h
        src/APIImplementations/StatusCode.h
        src/Exeptions/HttpException.cpp
        src/Exeptions/HttpException.h)

target_link_libraries(CppBackendCommunication
        CURL::libcurl
        nlohmann_json::nlohmann_json
        boost::boost
        CPSBaseLib)

target_include_directories(CppBackendCommunication PUBLIC
                                ../CpsBaseLib/src)

if(MSVC)
    target_compile_options(CppBackendCommunication PRIVATE /W4 /WX)
else()
    target_compile_options(CppBackendCommunication PRIVATE -Wall -Werror)
endif()

install(TARGETS CppBackendCommunication DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
