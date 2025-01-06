cmake_minimum_required(VERSION 3.15)

project(CppDigitalTwinServer CXX C)

find_package(mqtt_cpp REQUIRED CONFIG)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Charts)
find_package(md4c REQUIRED)

qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

set(SOURCES src/Widgets/DigitalTwinMainWindow.cpp
        src/Widgets/DigitalTwinMainWindow.h
        src/main.cpp
        src/Widgets/Dialogs/SettingsDialog.cpp
        src/Widgets/Dialogs/SettingsDialog.h
        src/Models/DigitalTwinClientSettings.cpp
        src/Models/DigitalTwinClientSettings.h
        ui/Resources.qrc
        src/Models/MainWindowStatus.h
        src/Models/MainWindowModel.cpp
        src/Models/MainWindowModel.h
        src/Widgets/TreeViewModels/ProjectTreeViewModel.cpp
        src/Widgets/TreeViewModels/ProjectTreeViewModel.h
        src/Widgets/TreeViewModels/ProjectTreeViewItem.cpp
        src/Widgets/TreeViewModels/ProjectTreeViewItem.h
        src/Widgets/DigitalTwinTabWidget/DigitalTwinTabWidget.cpp
        src/Widgets/DigitalTwinTabWidget/DigitalTwinTabWidget.h
        src/Models/Markdown/MarkdownParser.cpp
        src/Models/Markdown/MarkdownParser.h
        src/Widgets/DigitalTwinTabWidget/TabContentWidget.cpp
        src/Widgets/DigitalTwinTabWidget/TabContentWidget.h
        src/Widgets/TreeViewModels/VariableViewModel.h
        src/Widgets/TreeViewModels/VariableViewModel.cpp
        src/Widgets/TreeViewModels/VariableViewModelItem.h
        src/Widgets/TreeViewModels/VariableViewModelItem.cpp
        src/Widgets/UploadProjectFileToBackend.cpp
        src/Widgets/UploadProjectFileToBackend.h
        src/Models/Markdown/CommonmarkParser/blocks.c
        src/Models/Markdown/CommonmarkParser/buffer.c
        src/Models/Markdown/CommonmarkParser/buffer.h
        src/Models/Markdown/CommonmarkParser/chunk.h
        src/Models/Markdown/CommonmarkParser/cmark.c
        src/Models/Markdown/CommonmarkParser/cmark.h
        src/Models/Markdown/CommonmarkParser/cmark_ctype.c
        src/Models/Markdown/CommonmarkParser/cmark_ctype.h
        src/Models/Markdown/CommonmarkParser/commonmark.c
        src/Models/Markdown/CommonmarkParser/houdini.h
        src/Models/Markdown/CommonmarkParser/houdini_href_e.c
        src/Models/Markdown/CommonmarkParser/houdini_html_e.c
        src/Models/Markdown/CommonmarkParser/houdini_html_u.c
        src/Models/Markdown/CommonmarkParser/html.c
        src/Models/Markdown/CommonmarkParser/inlines.c
        src/Models/Markdown/CommonmarkParser/inlines.h
        src/Models/Markdown/CommonmarkParser/iterator.c
        src/Models/Markdown/CommonmarkParser/iterator.h
        src/Models/Markdown/CommonmarkParser/latex.c
        src/Models/Markdown/CommonmarkParser/man.c
        src/Models/Markdown/CommonmarkParser/node.c
        src/Models/Markdown/CommonmarkParser/node.h
        src/Models/Markdown/CommonmarkParser/parser.h
        src/Models/Markdown/CommonmarkParser/references.c
        src/Models/Markdown/CommonmarkParser/references.h
        src/Models/Markdown/CommonmarkParser/render.c
        src/Models/Markdown/CommonmarkParser/render.h
        src/Models/Markdown/CommonmarkParser/scanners.c
        src/Models/Markdown/CommonmarkParser/scanners.h
        src/Models/Markdown/CommonmarkParser/utf8.c
        src/Models/Markdown/CommonmarkParser/utf8.h
        src/Models/Markdown/CommonmarkParser/xml.c)

qt_wrap_ui(SOURCES ui/DigitalTwinTabWidget.ui
        ui/SettingsDialog.ui
        ui/DigitalTwinMainWindow.ui
        ui/TabContentWidget.ui
        ui/UploadProjectFileToBackend.ui)

add_executable(CppDigitalTwinClient ${SOURCES})



target_include_directories(CppDigitalTwinClient PUBLIC
        ../BackendCommunicationService/src/
        ../DigitalTwinLib/src/
        ../PhysicalTwinCommunicationService/src/
        ../CpsBaseLib/src
        "${CMAKE_CURRENT_BINARY_DIR}/")

target_link_libraries(CppDigitalTwinClient PUBLIC
        CppBackendCommunication
        CppDigitalTwinLib
        CppPhysicalTwinCommunication
        CPSBaseLib
        Qt6::Widgets
        Qt6::Gui
        Qt6::Charts
        Qt6::Core
        mqtt_cpp::mqtt_cpp
        md4c::md4c)

if (!UNIX)
set_target_properties(CppDigitalTwinClient PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
)
endif()

if(MSVC)
    target_compile_options(CppDigitalTwinClient PRIVATE /W4 /WX- /bigobj)
elseif (UNIX AND NOT APPLE)
    target_compile_options(CppDigitalTwinClient PRIVATE -Wall -Wextra -Wpedantic -Werror -fcoroutines)
else ()
    target_compile_options(CppDigitalTwinClient PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()