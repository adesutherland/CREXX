cmake_minimum_required(VERSION 3.24)

project(PLATFORM C)

set(CMAKE_C_STANDARD 90)

add_library(platform platform.h platform.c)
target_include_directories(platform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
