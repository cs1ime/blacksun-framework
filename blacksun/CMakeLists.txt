cmake_minimum_required(VERSION 3.20)

project(blacksun C CXX)

add_subdirectory(libblacksun-util)
add_subdirectory(libblacksun-protocol)
add_subdirectory(libblacksun-dma)
add_subdirectory(libblacksun-controller)

add_subdirectory(moonlight-mydrawer)

