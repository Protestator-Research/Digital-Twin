cmake_minimum_required(VERSION 3.15)

add_library(DigitalTwinLib
        src/DigitalTwinManager.cpp
        src/DigitalTwinManager.h
        src/Model/DigitalTwinModel.cpp
        src/Model/DigitalTwinModel.h
        src/Model/Entities/Component.cpp
        src/Model/Entities/Component.h
        src/Model/Entities/Variable.cpp
        src/Model/Entities/Variable.h
        src/Model/Entities/IDigitalTwinElement.h
        src/cpp_digital_twin_lib_global.h
        src/Model/SupportedTypes/SupportedTypes.h
        src/Model/Entities/Port.cpp
        src/Model/Entities/Port.h
        src/Model/Entities/Connection.cpp
        src/Model/Entities/Connection.h
        src/Model/Exceptions/DigitalTwinAddressException.cpp
        src/Model/Exceptions/DigitalTwinAddressException.h
        src/Model/Itelligence/AiSecurityBreachDetector.cpp
        src/Model/Itelligence/AiSecurityBreachDetector.h
        src/Model/Itelligence/OutlierDetection.cpp
        src/Model/Itelligence/OutlierDetection.h
)

target_include_directories(DigitalTwinLib PUBLIC
        boost::boost
        ../PhysicalTwinCommunicationService/src/
        ../BackendCommunicationService/src/
        ../CpsBaseLib/src)

target_link_libraries(DigitalTwinLib
        boost::boost
        PhysicalTwinCommunication
        BackendCommunication
        CPSBaseLib
        antlr4_shared
        sysmllib::sysmllib)

if(MSVC)
    target_compile_options(DigitalTwinLib PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(DigitalTwinLib PRIVATE -Wall -Wextra -Wpedantic -Werror)
else ()
    target_compile_options(DigitalTwinLib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(TARGETS DigitalTwinLib DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

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