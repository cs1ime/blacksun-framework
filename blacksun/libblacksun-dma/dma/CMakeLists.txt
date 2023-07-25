cmake_minimum_required(VERSION 3.20)

project(COMPONMENT_DMA VERSION 0.1.0 LANGUAGES C CXX)

add_library(${PROJECT_NAME}
    src/dma_memsch.cpp
    src/dma_mmu.cpp
    src/dma_ntutil.cpp
    src/dma.cpp
    src/dma_peparser.cpp
)
target_compile_options(${PROJECT_NAME} PRIVATE -w -fdeclspec -fms-extensions)
target_include_directories(${PROJECT_NAME} PUBLIC include)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 17
)
