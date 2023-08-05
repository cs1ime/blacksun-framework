project(apex-dumper C CXX)

file(GLOB_RECURSE APEX_DUMPER_SOURCES "apex/*.cpp")
add_executable(${PROJECT_NAME} main.cpp ${APEX_DUMPER_SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -w)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} PRIVATE blacksun-dma)
