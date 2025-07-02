cmake_minimum_required(VERSION 3.10)
project(BitonicSortComparison)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "C:/OpenCL/opencl-headers-install")
list(APPEND CMAKE_PREFIX_PATH "C:/Program Files (x86)/OpenCL-ICD-Loader")

find_package(OpenCL REQUIRED)
find_package(Threads REQUIRED)

add_executable(BitonicSortComparison
        main.cpp
        src/BitonicSorter.cpp
        src/GpuHelper.cpp
)

target_include_directories(BitonicSortComparison PRIVATE
        src
        "C:/OpenCL/OpenCL-CLHPP/include"
)
target_link_libraries(BitonicSortComparison PRIVATE
        OpenCL::OpenCL
        Threads::Threads
)