cmake_minimum_required(VERSION 3.24)

project(driver C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i ${CMAKE_BINARY_DIR}/lib/rxfns -o crexx ${CMAKE_CURRENT_SOURCE_DIR}/crexx
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas crexx
#        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o globals ${CMAKE_CURRENT_SOURCE_DIR}/globals
        COMMAND ${CMAKE_BINARY_DIR}/cpacker/rxcpack crexx ${CMAKE_BINARY_DIR}/lib/rxfns/library
        DEPENDS rxas rxc crexx.rexx library rxcpack rxpa
        OUTPUT crexx.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble and package crexx ..."
)

add_executable(crexx crexx.c)

target_link_libraries(crexx rxvml rxpa machine avl_tree platform m)

add_dependencies(crexx rxvml machine rxpa avl_tree platform)
