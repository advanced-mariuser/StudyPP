FILE(GLOB_RECURSE SRC "*.h" "*.cpp" "*.tpp")
add_library(makeArchive ${SRC})
target_include_directories(makeArchive PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})