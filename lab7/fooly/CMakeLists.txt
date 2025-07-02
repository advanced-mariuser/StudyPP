cmake_minimum_required(VERSION 3.14)
project(FollyCoroExample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(folly CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_executable(folly_coro_example main.cpp)

target_include_directories(folly_coro_example
        PRIVATE
        /home/gleb_paybarshev/vcpkg/installed/x64-linux/include
)

target_link_libraries(folly_coro_example
        PRIVATE
        Folly::folly
        Threads::Threads
)

target_compile_options(folly_coro_example
        PRIVATE
        -fcoroutines
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(folly_coro_example
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()