cmake_minimum_required(VERSION 3.15)

project(CPSBaseLib CXX C)

find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)


add_library(CPSBaseLib
        src/BaseFuctions/StringExtention.hpp
        src/BaseFuctions/StringExtention.cpp
        src/SysMLv2Standard/entities/Identity.cpp
        src/SysMLv2Standard/entities/Identity.h
        src/SysMLv2Standard/entities/Record.cpp
        src/SysMLv2Standard/entities/Record.h
        src/SysMLv2Standard/entities/IEntity.h
        src/SysMLv2Standard/entities/Project.cpp
        src/SysMLv2Standard/entities/Project.h
        src/BaseFuctions/ListExtentions.cc
        src/BaseFuctions/ListExtentions.h
)

set_target_properties(CPSBaseLib PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(CPSBaseLib
                        boost::boost
                        nlohmann_json::nlohmann_json)

install(TARGETS CPSBaseLib DESTINATION "src"
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

option(RUN_TESTS "Build the tests" ON)

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()