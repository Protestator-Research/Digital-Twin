cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(energyprobedriver CONFIG REQUIRED)

if(UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -lrt")
endif(UNIX)


add_executable(example src/example.cpp)
target_link_libraries(example energyprobedriver::energyprobedriver)
