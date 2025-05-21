cmake_minimum_required(VERSION 3.10)
project(Simple2DRPG CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(rpg main.cpp)
target_link_libraries(rpg PRIVATE ncurses)
