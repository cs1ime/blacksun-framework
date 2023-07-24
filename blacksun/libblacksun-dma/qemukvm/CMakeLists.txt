cmake_minimum_required(VERSION 3.2.0)

project(COMPONMENT_QEMUKVM C CXX)
#set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
#set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and 
#set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs 
#set(Boost_USE_MULTITHREADED      ON)
#set(Boost_USE_STATIC_RUNTIME    OFF) 
find_package(Boost COMPONENTS regex)

if(Boost_FOUND) 
    add_library(${PROJECT_NAME} src/qemukvm2dma.cpp)
    target_compile_options(${PROJECT_NAME} PRIVATE -w -O3 -fdeclspec -fms-extensions)

    target_include_directories(${PROJECT_NAME} PUBLIC ./include)
    target_link_libraries(${PROJECT_NAME} fmt ${Boost_LIBRARIES} COMPONMENT_DMA)

    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        CXX_STANDARD 17
        )
endif()
