
find_package(GTest REQUIRED)
find_package(Boost REQUIRED)

add_executable(CpsBaseLibTests
        main.cpp
        BaseFunctions/StringExtentionTest.cpp
        SysMLv2Standard/entities/SerializationTests.cpp
        BaseFunctions/ListsExtentionTests.cpp
        SysMLv2Standard/entities/GenerationTest.cpp)

target_include_directories(CpsBaseLibTests PRIVATE
                                    ../src/)
target_link_libraries(CpsBaseLibTests
                                    boost::boost
                                    GTest::gtest_main
                                    CPSBaseLib)

add_test(NAME CpsBaseLibTests COMMAND CpsBaseLibTests)

set_tests_properties(${CpsBaseLibTests} PROPERTIES TIMEOUT 10)
