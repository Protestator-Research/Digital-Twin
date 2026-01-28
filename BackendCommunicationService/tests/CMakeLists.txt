

add_executable(BackendCommunicationLibTests
        main.cpp)

target_include_directories(BackendCommunicationLibTests PRIVATE
                                    ../src/)
target_link_libraries(BackendCommunicationLibTests
                                    boost::boost
                                    GTest::gtest_main
                                    PhysicalTwinCommunication
                                    BackendCommunication
                                    CPSBaseLib)

add_test(NAME BackendCommunicationLibTests COMMAND BackendCommunicationLibTests)

set_tests_properties(${BackendCommunicationLibTests} PROPERTIES TIMEOUT 10)