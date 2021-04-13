cmake_minimum_required(VERSION 3.00)

project(Machine C)

set(CMAKE_C_STANDARD 90)

add_library(machine STATIC operands.h operands.c)
#target_include_directories(machine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
