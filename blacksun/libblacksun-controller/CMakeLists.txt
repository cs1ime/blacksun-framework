project(blacksun-controller C CXX)




add_library(${PROJECT_NAME} blacksun-controller.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_link_libraries(${PROJECT_NAME} blacksun-dma blacksun-util blacksun-protocol)
