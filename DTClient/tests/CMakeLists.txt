# Tests of the Markdown handling of the client: the parser (no GUI) and, with a server, the upload of real Markdown files.
file(GLOB COMMONMARK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../src/Models/Markdown/CommonmarkParser/*.c)
# main.c is the command line tool of cmark and would replace the main of the test runner.
list(FILTER COMMONMARK_SOURCES EXCLUDE REGEX "/main\\.c$")

add_executable(DigitalTwinClientTests
        TestMarkdown.cpp
        TestClientSettings.cpp
        ../src/Models/DigitalTwinClientSettings.cpp
        ../src/Models/Markdown/MarkdownParser.cpp
        ${COMMONMARK_SOURCES})

target_include_directories(DigitalTwinClientTests PRIVATE
        ../src/Models/Markdown
        ../../BackendCommunicationService/src/
        ../../DigitalTwinLib/src/
        ../../PhysicalTwinCommunicationService/src
        ../../CpsBaseLib/src)

target_compile_definitions(DigitalTwinClientTests PRIVATE
        DT_MARKDOWN_DIR="${CMAKE_SOURCE_DIR}/Markdownfiles")

target_link_libraries(DigitalTwinClientTests PRIVATE
        GTest::gtest_main
        Qt::Core
        DigitalTwinLib
        BackendCommunication
        CPSBaseLib
        sysmllib::sysmllib)

add_test(NAME DigitalTwinClientTests COMMAND DigitalTwinClientTests)
