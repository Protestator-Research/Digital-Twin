cmake_minimum_required(VERSION 3.15)
project(digitwester CXX)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(energyprobedriver CONFIG)
find_package(Threads REQUIRED)

add_executable(digitwester
        src/RealTwester.cpp
        src/RealTwester.h
        src/RestAPIConnector.h
        src/RestAPIConnector.cpp
        src/main.cpp
        src/digitwester_global.h
        src/Exceptions/ServerNotReachableException.cpp
        src/Exceptions/ServerNotReachableException.h
        src/Data/Project.cpp
        src/Data/Project.h
        src/Data/DataPoint.cpp
        src/Data/DataPoint.h
        src/Data/IDataBase.h)

# if(energyprobedriver)
target_link_libraries(${PROJECT_NAME} CURL::libcurl nlohmann_json::nlohmann_json energyprobedriver::energyprobedriver Threads::Threads)
# else (energyprobedriver)
#     message("Using in Development Mode, since no EnergyProbeDriver via conan is installed")
#     target_link_libraries(${PROJECT_NAME} CURL::libcurl nlohmann_json::nlohmann_json Threads::Threads EnergyProbeDriver)
# endif (energyprobedriver)

install(TARGETS ${PROJECT_NAME} DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
