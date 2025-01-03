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
        src/Model/Entities/Component.cpp
        src/Model/Entities/Component.h
        src/Model/Parser/KerML/KerMLBaseListener.cpp
        src/Model/Parser/KerML/KerMLBaseListener.h
        src/Model/Parser/KerML/KerMLLexer.h
        src/Model/Parser/KerML/KerMLLexer.cpp
        src/Model/Parser/KerML/KerMLListener.cpp
        src/Model/Parser/KerML/KerMLListener.h
        src/Model/Parser/KerML/KerMLParser.h
        src/Model/Parser/KerML/KerMLParser.cpp
        src/Model/Parser/SysMLv2/SysMLv2BaseListener.cpp
        src/Model/Parser/SysMLv2/SysMLv2BaseListener.h
        src/Model/Parser/SysMLv2/SysMLv2Lexer.h
        src/Model/Parser/SysMLv2/SysMLv2Lexer.cpp
        src/Model/Parser/SysMLv2/SysMLv2Listener.cpp
        src/Model/Parser/SysMLv2/SysMLv2Listener.h
        src/Model/Parser/SysMLv2/SysMLv2Parser.h
        src/Model/Parser/SysMLv2/SysMLv2Parser.cpp
        src/Model/Parser/Parser.cpp
        src/Model/Parser/Parser.h
        src/Model/Entities/Variable.cpp
        src/Model/Entities/Variable.h
        src/Model/Entities/IDigitalTwinElement.h
        src/cpp_digital_twin_lib_global.h
        src/Model/SupportedTypes/SupportedTypes.h
        src/Model/Parser/SupportedModels.h
        src/Model/Entities/Port.cpp
        src/Model/Entities/Port.h
        src/Model/Entities/Connection.cpp
        src/Model/Entities/Connection.h
)

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
    target_compile_options(CppDigitalTwinLib PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(CppDigitalTwinLib PRIVATE -Wall -Wextra -Wpedantic -Werror -fcoroutines)
else ()
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