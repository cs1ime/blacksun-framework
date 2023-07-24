cmake_minimum_required(VERSION 3.2.0)

project(blacksun C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -O3")

add_subdirectory(libblacksun-util)
add_subdirectory(libblacksun-protocol)
add_subdirectory(libblacksun-dma)
add_subdirectory(libblacksun-controller)

add_subdirectory(moonlight-mydrawer)

