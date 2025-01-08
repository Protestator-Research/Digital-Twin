cmake_minimum_required(VERSION 3.15)

project(CppDigitalTwinServer CXX)

find_package(mqtt_cpp REQUIRED CONFIG)

add_executable(CppDigitalTwinServer
        src/main.cpp
        src/DigitalTwinServerInstanceManager.cpp
        src/DigitalTwinServerInstanceManager.h)

target_include_directories(CppDigitalTwinServer PUBLIC
        ../BackendCommunicationService/src/
        ../DigitalTwinLib/src/
        ../PhysicalTwinCommunicationService/src/)

target_link_libraries(CppDigitalTwinServer
        CppBackendCommunication
        CppDigitalTwinLib
        CppPhysicalTwinCommunication
        CPSBaseLib
        mqtt_cpp::mqtt_cpp)


file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

if(MSVC)
    target_compile_options(CppDigitalTwinServer PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(CppDigitalTwinServer PRIVATE -Wall -Wextra -Wpedantic -Werror -fcoroutines)
else ()
    target_compile_options(CppDigitalTwinServer PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()