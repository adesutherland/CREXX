cmake_minimum_required(VERSION 3.10)

project(ICU C)

set(CMAKE_C_STANDARD 90)

add_library(icu decNumber.h decNumber.c decContext.h decContext.c)
target_include_directories(icu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

