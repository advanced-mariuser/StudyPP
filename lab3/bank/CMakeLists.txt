cmake_minimum_required(VERSION 3.8)
project(bank)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable(bank main.cpp ${SRC_FILES})
target_include_directories(bank PRIVATE src)
target_compile_definitions(bank PRIVATE PROJECT_ROOT="${PROJECT_ROOT}")

add_subdirectory(tests)

add_dependencies(bank run_tests)