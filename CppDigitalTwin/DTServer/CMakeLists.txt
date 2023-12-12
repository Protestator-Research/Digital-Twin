cmake_minimum_required(VERSION 3.15)

project(CppDigitalTwinServer CXX)

find_package(mqtt_cpp REQUIRED CONFIG)

add_executable(${PROJECT_NAME}
        src/main.cpp)

target_link_libraries(${PROJECT_NAME} mqtt_cpp::mqtt_cpp)

# if(energyprobedriver)
#
#else (energyprobedriver)
#    message("Using in Development Mode, since no EnergyProbeDriver via conan is installed")
#    target_link_libraries(${PROJECT_NAME} CURL::libcurl nlohmann_json::nlohmann_json Threads::Threads EnergyProbeDriver PahoMqttCpp::paho-mqttpp3-static eclipse-paho-mqtt-c::paho-mqtt3as-static)
#endif (energyprobedriver)

#target_include_directories(${PROJECT_NAME} PRIVATE ${mqtt_cpp_INCLUDE_DIRS})
#target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

#install(TARGETS ${PROJECT_NAME} DESTINATION "."
#        RUNTIME DESTINATION bin
#        ARCHIVE DESTINATION lib
#        LIBRARY DESTINATION lib)
