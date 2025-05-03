FILE(GLOB_RECURSE SRC "*.h" "*.cpp" "*.tpp")
add_library(extractFiles ${SRC})
target_include_directories(extractFiles PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})