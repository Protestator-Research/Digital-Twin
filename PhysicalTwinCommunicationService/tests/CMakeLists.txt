

add_executable(PhysicalTwinCommunicationServiceLibTests
        main.cpp
        entities/SerializationTests.cpp)

target_include_directories(PhysicalTwinCommunicationServiceLibTests PRIVATE
                                    ../src/)
target_link_libraries(PhysicalTwinCommunicationServiceLibTests
                                    boost::boost
                                    GTest::gtest_main
                                    PhysicalTwinCommunication
                                    BackendCommunication
                                    CPSBaseLib)

add_test(NAME PhysicalTwinCommunicationServiceLibTests COMMAND PhysicalTwinCommunicationServiceLibTests)

set_tests_properties(${PhysicalTwinCommunicationServiceLibTests} PROPERTIES TIMEOUT 10)