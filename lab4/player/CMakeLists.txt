cmake_minimum_required(VERSION 3.14)
project(player)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        portaudio
        GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
        GIT_TAG        v19.7.0
)

FetchContent_MakeAvailable(portaudio)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable(player main.cpp ${SRC_FILES})

target_include_directories(player PRIVATE src ${portaudio_SOURCE_DIR}/include)
target_link_libraries(player PRIVATE portaudio)