cmake_minimum_required(VERSION 3.15)

add_library(DigitalTwinLib
        src/DigitalTwinManager.cpp
        src/DigitalTwinManager.h
        src/Model/DigitalTwinModel.cpp
        src/Model/DigitalTwinModel.h
        src/Model/ModelBlocks.cpp
        src/Model/ModelBlocks.h
        src/Model/Entities/Component.cpp
        src/Model/Entities/Component.h
        src/Model/Entities/Variables/Variable.cpp
        src/Model/Entities/Variables/Variable.hpp
        src/Model/Entities/IDigitalTwinElement.h
        src/cpp_digital_twin_lib_global.h
        src/Model/SupportedTypes/SupportedTypes.h
        src/Model/Entities/Port.cpp
        src/Model/Entities/Port.h
        src/Model/Entities/Connection.cpp
        src/Model/Entities/Connection.h
        src/Model/Entities/Package.h
        src/Model/Entities/Package.cpp
        src/Model/Exceptions/DigitalTwinAddressException.cpp
        src/Model/Exceptions/DigitalTwinAddressException.h
        src/Model/Itelligence/AiSecurityBreachDetector.cpp
        src/Model/Itelligence/AiSecurityBreachDetector.h
        src/Model/Itelligence/OutlierDetection.cpp
        src/Model/Itelligence/OutlierDetection.h
        src/Model/Entities/ICollectionType.h
        src/Model/Entities/Individual.cpp
        src/Model/Entities/Individual.h
        src/Model/Entities/Function.cpp
        src/Model/Entities/Function.h
        src/Model/Entities/Bindings.cpp
        src/Model/Entities/Bindings.h
        src/Model/Entities/Variables/BooleanVariable.cpp
        src/Model/Entities/Variables/BooleanVariable.h
        src/Model/Entities/Variables/StringVariable.cpp
        src/Model/Entities/Variables/StringVariable.h
        src/Model/Entities/Variables/ComplexVariable.cpp
        src/Model/Entities/Variables/ComplexVariable.h
        src/Model/Entities/Variables/RealVariable.cpp
        src/Model/Entities/Variables/RealVariable.h
        src/Model/Entities/Variables/RationalVariable.cpp
        src/Model/Entities/Variables/RationalVariable.h
        src/Model/Entities/Variables/IntegerVariable.cpp
        src/Model/Entities/Variables/IntegerVariable.h
        src/Model/Entities/Variables/NaturalVariable.cpp
        src/Model/Entities/Variables/NaturalVariable.h
        src/Model/Entities/Variables/PositiveVariable.cpp
        src/Model/Entities/Variables/PositiveVariable.h
        src/Model/Entities/Variables/ScalarQuantities.cpp
        src/Model/Entities/Variables/ScalarQuantities.h
        src/Model/Entities/ElementType.h
        src/Model/State/StateTypes.h
        src/Model/State/ValueCodec.h
        src/Model/State/ValueCodec.cpp
        src/Model/State/ModelTree.h
        src/Model/State/ModelTree.cpp
        src/Model/State/StateTracking.h
        src/Model/State/StateTracking.cpp)

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