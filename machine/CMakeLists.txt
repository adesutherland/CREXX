cmake_minimum_required(VERSION 3.00)

project(Machine C)

set(CMAKE_C_STANDARD 90)

add_library(machine STATIC operands.h operands.c instrset.h instrmiss.h)
