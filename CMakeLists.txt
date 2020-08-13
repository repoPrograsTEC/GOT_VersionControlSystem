cmake_minimum_required(VERSION 3.16)
project(prueba)

set(CMAKE_CXX_STANDARD 20)

add_executable(prueba main.cpp Huffmann.cpp)

target_link_libraries(prueba curlpp curl )
