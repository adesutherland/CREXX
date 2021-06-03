cmake_minimum_required(VERSION 3.00)

project(Disassembler C)

set(CMAKE_C_STANDARD 90)

add_executable(rxdas rxdamain.c rxdadism.h rxdadism.c)

target_link_libraries(rxdas machine avl_tree platform)

target_include_directories(rxdas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../assembler ../machine ../platform)
