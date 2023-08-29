cmake_minimum_required(VERSION 3.15)
project(digitwester CXX)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(energyprobedriver CONFIG)
find_package(Threads REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(eclipse-paho-mqtt-c REQUIRED)

add_executable(digitwester
        src/RealTwester.cpp
        src/RealTwester.h
        src/main.cpp
        src/digitwester_global.h
        src/Exceptions/ServerNotReachableException.cpp
        src/Exceptions/ServerNotReachableException.h
        src/Data/Project.cpp
        src/Data/Project.h
        src/Data/DataPoint.cpp
        src/Data/DataPoint.h
        src/Data/IDataBase.h
        src/Connections/REST/RestAPIConnector.h
        src/Connections/REST/RestAPIConnector.cpp
        src/Connections/MQTT/MQTTConnectionManager.h
        src/Connections/MQTT/MQTTConnectionManager.cpp
        src/Connections/MQTT/MQTTPersistence.h
        src/Connections/MQTT/MQTTPersistence.cpp
        src/Logging/LoggingService.h
        src/Logging/LoggingService.cpp
        src/Exceptions/NotInitializedException.cpp
        src/Exceptions/NotInitializedException.h)

if(energyprobedriver)
        target_link_libraries(${PROJECT_NAME} CURL::libcurl nlohmann_json::nlohmann_json energyprobedriver::energyprobedriver Threads::Threads PahoMqttCpp::paho-mqttpp3-static eclipse-paho-mqtt-c::paho-mqtt3as-static)
else (energyprobedriver)
    message("Using in Development Mode, since no EnergyProbeDriver via conan is installed")
    target_link_libraries(${PROJECT_NAME} CURL::libcurl nlohmann_json::nlohmann_json Threads::Threads EnergyProbeDriver PahoMqttCpp::paho-mqttpp3-static eclipse-paho-mqtt-c::paho-mqtt3as-static)
endif (energyprobedriver)

target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

install(TARGETS ${PROJECT_NAME} DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
