cmake_minimum_required(VERSION 3.15)

project(CppDigitalTwinLib CXX)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED CONFIG)
find_package(antlr4-runtime REQUIRED)

add_library(CppDigitalTwinLib
        src/DigitalTwinManager.cpp
        src/DigitalTwinManager.h
        src/Model/DigitalTwinModel.cpp
        src/Model/DigitalTwinModel.h
        src/Model/Component.cpp
        src/Model/Component.h
        src/Parser/KerML/KerMLBaseListener.cpp
        src/Parser/KerML/KerMLBaseListener.h
        src/Parser/KerML/KerMLLexer.h
        src/Parser/KerML/KerMLLexer.cpp
        src/Parser/KerML/KerMLListener.cpp
        src/Parser/KerML/KerMLListener.h
        src/Parser/KerML/KerMLParser.h
        src/Parser/KerML/KerMLParser.cpp
        src/Parser/SysMLv2/SysMLv2BaseListener.cpp
        src/Parser/SysMLv2/SysMLv2BaseListener.h
        src/Parser/SysMLv2/SysMLv2Lexer.h
        src/Parser/SysMLv2/SysMLv2Lexer.cpp
        src/Parser/SysMLv2/SysMLv2Listener.cpp
        src/Parser/SysMLv2/SysMLv2Listener.h
        src/Parser/SysMLv2/SysMLv2Parser.h
        src/Parser/SysMLv2/SysMLv2Parser.cpp
        src/Parser/Parser.cpp
        src/Parser/Parser.h
        src/Model/Variable.cpp
        src/Model/Variable.h
        src/Model/IDigitalTwinElement.h
        src/cpp_digital_twin_lib_global.h)

target_include_directories(CppDigitalTwinLib PUBLIC
        ../PhysicalTwinCommunicationService/src/
        ../BackendCommunicationService/src/
        ../CpsBaseLib/src)

target_link_libraries(CppDigitalTwinLib
        CppPhysicalTwinCommunication
        CppBackendCommunication
        CPSBaseLib
        antlr4_shared)

if(MSVC)
    target_compile_options(CppDigitalTwinLib PRIVATE /W4 /WX-)
else()
    target_compile_options(CppDigitalTwinLib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(TARGETS CppDigitalTwinLib DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()