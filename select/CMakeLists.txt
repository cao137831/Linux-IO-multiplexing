cmake_minimum_required(VERSION 3.0)
project(test)

set(HOME /home/cao/Desktop/Linux/IO多路复用/select)
set(EXECUTABLE_OUTPUT_PATH ${HOME}/bin)


file(GLOB_RECURSE server_process ${CMAKE_CURRENT_SOURCE_DIR}/src/server_process.cpp)
file(GLOB_RECURSE server_thread ${CMAKE_CURRENT_SOURCE_DIR}/src/server_thread.cpp)
file(GLOB_RECURSE client ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp)

add_executable(server_process ${server_process})
add_executable(server_thread ${server_thread})
add_executable(client ${client})