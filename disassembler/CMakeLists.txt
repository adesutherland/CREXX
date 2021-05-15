cmake_minimum_required(VERSION 3.00)

project(Disassembler C)

set(CMAKE_C_STANDARD 90)

add_executable(rxdas rxdasmain.c
        rxasdisa.h rxasdisa.c)

target_link_libraries(rxdas machine avl_tree)
target_include_directories(rxdas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_HOME_DIRECTORY}/assembler ${CMAKE_HOME_DIRECTORY}/machine)
