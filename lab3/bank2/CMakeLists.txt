cmake_minimum_required(VERSION 3.8)
project(bank2)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable(bank2 main.cpp ${SRC_FILES})
target_include_directories(bank2 PRIVATE src)
target_compile_definitions(bank2 PRIVATE PROJECT_ROOT="${PROJECT_ROOT}")