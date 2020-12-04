cmake_minimum_required(VERSION 3.17)
project(cRain C)

set(CMAKE_C_STANDARD 99)

add_executable(cRain cRain.c)

target_link_libraries(cRain ncursesw)