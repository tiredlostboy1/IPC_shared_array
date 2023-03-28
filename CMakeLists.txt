cmake_minimum_required(VERSION 3.10)

project(shared_array CXX)

add_executable(first first.cpp)
add_executable(second second.cpp)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}")