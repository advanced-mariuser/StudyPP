cmake_minimum_required(VERSION 3.8)
project(extract-files)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)

add_executable(extract-files main.cpp)
target_link_libraries(extract-files PRIVATE extractFiles)
