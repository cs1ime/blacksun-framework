project(apex C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -O3")

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE APEX_SOURCES "apex/*.cpp")

add_executable(${PROJECT_NAME} main.cpp ${APEX_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_link_libraries(${PROJECT_NAME} blacksun-dma blacksun-util blacksun-protocol blacksun-controller)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
