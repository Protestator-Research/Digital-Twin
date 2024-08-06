
find_package(GTest REQUIRED)
find_package(Boost REQUIRED)

add_executable(BackendCommunicationLibTests
        main.cpp)

target_include_directories(BackendCommunicationLibTests PRIVATE
                                    ../src/)
target_link_libraries(BackendCommunicationLibTests
                                    boost::boost
                                    GTest::gtest_main
                                    CppPhysicalTwinCommunication
                                    CppBackendCommunication
                                    CPSBaseLib)

add_test(NAME BackendCommunicationLibTests COMMAND BackendCommunicationLibTests)

set_tests_properties(${BackendCommunicationLibTests} PROPERTIES TIMEOUT 10)