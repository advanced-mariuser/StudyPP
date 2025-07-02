cmake_minimum_required(VERSION 3.15)
project(gaus_blur)

find_package(OpenCL REQUIRED)
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        ${OpenCV_INCLUDE_DIRS}
)

add_executable(blur
        BlurProcessor/ImageProcessor.h
        OpenCL/OpenCLManager.h
        OpenCL/OpenCLManager.cpp
        BlurProcessor/GaussianBlurProcessor.h
        BlurProcessor/GaussianBlurProcessor.cpp
        BlurProcessor/MotionBlurProcessor.h
        BlurProcessor/MotionBlurProcessor.cpp
        Application/Application.h
        Application/Application.cpp
        main.cpp)

target_compile_definitions(blur PRIVATE NOMINMAX)
target_link_libraries(blur PRIVATE OpenCL::OpenCL ${OpenCV_LIBS})

add_custom_command(TARGET blur POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/image_filters.cl"
        $<TARGET_FILE_DIR:blur>/image_filters.cl
        COMMENT "Copying image_filters.cl to build directory"
)

add_custom_command(TARGET blur POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/me.jpg"
        $<TARGET_FILE_DIR:blur>/me.jpg
        COMMENT "Copying me.jpg to build directory"
)