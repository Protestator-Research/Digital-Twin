cmake_minimum_required(VERSION 3.15)

project(DigitalTwin CXX)

add_subdirectory(./CppDigitalTwin/BackendCommunicationService)
add_subdirectory(./CppDigitalTwin/DTServer)

#add_subdirectory(./realtwester)