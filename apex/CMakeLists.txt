project(apex C CXX)

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE APEX_SOURCES "apex/*.cpp")

add_executable(${PROJECT_NAME} main.cpp ${APEX_SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -w)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} blacksun-dma blacksun-util blacksun-protocol blacksun-controller)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
