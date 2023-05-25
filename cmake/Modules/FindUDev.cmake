find_path(
        UDEV_INCLUDE_DIR
        libudev.h
        /usr/include
        /usr/local/include
        ${UDEV_PATH_INCLUDES}
)

find_library(
        UDEV_LIBRARIES
        NAMES udev libudev
        PATHS ${ADDITIONAL_LIBRARY_PATHS}
        ${UDEV_PATH_LIB}
)

if(UDEV_LIBRARIES AND UDEV_INCLUDE_DIR)
    set(UDEV_FOUND "YES")
    execute_process(COMMAND pkg-config --atleast-version=143 libudev RESULT_VARIABLE UDEV_STABLE)
    # retvale is 0 of the condition is "true" so we need to negate the value...
    if(UDEV_STABLE)
        set(UDEV_STABLE 0)
    else()
        set(UDEV_STABLE 1)
    endif()
    message(STATUS "libudev stable: ${UDEV_STABLE}")
endif()

if(UDEV_FOUND)
    message(STATUS "Found UDev: ${UDEV_LIBRARIES}")
    message(STATUS "   include: ${UDEV_INCLUDE_DIR}")
else()
    message(STATUS "UDev not found.")
    message(STATUS "UDev: You can specify includes: -DUDEV_PATH_INCLUDES=/opt/udev/include")
    message(STATUS "      currently found includes: ${UDEV_INCLUDE_DIR}")
    message(STATUS "UDev: You can specify libs: -DUDEV_PATH_LIB=/opt/udev/lib")
    message(STATUS "      currently found libs: ${UDEV_LIBRARIES}")
    if(UDev_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find UDev library")
    endif()
endif()

mark_as_advanced(UDEV_INCLUDE_DIR UDEV_LIBRARIES)