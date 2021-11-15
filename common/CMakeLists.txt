cmake_minimum_required(VERSION 3.00)

project(COMMON C)

set(CMAKE_C_STANDARD 90)

add_library(common INTERFACE)
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})