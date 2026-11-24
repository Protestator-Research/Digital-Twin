cmake_minimum_required(VERSION 3.15)

project(DigitalTwinStore CXX)

# The schema is embedded at build time: sql/0001_schema.sql is the only copy of its text.
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/sql/0001_schema.sql DT_SCHEMA_SQL)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sql/0001_schema.sql)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/EmbeddedSchema.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedSchema.cpp @ONLY)

add_library(DigitalTwinStore
        src/cpp_digital_twin_store_global.h
        src/StoreSettings.h
        src/StoreLogic.h
        src/StoreLogic.cpp
        src/RowQueue.h
        src/EmbeddedSchema.h
        src/DigitalTwinStore.h
        src/DigitalTwinStore.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedSchema.cpp)

# The store only uses the plain types of Model/State/StateTypes.h, it does not link the model library.
target_include_directories(DigitalTwinStore PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../DigitalTwinLib/src)

target_link_libraries(DigitalTwinStore
        PRIVATE libpqxx::pqxx
        PUBLIC Threads::Threads)

file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

if(MSVC)
    target_compile_options(DigitalTwinStore PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(DigitalTwinStore PRIVATE -Wall -Wextra -Wpedantic)
else ()
    target_compile_options(DigitalTwinStore PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS DigitalTwinStore DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
