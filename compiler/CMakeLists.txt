cmake_minimum_required(VERSION 3.17)

project(Compiler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/lemon/lemon -c ${CMAKE_CURRENT_SOURCE_DIR}/rxcpopgr.y
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rxcpopgr.y lemon
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/rxcpopgr.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcpopgr.h
        COMMENT "Create Options Parser..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 -o rxcposcn.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcposcn.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re ${CMAKE_CURRENT_SOURCE_DIR}/rxcposcn.re ${CMAKE_CURRENT_SOURCE_DIR}/rxcpopgr.h re2c
        OUTPUT rxcposcn.c
        COMMENT "Create Options Lexer..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/lemon/lemon -p -s -c ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbgmr.y || (exit 0)
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbgmr.y lemon
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbgmr.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbgmr.h
        COMMENT "Create REXX B Parser..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 -o rexbscan.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbscn.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbscn.re ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbgmr.h re2c
        OUTPUT rexbscan.c
        COMMENT "Create REXX B Lexer..."
)

add_executable(rxc rxcpmain.c rxcpast.c rxcpmain.h
               rxcposcn.c rxcpopgr.c rxcpopar.c rxcpopgr.h
               rexbscan.c rxcpbgmr.c rxcpbpar.c rxcpbgmr.h rxcpbval.c rxcpsymb.c
               rxcpdary.h rxcpemit.c
               ../platform/platform.h ../platform/platform.c
               ../utf8/utf.h)

add_dependencies(rxc re2c lemon)

target_link_libraries(rxc avl_tree platform)

target_include_directories(rxc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../avl_tree ../platform ../utf8)
