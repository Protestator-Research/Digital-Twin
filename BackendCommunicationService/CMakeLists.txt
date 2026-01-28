cmake_minimum_required(VERSION 3.15)

project(BackendCommunication CXX)


add_library(BackendCommunication
        src/BECommunicationService.h
        src/BECommunicationService.cpp
        src/cpp_backend_communication_global.h
        src/entities/DigitalTwin.cpp
        src/entities/DigitalTwin.h)

target_link_libraries(BackendCommunication
        CURL::libcurl
        nlohmann_json::nlohmann_json
        boost::boost
        sysmllib::sysmllib
        CPSBaseLib)

target_include_directories(BackendCommunication PUBLIC
                                ../CpsBaseLib/src)


file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

if(MSVC)
    target_compile_options(BackendCommunication PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(BackendCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror -fcoroutines)
else ()
    target_compile_options(BackendCommunication PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(TARGETS BackendCommunication DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
