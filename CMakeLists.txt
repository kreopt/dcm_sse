cmake_minimum_required(VERSION 3.1)
project(libsse)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp json11.cpp)
add_executable(libsse ${SOURCE_FILES})
FIND_LIBRARY(DCM_LIBRARY dcm /usr/local/lib/specforge/)

target_link_libraries(libsse
    pthread
    ${DCM_LIBRARY}
)