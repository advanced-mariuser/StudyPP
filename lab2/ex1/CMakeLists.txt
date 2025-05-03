cmake_minimum_required(VERSION 3.8)
project(game-life)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML 2 COMPONENTS audio window graphics system REQUIRED)

add_subdirectory(src)

add_executable(game-life main.cpp)
target_link_libraries(game-life PRIVATE gameLife sfml-graphics sfml-window sfml-system)
