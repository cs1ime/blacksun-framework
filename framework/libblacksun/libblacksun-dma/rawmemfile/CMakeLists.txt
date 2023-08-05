cmake_minimum_required(VERSION 3.20)

project(COMPONMENT_RAWMEMFILE C CXX)

add_library(${PROJECT_NAME} src/rawmem2dma.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include)
#target_compile_options(${PROJECT_NAME} PRIVATE -w -fdeclspec -fms-extensions)
target_link_libraries(${PROJECT_NAME} COMPONMENT_DMA)
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 17
    )
