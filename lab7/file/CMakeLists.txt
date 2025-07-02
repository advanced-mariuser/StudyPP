cmake_minimum_required(VERSION 3.20)
project(AsyncIOExample CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -g)

include_directories(src)

add_executable(CoroutineTaskExample  main.cpp)

find_library(URING_LIB uring REQUIRED)
target_link_libraries(CoroutineTaskExample ${URING_LIB})
