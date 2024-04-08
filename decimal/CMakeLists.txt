cmake_minimum_required(VERSION 3.10)

project(Decimal C)

set(CMAKE_C_STANDARD 90)

add_library(decimal decNumber.h decNumber.c decContext.h decContext.c)
target_include_directories(decimal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

