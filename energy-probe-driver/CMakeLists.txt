cmake_minimum_required(VERSION 3.15)

project(EnergyProbeDriver CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(UDev)

add_library(EnergyProbeDriver src/EnergyProbeDriver.cpp)

target_include_directories(EnergyProbeDriver PUBLIC include)

set_target_properties(EnergyProbeDriver PROPERTIES PUBLIC_HEADER "include/EnergyProbeDriver.h")

target_link_libraries(${PROJECT_NAME} ${UDEV_LIBRARIES})

install(TARGETS EnergyProbeDriver)
