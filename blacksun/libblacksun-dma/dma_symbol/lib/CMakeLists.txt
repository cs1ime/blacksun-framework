cmake_minimum_required(VERSION 3.2.0)
project(COMPONMENT_PDB C CXX)

add_library(${PROJECT_NAME}
downloader.cpp
dma_symbol_remote_pdb.cpp
ExampleMemoryMappedFile.cpp
pdb_helper.cpp
pdb_parser.cpp
)
target_compile_options(${PROJECT_NAME} PRIVATE -w -O3 -fdeclspec -fms-extensions)
target_include_directories(${PROJECT_NAME} PUBLIC ../include)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    CXX_STANDARD 17
    
    )
find_package(OpenSSL 1.1.1 REQUIRED)
target_link_libraries(
    ${PROJECT_NAME}
    raw_pdb
    spdlog
    cxxopts
    nlohmann_json
    httplib
    OpenSSL::SSL
    OpenSSL::Crypto
    COMPONMENT_DMA
)
