cmake_minimum_required(VERSION 3.8)
project(warehouse)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable(warehouse main.cpp ${SRC_FILES})
target_include_directories(warehouse PRIVATE src)
target_compile_definitions(warehouse PRIVATE PROJECT_ROOT="${PROJECT_ROOT}")