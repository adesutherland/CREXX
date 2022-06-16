cmake_minimum_required(VERSION 3.00)

project(Debugger C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -o rxdb ${CMAKE_CURRENT_SOURCE_DIR}/rxdb
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas rxdb
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o globals ${CMAKE_CURRENT_SOURCE_DIR}/globals
        COMMAND ${CMAKE_BINARY_DIR}/packager/rxpack rxdb globals ${CMAKE_BINARY_DIR}/lib/rxfns/library
        DEPENDS rxas rxc rxdb.rexx globals.rxas library rxpack
        OUTPUT rxdb.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble and package rxdb ..."
)

add_executable(rxdb rxdb.c)

target_link_libraries(rxdb rxvml machine avl_tree platform m)

add_dependencies(rxdb rxvml machine avl_tree platform)
