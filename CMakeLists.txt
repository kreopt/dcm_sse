cmake_minimum_required(VERSION 2.8)
project(sse_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp json11.cpp)
add_executable(sse_server ${SOURCE_FILES})
FIND_LIBRARY(DCM_LIBRARY dcm /usr/local/lib/dcm/)

target_link_libraries(sse_server
    pthread
    ${DCM_LIBRARY}
)