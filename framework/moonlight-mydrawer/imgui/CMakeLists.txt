
project(imgui C CXX)

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE IMGUI_SOURCES "src/*.cpp")

add_library(${PROJECT_NAME} ${IMGUI_SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)

target_link_directories(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC include)
