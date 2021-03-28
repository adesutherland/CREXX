cmake_minimum_required(VERSION 3.00)

project(Assembler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/lemon/lemon -c ${CMAKE_CURRENT_SOURCE_DIR}/rxasgrmr.y
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rxasgrmr.y lemon
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/rxasgrmr.c ${CMAKE_CURRENT_SOURCE_DIR}/rxasgrmr.h
        COMMENT "Create REXX ASSEMBLER Parser..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -o rxasscan.c ${CMAKE_CURRENT_SOURCE_DIR}/rxasscan.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rxasscan.re ${CMAKE_CURRENT_SOURCE_DIR}/rxasgrmr.h re2c
        OUTPUT rxasscan.c
        COMMENT "Create REXX ASSEMBLER Lexer..."
)

add_executable(rxas rxasmain.c rxastoke.c rxaseror.c rxasscan.c
               rxasgrmr.c rxasgrmr.h rxas.h operands.c operands.h avl_tree.c
               avl_tree.h rxasassm.h rxasassm.c rxasdisa.c strtree.h)
add_dependencies(rxas re2c lemon)
target_include_directories(rxas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
