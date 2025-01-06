cmake_minimum_required(VERSION 3.24)

project(Debugger C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i ${CMAKE_BINARY_DIR}/lib/rxfnsb -o rxdb ${CMAKE_CURRENT_SOURCE_DIR}/rxdb
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas rxdb
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o globals ${CMAKE_CURRENT_SOURCE_DIR}/globals
        COMMAND ${CMAKE_BINARY_DIR}/cpacker/rxcpack rxdb globals ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        DEPENDS rxas rxc rxdb.rexx globals.rxas library rxcpack
        OUTPUT rxdb.c
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rxdb.rexx ${CMAKE_CURRENT_SOURCE_DIR}/globals.rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble and package rxdb ..."
)

add_executable(rxdb rxdb.c)

target_link_libraries(rxdb rxvml machine avl_tree rxpa platform m)

add_dependencies(rxdb rxc rxas rxcpack library)
