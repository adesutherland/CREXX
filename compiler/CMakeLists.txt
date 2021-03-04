cmake_minimum_required(VERSION 3.17)

project(Compiler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/lemon/lemon -c ${CMAKE_CURRENT_SOURCE_DIR}/rexxgrmr.y
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rexxgrmr.y lemon
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/rexxgrmr.c ${CMAKE_CURRENT_SOURCE_DIR}/rexxgrmr.h
        COMMENT "Create REXX Parser..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -o rexx.c ${CMAKE_CURRENT_SOURCE_DIR}/rexx.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rexx.re ${CMAKE_CURRENT_SOURCE_DIR}/rexxgrmr.h re2c
        OUTPUT rexx.c
        COMMENT "Create REXX Lexer..."
)

add_executable(rexx compiler.c ast.c rexx.c rexxgrmr.c rexxgrmr.h compiler.h)
add_dependencies(rexx re2c lemon)
target_include_directories(rexx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
