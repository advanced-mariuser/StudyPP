cmake_minimum_required(VERSION 3.10)
project(radio)

set(CMAKE_CXX_STANDARD 20)

find_library(PULSE_SIMPLE_LIB pulse-simple)
find_library(PULSE_LIB pulse)
find_path(PULSE_INCLUDE pulse/pulseaudio.h)

if(NOT PULSE_SIMPLE_LIB OR NOT PULSE_LIB OR NOT PULSE_INCLUDE)
    message(FATAL_ERROR "PulseAudio not found! Install with: sudo apt install libpulse-dev")
endif()

include_directories(${PULSE_INCLUDE})
add_executable(radio main.cpp ${SRC_FILES})
target_link_libraries(radio  ${PULSE_SIMPLE_LIB} ${PULSE_LIB} pthread)
