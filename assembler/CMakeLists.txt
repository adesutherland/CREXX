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
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 -o rxasscan.c ${CMAKE_CURRENT_SOURCE_DIR}/rxasscan.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re ${CMAKE_CURRENT_SOURCE_DIR}/rxasscan.re ${CMAKE_CURRENT_SOURCE_DIR}/rxasgrmr.h re2c
        OUTPUT rxasscan.c
        COMMENT "Create REXX ASSEMBLER Lexer..."
)

add_executable(rxas rxasmain.c rxastoke.c rxaseror.c rxasscan.c
        rxasgrmr.c rxasgrmr.h rxas.h
        rxasassm.h rxasassm.c
        ../avl_tree/rxastree.h)

target_link_libraries(rxas machine avl_tree platform)

add_dependencies(rxas re2c lemon)

target_include_directories(rxas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../machine ../platform ../avl_tree)
