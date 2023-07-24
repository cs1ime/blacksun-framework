project(moonlight-mydrawer C CXX)

add_subdirectory(imgui)

find_package(SDL2 REQUIRED)

add_library(${PROJECT_NAME} SHARED gui.cpp main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES} blacksun-util blacksun-protocol imgui)
