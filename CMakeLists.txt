cmake_minimum_required(VERSION 3.17)
project(crain C)

set(CMAKE_C_STANDARD 99)

add_executable(crain crain.c)

target_link_libraries(crain ncursesw)