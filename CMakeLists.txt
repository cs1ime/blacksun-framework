project(blacksun-framework C CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

add_subdirectory(blacksun)
add_subdirectory(apex)
add_subdirectory(apex-dumper)

