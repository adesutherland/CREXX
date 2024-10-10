cmake_minimum_required(VERSION 3.24)

project(Compiler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/lemon/lemon -p -s -c -d${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rxcpopgr.y
        DEPENDS rxcpopgr.y lemon
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.h
        COMMENT "Create Options Parser..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 -o ${CMAKE_CURRENT_BINARY_DIR}/rxcposcn.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcposcn.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re rxcposcn.re ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.h re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcposcn.c
        COMMENT "Create Options Lexer..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/lemon/lemon -p -s -c -d${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbgmr.y
        DEPENDS rxcpbgmr.y lemon
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.h
        COMMENT "Create REXX B Parser..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 -o ${CMAKE_CURRENT_BINARY_DIR}/rexbscan.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbscn.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re rxcpbscn.re ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.h re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rexbscan.c
        COMMENT "Create REXX B Lexer..."
)

# Compiler Library - can be used to link plugins to make a extended compiler
add_library(rxclib STATIC
        rxcpmain.c rxcpast.c rxcpmain.h
        ${CMAKE_CURRENT_BINARY_DIR}/rxcposcn.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.h rxcpopar.c
        ${CMAKE_CURRENT_BINARY_DIR}/rexbscan.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.h rxcpbpar.c
        ${CMAKE_SOURCE_DIR}/assembler/rxas.h
        rxcpbval.c rxcpsymb.c
        rxcpdary.h rxcpemit.c rxcp_opt.c
        ../platform/platform.h ../platform/platform.c
        ../utf8/utf.h rxcpfunc.c
)

add_dependencies(rxclib re2c lemon machine avl_tree platform rxaslib rxpa)

target_link_libraries(rxclib machine avl_tree platform rxaslib rxpa m)

target_include_directories(rxclib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/assembler ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/machine
        ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa)

add_executable(rxc rxc_main.c)

add_dependencies(rxc rxclib)

target_link_libraries(rxc rxclib machine avl_tree platform rxaslib rxpa m)
