cmake_minimum_required(VERSION 3.8)
project(gaussian-blur)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")

include(FetchContent)

FetchContent_Declare(
        sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.2
)

FetchContent_MakeAvailable(sfml)
message(STATUS "SFML_SOURCE_DIR: ${sfml_SOURCE_DIR}")
message(STATUS "SFML_BINARY_DIR: ${sfml_BINARY_DIR}")

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable(gaussian-blur main.cpp ${SRC_FILES})
target_link_libraries(gaussian-blur PRIVATE sfml-graphics sfml-window sfml-system)
target_include_directories(gaussian-blur PRIVATE src)
target_compile_definitions(gaussian-blur PRIVATE PROJECT_ROOT="${PROJECT_ROOT}")