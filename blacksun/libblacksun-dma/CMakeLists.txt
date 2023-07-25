cmake_minimum_required(VERSION 3.20)

project(blacksun-dma C CXX)

add_subdirectory(thirdparty/cpp-httplib)
add_subdirectory(thirdparty/cxxopts)
add_subdirectory(thirdparty/nlohmann_json)
add_subdirectory(thirdparty/raw_pdb)
add_subdirectory(thirdparty/spdlog)

add_subdirectory(dma)
add_subdirectory(dma_symbol)
add_subdirectory(qemukvm)
add_subdirectory(rawmemfile)

add_library(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC COMPONMENT_DMA COMPONMENT_PDB COMPONMENT_QEMUKVM COMPONMENT_RAWMEMFILE)


