cmake_minimum_required(VERSION 3.24)

project(Compiler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND $<TARGET_FILE:lemon> -p -q -c -d${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rxcpopgr.y
        DEPENDS rxcpopgr.y lemon
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.h
        COMMENT "Create Options Parser..."
)

add_custom_command(
        COMMAND $<TARGET_FILE:re2c> -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 -o ${CMAKE_CURRENT_BINARY_DIR}/rxcposcn.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcposcn.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re rxcposcn.re ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.h re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcposcn.c
        COMMENT "Create Options Lexer..."
)

add_custom_command(
        COMMAND $<TARGET_FILE:lemon> -p -q -c -d${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbgmr.y
        DEPENDS rxcpbgmr.y lemon
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.h
        COMMENT "Create REXX B Parser..."
)

add_custom_command(
        COMMAND $<TARGET_FILE:re2c> -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 -o ${CMAKE_CURRENT_BINARY_DIR}/rexbscan.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbscn.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re rxcpbscn.re ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.h re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rexbscan.c
        COMMENT "Create REXX B Lexer..."
)

# Compiler Library - can be used to link plugins to make a extended compiler
add_library(rxclib STATIC
        rxcpmain.c rxcp_ast_core.c rxcp_ast_print.c rxcp_ast_walk.c rxcpmain.h rxcp_util.c
        ${CMAKE_CURRENT_BINARY_DIR}/rxcposcn.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.h rxcpopar.c
        ${CMAKE_CURRENT_BINARY_DIR}/rexbscan.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.h rxcpbpar.c
        ${CMAKE_SOURCE_DIR}/assembler/rxas.h
        rxcp_val_sym.c rxcp_val_type.c rxcp_val_trans.c rxcp_ast_rewrite.c rxcp_val_check.c rxcp_val_plugin.c rxcp_val_orch.c rxcpsymb.c rxcp_fixup.c rxcp_ast_val.c
        rxcpdary.h rxcpemit.c rxcp_emit_core.c rxcp_emit_reg.c rxcp_emit_expr.c rxcp_emit_flow.c rxcp_emit_proc.c rxcp_emit_meta.c rxcp_emit.h rxcp_opt.c
        ../platform/platform.h ../platform/platform.c
        ../utf8/utf.h rxcpfunc.c rxcp_exit.c
        rxcp_instr.c
)

add_dependencies(rxclib re2c lemon avl_tree platform rxaslib rxpa)

target_link_libraries(rxclib rxaslib rxbvml)

target_include_directories(rxclib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/assembler ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/binutils/include)

add_executable(rxc rxc_main.c)
target_include_directories(rxc PRIVATE ${CMAKE_SOURCE_DIR}/platform)
# Link only what is not already linked transitively by rxclib to reduce duplicate library warnings
# rxclib already links: avl_tree, platform, rxaslib, rxpa, rxbvml, m
# rxc additionally needs the VM plugin framework and decimal plugin
target_link_libraries(rxc PRIVATE rxclib)

add_subdirectory(tests)
add_subdirectory(exits)
