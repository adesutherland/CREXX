cmake_minimum_required(VERSION 3.24)

project(cpacker C)

set(CMAKE_C_STANDARD 90)

add_executable(rxcpack rxcpack.c)
target_include_directories(rxcpack PRIVATE ${CMAKE_SOURCE_DIR}/platform)
target_link_libraries(rxcpack platform)
add_dependencies(rxcpack platform)
