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
        src/APIImplementations/Standards/HttpReturnCodes.h
        src/APIImplementations/StatusCode.h
        src/Exeptions/HttpException.cpp
        src/Exeptions/HttpException.h
        src/APIImplementations/Interfaces/IAPIInterface.cpp
        src/APIImplementations/Interfaces/IAPIInterface.h
        src/cpp_backend_communication_global.h)

target_link_libraries(CppBackendCommunication
        CURL::libcurl
        nlohmann_json::nlohmann_json
        boost::boost
        CPSBaseLib)

target_include_directories(CppBackendCommunication PUBLIC
                                ../CpsBaseLib/src)


file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

if(MSVC)
    target_compile_options(CppBackendCommunication PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(CppBackendCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror -fcoroutines)
else ()
    target_compile_options(CppBackendCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(TARGETS CppBackendCommunication DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
