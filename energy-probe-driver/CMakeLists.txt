cmake_minimum_required(VERSION 3.15)

project(EnergyProbeDriver CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(UDev)

add_library(EnergyProbeDriver src/EnergyProbe.cpp src/Device.cpp include/EnergyProbe.h include/Device.h src/Fifo.cpp include/Fifo.h include/Definitions.h)

target_include_directories(EnergyProbeDriver PUBLIC include)

target_link_libraries(${PROJECT_NAME} ${UDEV_LIBRARIES})

install(TARGETS EnergyProbeDriver)
