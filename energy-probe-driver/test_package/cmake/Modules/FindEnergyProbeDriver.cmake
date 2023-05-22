find_path(
        UDEV_INCLUDE_DIR
        libudev.h
        /usr/include
        /usr/local/include
        ${ENERGYPROBEDRIVER_PATH_INCLUDES}
)

find_library(
        ENERGYPROBEDRIVER_LIBRARIES
        NAMES EnergyProbeDriver libEnergyProbeDriver
        PATHS ${ADDITIONAL_LIBRARY_PATHS}
        ${ENERGYPROBEDRIVER_PATH_LIB}
)

if(ENERGYPROBEDRIVER_LIBRARIES AND ENERGYPROBEDRIVER_PATH_INCLUDES)
    set(ENERGYPROBEDRIVER_FOUND "YES")
    execute_process(COMMAND pkg-config --atleast-version=2305 libEnergyProbeDriver RESULT_VARIABLE PROBEDRIVER_STABLE)
    # retvale is 0 of the condition is "true" so we need to negate the value...
    if(PROBEDRIVER_STABLE)
        set(PROBEDRIVER_STABLE 0)
    else()
        set(PROBEDRIVER_STABLE 1)
    endif()
    message(STATUS "Energy Probe Driver stable: ${ENERGYPROBEDRIVER_LIBRARIES}")
endif()

if(UDEV_FOUND)
    message(STATUS "Found Energy Probe Driver: ${ENERGYPROBEDRIVER_LIBRARIES}")
    message(STATUS "   include: ${UDEV_INCLUDE_DIR}")
else()
    message(STATUS "UDev not found.")
    message(STATUS "UDev: You can specify includes: -DUDEV_PATH_INCLUDES=/opt/udev/include")
    message(STATUS "      currently found includes: ${ENERGYPROBEDRIVER_PATH_INCLUDES}")
    message(STATUS "UDev: You can specify libs: -DUDEV_PATH_LIB=/opt/udev/lib")
    message(STATUS "      currently found libs: ${ENERGYPROBEDRIVER_LIBRARIES}")
    if(UDev_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find UDev library")
    endif()
endif()

mark_as_advanced(ENERGYPROBEDRIVER_PATH_INCLUDES ENERGYPROBEDRIVER_LIBRARIES)