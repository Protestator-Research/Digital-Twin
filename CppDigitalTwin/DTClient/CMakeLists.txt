cmake_minimum_required(VERSION 3.15)

project(CppDigitalTwinServer CXX)

find_package(mqtt_cpp REQUIRED CONFIG)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
qt_standard_project_setup()

add_executable(CppDigitalTwinClient
        src/Widgets/DigitalTwinMainWindow.cpp
        src/Widgets/DigitalTwinMainWindow.h
        ui/DigitalTwinMainWindow.ui
        src/main.cpp
        src/Dialogs/SettingsDialog.cpp
        src/Dialogs/SettingsDialog.h
        ui/SettingsDialog.ui
        src/Models/DigitalTwinClientSettings.cpp
        src/Models/DigitalTwinClientSettings.h
)

target_include_directories(CppDigitalTwinClient PUBLIC
        ../BackendCommunicationService/src/
        ../DigitalTwinLib/src/
        ../PhysicalTwinCommunicationService/src/)

target_link_libraries(CppDigitalTwinClient PUBLIC
        CppBackendCommunication
        CppDigitalTwinLib
        CppPhysicalTwinCommunication
        CPSBaseLib
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        mqtt_cpp::mqtt_cpp)

set_target_properties(CppDigitalTwinClient PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
)

if(MSVC)
    target_compile_options(CppDigitalTwinServer PRIVATE /W4 /WX)
else()
    target_compile_options(CppDigitalTwinServer PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()