cmake_minimum_required(VERSION 3.15)

project(EnergyProbeDriver CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(UNIX)
find_package(UDev)
endif(UNIX)

add_library(EnergyProbeDriver src/EnergyProbe.cpp src/Device.cpp include/EnergyProbe.h include/Device.h src/Fifo.cpp include/Fifo.h include/Definitions.h src/DriverSessionManager.cpp include/DriverSessionManager.h include/energy_probe_driver_global.h)

target_include_directories(EnergyProbeDriver PUBLIC include)

if(UNIX)
target_link_libraries(${PROJECT_NAME} ${UDEV_LIBRARIES})
endif(UNIX)

install(TARGETS EnergyProbeDriver)
