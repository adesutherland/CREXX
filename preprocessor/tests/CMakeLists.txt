cmake_minimum_required(VERSION 3.24)
project(preprocessor_tests C)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

# Register rxpp smoke tests here.
# This test runs the native rxpp precompiler on a tiny .rxpp input and validates
# the generated Rexx snippet contains expected lines.

enable_testing()

add_test(NAME rxpp_smoke
         COMMAND ${CMAKE_COMMAND}
             -DRXPP_BIN=$<TARGET_FILE:rxpp>
             -DRXC_BIN=$<TARGET_FILE:rxc>
             -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rxpp_smoke.rxpp
             -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/rxpp_smoke.out.crexx
             -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
             -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_smoke.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME rxpp_srcmap
         COMMAND ${CMAKE_COMMAND}
             -DRXPP_BIN=$<TARGET_FILE:rxpp>
             -DRXC_BIN=$<TARGET_FILE:rxc>
             -DINPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rxpp_srcmap.rxpp
             -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/rxpp_srcmap.out.crexx
             -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
             -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_srcmap.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if (ENABLE_PARSER_MODE)
    add_test(NAME rxpp_sh_srcmap
             COMMAND ${CMAKE_COMMAND}
                 -DRXPP_BIN=$<TARGET_FILE:rxpp>
                 -DRXPP_SH_BIN=$<TARGET_FILE:rxpp_sh>
                 -DPARSER_TESTER_BIN=$<TARGET_FILE:parser_tester>
                 -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
                 -DDUMP_AST_FILE=${CMAKE_SOURCE_DIR}/compiler/tests/highlighting/dump_ast.txt
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/rxpp_sh_srcmap
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_sh_srcmap.cmake
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(rxpp_sh_srcmap PROPERTIES
            LABELS "preprocessor;syntax_highlighting;srcmap"
            RESOURCE_LOCK syntax_highlighting_parser
            RUN_SERIAL TRUE)

    add_test(NAME rxpp_sh_default_lookup
             COMMAND ${CMAKE_COMMAND}
                 -DRXPP_BIN=$<TARGET_FILE:rxpp>
                 -DRXPP_SH_BIN=$<TARGET_FILE:rxpp_sh>
                 -DPARSER_TESTER_BIN=$<TARGET_FILE:parser_tester>
                 -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
                 -DDUMP_AST_FILE=${CMAKE_SOURCE_DIR}/compiler/tests/highlighting/dump_ast.txt
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/rxpp_sh_default_lookup
                 -DRXPP_SH_USE_ENV=OFF
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_sh_srcmap.cmake
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(rxpp_sh_default_lookup PROPERTIES
            LABELS "preprocessor;syntax_highlighting;srcmap"
            RESOURCE_LOCK syntax_highlighting_parser
            RUN_SERIAL TRUE)

    add_test(NAME rxpp_sh_tokens
             COMMAND ${CMAKE_COMMAND}
                 -DRXPP_BIN=$<TARGET_FILE:rxpp>
                 -DRXPP_SH_BIN=$<TARGET_FILE:rxpp_sh>
                 -DPARSER_TESTER_BIN=$<TARGET_FILE:parser_tester>
                 -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
                 -DDUMP_AST_FILE=${CMAKE_SOURCE_DIR}/compiler/tests/highlighting/dump_ast.txt
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/rxpp_sh_tokens
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_sh_tokens.cmake
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(rxpp_sh_tokens PROPERTIES
            LABELS "preprocessor;syntax_highlighting"
            RESOURCE_LOCK syntax_highlighting_parser
            RUN_SERIAL TRUE)
endif()

add_test(NAME rxpp_diagnostics
         COMMAND ${CMAKE_COMMAND}
             -DRXPP_BIN=$<TARGET_FILE:rxpp>
             -DWORK=${CMAKE_CURRENT_BINARY_DIR}/rxpp_diagnostics
             -DMACLIB_FILE=${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
             -DMESSAGE_PATH=${CMAKE_SOURCE_DIR}/messages
             -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_diagnostics.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxpp_diagnostics PROPERTIES LABELS "preprocessor;diagnostics")

add_test(NAME rxpp_diagnostic_catalogs
         COMMAND ${CMAKE_COMMAND}
             -DRXC_BIN=$<TARGET_FILE:rxc>
             -DRXAS_BIN=$<TARGET_FILE:rxas>
             -DRXVM_BIN=${CREXX_RXVM_EXECUTABLE}
             -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
             -DSOURCE_ROOT=${CMAKE_SOURCE_DIR}
             -DWORK=${CMAKE_CURRENT_BINARY_DIR}/rxpp_diagnostic_catalogs
             -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxpp_diagnostic_catalogs.cmake
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxpp_diagnostic_catalogs PROPERTIES LABELS "preprocessor;diagnostics")

crexx_label_smoke_tests(
        rxpp_smoke
        rxpp_srcmap
        rxpp_sh_tokens
        rxpp_diagnostics
)
