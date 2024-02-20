cmake_minimum_required(VERSION 3.15)

project(DigitalTwin CXX)

include(CMakeDependentOption)

add_subdirectory(./CppDigitalTwin/CpsBaseLib)
add_subdirectory(./CppDigitalTwin/BackendCommunicationService)
add_subdirectory(./CppDigitalTwin/DigitalTwinLib)
add_subdirectory(./CppDigitalTwin/DTServer)
add_subdirectory(./CppDigitalTwin/PhysicalTwinCommunicationService)