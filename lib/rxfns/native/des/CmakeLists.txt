cmake_minimum_required(VERSION 3.00)

project(des_external_function C)

set(CMAKE_C_STANDARD 90)

# Define the project's flags:
SET(${PROJECT_NAME}_C_FLAGS "-Wno-implicit-function-declaration -Wno-implicit-int -Wno-return-type" CACHE STRING
    "${PROJECT_NAME} C FLAGS")
# Enable the project's flags:
SET(CMAKE_C_FLAGS "${${PROJECT_NAME}_C_FLAGS}")

add_library(desbase SHARED desbase.c)
add_library(RxDes SHARED RxDes.c desbase)
