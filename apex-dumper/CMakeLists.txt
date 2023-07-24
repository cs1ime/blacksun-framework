cmake_minimum_required(VERSION 3.2.0)

project(apex-dumper C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -O3")

file(GLOB_RECURSE APEX_DUMPER_SOURCES "apex/*.cpp")
add_executable(${PROJECT_NAME} main.cpp ${APEX_DUMPER_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} PRIVATE blacksun-dma)
