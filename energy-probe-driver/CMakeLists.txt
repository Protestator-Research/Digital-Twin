cmake_minimum_required(VERSION 3.15)

project(EnergyProbeDriver CXX)

add_library(EnergyProbeDriver src/EnergyProbeDriver.cpp)

target_include_directories(EnergyProbeDriver PUBLIC include)

set_target_properties(EnergyProbeDriver PROPERTIES PUBLIC_HEADER "include/EnergyProbeDriver.h")

install(TARGETS EnergyProbeDriver)
