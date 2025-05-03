FILE(GLOB_RECURSE SRC "*.h" "*.cpp" "*.tpp")
add_library(gameLife ${SRC})
target_include_directories(gameLife PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})