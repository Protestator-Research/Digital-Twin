cmake_minimum_required(VERSION 3.15)

project(EnergyProbeDriver CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if(UNIX)
find_package(UDev)
endif(UNIX)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(HEADER_FILES 
        include/EnergyProbe.h
        include/Device.h
        include/Fifo.h
        include/Definitions.h
        include/DriverSessionManager.h
        include/energy_probe_driver_global.h
        include/MeasurePoint.h
        include/DataPointObserver.h)

add_library(EnergyProbeDriver 
        src/EnergyProbe.cpp
        src/Device.cpp
        src/Fifo.cpp
        src/DriverSessionManager.cpp
        src/MeasurePoint.cpp
        src/DataPointObserver.cpp
        ${HEADER_FILES})


include(GenerateExportHeader)
generate_export_header(EnergyProbeDriver)

target_include_directories(EnergyProbeDriver PUBLIC include)
target_compile_definitions(EnergyProbeDriver PRIVATE ENERGY_PROBE_DRIVER_EXPORT)

if(UNIX)
target_link_libraries(${PROJECT_NAME} ${UDEV_LIBRARIES})
else(UNIX)
target_link_libraries(${PROJECT_NAME} ws2_32.lib setupapi.lib)
endif(UNIX)

install(TARGETS ${PROJECT_NAME} DESTINATION lib/)
# install(EXPORT ${PROJECT_NAME}_targets DESTINATION lib/)
# install(FILES HEADER_FILES DESTINATION include/)
install(DIRECTORY "${EnergyProbeDriver_SOURCE_DIR}/include" # source directory
        DESTINATION "epd" # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
)

configure_file(
   ${EnergyProbeDriver_SOURCE_DIR}/cmake/pkg/energyprobedriver-config.cmake.in
   ${EnergyProbeDriver_BINARY_DIR}/cmake/pkg/energyprobedriver-config.cmake @ONLY)

configure_file(
   ${EnergyProbeDriver_SOURCE_DIR}/cmake/energyprobedriver-config-version.cmake.in
   ${EnergyProbeDriver_BINARY_DIR}/cmake/energyprobedriver-config-version.cmake @ONLY)

install(FILES ${EnergyProbeDriver_BINARY_DIR}/cmake/pkg/energyprobedriver-config.cmake
         ${EnergyProbeDriver_BINARY_DIR}/cmake/energyprobedriver-config-version.cmake
         DESTINATION lib/)

# Make project usable from build tree.
export(TARGETS EnergyProbeDriver FILE energyprobedriver-targets.cmake)
configure_file(${EnergyProbeDriver_SOURCE_DIR}/cmake/pkg/energyprobedriver-config.cmake.in
               ${EnergyProbeDriver_BINARY_DIR}/cmake/pkg/energyprobedriver-config.cmake @ONLY)