cmake_minimum_required(VERSION 3.8)
project(threadPool)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.71 REQUIRED COMPONENTS system)

if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found")
endif()

set(PROJECT_ROOT "${CMAKE_SOURCE_DIR}")

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable(threadPool ${SRC_FILES})
target_include_directories(threadPool PRIVATE src)
target_compile_definitions(threadPool PRIVATE PROJECT_ROOT="${PROJECT_ROOT}")

target_link_libraries(threadPool PRIVATE Boost::boost Boost::system)

add_subdirectory(test)

add_dependencies(threadPool run_tests)