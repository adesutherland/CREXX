cmake_minimum_required(VERSION 3.16)
project(Lemon C)

set(CMAKE_C_STANDARD 90)

enable_testing()

add_executable(lemon lemon.c)

add_custom_command(
        TARGET lemon PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/lempar.c
        ${CMAKE_CURRENT_BINARY_DIR}/lempar.c
        COMMENT "Create Lemon Parser Generator ..."
)
