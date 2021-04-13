cmake_minimum_required(VERSION 3.00)

project(AVL_TREE C)

set(CMAKE_C_STANDARD 90)

add_library(avl_tree avl_tree.h avl_tree.c)
target_include_directories(avl_tree PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
