cmake_minimum_required(VERSION 3.15)

project(CPSBaseLib CXX C)

add_library(CPSBaseLib
        src/BaseFuctions/StringExtention.hpp
        src/BaseFuctions/StringExtention.cpp
        src/BaseFuctions/ListExtentions.cc
        src/BaseFuctions/ListExtentions.h
        src/cps_base_global.h)

target_link_libraries(CPSBaseLib
                        boost::boost
                        nlohmann_json::nlohmann_json
                        date::date
                        sysmllib::sysmllib)

install(TARGETS CPSBaseLib DESTINATION "src"
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

if(MSVC)
    target_compile_options(CPSBaseLib PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(CPSBaseLib PRIVATE -Wall -Wextra -Wpedantic -Werror -fcoroutines)
else ()
    target_compile_options(CPSBaseLib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()