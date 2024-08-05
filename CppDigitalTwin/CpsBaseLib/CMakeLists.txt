cmake_minimum_required(VERSION 3.15)

project(CPSBaseLib CXX C)

find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)
find_package(date REQUIRED)


add_library(CPSBaseLib
        src/BaseFuctions/StringExtention.hpp
        src/BaseFuctions/StringExtention.cpp
        src/SysMLv2Standard/entities/DataIdentity.cpp
        src/SysMLv2Standard/entities/DataIdentity.h
        src/SysMLv2Standard/entities/Record.cpp
        src/SysMLv2Standard/entities/Record.h
        src/SysMLv2Standard/entities/IEntity.h
        src/SysMLv2Standard/entities/Project.cpp
        src/SysMLv2Standard/entities/Project.h
        src/BaseFuctions/ListExtentions.cc
        src/BaseFuctions/ListExtentions.h
        src/SysMLv2Standard/entities/JSONEntities.h
        src/SysMLv2Standard/entities/DigitalTwin.cpp
        src/SysMLv2Standard/entities/DigitalTwin.h
        src/SysMLv2Standard/entities/Commit.cpp
        src/SysMLv2Standard/entities/Commit.h
        src/SysMLv2Standard/entities/CommitReference.cpp
        src/SysMLv2Standard/entities/CommitReference.h
        src/SysMLv2Standard/entities/DataVersion.cpp
        src/SysMLv2Standard/entities/DataVersion.h
        src/SysMLv2Standard/entities/Query.cpp
        src/SysMLv2Standard/entities/Query.h
        src/SysMLv2Standard/entities/Tag.cpp
        src/SysMLv2Standard/entities/Tag.h
        src/SysMLv2Standard/entities/Branch.cpp
        src/SysMLv2Standard/entities/Branch.h
        src/SysMLv2Standard/entities/Data.cpp
        src/SysMLv2Standard/entities/Data.h
        src/SysMLv2Standard/entities/ProjectUsage.cpp
        src/SysMLv2Standard/entities/ProjectUsage.h
        src/SysMLv2Standard/entities/Element.cpp
        src/SysMLv2Standard/entities/Element.h
        src/SysMLv2Standard/entities/Relationship.cpp
        src/SysMLv2Standard/entities/Relationship.h
        src/SysMLv2Standard/entities/ExternalData.cpp
        src/SysMLv2Standard/entities/ExternalData.h
        src/SysMLv2Standard/entities/ExternalRelationship.cpp
        src/SysMLv2Standard/entities/ExternalRelationship.h
        src/SysMLv2Standard/SysMLv2Deserializer.cpp
        src/SysMLv2Standard/SysMLv2Deserializer.h
        src/AGILABackendImplementation/Authentication.cpp
        src/AGILABackendImplementation/Authentication.h
)

set_target_properties(CPSBaseLib PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(CPSBaseLib
                        boost::boost
                        nlohmann_json::nlohmann_json
                        date::date)

install(TARGETS CPSBaseLib DESTINATION "src"
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

if(MSVC)
    target_compile_options(CPSBaseLib PRIVATE /W4 /WX)
else()
    target_compile_options(CPSBaseLib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

option(RUN_TESTS "Build the tests" ON)

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()