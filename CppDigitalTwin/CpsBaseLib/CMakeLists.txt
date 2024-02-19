cmake_minimum_required(VERSION 3.15)

project(CPSBaseLib CXX)

add_library(CPSBaseLib
        src/BaseFuctions/StringExtention.hpp
        src/BaseFuctions/StringExtention.cpp)

set_target_properties(CPSBaseLib PROPERTIES LINKER_LANGUAGE CXX)

#target_link_libraries(CPSBaseLib)

install(TARGETS CPSBaseLib DESTINATION "src"
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

option(RUN_TESTS "Build the tests" ON)
if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()