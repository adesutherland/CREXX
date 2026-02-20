cmake_minimum_required(VERSION 3.24)

project(Assembler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND $<TARGET_FILE:lemon> -c -d${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rxasgrmr.y
        DEPENDS rxasgrmr.y lemon
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxasgrmr.c ${CMAKE_CURRENT_BINARY_DIR}/rxasgrmr.h
        COMMENT "Create REXX ASSEMBLER Parser..."
)

add_custom_command(
        COMMAND $<TARGET_FILE:re2c> -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 -o ${CMAKE_CURRENT_BINARY_DIR}/rxasscan.c ${CMAKE_CURRENT_SOURCE_DIR}/rxasscan.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re ${CMAKE_CURRENT_SOURCE_DIR}/rxasscan.re ${CMAKE_CURRENT_BINARY_DIR}/rxasgrmr.h re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxasscan.c
        COMMENT "Create REXX ASSEMBLER Lexer..."
)

# RXAS Library
add_library(rxaslib STATIC rxastoke.c rxaseror.c ${CMAKE_CURRENT_BINARY_DIR}/rxasscan.c rxas_opt.c
        ${CMAKE_CURRENT_BINARY_DIR}/rxasgrmr.c ${CMAKE_CURRENT_BINARY_DIR}/rxasgrmr.h rxas.h
        rxasassm.h rxasassm.c
        ../avl_tree/rxastree.h
        ../utf8/utf.h ../binutils/include/rxbin.h rxaslib.c)
add_dependencies(rxaslib re2c lemon avl_tree platform)
target_include_directories(rxaslib PRIVATE . ${CMAKE_CURRENT_BINARY_DIR} ../platform ../avl_tree ../utf8 ../binutils/include)

# RXAS Exec
add_executable(rxas rxasmain.c)
target_link_libraries(rxas rxaslib avl_tree platform)
add_dependencies(rxas rxaslib avl_tree platform)
target_include_directories(rxas PRIVATE . ${CMAKE_CURRENT_BINARY_DIR} ../platform ../binutils/include)
