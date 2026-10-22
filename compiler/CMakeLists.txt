cmake_minimum_required(VERSION 3.24)

project(Compiler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND $<TARGET_FILE:lemon> -q -c -d${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rxcpopgr.y
        DEPENDS rxcpopgr.y lemon
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.h
        COMMENT "Create Options Parser..."
)

add_custom_command(
        COMMAND $<TARGET_FILE:re2c> -I ${CMAKE_CURRENT_SOURCE_DIR}/../platform -8 -o ${CMAKE_CURRENT_BINARY_DIR}/rxcposcn.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcposcn.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../platform/encoding.re rxcposcn.re ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.h re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcposcn.c
        COMMENT "Create Options Lexer..."
)

add_custom_command(
        COMMAND $<TARGET_FILE:lemon> -q -c -d${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbgmr.y
        COMMAND ${CMAKE_COMMAND} -DSHARED=${CMAKE_CURRENT_SOURCE_DIR}/rxcp_tokens.h
            -DGENERATED=${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.h
            -P ${CMAKE_CURRENT_SOURCE_DIR}/CheckSharedTokens.cmake
        DEPENDS rxcpbgmr.y rxcp_tokens.h CheckSharedTokens.cmake lemon
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.h
        COMMENT "Create REXX B Parser..."
)

add_custom_command(
        COMMAND $<TARGET_FILE:re2c> -I ${CMAKE_CURRENT_SOURCE_DIR}/../platform -8 -o ${CMAKE_CURRENT_BINARY_DIR}/rexbscan.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcpbscn.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../platform/encoding.re rxcpbscn.re ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.h re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rexbscan.c
        COMMENT "Create REXX B Lexer..."
)

add_custom_command(
        COMMAND $<TARGET_FILE:lemon> -q -c -d${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rxcpcgmr.y
        DEPENDS rxcpcgmr.y lemon
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcpcgmr.c ${CMAKE_CURRENT_BINARY_DIR}/rxcpcgmr.h
        COMMENT "Create REXX C Parser..."
)

add_custom_command(
        COMMAND $<TARGET_FILE:re2c> -I ${CMAKE_CURRENT_SOURCE_DIR}/../platform -8 -o ${CMAKE_CURRENT_BINARY_DIR}/rexcscan.c ${CMAKE_CURRENT_SOURCE_DIR}/rxcpcscn.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../platform/encoding.re rxcpcscn.re rxcp_tokens.h re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rexcscan.c
        COMMENT "Create REXX C Lexer..."
)

# Compiler Library - can be used to link plugins to make a extended compiler
set_source_files_properties(
        rxcp_inline_bind.c rxcp_inline_clone.c rxcp_inline_rewrite.c rxcp_inline_analysis.c rxcp_inline_payload.c
        PROPERTIES HEADER_FILE_ONLY TRUE
)

# Compile common services once. No backend target depends on a frontend.
set(RXCP_BACKEND_SOURCES
    rxcpmain.c
    rxcp_ast_core.c
    rxcp_ast_print.c
    rxcp_ast_walk.c
    rxcpmain.h
    rxcp_util.c
    rxcp_diag.c
    rxcp_diag.h
    rxcp_diag_fallback.c
    rxcp_source_tree.c
    rxcp_source_ext.c
    rxcp_source_ext.h
    rxcp_srcmap.c
    rxcp_srcmap.h
    ${CMAKE_CURRENT_BINARY_DIR}/rxcposcn.c
    ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.c
    ${CMAKE_CURRENT_BINARY_DIR}/rxcpopgr.h
    rxcpopar.c
    ${CMAKE_SOURCE_DIR}/assembler/rxas.h
    rxcp_val_sym.c
    rxcp_val_type.c
    rxcp_val_trans.c
    rxcp_ast_rewrite.c
    rxcp_val_check.c
    rxcp_val_plugin.c
    rxcp_val_orch.c
    rxcpsymb.c
    rxcp_fixup.c
    rxcp_ast_val.c
    rxcp_remap.c
    rxcp_remap.h
    rxcp_remap_build.c
    rxcp_remap_build.h
    rxcp_inline_internal.h
    rxcp_inline.c
    rxcp_inline_rules.c
    rxcp_inline_bind.c
    rxcp_inline_clone.c
    rxcp_inline_rewrite.c
    rxcp_inline_analysis.c
    rxcp_inline_payload.c
    rxcpdary.h
    rxcpemit.c
    rxcp_emit_core.c
    rxcp_emit_super.c
    rxcp_emit_reg.c
    rxcp_emit_expr.c
    rxcp_emit_flow.c
    rxcp_emit_proc.c
    rxcp_emit_meta.c
    rxcp_emit.h
    rxcp_opt.c
    rxcp_constant.h
    rxcp_partial_call.c
    rxcp_partial_call.h
    rxcp_flow.c
    rxcp_flow.h
    rxcp_dispatch.c
    rxcp_dispatch.h
    ../binutils/include/rxsignature.h
    ../platform/platform.h
    ../platform/platform.c
    ../platform/platform_tso.c
    ../platform/text_codec.c
    ../platform/platform_native.c
    ../platform/oom.c
    ../platform/rxinteger.h
    ../platform/rxinteger.c
    ../third_party/utf8/utf.h
    rxcpfunc.c
    rxcp_parse_cursor_decl.c
    rxcp_invoke_decl.c
    rxcp_import_report.c
    rxcp_import_report.h
    rxcp_project_dependencies.c
    rxcp_import_publish.c
    rxcp_import_publish.h
    rxcp_instr.c
    rxcp_contract.c
    rxcp_frontend.c
    rxcp_frontend.h
    rxcp_tokens.h
)
add_library(rxc_backend_objects OBJECT ${RXCP_BACKEND_SOURCES})
add_library(rxc_classic_objects OBJECT
    ${CMAKE_CURRENT_BINARY_DIR}/rexcscan.c
    ${CMAKE_CURRENT_BINARY_DIR}/rxcpcgmr.c
    rxcpcpar.c rxcpcdiag.c rxcpcsym.c rxcpcval.c rxcp_levelc_lower.c
    rxcp_frontend_c.c rxcp_c_source_import.c rxcc.c)
add_library(rxc_typed_objects OBJECT
    ${CMAKE_CURRENT_BINARY_DIR}/rexbscan.c
    ${CMAKE_CURRENT_BINARY_DIR}/rxcpbgmr.c
    rxcpbpar.c rxcp_task_lower.c rxcp_typed_orch.c rxcp_exit.c
    rxcp_parse_buffer.c rxcp_frontend_full.c
    rxcp_dsl.c rxcp_highlight_controller.c)

foreach(component IN ITEMS rxc_backend_objects rxc_classic_objects rxc_typed_objects)
    target_include_directories(${component} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/generated
        ${CMAKE_SOURCE_DIR}/assembler ${CMAKE_SOURCE_DIR}/third_party/avl_tree
        ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/third_party/utf8 ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/binutils/include)
    target_link_libraries(${component} PRIVATE rxaslib rxbvml rxbin)
endforeach()

# Existing compiler library/desktop editor keep their complete compatibility API.
add_library(rxclib STATIC $<TARGET_OBJECTS:rxc_backend_objects>
    $<TARGET_OBJECTS:rxc_classic_objects> $<TARGET_OBJECTS:rxc_typed_objects>)
get_target_property(_rxcp_include_dirs rxc_backend_objects INCLUDE_DIRECTORIES)
target_include_directories(rxclib PUBLIC ${_rxcp_include_dirs})
target_link_libraries(rxclib PUBLIC rxaslib rxbvml rxbin)
# Classic standalone/library closure has no typed parser, task mapper or exits.
add_library(rxcc_lib STATIC $<TARGET_OBJECTS:rxc_backend_objects>
    $<TARGET_OBJECTS:rxc_classic_objects>)
target_include_directories(rxcc_lib PUBLIC ${_rxcp_include_dirs})
target_link_libraries(rxcc_lib PUBLIC rxaslib rxbvml rxbin)

if(ENABLE_PARSER_MODE)
    target_link_libraries(rxc_typed_objects PRIVATE DSLSH::parser)
    target_compile_definitions(rxc_typed_objects PRIVATE ENABLE_PARSER_MODE)
    target_compile_definitions(rxc_backend_objects PRIVATE ENABLE_PARSER_MODE)
    target_link_libraries(rxclib PUBLIC DSLSH::parser)
endif()

add_executable(rxc rxc_main.c)
target_include_directories(rxc PRIVATE ${CMAKE_SOURCE_DIR}/platform)
target_link_libraries(rxc PRIVATE rxclib)
add_executable(rxc-c rxc_c_main.c)
target_include_directories(rxc-c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/platform)
target_link_libraries(rxc-c PRIVATE rxcc_lib)

add_subdirectory(tests)
if(BUILD_TESTING)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_executable(test_windows_argv tests/src/test_windows_argv.c)
target_include_directories(test_windows_argv PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME compiler_windows_argv COMMAND test_windows_argv)
set_tests_properties(compiler_windows_argv PROPERTIES
    LABELS "compiler;imports;smoke" TIMEOUT 30 CREXX_PREP_TARGETS "test_windows_argv")
add_executable(test_contract_builder tests/src/test_contract_builder.c)
target_include_directories(test_contract_builder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_contract_builder PRIVATE rxclib)
if(MSVC)
    target_compile_options(test_contract_builder PRIVATE /UNDEBUG)
else()
    target_compile_options(test_contract_builder PRIVATE -UNDEBUG)
endif()
add_test(NAME compiler_contract_builder COMMAND test_contract_builder)
set_tests_properties(compiler_contract_builder PROPERTIES
    LABELS "compiler;imports;smoke" TIMEOUT 60 CREXX_PREP_TARGETS "test_contract_builder")
add_executable(test_rxcc_frontend tests/src/test_rxcc_frontend.c)
target_include_directories(test_rxcc_frontend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_rxcc_frontend PRIVATE rxcc_lib)
add_test(NAME classic_frontend_service COMMAND test_rxcc_frontend)
set_tests_properties(classic_frontend_service PROPERTIES
    LABELS "compiler;levelc;smoke" TIMEOUT 60 CREXX_PREP_TARGETS "test_rxcc_frontend")
add_custom_target(qa-measure-classic-frontend
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/classic_frontend_parity.py
        --bindir ${CMAKE_BINARY_DIR}/bin --source ${CMAKE_SOURCE_DIR}
        --work ${CMAKE_CURRENT_BINARY_DIR}/classic-frontend-measure
    COMMAND $<TARGET_FILE:test_rxcc_frontend>
    COMMAND $<TARGET_FILE:test_contract_builder>
    COMMAND $<TARGET_FILE:test_rxcontract>
    DEPENDS rxc rxc-c rxas rxlink rxvm library classlib rxfnsc
        test_rxcc_frontend test_contract_builder test_rxcontract
    USES_TERMINAL)
add_test(NAME classic_frontend_parity
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/classic_frontend_parity.py
        --bindir ${CMAKE_BINARY_DIR}/bin --source ${CMAKE_SOURCE_DIR}
        --work ${CMAKE_CURRENT_BINARY_DIR}/classic-frontend-parity)
# Measured separately in Debug and maintained Apple ASan before registration.
# This launches compiler/assembler/linker/VM commands; serialize the aggregate.
set_tests_properties(classic_frontend_parity PROPERTIES
    LABELS "compiler;levelc;imports;comprehensive" RUN_SERIAL TRUE TIMEOUT 1800
    CREXX_PREP_TARGETS "rxc;rxc-c;rxas;rxlink;rxvm;library;classlib;rxfnsc")
# These tests belong to the compiler directory, outside tests/'s finalizer.
crexx_schedule_directory_qa_tier_finalization()
endif()
add_subdirectory(exits)

# Published products owned by this component.
crexx_install_targets(rxc rxc-c rxclib rxcc_lib)
crexx_install_bytecode(compiler_exit_bin "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin")
