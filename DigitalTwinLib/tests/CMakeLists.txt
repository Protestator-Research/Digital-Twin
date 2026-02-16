

add_executable(DigitalTwinLibTests
        main.cpp)

target_include_directories(DigitalTwinLibTests PRIVATE
                                    ../src/)
target_link_libraries(DigitalTwinLibTests
                                    boost::boost
                                    GTest::gtest_main
                                    PhysicalTwinCommunication
                                    BackendCommunication
                                    CPSBaseLib)

add_test(NAME DigitalTwinLibTests COMMAND DigitalTwinLibTests)

set_tests_properties(${DigitalTwinLibTests} PROPERTIES TIMEOUT 10)