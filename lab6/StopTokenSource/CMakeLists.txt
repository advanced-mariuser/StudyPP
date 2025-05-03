cmake_minimum_required(VERSION 3.8)
project(stopTokenSource)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

find_package(TBB REQUIRED)

add_executable(stopTokenSource main.cpp ${SRC_FILES})
target_include_directories(stopTokenSource PRIVATE TBB::tbb src)
target_compile_definitions(stopTokenSource PRIVATE PROJECT_ROOT="${PROJECT_ROOT}")

add_subdirectory(test)

add_dependencies(stopTokenSource run_tests)