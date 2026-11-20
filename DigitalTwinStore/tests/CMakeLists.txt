
add_executable(DigitalTwinStoreTests
        main.cpp
        TestStoreLogic.cpp
        TestStoreDatabase.cpp)

target_include_directories(DigitalTwinStoreTests PRIVATE
                                    ../src/)
target_link_libraries(DigitalTwinStoreTests
                                    GTest::gtest_main
                                    DigitalTwinStore
                                    libpqxx::pqxx)

# The test compares the embedded schema with the file.
target_compile_definitions(DigitalTwinStoreTests PRIVATE
                                    DT_STORE_SCHEMA_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../sql/0001_schema.sql")

if(MSVC)
    target_compile_options(DigitalTwinStoreTests PRIVATE /bigobj)
endif()

add_test(NAME DigitalTwinStoreTests COMMAND DigitalTwinStoreTests)

# The database tests need DT_TEST_DATABASE_URL, without it they are skipped. Generous: they start a store and wait.
set_tests_properties(DigitalTwinStoreTests PROPERTIES TIMEOUT 300)
