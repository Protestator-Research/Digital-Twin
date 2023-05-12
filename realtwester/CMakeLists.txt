cmake_minimum_required(VERSION 3.15)
project(digitwester CXX)

find_package(CURL REQUIRED)

add_executable(digitwester
        src/RealTwester.cpp
        src/RealTwester.h
               src/RestAPIConnector.h
               src/RestAPIConnector.cpp
               src/main.cpp
               src/digitwester_global.h src/Exceptions/ServerNotReachableException.cpp src/Exceptions/ServerNotReachableException.h src/Data/Project.cpp src/Data/Project.h src/Data/DataPoint.cpp src/Data/DataPoint.h)

target_link_libraries(${PROJECT_NAME} CURL::libcurl)

install(TARGETS digitwester DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
