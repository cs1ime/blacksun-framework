cmake_minimum_required(VERSION 3.2.0)

project(blacksun-protocol C CXX)

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE BLACKSUN_PROTOCOL_SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} ${BLACKSUN_PROTOCOL_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} blacksun-util)

target_include_directories(${PROJECT_NAME} PUBLIC include)
