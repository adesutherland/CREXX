cmake_minimum_required(VERSION 3.24)

project(CompilerTests C)

find_package(Threads REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxConcurrencyTests.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(test_symbol_allocation test_symbol_allocation.c)
get_target_property(symbol_test_includes rxclib INCLUDE_DIRECTORIES)
target_include_directories(test_symbol_allocation PRIVATE ${symbol_test_includes})
target_link_libraries(test_symbol_allocation PRIVATE rxclib)
add_test(NAME symbol_allocation_failure COMMAND test_symbol_allocation)
set_tests_properties(symbol_allocation_failure PROPERTIES
        LABELS "unit;compiler;allocation" TIMEOUT 30 CREXX_PREP_TARGETS test_symbol_allocation)

add_executable(test_ast_allocation test_ast_allocation.c)
target_include_directories(test_ast_allocation PRIVATE ${symbol_test_includes})
target_link_libraries(test_ast_allocation PRIVATE rxclib)
add_test(NAME ast_allocation_failure COMMAND test_ast_allocation)
set_tests_properties(ast_allocation_failure PROPERTIES
        LABELS "unit;compiler;allocation" TIMEOUT 30 CREXX_PREP_TARGETS test_ast_allocation)

add_executable(test_frame_control_ast test_frame_control_ast.c)
target_include_directories(test_frame_control_ast PRIVATE ${symbol_test_includes})
target_link_libraries(test_frame_control_ast PRIVATE rxclib)
add_test(NAME frame_control_ast COMMAND test_frame_control_ast)
set_tests_properties(frame_control_ast PROPERTIES
        LABELS "unit;compiler;levelc;control_flow" TIMEOUT 30
        CREXX_PREP_TARGETS test_frame_control_ast)

add_executable(test_import_directory_errors test_import_directory_errors.c)
target_include_directories(test_import_directory_errors PRIVATE ${symbol_test_includes})
target_link_libraries(test_import_directory_errors PRIVATE rxclib)
add_test(NAME import_directory_errors COMMAND test_import_directory_errors)
set_tests_properties(import_directory_errors PROPERTIES
    LABELS "unit;compiler;imports" TIMEOUT 30 CREXX_PREP_TARGETS test_import_directory_errors)

# The test driver itself must be compiled as strict C90
add_executable(crexx_test_driver src/test_driver.c)

add_executable(mutate_rxbin_feature mutate_rxbin_feature.c)

if(UNIX)
    add_executable(test_import_discovery_snapshot test_import_discovery_snapshot.c)
    target_link_libraries(test_import_discovery_snapshot PRIVATE rxclib Threads::Threads)
    target_include_directories(test_import_discovery_snapshot PRIVATE ${CMAKE_SOURCE_DIR}/compiler)
    add_test(NAME import_discovery_snapshot COMMAND test_import_discovery_snapshot)
    set_tests_properties(import_discovery_snapshot PROPERTIES LABELS "compiler;imports;concurrency")
    crexx_label_concurrency_tests(SP01 import_discovery_snapshot)
endif()

# Ensure the driver is built when tests are built
add_dependencies(crexx_test_driver rxc)

if (ENABLE_PARSER_MODE)
    add_executable(crexx_fake_editor src/fake_editor.c)
    target_link_libraries(crexx_fake_editor DSLSH::editor)

    set(RXC_HELP_SYNTAXHIGHLIGHT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_help_syntaxhighlight.cmake")
    file(GENERATE OUTPUT "${RXC_HELP_SYNTAXHIGHLIGHT_SCRIPT}" CONTENT
"execute_process(
    COMMAND \"$<TARGET_FILE:rxc>\" -h
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res
)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"rxc -h failed with exit code \${res}.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")
endif()
if(NOT out MATCHES \"--syntaxhighlight\")
  message(FATAL_ERROR \"rxc help does not advertise --syntaxhighlight.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")
endif()
if(out MATCHES \"--parser\")
  message(FATAL_ERROR \"rxc help still advertises --parser instead of --syntaxhighlight.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")
endif()
execute_process(
    COMMAND \"$<TARGET_FILE:rxc>\" --help
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
    OUTPUT_VARIABLE long_out
    ERROR_VARIABLE long_err
    RESULT_VARIABLE long_res
)
if(NOT long_res EQUAL 0)
  message(FATAL_ERROR \"rxc --help failed with exit code \${long_res}.\\nSTDOUT:\\n\${long_out}\\nSTDERR:\\n\${long_err}\")
endif()
if(NOT long_out MATCHES \"--syntaxhighlight\")
  message(FATAL_ERROR \"rxc --help does not advertise --syntaxhighlight.\\nSTDOUT:\\n\${long_out}\\nSTDERR:\\n\${long_err}\")
endif()
if(long_out MATCHES \"--parser\")
  message(FATAL_ERROR \"rxc --help still advertises --parser instead of --syntaxhighlight.\\nSTDOUT:\\n\${long_out}\\nSTDERR:\\n\${long_err}\")
endif()
")
    add_test(NAME rxc_help_syntaxhighlight
             COMMAND ${CMAKE_COMMAND} -P "${RXC_HELP_SYNTAXHIGHLIGHT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(rxc_help_syntaxhighlight PROPERTIES LABELS "compiler;syntax_highlighting")

    function(add_crexx_syntax_highlight_test NAME SOURCE_FILE)
        set(options)
        set(oneValueArgs)
        set(multiValueArgs MUST_MATCH MUST_NOT_MATCH)
        cmake_parse_arguments(HL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
        set(SCRIPT_CONTENT "")

        string(APPEND SCRIPT_CONTENT
               "execute_process(\n"
               "    COMMAND \"$<TARGET_FILE:parser_tester>\" -q -p \"$<TARGET_FILE:rxc>\" -s \"${SOURCE_FILE}\" \"${CMAKE_CURRENT_SOURCE_DIR}/highlighting/dump_ast.txt\"\n"
               "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
               "    OUTPUT_VARIABLE out\n"
               "    ERROR_VARIABLE err\n"
               "    RESULT_VARIABLE res\n"
               ")\n")
        string(APPEND SCRIPT_CONTENT
               "if(NOT res EQUAL 0)\n"
               "  message(FATAL_ERROR \"parser_tester failed with exit code \${res}\\nSTDERR:\\n\${err}\\nSTDOUT:\\n\${out}\")\n"
               "endif()\n")

        foreach(_pattern IN LISTS HL_MUST_MATCH)
            string(APPEND SCRIPT_CONTENT
                   "if(NOT out MATCHES [=[${_pattern}]=])\n"
                   "  message(FATAL_ERROR \"Missing expected syntax-highlighting assertion.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")\n"
                   "endif()\n")
        endforeach()

        foreach(_pattern IN LISTS HL_MUST_NOT_MATCH)
            string(APPEND SCRIPT_CONTENT
                   "if(out MATCHES [=[${_pattern}]=])\n"
                   "  message(FATAL_ERROR \"Found forbidden syntax-highlighting pattern.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")\n"
                   "endif()\n")
        endforeach()

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        set_tests_properties(${NAME} PROPERTIES
                LABELS "compiler;syntax_highlighting;stage0"
                # The fake editor takes an asynchronous parser snapshot. Keep
                # the existing parser lock and also isolate that snapshot from
                # compiler/runtime tests which launch CPU-heavy child processes.
                RESOURCE_LOCK syntax_highlighting_parser
                RUN_SERIAL TRUE)
    endfunction()
endif()

enable_testing()
include(${CMAKE_SOURCE_DIR}/cmake/CrexxLinkedRuntime.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

add_test(NAME rxc_help_no_exe_import COMMAND $<TARGET_FILE:rxc> -h)
set_tests_properties(rxc_help_no_exe_import PROPERTIES
        PASS_REGULAR_EXPRESSION "--no-exe-import"
        LABELS "compiler;imports")

add_test(NAME rxc_help_import_resolution_report COMMAND $<TARGET_FILE:rxc> -h)
set_tests_properties(rxc_help_import_resolution_report PROPERTIES
        PASS_REGULAR_EXPRESSION "--import-resolution-report"
        LABELS "compiler;imports;newbuild")

add_test(NAME source_root_namespace_order
    COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc> -DRXAS=$<TARGET_FILE:rxas>
        -DRXBVM=$<TARGET_FILE:rxbvm>
        -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/source_root_namespace_order
        -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/source_root_namespace_order_ws
        -P ${CMAKE_CURRENT_SOURCE_DIR}/source_root_namespace_order.cmake)
set_tests_properties(source_root_namespace_order PROPERTIES
    LABELS "compiler;imports;regression"
    RUN_SERIAL TRUE TIMEOUT 300
    CREXX_PREP_TARGETS "rxc;rxas;rxbvm")

find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(compiler_load_failure_command ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler_load_failures.py
    --rxc $<TARGET_FILE:rxc> --rxas $<TARGET_FILE:rxas> --vm $<TARGET_FILE:rxbvm>
    --library ${CMAKE_BINARY_DIR}/bin/library.rxbin
    --exits ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
    --work ${CMAKE_CURRENT_BINARY_DIR}/compiler-load-failures)
add_custom_target(check_compiler_load_failures COMMAND ${compiler_load_failure_command}
    DEPENDS rxc rxas rxbvm compiler_exit_bin library VERBATIM)
add_test(NAME compiler_load_failures COMMAND ${compiler_load_failure_command})
set_tests_properties(compiler_load_failures PROPERTIES
    LABELS "compiler;imports;regression" RUN_SERIAL TRUE TIMEOUT 300
    CREXX_PREP_TARGETS "rxc;rxas;rxbvm;compiler_exit_bin;library")

add_executable(test_project_dependencies test_project_dependencies.c)
target_link_libraries(test_project_dependencies PRIVATE rxclib)
target_include_directories(test_project_dependencies PRIVATE ${CMAKE_SOURCE_DIR}/compiler)
add_test(NAME project_dependencies COMMAND test_project_dependencies)
set_tests_properties(project_dependencies PROPERTIES
        LABELS "compiler;imports;incremental;regression"
        CREXX_PREP_TARGETS "test_project_dependencies")

add_test(NAME binary_forward_dependencies
    COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc> -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink> -DRXVME=$<TARGET_FILE:rxvme>
        -DBIN=${CMAKE_BINARY_DIR}/bin
        -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/binary_forward_dependencies_ws
        -P ${CMAKE_CURRENT_SOURCE_DIR}/binary_forward_dependencies.cmake)
set_tests_properties(binary_forward_dependencies PROPERTIES
    LABELS "compiler;imports;incremental;regression"
    CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvme;library;classlib")

foreach(kind IN ITEMS convergence provider)
    add_test(NAME source_import_private_dependency_${kind}
        COMMAND ${CMAKE_COMMAND}
            -DRXC=$<TARGET_FILE:rxc> -DRXAS=$<TARGET_FILE:rxas>
            -DBIN=${CMAKE_BINARY_DIR}/bin
            -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/fixtures/issue699
            -DKIND=${kind}
            -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/source_import_private_dependency_${kind}_ws
            -P ${CMAKE_CURRENT_SOURCE_DIR}/source_import_private_dependency.cmake)
    set_tests_properties(source_import_private_dependency_${kind} PROPERTIES
        LABELS "compiler;imports;inline;incremental;regression;smoke"
        CREXX_PREP_TARGETS "rxc;rxas;library;classlib"
        # Each test runs an isolated opt/noopt import-control matrix.
        RUN_SERIAL TRUE TIMEOUT 300)
endforeach()

add_test(NAME inline_return_conversions
    COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc> -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXBVM=$<TARGET_FILE:rxbvm> -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBIN=${CMAKE_BINARY_DIR}/bin
        -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_return_conversions.crexx
        -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/inline_return_conversions_ws
        -P ${CMAKE_CURRENT_SOURCE_DIR}/inline_return_conversions.cmake)
set_tests_properties(inline_return_conversions PROPERTIES
    LABELS "compiler;imports;inline;regression"
    CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxbvm;rxvm;library;classlib"
    TIMEOUT 600)

add_test(NAME global_import_types
    COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc> -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink> -DRXVME=$<TARGET_FILE:rxvme>
        -DBIN=${CMAKE_BINARY_DIR}/bin
        -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/global_import_types.crexx
        -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/global_import_types_ws
        -P ${CMAKE_CURRENT_SOURCE_DIR}/global_import_types.cmake)
set_tests_properties(global_import_types PROPERTIES
    LABELS "compiler;imports;regression"
    CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvme;library;classlib"
    TIMEOUT 240)

add_test(NAME binary_global_import_types
    COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc> -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXBVM=$<TARGET_FILE:rxbvm> -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBIN=${CMAKE_BINARY_DIR}/bin
        -DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_global_import_types.crexx
        -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/binary_global_import_types_ws
        -P ${CMAKE_CURRENT_SOURCE_DIR}/binary_global_import_types.cmake)
set_tests_properties(binary_global_import_types PROPERTIES
    LABELS "compiler;imports;regression"
    CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxbvm;rxvm;library;classlib"
    # This matrix launches many complete compiler/assembler/linker/VM paths.
    # Running it beside other aggregate tests can exhaust its outer timeout
    # even though the unchanged matrix passes comfortably in isolation.
    RUN_SERIAL TRUE
    TIMEOUT 900)

add_test(NAME binary_storage_selector_dependencies
    COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc> -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink> -DRXVME=$<TARGET_FILE:rxvme>
        -DBIN=${CMAKE_BINARY_DIR}/bin
        -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/binary_storage_selector_dependencies_ws
        -P ${CMAKE_CURRENT_SOURCE_DIR}/binary_storage_selector_dependencies.cmake)
set_tests_properties(binary_storage_selector_dependencies PROPERTIES
    LABELS "compiler;imports;incremental;binary;regression"
    CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvme;library;classlib;compiler_exit_bin")

add_executable(test_import_resolution_report test_import_resolution_report.c)
target_link_libraries(test_import_resolution_report PRIVATE rxclib)
target_include_directories(test_import_resolution_report PRIVATE ${CMAKE_SOURCE_DIR}/compiler)
add_test(NAME import_resolution_report COMMAND test_import_resolution_report)
set_tests_properties(import_resolution_report PROPERTIES LABELS "compiler;imports;newbuild;stage0")

set(RXC_IMPORT_RESOLUTION_REPORT_CLI_SCRIPT
        "${CMAKE_CURRENT_BINARY_DIR}/rxc_import_resolution_report_cli.cmake")
file(GENERATE OUTPUT "${RXC_IMPORT_RESOLUTION_REPORT_CLI_SCRIPT}" CONTENT
"set(report \"${CMAKE_CURRENT_BINARY_DIR}/rxc-import-resolution-report-cli.json\")
file(REMOVE \"\${report}\" \"\${report}.tmp\")
execute_process(
    COMMAND \"$<TARGET_FILE:rxc>\"
            -x
            --no-exe-import
            --import-resolution-report \"\${report}\"
            -o \"${CMAKE_CURRENT_BINARY_DIR}/rxc-import-resolution-report-cli\"
            \"${CMAKE_CURRENT_SOURCE_DIR}/import_resolution_fixture/consumer.crexx\"
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res
)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"rxc report fixture failed with exit code \${res}.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")
endif()
file(READ \"\${report}\" json)
string(JSON schema_version GET \"\${json}\" schema_version)
string(JSON report_status GET \"\${json}\" status)
string(JSON executable_directory GET \"\${json}\" options executable_directory)
string(JSON provider_count LENGTH \"\${json}\" provider_bindings)
if(NOT schema_version STREQUAL \"crexx.import-resolution-report/v1\")
  message(FATAL_ERROR \"Unexpected import report schema: \${schema_version}\")
endif()
if(NOT report_status STREQUAL \"observed\")
  message(FATAL_ERROR \"Unexpected import report status: \${report_status}\")
endif()
if(NOT executable_directory STREQUAL \"excluded\")
  message(FATAL_ERROR \"--no-exe-import was not represented in the report\")
endif()
if(NOT provider_count EQUAL 0)
  message(FATAL_ERROR \"Observe-only fixture unexpectedly emitted final provider bindings\")
endif()
if(EXISTS \"\${report}.tmp\")
  message(FATAL_ERROR \"Import report temporary sibling remained after publication\")
endif()
file(REMOVE \"\${report}\" \"\${report}.tmp\"
            \"${CMAKE_CURRENT_BINARY_DIR}/rxc-import-resolution-report-cli.rxas\")
")
add_test(NAME rxc_import_resolution_report_cli
         COMMAND ${CMAKE_COMMAND} -P "${RXC_IMPORT_RESOLUTION_REPORT_CLI_SCRIPT}")
set_tests_properties(rxc_import_resolution_report_cli PROPERTIES
        LABELS "compiler;imports;newbuild;stage0")

add_executable(test_rxcp_super_combiner test_rxcp_super_combiner.c)
target_link_libraries(test_rxcp_super_combiner PRIVATE rxclib)
target_include_directories(test_rxcp_super_combiner PRIVATE ${CMAKE_SOURCE_DIR}/compiler)
add_test(NAME rxcp_super_combiner COMMAND test_rxcp_super_combiner)
set_tests_properties(rxcp_super_combiner PROPERTIES LABELS "compiler;performance;nr09")

# Compiler flags for optimization levels
# Based on rxcpmain.c: -n is for No Optimising, and it's on by default.
set(COMPILER_NOOPT_FLAG "-n")
set(COMPILER_OPT_FLAG "") # Default is optimized

# Macro to add a compiler test
# NAME: Name of the test
# SOURCE_FILE: Path to the Rexx source file (relative to this directory)
macro(add_crexx_compiler_test NAME SOURCE_FILE)
    # Test 1 (No-Opt)
    add_test(NAME ${NAME}_noopt
             COMMAND crexx_test_driver
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/noopt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_noopt.rxas
             $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin ${COMPILER_NOOPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_noopt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})

    # Test 2 (Opt)
    add_test(NAME ${NAME}_opt
             COMMAND crexx_test_driver
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/opt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt.rxas
             $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin ${COMPILER_OPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

# Macro to add a compiler warning test
macro(add_crexx_compiler_warning_test NAME SOURCE_FILE GOLDEN_FILE)
    add_test(NAME ${NAME}_warn
             COMMAND crexx_test_driver --warnings
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/warnings/${GOLDEN_FILE}
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_warn.txt
             $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

# Macro to add a compiler failure test
macro(add_crexx_compiler_fail_test NAME SOURCE_FILE GOLDEN_FILE)
    add_test(NAME ${NAME}_fail
             COMMAND crexx_test_driver --expect-fail
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/${GOLDEN_FILE}
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_fail.txt
             $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

# Macro to add a compiler test with one dependency
# NAME: Name of the test
# SOURCE_FILE: Path to the Rexx source file
# DEP_FILE: Path to the dependency file
macro(add_crexx_compiler_dep_test NAME SOURCE_FILE DEP_FILE)
    # Test 1 (No-Opt)
    add_test(NAME ${NAME}_noopt
             COMMAND crexx_test_driver --copy ${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE}
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/noopt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_noopt.rxas
             $<TARGET_FILE:rxc> ${COMPILER_NOOPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_noopt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})

    # Test 2 (Opt)
    add_test(NAME ${NAME}_opt
             COMMAND crexx_test_driver --copy ${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE}
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/opt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt.rxas
             $<TARGET_FILE:rxc> ${COMPILER_OPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

# Macro to add a parsing test
macro(add_crexx_parsing_test NAME SOURCE_FILE)
    add_test(NAME ${NAME}_parse
             COMMAND crexx_test_driver --ast
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/parsing/${NAME}.txt
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ast.txt
             $<TARGET_FILE:rxc> -dp ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
endmacro()

set(RXAS_DIAG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_diagnostic.cmake")
set(RXAS_DIAG_CONTENT "")
set(RXAS_DIAG_DOLLAR "$")
string(APPEND RXAS_DIAG_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_diagnostic.rxas\" \"main() .locals=1\\n   bogus r0\\n\")\n")
string(APPEND RXAS_DIAG_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"rxas_invalid_diagnostic\" \"${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_diagnostic.rxas\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n")
string(APPEND RXAS_DIAG_CONTENT
       "set(full_out \"${RXAS_DIAG_DOLLAR}{out}${RXAS_DIAG_DOLLAR}{err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxas to fail on invalid mnemonic, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"invalid instruction mnemonic\")\n"
       "  message(FATAL_ERROR \"Expected rxas diagnostic was not emitted.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXAS_DIAG_SCRIPT}" CONTENT "${RXAS_DIAG_CONTENT}")

add_test(NAME rxas_invalid_diagnostic
         COMMAND ${CMAKE_COMMAND} -P "${RXAS_DIAG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME rxas_exposed_global_bounds
         COMMAND ${CMAKE_COMMAND}
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/rxas_exposed_global_bounds_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/rxas_exposed_global_bounds.cmake)
set_tests_properties(rxas_exposed_global_bounds PROPERTIES
        LABELS "rxas;assembler;negative;regression"
        CREXX_PREP_TARGETS "rxas")

set(RXAS_INVALID_UTF8_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_utf8_string.cmake")
set(RXAS_INVALID_UTF8_CONTENT "")
string(APPEND RXAS_INVALID_UTF8_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_utf8_string.rxas\" \"main() .locals=1\\n   load r0,\\\"\\\\xff\\\"\\n\")\n")
string(APPEND RXAS_INVALID_UTF8_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"rxas_invalid_utf8_string\" \"${CMAKE_CURRENT_BINARY_DIR}/rxas_invalid_utf8_string.rxas\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n")
string(APPEND RXAS_INVALID_UTF8_CONTENT
       "set(full_out \"${RXAS_DIAG_DOLLAR}{out}${RXAS_DIAG_DOLLAR}{err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxas to fail on invalid UTF-8 string, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"string constant is not valid UTF-8\")\n"
       "  message(FATAL_ERROR \"Expected invalid UTF-8 diagnostic was not emitted.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXAS_INVALID_UTF8_SCRIPT}" CONTENT "${RXAS_INVALID_UTF8_CONTENT}")

add_test(NAME rxas_invalid_utf8_string
         COMMAND ${CMAKE_COMMAND} -P "${RXAS_INVALID_UTF8_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_INVALID_UTF8_HEX_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_hex_literal.cmake")
set(RXC_INVALID_UTF8_HEX_CONTENT "")
string(APPEND RXC_INVALID_UTF8_HEX_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_hex_literal.crexx\" \"options levelb\\n\\nmain: procedure\\n  say 'FFFFFF'x\\n\")\n")
string(APPEND RXC_INVALID_UTF8_HEX_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -x -o \"rxc_invalid_utf8_hex_literal\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_hex_literal.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n")
string(APPEND RXC_INVALID_UTF8_HEX_CONTENT
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject invalid UTF-8 hex literal, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"CANNOT_CAST_BINARY\")\n"
       "  message(FATAL_ERROR \"Expected binary-to-text diagnostic was not emitted for source hex literal.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_UTF8_HEX_SCRIPT}" CONTENT "${RXC_INVALID_UTF8_HEX_CONTENT}")

add_test(NAME rxc_invalid_utf8_hex_literal
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_UTF8_HEX_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_INVALID_UTF8_BINARY_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_binary_literal.cmake")
set(RXC_INVALID_UTF8_BINARY_CONTENT "")
string(APPEND RXC_INVALID_UTF8_BINARY_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_binary_literal.crexx\" \"options levelb\\n\\nmain: procedure\\n  say '11111111'b\\n\")\n")
string(APPEND RXC_INVALID_UTF8_BINARY_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -x -o \"rxc_invalid_utf8_binary_literal\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_utf8_binary_literal.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n")
string(APPEND RXC_INVALID_UTF8_BINARY_CONTENT
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject invalid UTF-8 binary literal, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"CANNOT_CAST_BINARY\")\n"
       "  message(FATAL_ERROR \"Expected binary-to-text diagnostic was not emitted for source binary literal.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_UTF8_BINARY_SCRIPT}" CONTENT "${RXC_INVALID_UTF8_BINARY_CONTENT}")

add_test(NAME rxc_invalid_utf8_binary_literal
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_UTF8_BINARY_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_INVALID_BINARY_ASSIGN_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_assign_literal.cmake")
set(RXC_INVALID_BINARY_ASSIGN_CONTENT "")
string(APPEND RXC_INVALID_BINARY_ASSIGN_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_assign_literal.crexx\" \"options levelb\\n\\nmain: procedure = .int\\n  x = 'ffff'x\\n  return 0\\n\")\n")
string(APPEND RXC_INVALID_BINARY_ASSIGN_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -x -o \"rxc_invalid_binary_assign_literal\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_assign_literal.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject untyped invalid UTF-8 byte literal assignment, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"CANNOT_CAST_BINARY\")\n"
       "  message(FATAL_ERROR \"Expected binary-to-text diagnostic was not emitted for untyped invalid byte literal assignment.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_BINARY_ASSIGN_SCRIPT}" CONTENT "${RXC_INVALID_BINARY_ASSIGN_CONTENT}")

add_test(NAME rxc_invalid_binary_assign_literal
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_BINARY_ASSIGN_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_INVALID_BINARY_AS_STRING_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_literal_as_string.cmake")
set(RXC_INVALID_BINARY_AS_STRING_CONTENT "")
string(APPEND RXC_INVALID_BINARY_AS_STRING_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_literal_as_string.crexx\" \"options levelb\\n\\nmain: procedure = .int\\n  x = 'ffff'x as .string\\n  say x\\n  return 0\\n\")\n")
string(APPEND RXC_INVALID_BINARY_AS_STRING_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -x -o \"rxc_invalid_binary_literal_as_string\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_literal_as_string.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject invalid byte literal cast to .string, but it succeeded.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES \"CANNOT_CAST_BINARY\")\n"
       "  message(FATAL_ERROR \"Expected binary-to-text diagnostic was not emitted for invalid byte literal cast to .string.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_BINARY_AS_STRING_SCRIPT}" CONTENT "${RXC_INVALID_BINARY_AS_STRING_CONTENT}")

add_test(NAME rxc_invalid_binary_literal_as_string
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_BINARY_AS_STRING_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_FUNCTION_LOOKUP_DIAG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_function_lookup_diagnostic.cmake")
set(RXC_FUNCTION_LOOKUP_DIAG_CONTENT "")
string(APPEND RXC_FUNCTION_LOOKUP_DIAG_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_function_lookup_diagnostic.crexx\" [=[options levelb\n\nmain: procedure = .int\n  box = .MethodBox()\n  if length(box.current()) <> 1 then return 1\n  return 0\n\nMethodBox: class\n  *: factory\n    return\n\n  current: method = .string\n    return \"x\"\n]=])\n")
string(APPEND RXC_FUNCTION_LOOKUP_DIAG_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -x -o \"rxc_function_lookup_diagnostic\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_function_lookup_diagnostic.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"Expected rxc to reject the unresolved length function.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(NOT full_out MATCHES [=[#FUNCTION_NOT_FOUND, \"length\"]=])\n"
       "  message(FATAL_ERROR \"Expected unresolved function diagnostic to point at the callee token.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "if(full_out MATCHES [=[#FUNCTION_NOT_FOUND, \"length[(]box[.]current]=])\n"
       "  message(FATAL_ERROR \"Unresolved function diagnostic still includes a truncated nested call span.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_FUNCTION_LOOKUP_DIAG_SCRIPT}" CONTENT "${RXC_FUNCTION_LOOKUP_DIAG_CONTENT}")

add_test(NAME rxc_function_lookup_diagnostic
         COMMAND ${CMAKE_COMMAND} -P "${RXC_FUNCTION_LOOKUP_DIAG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_function_lookup_diagnostic PROPERTIES LABELS "compiler;diagnostics")

set(RXC_INVALID_BINARY_TO_STRING_RUNTIME_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_to_string_runtime.cmake")
set(RXC_INVALID_BINARY_TO_STRING_RUNTIME_CONTENT "")
string(APPEND RXC_INVALID_BINARY_TO_STRING_RUNTIME_CONTENT
       "file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_to_string_runtime.crexx\" \"options levelb\\n\\nmain: procedure = .int\\n  payload = .binary\\n  text = \\\"\\\"\\n  payload = 'ffff'x\\n  text = payload as .string\\n  say text\\n  return 0\\n\")\n")
string(APPEND RXC_INVALID_BINARY_TO_STRING_RUNTIME_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"rxc_invalid_binary_to_string_runtime\" \"${CMAKE_CURRENT_BINARY_DIR}/rxc_invalid_binary_to_string_runtime.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for runtime invalid binary-to-string test.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"rxc_invalid_binary_to_string_runtime.rxbin\" \"rxc_invalid_binary_to_string_runtime\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxas_out\n"
       "  ERROR_VARIABLE rxas_err\n"
       "  RESULT_VARIABLE rxas_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{full_out}${RXAS_DIAG_DOLLAR}{rxas_out}${RXAS_DIAG_DOLLAR}{rxas_err}\")\n"
       "if(NOT rxas_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxas failed for runtime invalid binary-to-string test.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"rxc_invalid_binary_to_string_runtime.rxbin\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxvm_out\n"
       "  ERROR_VARIABLE rxvm_err\n"
       "  RESULT_VARIABLE rxvm_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{full_out}${RXAS_DIAG_DOLLAR}{rxvm_out}${RXAS_DIAG_DOLLAR}{rxvm_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT full_out MATCHES \"UNICODE_ERROR\")\n"
       "  message(FATAL_ERROR \"Expected runtime binary-to-string conversion to raise UNICODE_ERROR.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INVALID_BINARY_TO_STRING_RUNTIME_SCRIPT}" CONTENT "${RXC_INVALID_BINARY_TO_STRING_RUNTIME_CONTENT}")

add_test(NAME rxc_invalid_binary_to_string_runtime
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INVALID_BINARY_TO_STRING_RUNTIME_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_BINARY_LITERAL_LOAD_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_binary_literal_load.cmake")
set(RXC_BINARY_LITERAL_LOAD_CONTENT "")
string(APPEND RXC_BINARY_LITERAL_LOAD_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"binary_literal_load_rxas_check\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_literal_load.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for binary literal load check.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/binary_literal_load_rxas_check.rxas\" rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" rxas_out \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,0x00ff41]=])\n"
       "  message(FATAL_ERROR \"Binary hex literal did not lower to an RXAS binary load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,0x414243]=])\n"
       "  message(FATAL_ERROR \"String assigned to .binary did not lower to an RXAS binary load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,0xffff]=])\n"
       "  message(FATAL_ERROR \"Explicit binary cast literal did not lower to an RXAS binary load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[stobin r[0-9]+]=])\n"
       "  message(FATAL_ERROR \"String cast to .binary did not emit stobin.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[bintos r[0-9]+]=])\n"
       "  message(FATAL_ERROR \"Binary cast to .string did not emit bintos.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"binary_literal_load_rxas_check_opt\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_literal_load.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_opt_out\n"
       "  ERROR_VARIABLE rxc_opt_err\n"
       "  RESULT_VARIABLE rxc_opt_res\n"
       ")\n"
       "set(full_opt_out \"${RXAS_DIAG_DOLLAR}{rxc_opt_out}${RXAS_DIAG_DOLLAR}{rxc_opt_err}\")\n"
       "if(NOT rxc_opt_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"optimized rxc failed for binary literal load check.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_opt_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/binary_literal_load_rxas_check_opt.rxas\" opt_rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" opt_rxas_out \"${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "if(NOT opt_rxas_out MATCHES [=[load r[0-9]+,0xceb1]=])\n"
       "  message(FATAL_ERROR \"Optimized string-to-binary constant cast did not fold to a binary load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n"
       "if(NOT opt_rxas_out MATCHES [=[load r[0-9]+,\"AB\"]=])\n"
       "  message(FATAL_ERROR \"Optimized binary-to-string constant cast did not fold to a string load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n"
       "if(opt_rxas_out MATCHES [=[ret[ \\t]+0x]=])\n"
       "  message(FATAL_ERROR \"Optimized binary constant return used an unsupported RXAS immediate operand.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n"
       "if(opt_rxas_out MATCHES [=[ret[ \\t]+[-+0-9.]+d]=])\n"
       "  message(FATAL_ERROR \"Optimized decimal constant return used an unsupported RXAS immediate operand.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n"
       "if(NOT opt_rxas_out MATCHES [=[load r[0-9]+,0xaabb]=])\n"
       "  message(FATAL_ERROR \"Optimized binary constant return was not materialized in a register.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n"
       "if(NOT opt_rxas_out MATCHES [=[load r[0-9]+,12\\.5d]=])\n"
       "  message(FATAL_ERROR \"Optimized decimal constant return was not materialized in a register.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n"
       "if(opt_rxas_out MATCHES [=[0x3078]=])\n"
       "  message(FATAL_ERROR \"Optimized binary constants were re-encoded as ASCII hex text.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{opt_rxas_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_BINARY_LITERAL_LOAD_SCRIPT}" CONTENT "${RXC_BINARY_LITERAL_LOAD_CONTENT}")

add_test(NAME rxc_binary_literal_load
         COMMAND ${CMAKE_COMMAND} -P "${RXC_BINARY_LITERAL_LOAD_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_BINARY_MEMORY_SOURCE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_binary_memory_source.cmake")
set(RXC_BINARY_MEMORY_SOURCE_CONTENT "")
string(APPEND RXC_BINARY_MEMORY_SOURCE_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"binary_memory_source_rxas_check\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_memory_source.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for binary memory source check.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/binary_memory_source_rxas_check.rxas\" rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" rxas_out \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "foreach(_opcode blen bgetu32 bgetu8 bsetu16 bgetu16 bgetf64 bgeti64 bseti64 bgetf32 bsetf32 bresize bcopy bintos bgets bsets bcmpb bcmps stod dtos)\n"
       "  if(NOT rxas_out MATCHES \"${RXAS_DIAG_DOLLAR}{_opcode}\")\n"
       "    message(FATAL_ERROR \"Binary memory source did not emit ${RXAS_DIAG_DOLLAR}{_opcode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "  endif()\n"
       "endforeach()\n"
       "string(REGEX MATCH [=[\\.const ([^ ]+) binary 0x78563412fe7f0000000000000000f03f4f4b]=] header_alias_decl \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "set(header_alias \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
       "if(header_alias STREQUAL \"\")\n"
       "  message(FATAL_ERROR \"Binary memory source did not emit one named HEADER constant.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "string(REGEX MATCHALL [=[0x78563412fe7f0000000000000000f03f4f4b]=] header_payloads \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "list(LENGTH header_payloads header_payload_count)\n"
       "if(NOT header_payload_count EQUAL 1)\n"
       "  message(FATAL_ERROR \"Binary memory HEADER payload was not emitted exactly once; count=${RXAS_DIAG_DOLLAR}{header_payload_count}.\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES \"bgetu32 [^\\n]*,${RXAS_DIAG_DOLLAR}{header_alias},\")\n"
       "  message(FATAL_ERROR \"Binary memory source did not read a fixed-width field through its named constant.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "foreach(_opcode bcheckrange bslice)\n"
       "  if(rxas_out MATCHES \"${RXAS_DIAG_DOLLAR}{_opcode}\")\n"
       "    message(FATAL_ERROR \"Binary memory source emitted unnecessary checked-range/slice opcode ${RXAS_DIAG_DOLLAR}{_opcode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "  endif()\n"
       "endforeach()\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,4]=])\n"
       "  message(FATAL_ERROR \"Binary memory <sizeof..type> did not lower to a literal width load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES [=[load r[0-9]+,8]=])\n"
       "  message(FATAL_ERROR \"Binary memory <sizeof..int> did not lower to an 8-byte literal width load.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_BINARY_MEMORY_SOURCE_SCRIPT}" CONTENT "${RXC_BINARY_MEMORY_SOURCE_CONTENT}")

add_test(NAME rxc_binary_memory_source
         COMMAND ${CMAKE_COMMAND} -P "${RXC_BINARY_MEMORY_SOURCE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_BINARY_CONSTANT_ALIAS_OPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_binary_constant_alias_opt.cmake")
file(GENERATE OUTPUT "${RXC_BINARY_CONSTANT_ALIAS_OPT_SCRIPT}" CONTENT
"execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"binary_constant_alias_opt\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_memory_source.crexx\"
  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
  OUTPUT_VARIABLE rxc_out
  ERROR_VARIABLE rxc_err
  RESULT_VARIABLE rxc_res
)
if(NOT rxc_res EQUAL 0)
  message(FATAL_ERROR \"optimized rxc failed for named binary constant check.\\nOutput:\\n\${rxc_out}\${rxc_err}\")
endif()
file(READ \"${CMAKE_CURRENT_BINARY_DIR}/binary_constant_alias_opt.rxas\" rxas_out)
string(REGEX MATCH [=[\\.const ([^ ]+) binary 0x78563412fe7f0000000000000000f03f4f4b]=] header_alias_decl \"\${rxas_out}\")
set(header_alias \"\${CMAKE_MATCH_1}\")
if(header_alias STREQUAL \"\")
  message(FATAL_ERROR \"optimized rxc did not emit one named HEADER constant.\")
endif()
string(REGEX MATCHALL [=[0x78563412fe7f0000000000000000f03f4f4b]=] header_payloads \"\${rxas_out}\")
list(LENGTH header_payloads header_payload_count)
if(NOT header_payload_count EQUAL 1)
  message(FATAL_ERROR \"optimized HEADER payload was not emitted exactly once; count=\${header_payload_count}.\")
endif()
if(NOT rxas_out MATCHES \"load r[0-9]+,\${header_alias}\")
  message(FATAL_ERROR \"optimized named binary constant load is missing.\")
endif()
if(NOT rxas_out MATCHES \"blen r[0-9]+,\${header_alias}\")
  message(FATAL_ERROR \"optimized named binary constant length read is missing.\")
endif()
if(NOT rxas_out MATCHES \"bgetu32 [^\\n]*,\${header_alias},\")
  message(FATAL_ERROR \"optimized named binary constant field read is missing.\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxas>\" -o \"binary_constant_alias_opt\" \"binary_constant_alias_opt\"
  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
  OUTPUT_VARIABLE rxas_diag
  ERROR_VARIABLE rxas_diag
  RESULT_VARIABLE rxas_res
)
if(NOT rxas_res EQUAL 0 OR NOT rxas_diag STREQUAL \"\")
  message(FATAL_ERROR \"optimized named binary constant RXAS did not assemble cleanly.\\nOutput:\\n\${rxas_diag}\")
endif()
")
add_test(NAME rxc_binary_constant_alias_opt
         COMMAND ${CMAKE_COMMAND} -P "${RXC_BINARY_CONSTANT_ALIAS_OPT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_PACKED_NUMERIC_SOURCE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_packed_numeric_source.cmake")
set(RXC_PACKED_NUMERIC_SOURCE_CONTENT "")
string(APPEND RXC_PACKED_NUMERIC_SOURCE_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"packed_numeric_source_rxas_check\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/packed_numeric_source.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for packed numeric source check.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/packed_numeric_source_rxas_check.rxas\" rxas_out)\n"
       "foreach(_opcode pgeti pseti pgetf psetf)\n"
       "  if(NOT rxas_out MATCHES \"${RXAS_DIAG_DOLLAR}{_opcode}\")\n"
       "    message(FATAL_ERROR \"Packed numeric source did not emit ${RXAS_DIAG_DOLLAR}{_opcode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "  endif()\n"
       "endforeach()\n"
       "string(REGEX MATCH [=[\\.const ([^ ]+) binary 0x0000000000000000]=] zero_alias_decl \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "set(zero_alias \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
       "if(zero_alias STREQUAL \"\")\n"
       "  message(FATAL_ERROR \"Packed constant source was not emitted as aligned shared binary storage.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(NOT rxas_out MATCHES \"load r[0-9]+,${RXAS_DIAG_DOLLAR}{zero_alias}\")\n"
       "  message(FATAL_ERROR \"Packed constant source was not materialized in a register from its shared binary constant.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "if(rxas_out MATCHES [=[pget[if] [^\\n]*,0x]=])\n"
       "  message(FATAL_ERROR \"Packed access bypassed runtime-aligned binary storage.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_PACKED_NUMERIC_SOURCE_SCRIPT}" CONTENT "${RXC_PACKED_NUMERIC_SOURCE_CONTENT}")

add_test(NAME rxc_packed_numeric_source
         COMMAND ${CMAKE_COMMAND} -P "${RXC_PACKED_NUMERIC_SOURCE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_BINARY_MEMORY_PACKED_LOOKUP_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_binary_memory_packed_lookup.cmake")
set(RXC_BINARY_MEMORY_PACKED_LOOKUP_CONTENT "")
string(APPEND RXC_BINARY_MEMORY_PACKED_LOOKUP_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"binary_memory_packed_lookup_rxas_check\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_memory_packed_lookup.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for packed binary lookup check.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/binary_memory_packed_lookup_rxas_check.rxas\" rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" rxas_out \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "foreach(_opcode blen bgetu32 bgetu16 bcmpb)\n"
       "  if(NOT rxas_out MATCHES \"${RXAS_DIAG_DOLLAR}{_opcode}\")\n"
       "    message(FATAL_ERROR \"Packed binary lookup did not emit ${RXAS_DIAG_DOLLAR}{_opcode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "  endif()\n"
       "endforeach()\n"
       "foreach(_opcode bslice bintos)\n"
       "  if(rxas_out MATCHES \"${RXAS_DIAG_DOLLAR}{_opcode}\")\n"
       "    message(FATAL_ERROR \"Packed binary lookup materialized a key slice via ${RXAS_DIAG_DOLLAR}{_opcode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "  endif()\n"
       "endforeach()\n")
file(GENERATE OUTPUT "${RXC_BINARY_MEMORY_PACKED_LOOKUP_SCRIPT}" CONTENT "${RXC_BINARY_MEMORY_PACKED_LOOKUP_CONTENT}")

add_test(NAME rxc_binary_memory_packed_lookup
         COMMAND ${CMAKE_COMMAND} -P "${RXC_BINARY_MEMORY_PACKED_LOOKUP_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_CONSTANTS_LEVELB_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_constants_levelb.cmake")
set(RXC_CONSTANTS_LEVELB_CONTENT "")
foreach(_mode IN ITEMS noopt opt)
    if(_mode STREQUAL "noopt")
        set(_flag "-n")
    else()
        set(_flag "")
    endif()
    string(APPEND RXC_CONSTANTS_LEVELB_CONTENT
           "execute_process(\n"
           "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${_flag} -o \"constants_levelb_${_mode}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/constants_levelb.crexx\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "  OUTPUT_VARIABLE rxc_${_mode}_out\n"
           "  ERROR_VARIABLE rxc_${_mode}_err\n"
           "  RESULT_VARIABLE rxc_${_mode}_res\n"
           ")\n"
           "set(full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxc_${_mode}_out}${RXAS_DIAG_DOLLAR}{rxc_${_mode}_err}\")\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "if(NOT rxc_${_mode}_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc ${_mode} failed for constants_levelb.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "endif()\n"
           "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/constants_levelb_${_mode}.rxas\" rxas_${_mode}_out)\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" rxas_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[settp r[0-9]+,196608]=])\n"
           "  message(FATAL_ERROR \"Named constants did not fold to an assembler immediate in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[(load r[0-9]+,196608|ret 196608)]=])\n"
           "  message(FATAL_ERROR \"Named constants did not fold to a normal integer immediate in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n")
endforeach()
file(GENERATE OUTPUT "${RXC_CONSTANTS_LEVELB_SCRIPT}" CONTENT "${RXC_CONSTANTS_LEVELB_CONTENT}")

add_test(NAME rxc_constants_levelb
         COMMAND ${CMAKE_COMMAND} -P "${RXC_CONSTANTS_LEVELB_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_REGISTER_FLAG_VIEWS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_register_flag_views.cmake")
set(RXC_REGISTER_FLAG_VIEWS_CONTENT "")
foreach(_mode IN ITEMS noopt opt)
    if(_mode STREQUAL "noopt")
        set(_flag "-n")
    else()
        set(_flag "")
    endif()
    string(APPEND RXC_REGISTER_FLAG_VIEWS_CONTENT
           "execute_process(\n"
           "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${_flag} -o \"register_flag_views_${_mode}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/class_attr_register_views.crexx\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "  OUTPUT_VARIABLE rxc_${_mode}_out\n"
           "  ERROR_VARIABLE rxc_${_mode}_err\n"
           "  RESULT_VARIABLE rxc_${_mode}_res\n"
           ")\n"
           "set(full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxc_${_mode}_out}${RXAS_DIAG_DOLLAR}{rxc_${_mode}_err}\")\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "if(NOT rxc_${_mode}_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc ${_mode} failed for register flag views.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "endif()\n"
           "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/register_flag_views_${_mode}.rxas\" rxas_${_mode}_out)\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" rxas_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[getandtp r[0-9]+,a1,768]=])\n"
           "  message(FATAL_ERROR \"Compiler flag view did not lower to masked getandtp in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[getandtp r[0-9]+,a1,64512]=])\n"
           "  message(FATAL_ERROR \"Language flag view did not lower to masked getandtp in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[bcopy r[0-9]+,(a1|r[0-9]+)]=])\n"
           "  message(FATAL_ERROR \"Binary register view did not lower through bcopy in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(rxas_${_mode}_out MATCHES [=[(^|\\n)[ \\t]*acopy[ \\t]]=])\n"
           "  message(FATAL_ERROR \"Register view codegen emitted acopy in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[settpmask a1,r[0-9]+,16711680]=])\n"
           "  message(FATAL_ERROR \"Library flag view write did not lower to settpmask in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "if(NOT rxas_${_mode}_out MATCHES [=[settpmask a1,r[0-9]+,2147418112]=])\n"
           "  message(FATAL_ERROR \"Public flag view write did not lower to source-writable settpmask in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n")
endforeach()
file(GENERATE OUTPUT "${RXC_REGISTER_FLAG_VIEWS_SCRIPT}" CONTENT "${RXC_REGISTER_FLAG_VIEWS_CONTENT}")

add_test(NAME rxc_register_flag_views
         COMMAND ${CMAKE_COMMAND} -P "${RXC_REGISTER_FLAG_VIEWS_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(RXC_SCOPED_REGISTER_REUSE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_scoped_register_reuse.cmake")
set(RXC_SCOPED_REGISTER_REUSE_CONTENT "")
foreach(_mode IN ITEMS noopt)
    if(_mode STREQUAL "noopt")
        set(_flag "-n")
    else()
        set(_flag "")
    endif()
    string(APPEND RXC_SCOPED_REGISTER_REUSE_CONTENT
           "execute_process(\n"
           "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${_flag} -o \"scoped_register_reuse_${_mode}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/scoped_register_reuse.crexx\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "  OUTPUT_VARIABLE rxc_${_mode}_out\n"
           "  ERROR_VARIABLE rxc_${_mode}_err\n"
           "  RESULT_VARIABLE rxc_${_mode}_res\n"
           ")\n"
           "set(full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxc_${_mode}_out}${RXAS_DIAG_DOLLAR}{rxc_${_mode}_err}\")\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "if(NOT rxc_${_mode}_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc ${_mode} failed for scoped register reuse.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "endif()\n"
           "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/scoped_register_reuse_${_mode}.rxas\" rxas_${_mode}_out)\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" rxas_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.left\"=\"b\" \"\\.int\" r([0-9]+)]=] left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT left_match)\n"
           "  message(FATAL_ERROR \"Left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.right\"=\"b\" \"\\.int\" r([0-9]+)]=] right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT right_match)\n"
           "  message(FATAL_ERROR \"Right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT left_reg STREQUAL right_reg)\n"
           "  message(FATAL_ERROR \"Scoped scalar locals did not reuse a register in ${_mode}: left=r${RXAS_DIAG_DOLLAR}{left_reg}, right=r${RXAS_DIAG_DOLLAR}{right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.object_left\"=\"b\" \"\\.box\" r([0-9]+)]=] object_left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT object_left_match)\n"
           "  message(FATAL_ERROR \"Object-left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(object_left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.object_right\"=\"b\" \"\\.box\" r([0-9]+)]=] object_right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT object_right_match)\n"
           "  message(FATAL_ERROR \"Object-right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(object_right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT object_left_reg STREQUAL object_right_reg)\n"
           "  message(FATAL_ERROR \"Scoped object locals did not reuse a register in ${_mode}: object_left=r${RXAS_DIAG_DOLLAR}{object_left_reg}, object_right=r${RXAS_DIAG_DOLLAR}{object_right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.reference_left\"=\"b\" \"reference \\.int\" r([0-9]+)]=] reference_left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT reference_left_match)\n"
           "  message(FATAL_ERROR \"Reference-left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(reference_left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.reference_right\"=\"b\" \"reference \\.int\" r([0-9]+)]=] reference_right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT reference_right_match)\n"
           "  message(FATAL_ERROR \"Reference-right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(reference_right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT reference_left_reg STREQUAL reference_right_reg)\n"
           "  message(FATAL_ERROR \"Scoped reference locals did not reuse a register in ${_mode}: reference_left=r${RXAS_DIAG_DOLLAR}{reference_left_reg}, reference_right=r${RXAS_DIAG_DOLLAR}{reference_right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.binary_left\"=\"b\" \"\\.binary\" r([0-9]+)]=] binary_left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT binary_left_match)\n"
           "  message(FATAL_ERROR \"Binary-left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(binary_left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.binary_right\"=\"b\" \"\\.binary\" r([0-9]+)]=] binary_right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT binary_right_match)\n"
           "  message(FATAL_ERROR \"Binary-right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(binary_right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT binary_left_reg STREQUAL binary_right_reg)\n"
           "  message(FATAL_ERROR \"Scoped binary locals did not reuse a register in ${_mode}: binary_left=r${RXAS_DIAG_DOLLAR}{binary_left_reg}, binary_right=r${RXAS_DIAG_DOLLAR}{binary_right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.array_left\"=\"b\" \"\\.int\\[2\\]\" r([0-9]+)]=] array_left_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT array_left_match)\n"
           "  message(FATAL_ERROR \"Array-left scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(array_left_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "string(REGEX MATCH [=[\\.meta \"scoped_register_reuse.main.array_right\"=\"b\" \"\\.int\\[2\\]\" r([0-9]+)]=] array_right_match \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT array_right_match)\n"
           "  message(FATAL_ERROR \"Array-right scoped local metadata missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(array_right_reg \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(NOT array_left_reg STREQUAL array_right_reg)\n"
           "  message(FATAL_ERROR \"Scoped array locals did not reuse a register in ${_mode}: array_left=r${RXAS_DIAG_DOLLAR}{array_left_reg}, array_right=r${RXAS_DIAG_DOLLAR}{array_right_reg}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "string(REGEX MATCHALL [=[\\.meta \"scoped_register_reuse.main.left\"]=] left_meta_entries \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "list(LENGTH left_meta_entries left_meta_count)\n"
           "if(NOT left_meta_count EQUAL 2)\n"
           "  message(FATAL_ERROR \"Expected definition and clear metadata records for left scoped local in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n")
endforeach()
string(APPEND RXC_SCOPED_REGISTER_REUSE_CONTENT
       "function(assert_scope_lifetime symbol reg expected)\n"
       "  set(definition \".meta \\\"scoped_register_reuse.main.${RXAS_DIAG_DOLLAR}{symbol}\\\"=\\\"b\\\"\")\n"
       "  set(clear \"\\n   .meta \\\"scoped_register_reuse.main.${RXAS_DIAG_DOLLAR}{symbol}\\\"\\n\")\n"
       "  string(FIND \"${RXAS_DIAG_DOLLAR}{rxas_noopt_out}\" \"${RXAS_DIAG_DOLLAR}{definition}\" scope_start)\n"
       "  string(FIND \"${RXAS_DIAG_DOLLAR}{rxas_noopt_out}\" \"${RXAS_DIAG_DOLLAR}{clear}\" scope_end)\n"
       "  if(scope_start EQUAL -1 OR scope_end EQUAL -1 OR NOT scope_end GREATER scope_start)\n"
       "    message(FATAL_ERROR \"Could not isolate scoped lifetime block for ${RXAS_DIAG_DOLLAR}{symbol}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_noopt_out}\")\n"
       "  endif()\n"
       "  math(EXPR scope_length \"${RXAS_DIAG_DOLLAR}{scope_end} - ${RXAS_DIAG_DOLLAR}{scope_start}\")\n"
       "  string(SUBSTRING \"${RXAS_DIAG_DOLLAR}{rxas_noopt_out}\" ${RXAS_DIAG_DOLLAR}{scope_start} ${RXAS_DIAG_DOLLAR}{scope_length} scope_block)\n"
       "  string(FIND \"${RXAS_DIAG_DOLLAR}{scope_block}\" \"endlife r${RXAS_DIAG_DOLLAR}{reg}\" endlife_pos)\n"
       "  if(expected STREQUAL \"absent\" AND NOT endlife_pos EQUAL -1)\n"
       "    message(FATAL_ERROR \"Reference-lifetime-free scalar ${RXAS_DIAG_DOLLAR}{symbol} retained endlife.\\nBlock:\\n${RXAS_DIAG_DOLLAR}{scope_block}\")\n"
       "  elseif(expected STREQUAL \"present\" AND endlife_pos EQUAL -1)\n"
       "    message(FATAL_ERROR \"Reference-capable scoped local ${RXAS_DIAG_DOLLAR}{symbol} lost endlife.\\nBlock:\\n${RXAS_DIAG_DOLLAR}{scope_block}\")\n"
       "  endif()\n"
       "endfunction()\n"
       "assert_scope_lifetime(left ${RXAS_DIAG_DOLLAR}{left_reg} absent)\n"
       "assert_scope_lifetime(right ${RXAS_DIAG_DOLLAR}{right_reg} absent)\n"
       "assert_scope_lifetime(object_left ${RXAS_DIAG_DOLLAR}{object_left_reg} present)\n"
       "assert_scope_lifetime(object_right ${RXAS_DIAG_DOLLAR}{object_right_reg} present)\n"
       "assert_scope_lifetime(reference_left ${RXAS_DIAG_DOLLAR}{reference_left_reg} present)\n"
       "assert_scope_lifetime(reference_right ${RXAS_DIAG_DOLLAR}{reference_right_reg} present)\n"
       "assert_scope_lifetime(binary_left ${RXAS_DIAG_DOLLAR}{binary_left_reg} present)\n"
       "assert_scope_lifetime(binary_right ${RXAS_DIAG_DOLLAR}{binary_right_reg} present)\n"
       "assert_scope_lifetime(array_left ${RXAS_DIAG_DOLLAR}{array_left_reg} present)\n"
       "assert_scope_lifetime(array_right ${RXAS_DIAG_DOLLAR}{array_right_reg} present)\n")
file(GENERATE OUTPUT "${RXC_SCOPED_REGISTER_REUSE_SCRIPT}" CONTENT "${RXC_SCOPED_REGISTER_REUSE_CONTENT}")

add_test(NAME rxc_scoped_register_reuse
         COMMAND ${CMAKE_COMMAND} -P "${RXC_SCOPED_REGISTER_REUSE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_scoped_register_reuse PROPERTIES LABELS "compiler;registers")

set(RXC_CALL_STATEMENT_REGISTER_REUSE_SCRIPT
    "${CMAKE_CURRENT_BINARY_DIR}/rxc_call_statement_register_reuse.cmake")
set(RXC_CALL_STATEMENT_REGISTER_REUSE_CONTENT "")
foreach(_mode IN ITEMS opt noopt)
    if(_mode STREQUAL "noopt")
        set(_flag "-n")
        set(_max_dispatch_locals 40)
    else()
        set(_flag "")
        set(_max_dispatch_locals 8)
    endif()
    string(APPEND RXC_CALL_STATEMENT_REGISTER_REUSE_CONTENT
           "execute_process(\n"
           "  COMMAND \"$<TARGET_FILE:rxc>\" -x -i \"${CMAKE_BINARY_DIR}/bin\" ${_flag} -o \"call_statement_register_reuse_${_mode}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/call_statement_register_reuse.crexx\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "  OUTPUT_VARIABLE rxc_${_mode}_out\n"
           "  ERROR_VARIABLE rxc_${_mode}_err\n"
           "  RESULT_VARIABLE rxc_${_mode}_res\n"
           ")\n"
           "set(full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxc_${_mode}_out}${RXAS_DIAG_DOLLAR}{rxc_${_mode}_err}\")\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" full_${_mode}_out \"${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "if(NOT rxc_${_mode}_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc ${_mode} failed for call-statement register reuse.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_${_mode}_out}\")\n"
           "endif()\n"
           "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/call_statement_register_reuse_${_mode}.rxas\" rxas_${_mode}_out)\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" rxas_${_mode}_out \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "string(REGEX MATCH [=[§call_statement_register_reuse\\.wrapper\\.dispatch\\(\\) \\.locals=([0-9]+)]=] dispatch_header \"${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "if(NOT dispatch_header)\n"
           "  message(FATAL_ERROR \"dispatch locals record missing in ${_mode}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "set(dispatch_locals \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
           "if(dispatch_locals GREATER ${_max_dispatch_locals})\n"
           "  message(FATAL_ERROR \"Discarded calls or member receivers were not recycled in ${_mode}; dispatch locals=${RXAS_DIAG_DOLLAR}{dispatch_locals}, expected at most ${_max_dispatch_locals}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_${_mode}_out}\")\n"
           "endif()\n"
           "execute_process(\n"
           "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"call_statement_register_reuse_${_mode}.rxbin\" \"call_statement_register_reuse_${_mode}\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "  OUTPUT_VARIABLE rxas_${_mode}_diag\n"
           "  ERROR_VARIABLE rxas_${_mode}_err\n"
           "  RESULT_VARIABLE rxas_${_mode}_res\n"
           ")\n"
           "if(NOT rxas_${_mode}_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxas ${_mode} failed for call-statement register reuse: ${RXAS_DIAG_DOLLAR}{rxas_${_mode}_diag}${RXAS_DIAG_DOLLAR}{rxas_${_mode}_err}\")\n"
           "endif()\n"
           "execute_process(\n"
           "  COMMAND \"$<TARGET_FILE:rxbvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"call_statement_register_reuse_${_mode}.rxbin\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "  OUTPUT_VARIABLE rxvm_${_mode}_out\n"
           "  ERROR_VARIABLE rxvm_${_mode}_err\n"
           "  RESULT_VARIABLE rxvm_${_mode}_res\n"
           ")\n"
           "if(NOT rxvm_${_mode}_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxvm ${_mode} failed for call-statement register reuse: ${RXAS_DIAG_DOLLAR}{rxvm_${_mode}_out}${RXAS_DIAG_DOLLAR}{rxvm_${_mode}_err}\")\n"
           "endif()\n")
endforeach()
file(GENERATE OUTPUT "${RXC_CALL_STATEMENT_REGISTER_REUSE_SCRIPT}"
     CONTENT "${RXC_CALL_STATEMENT_REGISTER_REUSE_CONTENT}")

add_test(NAME rxc_call_statement_register_reuse
         COMMAND ${CMAKE_COMMAND} -P "${RXC_CALL_STATEMENT_REGISTER_REUSE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_call_statement_register_reuse PROPERTIES
        LABELS "compiler;registers;runtime")

set(RXC_INLINE_BYVALUE_ARG_REUSE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_inline_byvalue_arg_reuse.cmake")
set(RXC_INLINE_BYVALUE_ARG_REUSE_CONTENT "")
string(APPEND RXC_INLINE_BYVALUE_ARG_REUSE_CONTENT
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_test_byvalue_arg_reuse_diag\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_test_byvalue_arg_reuse.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE rxc_out\n"
       "  ERROR_VARIABLE rxc_err\n"
       "  RESULT_VARIABLE rxc_res\n"
       ")\n"
       "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "if(NOT rxc_res EQUAL 0)\n"
       "  message(FATAL_ERROR \"rxc failed for inline by-value arg reuse.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
       "endif()\n"
       "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/inline_test_byvalue_arg_reuse_diag.rxas\" rxas_out)\n"
       "string(REPLACE \"\\r\\n\" \"\\n\" rxas_out \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "string(REGEX MATCH [=[main\\(\\) \\.locals=([0-9]+)]=] main_locals_match \"${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "if(NOT main_locals_match)\n"
       "  message(FATAL_ERROR \"main locals record missing for inline by-value arg reuse.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n"
       "set(main_locals \"${RXAS_DIAG_DOLLAR}{CMAKE_MATCH_1}\")\n"
       "if(main_locals GREATER 12)\n"
       "  message(FATAL_ERROR \"Inlined by-value formals were not recycled; main locals=${RXAS_DIAG_DOLLAR}{main_locals}.\\nRXAS:\\n${RXAS_DIAG_DOLLAR}{rxas_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_INLINE_BYVALUE_ARG_REUSE_SCRIPT}" CONTENT "${RXC_INLINE_BYVALUE_ARG_REUSE_CONTENT}")

add_test(NAME rxc_inline_byvalue_arg_reuse
         COMMAND ${CMAKE_COMMAND} -P "${RXC_INLINE_BYVALUE_ARG_REUSE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_inline_byvalue_arg_reuse PROPERTIES LABELS "compiler;registers")

# Robustness tests
macro(add_crexx_robustness_test NAME MAIN_FILE)
    # The list of dependencies are passed as additional arguments
    set(DEPS ${ARGN})
    set(DEP_BINS "")
    
    # Generate a CMake script for this test to replace bash -c
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_robustness_${NAME}.cmake")
    set(SCRIPT_CONTENT "")
    
    # Build main file
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/${MAIN_FILE}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on main\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}.rxbin\" \"${NAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on main\")\nendif()\n")

    foreach(DEP ${DEPS})
        # Extract lib name from filename
        get_filename_component(DEP_NAME ${DEP} NAME_WE)
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness\" -o \"${DEP_NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/${DEP}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on ${DEP}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_NAME}.rxbin\" \"${DEP_NAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on ${DEP}\")\nendif()\n")
        
        # Build list for rxvm
        set(DEP_BINS "${DEP_BINS};${DEP_NAME}.rxbin")
    endforeach()

    # Run rxvm and compare output
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${NAME}.rxbin\" ${DEP_BINS} \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" OUTPUT_VARIABLE vm_out RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed\")\nendif()\n")
    
    # Read golden file
    string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    # Normalize line endings
    string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" vm_out \"\${vm_out}\")\n")
    string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    
    string(APPEND SCRIPT_CONTENT "if(NOT vm_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${vm_out}\")\nendif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(NAME robustness_${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

add_crexx_robustness_test(set1_main set1_main.crexx set1_lib.crexx)
add_crexx_robustness_test(set2_main set2_main.crexx set2_lib.crexx)
add_crexx_robustness_test(set3_main set3_main.crexx set3_lib.crexx)
add_crexx_robustness_test(set4_main set4_main.crexx set4_mid.crexx set4_base.crexx)
add_crexx_robustness_test(set6_main set6_main.crexx Counter.crexx CounterIterator.crexx)

# Set 5 - Encapsulation (Expected to fail compilation)
set(SCRIPT_FILE_SET5 "${CMAKE_CURRENT_BINARY_DIR}/run_robustness_set5.cmake")
set(SCRIPT_CONTENT_SET5
        "include([=[${CMAKE_SOURCE_DIR}/cmake/CrexxTestAssertions.cmake]=])\n")
string(APPEND SCRIPT_CONTENT_SET5
        "execute_process(\n"
        "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness\" -o \"set5_main\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/set5_main.crexx\"\n"
        "  RESULT_VARIABLE res\n"
        "  OUTPUT_VARIABLE out\n"
        "  ERROR_VARIABLE err)\n"
        "set(combined \"\${out}\${err}\")\n"
        "if(NOT \"\${combined}\" STREQUAL \"\")\n  message(\"\${combined}\")\nendif()\n"
        "crexx_assert_expected_failure(\n"
        "  NAME robustness_set5_encapsulation\n"
        "  RESULT \"\${res}\"\n"
        "  EXPECTED_EXIT_CODE 2\n"
        "  OUTPUT \"\${combined}\"\n"
        "  DESCRIPTION \"private function access was rejected\"\n"
        "  REQUIRED_OUTPUT_REGEX [=[#FUNCTION_NOT_FOUND, \"private_func\"]=])\n")
file(GENERATE OUTPUT "${SCRIPT_FILE_SET5}" CONTENT "${SCRIPT_CONTENT_SET5}")

add_test(NAME robustness_set5_encapsulation
         COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE_SET5}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Example tests
add_crexx_compiler_test(01_assign rexx_src/01_assign.crexx)
add_crexx_compiler_test(02_flow rexx_src/02_flow.crexx)
add_crexx_compiler_test(03_calc rexx_src/03_calc.crexx)
add_crexx_compiler_test(04_literals rexx_src/04_literals.crexx)
add_crexx_compiler_test(05_ops_common rexx_src/05_ops_common.crexx)
add_crexx_compiler_test(05_ops_classic rexx_src/05_ops_classic.crexx)
add_crexx_compiler_fail_test(err_xor_common rexx_src/err_xor_common.crexx err_xor_common.txt)
add_crexx_compiler_fail_test(int64_literal_overflow rexx_src/int64_literal_overflow.crexx int64_literal_overflow.txt)
add_crexx_compiler_fail_test(int64_fold_overflow rexx_src/int64_fold_overflow.crexx int64_fold_overflow.txt)
add_crexx_compiler_fail_test(int64_fold_division_zero rexx_src/int64_fold_division_zero.crexx int64_fold_division_zero.txt)
add_crexx_compiler_test(strict_compare_codegen rexx_src/strict_compare_codegen.crexx)
add_crexx_compiler_test(address_expose rexx_src/address_expose.crexx)
add_crexx_compiler_test(div_ops_common rexx_src/div_ops_common.crexx)
add_crexx_compiler_test(div_ops_classic rexx_src/div_ops_classic.crexx)
add_crexx_compiler_test(06_logic rexx_src/06_logic.crexx)
add_crexx_compiler_test(named_operators rexx_src/named_operators.crexx)
add_crexx_compiler_test(07_loops rexx_src/07_loops.crexx)
add_crexx_compiler_test(08_select rexx_src/08_select.crexx)
add_crexx_compiler_test(test_select_classic rexx_src/test_select_classic.crexx)
add_crexx_compiler_test(test_select_c rexx_src/test_select_c.crexx)
add_crexx_compiler_test(select_dispatch_integer rexx_src/select_dispatch_integer.crexx)
add_crexx_compiler_test(select_dispatch_fallback rexx_src/select_dispatch_fallback.crexx)
add_crexx_compiler_test(select_dispatch_mixed rexx_src/select_dispatch_mixed.crexx)
add_crexx_compiler_test(select_dispatch_general rexx_src/select_dispatch_general.crexx)
add_crexx_compiler_test(select_dispatch_strings rexx_src/select_dispatch_strings.crexx)
add_crexx_compiler_test(repro_duplicate_call_argument rexx_src/repro_duplicate_call_argument.crexx)
add_crexx_compiler_test(09_nested rexx_src/09_nested.crexx)
add_crexx_compiler_test(10_procs rexx_src/10_procs.crexx)
add_crexx_compiler_test(11_scope rexx_src/11_scope.crexx)
add_crexx_compiler_test(12_arrays_lvlb rexx_src/12_arrays_lvlb.crexx)
add_crexx_compiler_test(array_mutation_statements rexx_src/array_mutation_statements.crexx)
add_crexx_compiler_test(13_stems rexx_src/13_stems.crexx)
add_crexx_compiler_test(repro_multi_tail_stems rexx_src/repro_multi_tail_stems.crexx)
add_crexx_compiler_test(14_strings rexx_src/14_strings.crexx)
add_crexx_compiler_test(17_string_arrays rexx_src/17_string_arrays.crexx)
add_crexx_compiler_dep_test(18_array_class rexx_src/18_array_class_main.crexx rexx_src/array_class_lib.crexx)
add_crexx_compiler_test(val_magic_01 rexx_src/val_magic_01.crexx)
add_crexx_compiler_test(val_magic_02 rexx_src/val_magic_02.crexx)
add_crexx_compiler_test(opt_decimal_safety rexx_src/opt_decimal_safety.crexx)
add_crexx_compiler_test(opt_numeric_safety rexx_src/opt_numeric_safety.crexx)
add_crexx_compiler_dep_test(15_imports rexx_src/15_imports.crexx rexx_src/mymath.crexx)
add_crexx_compiler_dep_test(class_import rexx_src/class_import_main.crexx rexx_src/class_import_lib.crexx)
add_crexx_parsing_test(16_classes rexx_src/16_classes.crexx)

# Negative tests
add_crexx_compiler_fail_test(err_01_type rexx_src/err_01_type.crexx err_01_type.txt)
add_crexx_compiler_fail_test(err_02_syntax rexx_src/err_02_syntax.crexx err_02_syntax.txt)
add_crexx_compiler_fail_test(err_04_getter rexx_src/err_04_getter.crexx err_04_getter.txt)
add_crexx_compiler_fail_test(err_05_setter rexx_src/err_05_setter.crexx err_05_setter.txt)
add_crexx_compiler_fail_test(err_06_missing_method rexx_src/err_06_missing_method.crexx err_06_missing_method.txt)
add_crexx_compiler_fail_test(array_mutation_bad_index rexx_src/array_mutation_bad_index.crexx array_mutation_bad_index.txt)
add_crexx_compiler_fail_test(array_mutation_bad_value rexx_src/array_mutation_bad_value.crexx array_mutation_bad_value.txt)
add_crexx_compiler_fail_test(array_mutation_fixed_target rexx_src/array_mutation_fixed_target.crexx array_mutation_fixed_target.txt)
add_crexx_compiler_fail_test(array_mutation_scalar_target rexx_src/array_mutation_scalar_target.crexx array_mutation_scalar_target.txt)
add_crexx_compiler_fail_test(err_symbol_boundary_whitespace rexx_src/err_symbol_boundary_whitespace.crexx err_symbol_boundary_whitespace.txt)
add_crexx_compiler_fail_test(err_loop_class rexx_src/err_loop_class.crexx err_loop_class.txt)
add_crexx_compiler_fail_test(err_select_no_when rexx_src/err_select_no_when.crexx err_select_no_when.txt)
add_crexx_compiler_fail_test(err_select_c_no_when rexx_src/err_select_c_no_when.crexx err_select_c_no_when.txt)
add_crexx_compiler_fail_test(err_class_assign rexx_src/err_class_assign.crexx err_class_assign.txt)
add_crexx_compiler_fail_test(err_register_flag_compiler_write rexx_src/err_register_flag_compiler_write.crexx err_register_flag_compiler_write.txt)
add_crexx_compiler_fail_test(err_register_flag_language_write rexx_src/err_register_flag_language_write.crexx err_register_flag_language_write.txt)
add_crexx_compiler_fail_test(err_register_flag_unknown rexx_src/err_register_flag_unknown.crexx err_register_flag_unknown.txt)
add_crexx_compiler_fail_test(err_register_flag_type rexx_src/err_register_flag_type.crexx err_register_flag_type.txt)
add_crexx_compiler_fail_test(err_address_commandless_sandbox rexx_src/err_address_commandless_sandbox.crexx err_address_commandless_sandbox.txt)
add_crexx_compiler_fail_test(err_block_expr_type_mismatch rexx_src/err_block_expr_type_mismatch.crexx err_block_expr_type_mismatch.txt)
add_crexx_compiler_fail_test(err_block_expr_no_leave rexx_src/err_block_expr_no_leave.crexx err_block_expr_no_leave.txt)
add_crexx_compiler_fail_test(err_block_expr_leave_plain rexx_src/err_block_expr_leave_plain.crexx err_block_expr_leave_plain.txt)
add_crexx_compiler_fail_test(err_block_expr_missing_end rexx_src/err_block_expr_missing_end.crexx err_block_expr_missing_end.txt)
add_crexx_compiler_fail_test(err_block_expr_incomplete rexx_src/err_block_expr_incomplete.crexx err_block_expr_incomplete.txt)
add_crexx_compiler_fail_test(err_leave_with_not_in_block rexx_src/err_leave_with_not_in_block.crexx err_leave_with_not_in_block.txt)
add_crexx_compiler_fail_test(initializer_direct_call rexx_src/initializer_direct_call.crexx initializer_direct_call.txt)
add_crexx_compiler_fail_test(initializer_namespace_expose rexx_src/initializer_namespace_expose.crexx initializer_namespace_expose.txt)
add_crexx_compiler_fail_test(initializer_argument rexx_src/initializer_argument.crexx initializer_argument.txt)
add_crexx_compiler_fail_test(initializer_return_value rexx_src/initializer_return_value.crexx initializer_return_value.txt)
add_crexx_compiler_fail_test(interface_missing_impl rexx_src/interface_missing_impl.crexx interface_missing_impl.txt)
add_crexx_compiler_fail_test(interface_match_bad_args rexx_src/interface_match_bad_args.crexx interface_match_bad_args.txt)
add_crexx_compiler_fail_test(interface_match_without_factory rexx_src/interface_match_without_factory.crexx interface_match_without_factory.txt)
add_crexx_compiler_fail_test(interface_default_redefine rexx_src/interface_default_redefine.crexx interface_default_redefine.txt)
add_crexx_compiler_fail_test(interface_factory_return_type rexx_src/interface_factory_return_type.crexx interface_factory_return_type.txt)
add_crexx_compiler_fail_test(interface_signature_static_bad_method rexx_src/interface_signature_static_bad_method.crexx interface_signature_static_bad_method.txt)
add_crexx_compiler_fail_test(qualified_namespace_missing_import rexx_src/qualified_namespace_missing_import.crexx qualified_namespace_missing_import.txt)
add_crexx_compiler_fail_test(type_ops_reserved_keyword rexx_src/type_ops_reserved_keyword.crexx type_ops_reserved_keyword.txt)
add_crexx_compiler_fail_test(reference_source_value_to_ref rexx_src/reference_source_value_to_ref_fail.crexx reference_source_value_to_ref_fail.txt)
add_crexx_compiler_fail_test(reference_source_ref_to_value rexx_src/reference_source_ref_to_value_fail.crexx reference_source_ref_to_value_fail.txt)
add_crexx_compiler_fail_test(reference_source_non_storage rexx_src/reference_source_non_storage_fail.crexx reference_source_non_storage_fail.txt)
add_crexx_compiler_fail_test(reference_source_ref_target rexx_src/reference_source_ref_target_fail.crexx reference_source_ref_target_fail.txt)
add_crexx_compiler_fail_test(reference_source_nested_type rexx_src/reference_source_nested_type_fail.crexx reference_source_nested_type_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_nonref rexx_src/reference_source_deref_nonref_fail.crexx reference_source_deref_nonref_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_expr rexx_src/reference_source_deref_expr_fail.crexx reference_source_deref_expr_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_attr rexx_src/reference_source_deref_attr_fail.crexx reference_source_deref_attr_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_index rexx_src/reference_source_deref_index_fail.crexx reference_source_deref_index_fail.txt)
add_crexx_compiler_fail_test(reference_source_deref_global rexx_src/reference_source_deref_global_fail.crexx reference_source_deref_global_fail.txt)
add_crexx_compiler_fail_test(reference_source_refvalid_nonref rexx_src/reference_source_refvalid_nonref_fail.crexx reference_source_refvalid_nonref_fail.txt)
add_crexx_compiler_fail_test(reference_source_comparison rexx_src/reference_source_comparison_fail.crexx reference_source_comparison_fail.txt)
add_crexx_compiler_fail_test(reference_source_refsame_nonref rexx_src/reference_source_refsame_nonref_fail.crexx reference_source_refsame_nonref_fail.txt)
add_crexx_compiler_fail_test(reference_source_refsame_type rexx_src/reference_source_refsame_type_fail.crexx reference_source_refsame_type_fail.txt)
add_crexx_compiler_fail_test(eq_levelb rexx_src/eq_levelb_fail.crexx eq_levelb_fail.txt)
add_crexx_compiler_fail_test(eq_missing_interface rexx_src/eq_missing_interface_fail.crexx eq_missing_interface_fail.txt)
add_crexx_compiler_fail_test(eq_nonobject rexx_src/eq_nonobject_fail.crexx eq_nonobject_fail.txt)

set(_reference_driver_workdir
        "${CMAKE_CURRENT_BINARY_DIR}/reference_driver_failure_ws")
set(_reference_driver_script
        "${CMAKE_CURRENT_BINARY_DIR}/reference_driver_failure.cmake")
file(GENERATE OUTPUT "${_reference_driver_script}" CONTENT
"file(REMOVE_RECURSE \"${_reference_driver_workdir}\")
file(MAKE_DIRECTORY \"${_reference_driver_workdir}\")
file(COPY
  \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/reference_source_comparison_fail.crexx\"
  DESTINATION \"${_reference_driver_workdir}\")
execute_process(
  COMMAND \"$<TARGET_FILE:crexx>\"
          reference_source_comparison_fail.crexx -noexec -nocolor
  WORKING_DIRECTORY \"${_reference_driver_workdir}\"
  RESULT_VARIABLE result
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error
  TIMEOUT 20)
set(combined \"\${output}\${error}\")
if(NOT \"\${result}\" STREQUAL \"2\")
  message(FATAL_ERROR
          \"crexx driver did not return the compiler failure (\${result}):\\n\${combined}\")
endif()
string(FIND \"\${combined}\" \"#REFERENCE_COMPARISON_UNSUPPORTED\" diagnostic)
if(diagnostic EQUAL -1)
  message(FATAL_ERROR
          \"crexx driver hid the compiler diagnostic:\\n\${combined}\")
endif()
")
add_test(NAME crexx_driver_reference_failure_propagation
         COMMAND ${CMAKE_COMMAND} -P "${_reference_driver_script}")
set_tests_properties(crexx_driver_reference_failure_propagation PROPERTIES
        LABELS "compiler;driver;negative;references"
        TIMEOUT 30)

add_crexx_compiler_fail_test(binary_memory_invalid_storage rexx_src/binary_memory_invalid_storage.crexx binary_memory_invalid_storage.txt)
add_crexx_compiler_fail_test(binary_memory_fixed_for rexx_src/binary_memory_fixed_for.crexx binary_memory_fixed_for.txt)
add_crexx_compiler_fail_test(binary_memory_missing_offset rexx_src/binary_memory_missing_offset.crexx binary_memory_missing_offset.txt)
add_crexx_compiler_fail_test(binary_memory_readonly_write rexx_src/binary_memory_readonly_write.crexx binary_memory_readonly_write.txt)
add_crexx_compiler_fail_test(binary_memory_span_write rexx_src/binary_memory_span_write.crexx binary_memory_span_write.txt)
add_crexx_compiler_fail_test(binary_memory_target_not_binary rexx_src/binary_memory_target_not_binary.crexx binary_memory_target_not_binary.txt)
add_crexx_compiler_fail_test(binary_memory_length_required rexx_src/binary_memory_length_required.crexx binary_memory_length_required.txt)
add_crexx_compiler_fail_test(binary_memory_decimal_length rexx_src/binary_memory_decimal_length.crexx binary_memory_decimal_length.txt)
add_crexx_compiler_fail_test(binary_memory_extra_argument rexx_src/binary_memory_extra_argument.crexx binary_memory_extra_argument.txt)
add_crexx_compiler_fail_test(binary_memory_missing_length_argument rexx_src/binary_memory_missing_length_argument.crexx binary_memory_missing_length_argument.txt)
add_crexx_compiler_fail_test(packed_invalid_storage rexx_src/packed_invalid_storage.crexx packed_invalid_storage.txt)
add_crexx_compiler_fail_test(packed_missing_index rexx_src/packed_missing_index.crexx packed_missing_index.txt)
add_crexx_compiler_fail_test(packed_extra_argument rexx_src/packed_extra_argument.crexx packed_extra_argument.txt)
add_crexx_compiler_fail_test(packed_target_not_binary rexx_src/packed_target_not_binary.crexx packed_target_not_binary.txt)
add_crexx_compiler_fail_test(packed_readonly_write rexx_src/packed_readonly_write.crexx packed_readonly_write.txt)
add_crexx_compiler_fail_test(binary_memory_generic_types_unsupported rexx_src/binary_memory_generic_types_unsupported.crexx binary_memory_generic_types_unsupported.txt)
add_crexx_compiler_fail_test(binary_memory_blen_syntax rexx_src/binary_memory_blen_syntax.crexx binary_memory_blen_syntax.txt)
add_crexx_compiler_fail_test(binary_memory_blen_target_not_binary rexx_src/binary_memory_blen_target_not_binary.crexx binary_memory_blen_target_not_binary.txt)
add_crexx_compiler_fail_test(binary_memory_compare_arguments rexx_src/binary_memory_compare_arguments.crexx binary_memory_compare_arguments.txt)
add_crexx_compiler_fail_test(binary_memory_compare_explicit_length rexx_src/binary_memory_compare_explicit_length.crexx binary_memory_compare_explicit_length.txt)
add_crexx_compiler_fail_test(binary_memory_compare_needle_type rexx_src/binary_memory_compare_needle_type.crexx binary_memory_compare_needle_type.txt)
add_crexx_compiler_fail_test(binary_memory_compare_target_not_binary rexx_src/binary_memory_compare_target_not_binary.crexx binary_memory_compare_target_not_binary.txt)
add_crexx_compiler_fail_test(binary_memory_compare_type rexx_src/binary_memory_compare_type.crexx binary_memory_compare_type.txt)
add_crexx_compiler_fail_test(literal_continuation_suffix_error rexx_src/literal_continuation_suffix_error.crexx literal_continuation_suffix_error.txt)
add_crexx_compiler_fail_test(intrinsic_argexists_invalid rexx_src/intrinsic_argexists_invalid.crexx intrinsic_argexists_invalid.txt)
add_crexx_compiler_fail_test(legacy_arg_call_removed rexx_src/legacy_arg_call_removed.crexx legacy_arg_call_removed.txt)
add_crexx_compiler_fail_test(legacy_arg_stem_removed rexx_src/legacy_arg_stem_removed.crexx legacy_arg_stem_removed.txt)
add_crexx_compiler_fail_test(legacy_typeof_removed rexx_src/legacy_typeof_removed.crexx legacy_typeof_removed.txt)
add_crexx_compiler_fail_test(legacy_refvalid_removed rexx_src/legacy_refvalid_removed.crexx legacy_refvalid_removed.txt)
add_test(NAME err_address_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_address_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_address_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_address_exit_disabled.crexx)

add_test(NAME err_parse_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_parse_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_parse_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_parse_exit_disabled.crexx)

add_test(NAME err_parse_inline_then_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_parse_inline_then_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_parse_inline_then_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_parse_inline_then_exit_disabled.crexx)

add_test(NAME err_trace_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_trace_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_trace_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_trace_exit_disabled.crexx)

add_crexx_compiler_fail_test(err_parse_missing_arguments rexx_src/err_parse_missing_arguments.crexx err_parse_missing_arguments.txt)

# Warning tests
add_crexx_compiler_warning_test(loop rexx_src/loop.crexx loop.txt)
add_crexx_compiler_warning_test(implicit_member_call_address rexx_src/implicit_member_call_address_warning.crexx implicit_member_call_address.txt)
add_crexx_compiler_warning_test(implicit_exit_reject_address rexx_src/implicit_exit_reject_address_warning.crexx implicit_exit_reject_address.txt)
add_crexx_compiler_warning_test(procedure_expose_no_duplicate rexx_src/procedure_expose_no_duplicate_warning.crexx procedure_expose_no_duplicate.txt)
add_crexx_compiler_warning_test(unused_import rexx_src/unused_import_warning.crexx unused_import.txt)

add_test(NAME unused_import_used_proc_warn
         COMMAND crexx_test_driver --warnings
         --copy ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_used_dep.crexx
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/warnings/unused_import_used_proc.txt
         ${CMAKE_CURRENT_BINARY_DIR}/unused_import_used_proc_warn.txt
         $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_used_proc_warning.crexx)

add_test(NAME unused_import_used_type_warn
         COMMAND crexx_test_driver --warnings
         --copy ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_used_dep.crexx
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/warnings/unused_import_used_type.txt
         ${CMAKE_CURRENT_BINARY_DIR}/unused_import_used_type_warn.txt
         $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_used_type_warning.crexx)

add_test(NAME unused_import_inline_used_warn
         COMMAND crexx_test_driver --warnings
         --copy ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/warnings/unused_import_inline_used.txt
         ${CMAKE_CURRENT_BINARY_DIR}/unused_import_inline_used_warn.txt
         $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_inline_used_warning.crexx)

# Regression tests for rxpp issue
add_crexx_compiler_test(repro_dotted_index rexx_src/repro_dotted_index.crexx)
add_crexx_compiler_test(repro_expand_until rexx_src/repro_expand_until.crexx)
add_crexx_compiler_test(repro_array_arg rexx_src/repro_array_arg.crexx)
add_crexx_compiler_test(repro_opt_args rexx_src/repro_opt_args.crexx)
add_crexx_compiler_test(arg_semantics_scalar rexx_src/arg_semantics_scalar.crexx)
add_crexx_compiler_test(arg_semantics_array rexx_src/arg_semantics_array.crexx)
add_crexx_compiler_test(arg_semantics_object rexx_src/arg_semantics_object.crexx)
add_crexx_compiler_test(object_reference_regression rexx_src/object_reference_regression.crexx)
add_crexx_compiler_test(basic_loops rexx_src/basic_loops.crexx)
add_crexx_compiler_test(basic_strings rexx_src/basic_strings.crexx)
add_crexx_compiler_test(basic_arrays rexx_src/basic_arrays.crexx)
add_crexx_compiler_test(block_expr_basic rexx_src/block_expr_basic.crexx)
add_crexx_compiler_test(block_expr_short_circuit rexx_src/block_expr_short_circuit.crexx)
add_crexx_compiler_test(inline_test1 rexx_src/inline_test1.crexx)
add_crexx_compiler_test(inline_test2 rexx_src/inline_test2.crexx)
add_crexx_compiler_test(inline_test_call rexx_src/inline_test_call.crexx)
add_crexx_compiler_test(inline_test_expr rexx_src/inline_test_expr.crexx)
add_crexx_compiler_test(inline_test_expr_negative rexx_src/inline_test_expr_negative.crexx)
add_crexx_compiler_test(inline_test_nested_call_expr rexx_src/inline_test_nested_call_expr.crexx)
add_crexx_compiler_test(inline_test_concat_expr rexx_src/inline_test_concat_expr.crexx)
add_crexx_compiler_test(inline_test_say_expr rexx_src/inline_test_say_expr.crexx)
add_crexx_compiler_test(inline_test_say_expr_negative rexx_src/inline_test_say_expr_negative.crexx)
add_crexx_compiler_test(inline_test_return_expr rexx_src/inline_test_return_expr.crexx)
add_crexx_compiler_test(inline_test_unary_expr rexx_src/inline_test_unary_expr.crexx)
add_crexx_compiler_test(inline_test_compare_expr rexx_src/inline_test_compare_expr.crexx)
add_crexx_compiler_test(inline_test_call_arg_expr rexx_src/inline_test_call_arg_expr.crexx)
add_crexx_compiler_test(inline_test_call_like_arg_expr rexx_src/inline_test_call_like_arg_expr.crexx)
add_crexx_compiler_test(inline_test_short_circuit_expr rexx_src/inline_test_short_circuit_expr.crexx)
add_crexx_compiler_test(inline_test_composed_expr_contexts rexx_src/inline_test_composed_expr_contexts.crexx)
add_crexx_compiler_test(inline_test_byvalue_arg_reuse rexx_src/inline_test_byvalue_arg_reuse.crexx)
add_crexx_compiler_test(inline_test_stateful_assembler rexx_src/inline_test_stateful_assembler.crexx)
add_crexx_compiler_test(inline_test_condition_contexts rexx_src/inline_test_condition_contexts.crexx)
add_crexx_compiler_test(inline_test_loop_bound_expr rexx_src/inline_test_loop_bound_expr.crexx)
add_crexx_compiler_test(inline_test_member_receiver_expr rexx_src/inline_test_member_receiver_expr.crexx)
add_crexx_compiler_test(inline_test_named_factory_selector rexx_src/inline_test_named_factory_selector.crexx)
add_crexx_compiler_test(inline_test_ref_opt rexx_src/inline_test_ref_opt.crexx)
add_crexx_compiler_test(inline_test_ref_indexed rexx_src/inline_test_ref_indexed.crexx)
add_crexx_compiler_test(inline_test_ref_stem rexx_src/inline_test_ref_stem.crexx)
add_crexx_compiler_test(inline_test_ref_computed rexx_src/inline_test_ref_computed.crexx)
add_crexx_compiler_test(inline_test_ref_vararg rexx_src/inline_test_ref_vararg.crexx)
add_crexx_compiler_test(inline_test_ref_vararg_runtime rexx_src/inline_test_ref_vararg_runtime.crexx)
add_crexx_compiler_test(inline_test_ref_negative rexx_src/inline_test_ref_negative.crexx)
add_crexx_compiler_test(inline_test_bool_expr_negative rexx_src/inline_test_bool_expr_negative.crexx)
add_crexx_compiler_test(inline_test_vararg rexx_src/inline_test_vararg.crexx)
add_crexx_compiler_test(inline_test_vararg_dynamic rexx_src/inline_test_vararg_dynamic.crexx)
add_crexx_compiler_test(inline_test_vararg_dynamic_loop rexx_src/inline_test_vararg_dynamic_loop.crexx)
add_crexx_compiler_test(inline_test_vararg_negative rexx_src/inline_test_vararg_negative.crexx)
add_crexx_compiler_test(inline_test_exposed_nested_call rexx_src/inline_test_exposed_nested_call.crexx)
add_crexx_compiler_test(inline_test_self_recursive_negative rexx_src/inline_test_self_recursive_negative.crexx)
add_crexx_compiler_test(inline_test_mutual_recursive_negative rexx_src/inline_test_mutual_recursive_negative.crexx)
add_crexx_compiler_test(inline_test_nested_scope_do rexx_src/inline_test_nested_scope_do.crexx)
add_crexx_compiler_test(inline_test_nested_scope_if_do rexx_src/inline_test_nested_scope_if_do.crexx)
add_crexx_compiler_test(inline_test_nested_scope_exposed_call rexx_src/inline_test_nested_scope_exposed_call.crexx)
add_crexx_compiler_test(inline_test_multi_return_assign rexx_src/inline_test_multi_return_assign.crexx)
add_crexx_compiler_test(inline_test_multi_return_expr rexx_src/inline_test_multi_return_expr.crexx)
add_crexx_compiler_test(inline_test_multi_return_call rexx_src/inline_test_multi_return_call.crexx)
add_crexx_compiler_test(inline_test_void_fallthrough_call rexx_src/inline_test_void_fallthrough_call.crexx)
add_crexx_compiler_test(inline_test_array_return_assign rexx_src/inline_test_array_return_assign.crexx)
add_crexx_compiler_test(inline_test_array_return_expr rexx_src/inline_test_array_return_expr.crexx)
add_crexx_compiler_test(inline_test_array_multi_return_expr rexx_src/inline_test_array_multi_return_expr.crexx)
add_crexx_compiler_test(inline_test_array_multi_return_assign rexx_src/inline_test_array_multi_return_assign.crexx)
add_crexx_compiler_test(inline_test_object_return_assign rexx_src/inline_test_object_return_assign.crexx)
add_crexx_compiler_test(inline_test_object_return_expr rexx_src/inline_test_object_return_expr.crexx)
add_crexx_compiler_test(inline_test_object_const_arg_mutation rexx_src/inline_test_object_const_arg_mutation.crexx)
add_crexx_compiler_test(inline_test_object_writable_arg rexx_src/inline_test_object_writable_arg.crexx)
add_crexx_compiler_test(inline_test_array_expr_arg rexx_src/inline_test_array_expr_arg.crexx)
add_crexx_compiler_test(inline_test_object_expr_arg rexx_src/inline_test_object_expr_arg.crexx)
add_crexx_compiler_test(inline_test_array_return_temp rexx_src/inline_test_array_return_temp.crexx)
add_crexx_compiler_test(inline_test_complex_lhs_assign rexx_src/inline_test_complex_lhs_assign.crexx)
add_crexx_compiler_test(inline_test_binary_arg_return rexx_src/inline_test_binary_arg_return.crexx)
add_crexx_compiler_test(inline_test_block_expr_live_sibling rexx_src/inline_test_block_expr_live_sibling.crexx)
add_crexx_compiler_test(inline_test_class_methods rexx_src/inline_test_class_methods.crexx)

add_test(NAME nr06_codegen_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr06_codegen_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr06_codegen_contract.cmake)
set_tests_properties(nr06_codegen_contract PROPERTIES
        LABELS "compiler;performance")

add_test(NAME nr21_fixed_call_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXLINK=$<TARGET_FILE:rxlink>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DMUTATE_FEATURE=$<TARGET_FILE:mutate_rxbin_feature>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DBASIC_RXAS=${CMAKE_SOURCE_DIR}/interpreter/tests/tests_basic.rxas
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr21_fixed_call_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr21_fixed_call_contract.cmake)
set_tests_properties(nr21_fixed_call_contract PROPERTIES
        LABELS "compiler;codegen;rxbin"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxdas;rxvm;rxbvm;library;mutate_rxbin_feature")

add_test(NAME nr14_frozen_parse_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXLINK=$<TARGET_FILE:rxlink>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DMUTATE_FEATURE=$<TARGET_FILE:mutate_rxbin_feature>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr14_frozen_parse_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr14_frozen_parse_contract.cmake)
set_tests_properties(nr14_frozen_parse_contract PROPERTIES
        LABELS "compiler;codegen;rxbin;parse"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxdas;rxvm;rxbvm;library;mutate_rxbin_feature")

add_test(NAME nr15_native_stem_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXLINK=$<TARGET_FILE:rxlink>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DMUTATE_FEATURE=$<TARGET_FILE:mutate_rxbin_feature>
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/nr15_native_stem_contract.rxas
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr15_native_stem_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr15_native_stem_contract.cmake)
set_tests_properties(nr15_native_stem_contract PROPERTIES
        LABELS "compiler;codegen;rxbin;stem;nr15"
        CREXX_PREP_TARGETS "rxas;rxlink;rxdas;rxvm;rxbvm;mutate_rxbin_feature")

add_test(NAME gate_f_channel_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXLINK=$<TARGET_FILE:rxlink>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DMUTATE_FEATURE=$<TARGET_FILE:mutate_rxbin_feature>
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/gate_f_channel_contract.rxas
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/gate_f_channel_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/gate_f_channel_contract.cmake)
set_tests_properties(gate_f_channel_contract PROPERTIES
        LABELS "compiler;codegen;rxas;rxbin;gate-f;channels"
        CREXX_PREP_TARGETS "rxas;rxlink;rxdas;mutate_rxbin_feature")
crexx_label_concurrency_tests(SP02 gate_f_channel_contract)

add_test(NAME nr09_numeric_context_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr09_numeric_context_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr09_numeric_context_contract.cmake)
set_tests_properties(nr09_numeric_context_contract PROPERTIES
        LABELS "compiler;performance;numeric")

add_test(NAME nr09_codegen_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DBENCH_DIR=${CMAKE_SOURCE_DIR}/tests/benchmarks
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr09_codegen_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr09_codegen_contract.cmake)
set_tests_properties(nr09_codegen_contract PROPERTIES
        LABELS "compiler;performance")

add_test(NAME nr26_flow_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/nr26_flow_contract_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/nr26_flow_contract.cmake)
set_tests_properties(nr26_flow_contract PROPERTIES
        LABELS "compiler;performance;flow")

add_test(NAME perf2_04_inline_assembler_effects
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_inline_assembler_effects.crexx
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/perf2_04_inline_assembler_effects_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_04_inline_assembler_effects.cmake)
set_tests_properties(perf2_04_inline_assembler_effects PROPERTIES
        LABELS "compiler;performance;inlining;perf2_04")

add_test(NAME perf2_04_inline_assembler_imports
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DDEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_case_import/dep/case_helpers.crexx
                 -DDEP_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_case_import/dep
                 -DMAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_case_import/main/case_main.crexx
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/perf2_04_inline_assembler_imports_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_04_inline_assembler_imports.cmake)
set_tests_properties(perf2_04_inline_assembler_imports PROPERTIES
        LABELS "compiler;performance;imports;inlining;perf2_04")

add_test(NAME perf2_04_certified_call
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DBIF_SOURCE_DIR=${CMAKE_SOURCE_DIR}/lib/rxfnsb/rexx
                 -DSELECTED_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/selected.crexx
                 -DFALLBACK_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/fallback.crexx
                 -DSPOOF_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/spoof_upper.crexx
                 -DSPOOF_LOWER_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/spoof_lower.crexx
                 -DSPOOF_WORD_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/spoof_word
                 -DHUGE_WIDTH_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_04_certified_call/huge_width.crexx
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/perf2_04_certified_call_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_04_certified_call.cmake)
set_tests_properties(perf2_04_certified_call PROPERTIES
        LABELS "compiler;performance;constants;imports;perf2_04")

add_test(NAME perf2_05_partial_call
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DSELECTED_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_05_partial_call/selected.crexx
                 -DCURSOR_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_05_partial_call/cursor_inline.crexx
                 -DARRAY_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_05_partial_call/array_result.crexx
                 -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/perf2_05_partial_call_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_05_partial_call.cmake)
set_tests_properties(perf2_05_partial_call PROPERTIES
        LABELS "compiler;performance;constants;inlining;perf2_05")

add_test(NAME perf2_07_v3_representation
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DLIBRARY_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_v3_representation.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/perf2_07_v3_representation_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_07_v3_representation.cmake)
set_tests_properties(perf2_07_v3_representation PROPERTIES
        LABELS "compiler;interpreter;representation;reference;performance;perf2_07")

add_test(NAME perf2_07_binary_byvalue_contract
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DLIBRARY_DIR=${CMAKE_BINARY_DIR}/bin
                 -DCODEGEN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_binary_byvalue_codegen.crexx
                 -DSEMANTICS_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_binary_byvalue_semantics.crexx
                 -DIMPORT_DEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_binary_byvalue_import_dep.crexx
                 -DIMPORT_MAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/perf2_07_binary_byvalue_import_main.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/perf2_07_binary_byvalue_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/perf2_07_binary_byvalue_contract.cmake)
set_tests_properties(perf2_07_binary_byvalue_contract PROPERTIES
        LABELS "compiler;interpreter;binary;inlining;performance;perf2_07")

# Macro to add a run test
macro(add_crexx_run_test NAME SOURCE_FILE)
    set(RUNTIME_MODULES ${ARGN})

    # Keep no-opt and opt runtime coverage paired so semantic shifts in
    # argument handling or inlining are caught even when compiler-output
    # goldens still match within one mode.
    
    # No-opt script
    set(NOOPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_noopt.cmake")
    set(NOOPT_CONTENT "set(full_out \"\")\n")
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_NOOPT_FLAG} -o \"${NAME}_run_noopt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
    
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_noopt.rxbin\" \"${NAME}_run_noopt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
    
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${NAME}_run_noopt.rxbin\" ${RUNTIME_MODULES} OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
    
    string(APPEND NOOPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND NOOPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND NOOPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${NOOPT_SCRIPT}" CONTENT "${NOOPT_CONTENT}")

    add_test(NAME ${NAME}_run_noopt
             COMMAND ${CMAKE_COMMAND} -P "${NOOPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    # Opt script
    set(OPT_LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_opt_linked.cmake")
    crexx_write_linked_runtime_script(
            OUTPUT "${OPT_LINK_SCRIPT}"
            NAME "${NAME}_run_opt"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNNER "${CREXX_RXVM_EXECUTABLE}"
            RXLINK "$<TARGET_FILE:rxlink>"
            PROGRAM "${NAME}_run_opt.rxbin"
            EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin" ${RUNTIME_MODULES}
    )
    set(OPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_opt.cmake")
    set(OPT_CONTENT "set(full_out \"\")\n")
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}_run_opt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
    
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_opt.rxbin\" \"${NAME}_run_opt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${OPT_LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"linked rxvm failed: \${out}\")\nendif()\n")
    
    string(APPEND OPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${OPT_SCRIPT}" CONTENT "${OPT_CONTENT}")

    add_test(NAME ${NAME}_run_opt
             COMMAND ${CMAKE_COMMAND} -P "${OPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME}_run_opt PROPERTIES LABELS "linked_opt")
endmacro()

function(add_crexx_reference_source_test FIXTURE RUNNER MODE)
    set(NAME "${FIXTURE}_${RUNNER}_${MODE}")
    set(BASE "${NAME}")
    set(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ws")
    set(RXC_FLAGS "")
    set(RXAS_FLAGS "")
    set(LABELS "reference;compiler;source")
    if(RUNNER STREQUAL "rxvm")
        set(RUNNER_EXECUTABLE "${CREXX_RXVM_EXECUTABLE}")
    else()
        set(RUNNER_EXECUTABLE "$<TARGET_FILE:${RUNNER}>")
    endif()

    if(MODE STREQUAL "noopt")
        set(RXC_FLAGS "-n")
        set(RXAS_FLAGS "-n")
    else()
        set(LABELS "reference;compiler;source;linked_opt")
    endif()

    if(MODE STREQUAL "opt")
        set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_linked.cmake")
        crexx_write_linked_runtime_script(
                OUTPUT "${LINK_SCRIPT}"
                NAME "${NAME}"
                WORKING_DIRECTORY "${WORK_DIR}"
                RUNNER "${RUNNER_EXECUTABLE}"
                RXLINK "$<TARGET_FILE:rxlink>"
                PROGRAM "${BASE}.rxbin"
                EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin"
        )
    endif()

    set(CONTENT "set(full_out \"\")\n")
    string(APPEND CONTENT "file(MAKE_DIRECTORY \"${WORK_DIR}\")\n")
    string(APPEND CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${RXC_FLAGS} -o \"${BASE}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/${FIXTURE}.crexx\" WORKING_DIRECTORY \"${WORK_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
    string(APPEND CONTENT "file(READ \"${WORK_DIR}/${BASE}.rxas\" rxas)\n")
    if(FIXTURE STREQUAL "reference_source_dereference_live" OR
       FIXTURE STREQUAL "reference_source_iterator" OR
       FIXTURE STREQUAL "reference_source_inline_lifetime")
        string(APPEND CONTENT "foreach(op IN ITEMS mkref deref linkref refvalid)\n")
    elseif(FIXTURE STREQUAL "reference_source_contract")
        string(APPEND CONTENT "foreach(op IN ITEMS mkref deref refvalid refsame)\n")
    else()
        string(APPEND CONTENT "foreach(op IN ITEMS mkref deref refvalid)\n")
    endif()
    string(APPEND CONTENT "  string(FIND \"\${rxas}\" \"\${op} \" op_pos)\n")
    string(APPEND CONTENT "  if(op_pos EQUAL -1)\n")
    string(APPEND CONTENT "    message(FATAL_ERROR \"Expected \${op} in generated reference source RXAS\")\n")
    string(APPEND CONTENT "  endif()\n")
    string(APPEND CONTENT "endforeach()\n")
    string(APPEND CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" ${RXAS_FLAGS} -o \"${BASE}.rxbin\" \"${BASE}\" WORKING_DIRECTORY \"${WORK_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
    if(MODE STREQUAL "opt")
        string(APPEND CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"linked ${RUNNER} failed: \${out}\")\nendif()\n")
    else()
        string(APPEND CONTENT "execute_process(COMMAND \"${RUNNER_EXECUTABLE}\" \"${BASE}.rxbin\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" WORKING_DIRECTORY \"${WORK_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"${RUNNER} failed: \${out}\")\nendif()\n")
    endif()
    string(APPEND CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${FIXTURE}.out\" golden_out)\n")
    string(APPEND CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${SCRIPT}" CONTENT "${CONTENT}")

    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES
            LABELS "${LABELS}")
endfunction()

foreach(_crexx_reference_fixture IN ITEMS reference_source_explicit reference_source_contract reference_source_iterator reference_source_dereference_live reference_source_inline_lifetime reference_source_block_lifetime)
    foreach(_crexx_runner IN ITEMS rxvm)
        foreach(_crexx_mode IN ITEMS noopt opt)
            add_crexx_reference_source_test(${_crexx_reference_fixture} ${_crexx_runner} ${_crexx_mode})
        endforeach()
    endforeach()
endforeach()

function(add_crexx_interface_import_run_test NAME MAIN_FILE DEP_FILE MODE)
    get_filename_component(MAIN_BASENAME "${MAIN_FILE}" NAME)
    get_filename_component(DEP_BASENAME "${DEP_FILE}" NAME)
    get_filename_component(DEP_MODULE "${DEP_FILE}" NAME_WE)

    foreach(TEST_MODE IN ITEMS noopt opt)
        if(TEST_MODE STREQUAL "noopt")
            set(COMPILER_FLAG "${COMPILER_NOOPT_FLAG}")
            set(TEST_SUFFIX "_run_noopt")
        else()
            set(COMPILER_FLAG "${COMPILER_OPT_FLAG}")
            set(TEST_SUFFIX "_run_opt")
        endif()

        set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_${MODE}_${TEST_MODE}_ws")
        if(TEST_MODE STREQUAL "opt")
            set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_${MODE}_${TEST_MODE}_linked.cmake")
            if(MODE STREQUAL "source")
                set(LINK_EXTRA_ARGS
                        "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                        "${WORK_DIR}/${DEP_MODULE}${TEST_SUFFIX}.rxbin")
            else()
                set(LINK_EXTRA_ARGS
                        "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                        "${WORK_DIR}/${DEP_MODULE}.rxbin")
            endif()
            crexx_write_linked_runtime_script(
                    OUTPUT "${LINK_SCRIPT}"
                    NAME "${NAME}${TEST_SUFFIX}"
                    WORKING_DIRECTORY "${WORK_DIR}"
                    RUNNER "${CREXX_RXVM_EXECUTABLE}"
                    RXLINK "$<TARGET_FILE:rxlink>"
                    PROGRAM "${WORK_DIR}/main${TEST_SUFFIX}.rxbin"
                    EXTRA_ARGS ${LINK_EXTRA_ARGS}
            )
        endif()

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_${MODE}_${TEST_MODE}.cmake")
        set(SCRIPT_CONTENT "set(full_out \"\")\n")
        string(APPEND SCRIPT_CONTENT "set(work \"${WORK_DIR}\")\n")
        string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_FILE}\" \"\${work}/${MAIN_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage main source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE}\" \"\${work}/${DEP_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage dependency source\")\nendif()\n")

        if(MODE STREQUAL "source")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"main${TEST_SUFFIX}\" \"\${work}/${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"main${TEST_SUFFIX}.rxbin\" \"main${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${DEP_MODULE}${TEST_SUFFIX}\" \"\${work}/${DEP_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import dependency: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_MODULE}${TEST_SUFFIX}.rxbin\" \"${DEP_MODULE}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import dependency: \${out}\")\nendif()\n")
            if(TEST_MODE STREQUAL "opt")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            else()
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_MODULE}${TEST_SUFFIX}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            endif()
        else()
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${DEP_MODULE}${TEST_SUFFIX}\" \"\${work}/${DEP_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on binary-import dependency: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_MODULE}.rxbin\" \"${DEP_MODULE}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on binary-import dependency: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "file(REMOVE \"\${work}/${DEP_BASENAME}\")\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"main${TEST_SUFFIX}\" \"\${work}/${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on binary-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"main${TEST_SUFFIX}.rxbin\" \"main${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on binary-import main: \${out}\")\nendif()\n")
            if(TEST_MODE STREQUAL "opt")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            else()
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_MODULE}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            endif()
        endif()

        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}${TEST_SUFFIX}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        if(TEST_MODE STREQUAL "opt")
            set_tests_properties(${NAME}${TEST_SUFFIX} PROPERTIES LABELS "linked_opt")
        endif()
    endforeach()
endfunction()

function(add_crexx_dual_import_run_test NAME MAIN_FILE DEP_FILE_A DEP_FILE_B MODE)
    get_filename_component(MAIN_BASENAME "${MAIN_FILE}" NAME)
    get_filename_component(DEP_A_BASENAME "${DEP_FILE_A}" NAME)
    get_filename_component(DEP_A_MODULE "${DEP_FILE_A}" NAME_WE)
    get_filename_component(DEP_B_BASENAME "${DEP_FILE_B}" NAME)
    get_filename_component(DEP_B_MODULE "${DEP_FILE_B}" NAME_WE)

    foreach(TEST_MODE IN ITEMS noopt opt)
        if(TEST_MODE STREQUAL "noopt")
            set(COMPILER_FLAG "${COMPILER_NOOPT_FLAG}")
            set(TEST_SUFFIX "_run_noopt")
        else()
            set(COMPILER_FLAG "${COMPILER_OPT_FLAG}")
            set(TEST_SUFFIX "_run_opt")
        endif()

        set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_${MODE}_${TEST_MODE}_ws")
        if(TEST_MODE STREQUAL "opt")
            set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_${MODE}_${TEST_MODE}_linked.cmake")
            if(MODE STREQUAL "source")
                set(LINK_EXTRA_ARGS
                        "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                        "${WORK_DIR}/${DEP_A_MODULE}${TEST_SUFFIX}.rxbin"
                        "${WORK_DIR}/${DEP_B_MODULE}${TEST_SUFFIX}.rxbin")
            else()
                set(LINK_EXTRA_ARGS
                        "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                        "${WORK_DIR}/${DEP_A_MODULE}.rxbin"
                        "${WORK_DIR}/${DEP_B_MODULE}.rxbin")
            endif()
            crexx_write_linked_runtime_script(
                    OUTPUT "${LINK_SCRIPT}"
                    NAME "${NAME}${TEST_SUFFIX}"
                    WORKING_DIRECTORY "${WORK_DIR}"
                    RUNNER "${CREXX_RXVM_EXECUTABLE}"
                    RXLINK "$<TARGET_FILE:rxlink>"
                    PROGRAM "${WORK_DIR}/main${TEST_SUFFIX}.rxbin"
                    EXTRA_ARGS ${LINK_EXTRA_ARGS}
            )
        endif()

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_${MODE}_${TEST_MODE}.cmake")
        set(SCRIPT_CONTENT "set(full_out \"\")\n")
        string(APPEND SCRIPT_CONTENT "set(work \"${WORK_DIR}\")\n")
        string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_FILE}\" \"\${work}/${MAIN_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage main source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE_A}\" \"\${work}/${DEP_A_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage dependency A source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE_B}\" \"\${work}/${DEP_B_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage dependency B source\")\nendif()\n")

        if(MODE STREQUAL "source")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"main${TEST_SUFFIX}\" \"\${work}/${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"main${TEST_SUFFIX}.rxbin\" \"main${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import main: \${out}\")\nendif()\n")
            foreach(DEP_SPEC IN ITEMS "${DEP_A_MODULE};${DEP_A_BASENAME}" "${DEP_B_MODULE};${DEP_B_BASENAME}")
                list(GET DEP_SPEC 0 DEP_MODULE)
                list(GET DEP_SPEC 1 DEP_BASENAME)
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${DEP_MODULE}${TEST_SUFFIX}\" \"\${work}/${DEP_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
                string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
                string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import dependency ${DEP_MODULE}: \${out}\")\nendif()\n")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_MODULE}${TEST_SUFFIX}.rxbin\" \"${DEP_MODULE}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
                string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
                string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import dependency ${DEP_MODULE}: \${out}\")\nendif()\n")
            endforeach()
            if(TEST_MODE STREQUAL "opt")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            else()
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_A_MODULE}${TEST_SUFFIX}.rxbin\" \"\${work}/${DEP_B_MODULE}${TEST_SUFFIX}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            endif()
        else()
            foreach(DEP_SPEC IN ITEMS "${DEP_A_MODULE};${DEP_A_BASENAME}" "${DEP_B_MODULE};${DEP_B_BASENAME}")
                list(GET DEP_SPEC 0 DEP_MODULE)
                list(GET DEP_SPEC 1 DEP_BASENAME)
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${DEP_MODULE}${TEST_SUFFIX}\" \"\${work}/${DEP_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
                string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
                string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on binary-import dependency ${DEP_MODULE}: \${out}\")\nendif()\n")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${DEP_MODULE}.rxbin\" \"${DEP_MODULE}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
                string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
                string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on binary-import dependency ${DEP_MODULE}: \${out}\")\nendif()\n")
                string(APPEND SCRIPT_CONTENT "file(REMOVE \"\${work}/${DEP_BASENAME}\")\n")
            endforeach()
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"main${TEST_SUFFIX}\" \"\${work}/${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on binary-import main: \${out}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"main${TEST_SUFFIX}.rxbin\" \"main${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
            string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on binary-import main: \${out}\")\nendif()\n")
            if(TEST_MODE STREQUAL "opt")
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            else()
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_A_MODULE}.rxbin\" \"\${work}/${DEP_B_MODULE}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
            endif()
        endif()

        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}${TEST_SUFFIX}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        if(TEST_MODE STREQUAL "opt")
            set_tests_properties(${NAME}${TEST_SUFFIX} PROPERTIES LABELS "linked_opt")
        endif()
    endforeach()
endfunction()

function(add_crexx_isolated_run_test NAME SOURCE_FILE)
    get_filename_component(SOURCE_BASENAME "${SOURCE_FILE}" NAME)

    foreach(TEST_MODE IN ITEMS noopt opt)
        if(TEST_MODE STREQUAL "noopt")
            set(COMPILER_FLAG "${COMPILER_NOOPT_FLAG}")
            set(TEST_SUFFIX "_run_noopt")
        else()
            set(COMPILER_FLAG "${COMPILER_OPT_FLAG}")
            set(TEST_SUFFIX "_run_opt")
        endif()

        set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_isolated_${TEST_MODE}_ws")
        if(TEST_MODE STREQUAL "opt")
            set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_isolated_${TEST_MODE}_linked.cmake")
            crexx_write_linked_runtime_script(
                    OUTPUT "${LINK_SCRIPT}"
                    NAME "${NAME}${TEST_SUFFIX}"
                    WORKING_DIRECTORY "${WORK_DIR}"
                    RUNNER "${CREXX_RXVM_EXECUTABLE}"
                    RXLINK "$<TARGET_FILE:rxlink>"
                    PROGRAM "${WORK_DIR}/${NAME}${TEST_SUFFIX}.rxbin"
                    EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin"
            )
        endif()

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_isolated_${TEST_MODE}.cmake")
        set(SCRIPT_CONTENT "set(full_out \"\")\n")
        string(APPEND SCRIPT_CONTENT "set(work \"${WORK_DIR}\")\n")
        string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" \"\${work}/${SOURCE_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${NAME}${TEST_SUFFIX}\" \"\${work}/${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}${TEST_SUFFIX}.rxbin\" \"${NAME}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
        if(TEST_MODE STREQUAL "opt")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        else()
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${NAME}${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        endif()
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"linked rxvm failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}${TEST_SUFFIX}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        if(TEST_MODE STREQUAL "opt")
            set_tests_properties(${NAME}${TEST_SUFFIX} PROPERTIES LABELS "linked_opt")
        endif()
    endforeach()
endfunction()

function(add_crexx_isolated_run_fail_test NAME SOURCE_FILE)
    get_filename_component(SOURCE_BASENAME "${SOURCE_FILE}" NAME)

    foreach(TEST_MODE IN ITEMS noopt opt)
        if(TEST_MODE STREQUAL "noopt")
            set(COMPILER_FLAG "${COMPILER_NOOPT_FLAG}")
            set(TEST_SUFFIX "_run_noopt")
        else()
            set(COMPILER_FLAG "${COMPILER_OPT_FLAG}")
            set(TEST_SUFFIX "_run_opt")
        endif()

        set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_isolated_fail_${TEST_MODE}_ws")
        if(TEST_MODE STREQUAL "opt")
            set(LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_isolated_fail_${TEST_MODE}_linked.cmake")
            crexx_write_linked_runtime_script(
                    OUTPUT "${LINK_SCRIPT}"
                    NAME "${NAME}${TEST_SUFFIX}"
                    WORKING_DIRECTORY "${WORK_DIR}"
                    RUNNER "${CREXX_RXVM_EXECUTABLE}"
                    RXLINK "$<TARGET_FILE:rxlink>"
                    PROGRAM "${WORK_DIR}/${NAME}${TEST_SUFFIX}.rxbin"
                    ALLOW_RUN_FAILURE
                    EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin"
            )
        endif()

        set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_isolated_fail_${TEST_MODE}.cmake")
        set(SCRIPT_CONTENT "set(full_out \"\")\n")
        string(APPEND SCRIPT_CONTENT "set(work \"${WORK_DIR}\")\n")
        string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" \"\${work}/${SOURCE_BASENAME}\" RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage source\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_FLAG} -o \"${NAME}${TEST_SUFFIX}\" \"\${work}/${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}${TEST_SUFFIX}.rxbin\" \"${NAME}${TEST_SUFFIX}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
        if(TEST_MODE STREQUAL "opt")
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        else()
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${NAME}${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
        endif()
        string(APPEND SCRIPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
        string(APPEND SCRIPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
        string(APPEND SCRIPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
        string(APPEND SCRIPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")

        file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

        add_test(NAME ${NAME}${TEST_SUFFIX}
                 COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        if(TEST_MODE STREQUAL "opt")
            set_tests_properties(${NAME}${TEST_SUFFIX} PROPERTIES LABELS "linked_opt")
        endif()
    endforeach()
endfunction()

macro(add_crexx_run_args_test NAME SOURCE_FILE)
    set(PROGRAM_ARGS ${ARGN})
    set(PROGRAM_ARGS_TEXT "")
    foreach(PROGRAM_ARG ${PROGRAM_ARGS})
        string(APPEND PROGRAM_ARGS_TEXT " \"${PROGRAM_ARG}\"")
    endforeach()

    set(NOOPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_args_noopt.cmake")
    set(NOOPT_CONTENT "set(full_out \"\")\n")
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_NOOPT_FLAG} -o \"${NAME}_run_args_noopt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")

    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_args_noopt.rxbin\" \"${NAME}_run_args_noopt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")

    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${NAME}_run_args_noopt.rxbin\" -a${PROGRAM_ARGS_TEXT} OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND NOOPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")

    string(APPEND NOOPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND NOOPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND NOOPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND NOOPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${NOOPT_SCRIPT}" CONTENT "${NOOPT_CONTENT}")

    add_test(NAME ${NAME}_run_args_noopt
             COMMAND ${CMAKE_COMMAND} -P "${NOOPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    set(OPT_LINK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_args_opt_linked.cmake")
    crexx_write_linked_runtime_script(
            OUTPUT "${OPT_LINK_SCRIPT}"
            NAME "${NAME}_run_args_opt"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNNER "${CREXX_RXVM_EXECUTABLE}"
            RXLINK "$<TARGET_FILE:rxlink>"
            PROGRAM "${NAME}_run_args_opt.rxbin"
            EXTRA_ARGS "${CMAKE_BINARY_DIR}/bin/library.rxbin" -a ${PROGRAM_ARGS}
    )

    set(OPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_args_opt.cmake")
    set(OPT_CONTENT "set(full_out \"\")\n")
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}_run_args_opt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_args_opt.rxbin\" \"${NAME}_run_args_opt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -P \"${OPT_LINK_SCRIPT}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"linked rxvm failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${OPT_SCRIPT}" CONTENT "${OPT_CONTENT}")

    add_test(NAME ${NAME}_run_args_opt
             COMMAND ${CMAKE_COMMAND} -P "${OPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME}_run_args_opt PROPERTIES LABELS "linked_opt")
endmacro()

add_crexx_run_test(repro_dotted_index rexx_src/repro_dotted_index.crexx)
add_crexx_run_test(int64_contract rexx_src/int64_contract.crexx)
add_crexx_run_test(repro_expand_until rexx_src/repro_expand_until.crexx)
add_crexx_run_test(repro_array_arg rexx_src/repro_array_arg.crexx)
add_crexx_run_test(repro_array_trailing_concat rexx_src/repro_array_trailing_concat.crexx)
add_crexx_run_test(stem_index_concat rexx_src/stem_index_concat.crexx)
add_crexx_run_test(repro_opt_args rexx_src/repro_opt_args.crexx)
add_crexx_run_test(arg_semantics_scalar rexx_src/arg_semantics_scalar.crexx)
add_crexx_run_test(arg_semantics_array rexx_src/arg_semantics_array.crexx)
add_crexx_run_test(multiple_exposed_arrays rexx_src/multiple_exposed_arrays.crexx)
add_crexx_run_test(arg_semantics_object rexx_src/arg_semantics_object.crexx)
add_crexx_run_test(object_reference_regression rexx_src/object_reference_regression.crexx)
add_crexx_run_test(basic_loops rexx_src/basic_loops.crexx)
add_crexx_run_test(basic_strings rexx_src/basic_strings.crexx)
add_crexx_run_test(basic_arrays rexx_src/basic_arrays.crexx)
add_crexx_run_test(array_mutation_statements rexx_src/array_mutation_statements.crexx)
add_crexx_run_test(block_expr_basic rexx_src/block_expr_basic.crexx)
add_crexx_run_test(block_expr_short_circuit rexx_src/block_expr_short_circuit.crexx)
add_crexx_run_test(repro_args_parse rexx_src/repro_args_parse.crexx)
add_crexx_run_test(repro_args_alias rexx_src/repro_args_alias.crexx)
add_crexx_run_test(repro_sconcat rexx_src/repro_sconcat.crexx)
add_crexx_run_test(repro_strict_compare rexx_src/repro_strict_compare.crexx)
add_crexx_run_test(08_select rexx_src/08_select.crexx)
add_crexx_run_test(select_dispatch_integer rexx_src/select_dispatch_integer.crexx)
add_crexx_run_test(select_dispatch_fallback rexx_src/select_dispatch_fallback.crexx)
add_crexx_run_test(select_dispatch_mixed rexx_src/select_dispatch_mixed.crexx)
add_crexx_run_test(select_dispatch_general rexx_src/select_dispatch_general.crexx)
add_crexx_run_test(select_dispatch_strings rexx_src/select_dispatch_strings.crexx)
add_crexx_run_test(repro_duplicate_call_argument rexx_src/repro_duplicate_call_argument.crexx)
add_crexx_run_test(nr06_call_window_scalar rexx_src/nr06_call_window_scalar.crexx)
add_crexx_run_test(13_stems rexx_src/13_stems.crexx)
add_crexx_run_test(repro_multi_tail_stems rexx_src/repro_multi_tail_stems.crexx)
add_crexx_run_test(repro_rexx_compare_method rexx_src/repro_rexx_compare_method.crexx "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
add_crexx_run_test(repro_rexx_format_sign_gap rexx_src/repro_rexx_format_sign_gap.crexx "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
add_crexx_run_test(numeric_mixed_compare rexx_src/numeric_mixed_compare.crexx)
add_crexx_run_test(repro_implicit_address_abuttal rexx_src/repro_implicit_address_abuttal.crexx)
add_crexx_run_test(implicit_address_indexed_stem rexx_src/implicit_address_indexed_stem.crexx)
add_crexx_run_test(test_explicit_address rexx_src/test_explicit_address.crexx)
add_crexx_run_test(test_address_redirect rexx_src/test_address_redirect.crexx)
crexx_label_concurrency_tests(SP04
        test_address_redirect_run_noopt
        test_address_redirect_run_opt)
add_crexx_run_test(address_crexx_flush_visible rexx_src/address_crexx_flush_visible.crexx)
add_crexx_run_test(lineout_flush_visible rexx_src/lineout_flush_visible.crexx)
add_crexx_run_test(optional_arg_presence_copy rexx_src/optional_arg_presence_copy.crexx)
add_crexx_run_test(test_implicit_address rexx_src/test_implicit_address.crexx)
add_crexx_run_test(test_parse_statement rexx_src/test_parse_statement.crexx)
add_crexx_isolated_run_test(qualified_namespace_same_module rexx_src/qualified_namespace_same_module.crexx)
add_crexx_isolated_run_test(class_attr_string_promotion rexx_src/class_attr_string_promotion.crexx)
add_crexx_run_test(class_attr_register_views rexx_src/class_attr_register_views.crexx)
add_crexx_run_test(constants_levelb_payloads rexx_src/constants_levelb_payloads.crexx)
add_crexx_run_test(binary_memory_source rexx_src/binary_memory_source.crexx)
add_crexx_run_test(binary_memory_span_oob rexx_src/binary_memory_span_oob.crexx)
add_crexx_run_test(binary_memory_packed_lookup rexx_src/binary_memory_packed_lookup.crexx)
add_crexx_run_test(packed_numeric_source rexx_src/packed_numeric_source.crexx)
add_crexx_run_test(class_binary_attribute_store rexx_src/class_binary_attribute_store.crexx)
add_crexx_run_test(literal_continuation rexx_src/literal_continuation.crexx)
add_crexx_isolated_run_test(qualified_bif_direct_namespace rexx_src/qualified_bif_direct_namespace.crexx)
add_crexx_dual_import_run_test(qualified_namespace_import_source rexx_src/qualified_namespace_main.crexx rexx_src/qualified_namespace_dep_a.crexx rexx_src/qualified_namespace_dep_b.crexx source)
add_crexx_dual_import_run_test(qualified_namespace_import_binary rexx_src/qualified_namespace_main.crexx rexx_src/qualified_namespace_dep_a.crexx rexx_src/qualified_namespace_dep_b.crexx binary)
add_crexx_interface_import_run_test(optional_binary_import_source rexx_src/optional_binary_import_main.crexx rexx_src/optional_binary_import_dep.crexx source)
add_crexx_interface_import_run_test(optional_binary_import_binary rexx_src/optional_binary_import_main.crexx rexx_src/optional_binary_import_dep.crexx binary)
add_crexx_isolated_run_test(interface_tracer rexx_src/interface_tracer.crexx)
add_crexx_isolated_run_test(interface_runtime_dispatch_multi rexx_src/interface_runtime_dispatch_multi.crexx)
add_crexx_isolated_run_test(interface_method_arg_same_module rexx_src/interface_method_arg_same_module.crexx)
add_crexx_isolated_run_test(interface_method_object_arg_same_module rexx_src/interface_method_object_arg_same_module.crexx)
add_crexx_isolated_run_test(interface_signature_static_positive rexx_src/interface_signature_static_positive.crexx)
add_crexx_isolated_run_test(interface_factory_tiebreak rexx_src/interface_factory_tiebreak.crexx)
add_crexx_isolated_run_test(interface_factory_signal_restore rexx_src/interface_factory_signal_restore.crexx)
add_crexx_isolated_run_fail_test(interface_no_impl rexx_src/interface_no_impl.crexx)
add_crexx_compiler_test(interface_default_codegen rexx_src/interface_default_codegen.crexx)
add_crexx_isolated_run_test(interface_default_same_module rexx_src/interface_default_same_module.crexx)
add_crexx_isolated_run_test(interface_multi_interface_same_module rexx_src/interface_multi_interface_same_module.crexx)
add_crexx_isolated_run_test(interface_showcase_same_module rexx_src/interface_showcase_same_module.crexx)
add_crexx_isolated_run_test(type_ops_showcase rexx_src/type_ops_showcase.crexx)
add_crexx_isolated_run_test(interface_match_same_module rexx_src/interface_match_same_module.crexx)
add_crexx_isolated_run_fail_test(interface_match_reject_single rexx_src/interface_match_reject_single.crexx)
add_crexx_isolated_run_fail_test(type_ops_fail rexx_src/type_ops_fail.crexx)

set(GATE_F_LEVELB_CONTRACT_SCRIPT
    "${CMAKE_CURRENT_BINARY_DIR}/gate_f_levelb_contract.cmake")
file(GENERATE OUTPUT "${GATE_F_LEVELB_CONTRACT_SCRIPT}" CONTENT
"execute_process(
    COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n
            -o \"gate_f_levelb_contract\"
            \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/gate_f_levelb_contract.crexx\"
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
    OUTPUT_VARIABLE rxc_out
    ERROR_VARIABLE rxc_err
    RESULT_VARIABLE rxc_result)
if(NOT rxc_result EQUAL 0)
  message(FATAL_ERROR \"Gate F Level B contract rxc failure: \\n\${rxc_out}\\n\${rxc_err}\")
endif()
execute_process(
    COMMAND \"$<TARGET_FILE:rxas>\" -o \"gate_f_levelb_contract.rxbin\"
            \"gate_f_levelb_contract\"
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
    OUTPUT_VARIABLE rxas_out
    ERROR_VARIABLE rxas_err
    RESULT_VARIABLE rxas_result)
if(NOT rxas_result EQUAL 0)
  message(FATAL_ERROR \"Gate F Level B contract rxas failure: \\n\${rxas_out}\\n\${rxas_err}\")
endif()
execute_process(
    COMMAND \"$<TARGET_FILE:rxdas>\" \"gate_f_levelb_contract.rxbin\"
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
    OUTPUT_VARIABLE rxdas_out
    ERROR_VARIABLE rxdas_err
    RESULT_VARIABLE rxdas_result)
if(NOT rxdas_result EQUAL 0)
  message(FATAL_ERROR \"Gate F Level B contract rxdas failure: \\n\${rxdas_out}\\n\${rxdas_err}\")
endif()
foreach(required IN ITEMS
        \"gate_f_levelb_contract.taskpool\"
        \"gate_f_levelb_contract.taskscope\"
        \"gate_f_levelb_contract.task\"
        \"gate_f_levelb_contract.tasktarget\"
        \"gate_f_levelb_contract.taskwork\"
        \"gate_f_levelb_contract.taskcontext\"
        \"gate_f_levelb_contract.completion\"
        \"gate_f_levelb_contract.channel\"
        \"gate_f_levelb_contract.channelrequest\"
        \"gate_f_levelb_contract.channelvalue\"
        \"gate_f_levelb_contract.channelcodec\"
        \"gate_f_levelb_contract.byteendpoint\"
        \"gate_f_levelb_contract.serviceref\"
        \"gate_f_levelb_contract.transferbuffer\")
  if(NOT rxdas_out MATCHES \"\${required}.*\\\\.interface\")
    message(FATAL_ERROR \"Gate F Level B contract is missing interface metadata for \${required}\")
  endif()
endforeach()
")
add_test(NAME gate_f_levelb_contract
         COMMAND ${CMAKE_COMMAND} -P "${GATE_F_LEVELB_CONTRACT_SCRIPT}")
set_tests_properties(gate_f_levelb_contract PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        LABELS "compiler;levelb;concurrency;gate_f")
crexx_label_concurrency_tests(SP05 gate_f_levelb_contract)

# Gate F F1f is a linked end-to-end contract: task target placeholders must
# travel through rxc and rxas, be sealed by rxlink, and then execute identically
# on both concrete VM implementations.  Keep optimized and no-opt images paired
# because expression planning and inlining meet at this boundary.
function(add_gate_f_levelg_conformance RUNNER MODE)
    set(TEST_NAME "gate_f_levelg_${RUNNER}_${MODE}")
    set(BASE "${TEST_NAME}")
    set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_ws")
    set(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.cmake")
    if(MODE STREQUAL "noopt")
        set(RXC_FLAGS "-n")
        set(RXAS_FLAGS "-n")
    else()
        set(RXC_FLAGS "")
        set(RXAS_FLAGS "")
    endif()

    file(GENERATE OUTPUT "${SCRIPT}" CONTENT
"file(MAKE_DIRECTORY \"${WORK_DIR}\")
execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${RXC_FLAGS}
          -o \"${BASE}\"
          \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/gate_f_levelg_conformance.crexx\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Gate F Level G rxc failure:\\n\${out}\\n\${err}\")
endif()
file(READ \"${WORK_DIR}/${BASE}.rxas\" rxas)
string(REGEX MATCHALL
       \"load r[0-9]+,\\\"gate_f_levelg_conformance\\.(first|second|third)\\\"\"
       dependency_targets \"\${rxas}\")
list(LENGTH dependency_targets dependency_count)
if(dependency_count LESS 3)
  message(FATAL_ERROR \"Missing Gate F dependency target loads in generated RXAS\")
endif()
list(GET dependency_targets 0 dependency_first)
list(GET dependency_targets 1 dependency_second)
list(GET dependency_targets 2 dependency_third)
if(NOT dependency_first MATCHES \"\\.first\\\"$\" OR
   NOT dependency_second MATCHES \"\\.second\\\"$\" OR
   NOT dependency_third MATCHES \"\\.third\\\"$\")
  message(FATAL_ERROR
          \"Dependency submission order is not first, second, third: \${dependency_targets}\")
endif()
string(REGEX MATCH
       \"sigpop \\\"FAILURE\\\"[\\r\\n]+[ ]+br l[0-9]+bexprend\"
       parallel_leave_unwind \"\${rxas}\")
if(parallel_leave_unwind STREQUAL \"\")
  message(FATAL_ERROR \"Parallel LEAVE WITH did not unwind its signal handler\")
endif()
string(FIND \"\${rxas}\" \"sigpush \\\"POSIX_CHLD\\\"\" parallel_posix_chld_handler)
if(NOT parallel_posix_chld_handler EQUAL -1)
  message(FATAL_ERROR
          \"Compiler-generated parallel guard intercepted POSIX_CHLD\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxas>\" ${RXAS_FLAGS}
          -o \"${BASE}.rxbin\" \"${BASE}\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Gate F Level G rxas failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxlink>\" -o \"${BASE}_linked.rxbin\"
          \"${BASE}.rxbin\"
          \"${CMAKE_BINARY_DIR}/bin/library.rxbin\"
          \"${CMAKE_BINARY_DIR}/bin/classlib.rxbin\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Gate F Level G rxlink failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:${RUNNER}>\" \"${BASE}_linked.rxbin\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
set(full_out \"\${out}\${err}\")
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Gate F Level G ${RUNNER} failure:\\n\${full_out}\")
endif()
foreach(required IN ITEMS
        \"ordinary\" \"GOOD\" \"writer-a\" \"writer-b\"
        \"PASS: Gate F Level G conformance\")
  string(FIND \"\${full_out}\" \"\${required}\" required_pos)
  if(required_pos EQUAL -1)
    message(FATAL_ERROR \"Missing runtime evidence '\${required}':\\n\${full_out}\")
  endif()
endforeach()
foreach(forbidden IN ITEMS \"BAD\" \"FAIL: Gate F\")
  string(FIND \"\${full_out}\" \"\${forbidden}\" forbidden_pos)
  if(NOT forbidden_pos EQUAL -1)
    message(FATAL_ERROR \"Forbidden runtime output '\${forbidden}':\\n\${full_out}\")
  endif()
endforeach()
")

    add_test(NAME ${TEST_NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT}")
    set_tests_properties(${TEST_NAME} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            LABELS "compiler;levelg;concurrency;gate_f;f1f;linked;${MODE};${RUNNER}"
            RESOURCE_LOCK gate_f_concurrency_runtime
            TIMEOUT 180)
    crexx_label_concurrency_tests(SP06 ${TEST_NAME})
    crexx_label_concurrency_tests(SP08 ${TEST_NAME})
endfunction()

add_gate_f_levelg_conformance(rxbvm noopt)
add_gate_f_levelg_conformance(rxbvm opt)
if(CREXX_THREADED_VM_SUPPORTED)
    add_gate_f_levelg_conformance(rxtvm noopt)
    add_gate_f_levelg_conformance(rxtvm opt)
endif()

function(add_gate_f_task_method_import RUNNER MODE)
    set(TEST_NAME "gate_f_task_method_import_${RUNNER}_${MODE}")
    set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_ws")
    set(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.cmake")
    if(MODE STREQUAL "noopt")
        set(RXC_FLAGS "-n")
        set(RXAS_FLAGS "-n")
    else()
        set(RXC_FLAGS "")
        set(RXAS_FLAGS "")
    endif()
    file(GENERATE OUTPUT "${SCRIPT}" CONTENT
"file(MAKE_DIRECTORY \"${WORK_DIR}\")
execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${RXC_FLAGS}
          -o task_method_lib
          \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/gate_f_task_method_lib.crexx\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported task-method library rxc failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxas>\" ${RXAS_FLAGS}
          -o task_method_lib.rxbin task_method_lib
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported task-method library rxas failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${WORK_DIR}\"
          -i \"${CMAKE_BINARY_DIR}/bin\" ${RXC_FLAGS}
          -o imported_task_levelb
          \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/gate_f_imported_task_levelb.crexx\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
set(full_out \"\${out}\${err}\")
if(res EQUAL 0 OR NOT full_out MATCHES \"#TASK_ONLY_LEVELG\")
  message(FATAL_ERROR \"Imported task method escaped the Level G gate:\\n\${full_out}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${WORK_DIR}\"
          -i \"${CMAKE_BINARY_DIR}/bin\" ${RXC_FLAGS}
          -o task_method_main
          \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/gate_f_task_method_import.crexx\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported task-method client rxc failure:\\n\${out}\\n\${err}\")
endif()
file(READ \"${WORK_DIR}/task_method_main.rxas\" task_method_main_rxas)
string(FIND \"\${task_method_main_rxas}\" \"submit_arguments\" submit_arguments_pos)
if(submit_arguments_pos EQUAL -1)
  message(FATAL_ERROR \"Imported task methods were not lowered through taskscope.submit_arguments\")
endif()
string(FIND \"\${task_method_main_rxas}\" \"result_value\" result_value_pos)
if(result_value_pos EQUAL -1)
  message(FATAL_ERROR \"Imported object task result was not materialized as a ChannelValue\")
endif()
string(FIND \"\${task_method_main_rxas}\" \"gate_f_task_method_lib.answerbox.§factory.from_channel\" from_channel_pos)
if(from_channel_pos EQUAL -1)
  message(FATAL_ERROR \"Imported object task result omitted answerbox.from_channel reconstruction\")
endif()
string(FIND \"\${task_method_main_rxas}\" \"gate_f_task_method_lib.inputbox.to_channel\" input_to_channel_pos)
if(input_to_channel_pos EQUAL -1)
  message(FATAL_ERROR \"Imported object task argument omitted inputbox.to_channel transfer\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxas>\" ${RXAS_FLAGS}
          -o task_method_main.rxbin task_method_main
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported task-method client rxas failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxlink>\" -o task_method_linked.rxbin
          task_method_main.rxbin task_method_lib.rxbin
          \"${CMAKE_BINARY_DIR}/bin/library.rxbin\"
          \"${CMAKE_BINARY_DIR}/bin/classlib.rxbin\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported task-method rxlink failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:${RUNNER}>\" task_method_linked.rxbin
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported task-method ${RUNNER} failure:\\n\${out}\\n\${err}\")
endif()
")
    add_test(NAME ${TEST_NAME} COMMAND ${CMAKE_COMMAND} -P "${SCRIPT}")
    set_tests_properties(${TEST_NAME} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            LABELS "compiler;levelg;concurrency;gate_f;f1f;linked;import;method;${MODE};${RUNNER}"
            RESOURCE_LOCK gate_f_concurrency_runtime
            TIMEOUT 180)
    crexx_label_concurrency_tests(SP03 ${TEST_NAME})
    crexx_label_concurrency_tests(SP06 ${TEST_NAME})
endfunction()

add_gate_f_task_method_import(rxbvm noopt)
add_gate_f_task_method_import(rxbvm opt)
if(CREXX_THREADED_VM_SUPPORTED)
    add_gate_f_task_method_import(rxtvm noopt)
    add_gate_f_task_method_import(rxtvm opt)
endif()

function(add_gate_f_taskwork_import RUNNER MODE)
    set(TEST_NAME "gate_f_taskwork_import_${RUNNER}_${MODE}")
    set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_ws")
    set(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.cmake")
    if(MODE STREQUAL "noopt")
        set(RXC_FLAGS "-n")
        set(RXAS_FLAGS "-n")
    else()
        set(RXC_FLAGS "")
        set(RXAS_FLAGS "")
    endif()
    file(GENERATE OUTPUT "${SCRIPT}" CONTENT
"file(MAKE_DIRECTORY \"${WORK_DIR}\")
execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${RXC_FLAGS}
          -o taskwork_lib
          \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/gate_f_taskwork_lib.crexx\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported taskwork library rxc failure:\\n\${out}\\n\${err}\")
endif()
file(READ \"${WORK_DIR}/taskwork_lib.rxas\" taskwork_rxas)
string(FIND \"\${taskwork_rxas}\" \".task3\" taskwork_meta)
if(taskwork_meta EQUAL -1)
  message(FATAL_ERROR \"Imported taskwork library omitted sealed .task3 metadata\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxas>\" ${RXAS_FLAGS}
          -o taskwork_lib.rxbin taskwork_lib
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported taskwork library rxas failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${WORK_DIR}\"
          -i \"${CMAKE_BINARY_DIR}/bin\" ${RXC_FLAGS}
          -o taskwork_main
          \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/gate_f_taskwork_import.crexx\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported taskwork client rxc failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxas>\" ${RXAS_FLAGS}
          -o taskwork_main.rxbin taskwork_main
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported taskwork client rxas failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxlink>\" -o taskwork_linked.rxbin
          taskwork_main.rxbin taskwork_lib.rxbin
          \"${CMAKE_BINARY_DIR}/bin/library.rxbin\"
          \"${CMAKE_BINARY_DIR}/bin/classlib.rxbin\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported taskwork rxlink failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:${RUNNER}>\" taskwork_linked.rxbin
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Imported taskwork ${RUNNER} failure:\\n\${out}\\n\${err}\")
endif()
")
    add_test(NAME ${TEST_NAME} COMMAND ${CMAKE_COMMAND} -P "${SCRIPT}")
    set_tests_properties(${TEST_NAME} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            LABELS "compiler;levelg;concurrency;gate_f;f1f;linked;import;taskwork;${MODE};${RUNNER}"
            RESOURCE_LOCK gate_f_concurrency_runtime
            TIMEOUT 180)
    crexx_label_concurrency_tests(SP06 ${TEST_NAME})
endfunction()

add_gate_f_taskwork_import(rxbvm noopt)
add_gate_f_taskwork_import(rxbvm opt)
if(CREXX_THREADED_VM_SUPPORTED)
    add_gate_f_taskwork_import(rxtvm noopt)
    add_gate_f_taskwork_import(rxtvm opt)
endif()

function(add_gate_f_levelg_failure NAME SOURCE DIAGNOSTIC)
    set(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ws")
    file(GENERATE OUTPUT "${SCRIPT}" CONTENT
"file(MAKE_DIRECTORY \"${WORK_DIR}\")
execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\"
          -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/${SOURCE}\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
set(full_out \"\${out}\${err}\")
if(res EQUAL 0)
  message(FATAL_ERROR \"Expected ${DIAGNOSTIC}, but compilation succeeded\")
endif()
string(FIND \"\${full_out}\" \"#${DIAGNOSTIC}\" diagnostic_pos)
if(diagnostic_pos EQUAL -1)
  message(FATAL_ERROR \"Expected #${DIAGNOSTIC}:\\n\${full_out}\")
endif()
")
    add_test(NAME ${NAME} COMMAND ${CMAKE_COMMAND} -P "${SCRIPT}")
    set_tests_properties(${NAME} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            LABELS "compiler;levelg;concurrency;gate_f;f1f;negative")
    crexx_label_concurrency_tests(SP06 ${NAME})
endfunction()

add_gate_f_levelg_failure(gate_f_bad_scope
        gate_f_bad_scope.crexx PARALLEL_SCOPE_TYPE)
add_gate_f_levelg_failure(gate_f_pending_mutation
        gate_f_pending_mutation.crexx PENDING_TASK_RESULT_MUTATION)
add_gate_f_levelg_failure(gate_f_pending_return
        gate_f_pending_return.crexx PENDING_TASK_RESULT_MUTATION)
add_gate_f_levelg_failure(gate_f_pending_reference
        gate_f_pending_reference.crexx PENDING_TASK_RESULT_MUTATION)
add_gate_f_levelg_failure(gate_f_nested_task_wait
        gate_f_nested_task_wait.crexx TASK_NESTED_WAIT)
add_gate_f_levelg_failure(gate_f_task_exposed_arg
        gate_f_task_exposed_arg.crexx TASK_EXPOSED_ARGUMENT)
add_gate_f_levelg_failure(gate_f_task_reference_arg
        gate_f_task_reference_arg.crexx TASK_REFERENCE_TYPE)
add_gate_f_levelg_failure(gate_f_task_receiver
        gate_f_task_receiver.crexx TASK_NONTRANSFERABLE_RECEIVER)
add_gate_f_levelg_failure(gate_f_dynamic_task_target
        gate_f_dynamic_task_target.crexx TASK_DYNAMIC_TARGET)
add_gate_f_levelg_failure(gate_f_scope_reused
        gate_f_scope_reused.crexx PARALLEL_SCOPE_REUSED)
add_gate_f_levelg_failure(gate_f_task_levelb
        gate_f_task_levelb.crexx TASK_ONLY_LEVELG)
add_gate_f_levelg_failure(gate_f_parallel_levelb
        gate_f_parallel_levelb.crexx PARALLEL_ONLY_LEVELG)
add_gate_f_levelg_failure(gate_f_parallel_expr_levelb
        gate_f_parallel_expr_levelb.crexx PARALLEL_ONLY_LEVELG)
add_gate_f_levelg_failure(gate_f_task_target_levelb
        gate_f_task_target_levelb.crexx TASK_TARGET_ONLY_LEVELG)
add_gate_f_levelg_failure(gate_f_task_factory_target_levelb
        gate_f_task_factory_target_levelb.crexx TASK_TARGET_ONLY_LEVELG)
add_gate_f_levelg_failure(gate_f_taskwork_required
        gate_f_taskwork_required.crexx TASKWORK_ADAPTER_REQUIRED)
add_gate_f_levelg_failure(gate_f_taskwork_factory_arg
        gate_f_taskwork_factory_arg.crexx TASK_NONTRANSFERABLE_TYPE)
add_gate_f_levelg_failure(gate_f_task_object_result_required
        gate_f_task_object_result_required.crexx TASK_NONTRANSFERABLE_TYPE)
add_gate_f_levelg_failure(gate_f_task_interface_result
        gate_f_task_interface_result.crexx TASK_NONTRANSFERABLE_TYPE)

set(GATE_F_CONTEXTUAL_LEVELB_SCRIPT
        "${CMAKE_CURRENT_BINARY_DIR}/gate_f_contextual_levelb.cmake")
set(GATE_F_CONTEXTUAL_LEVELB_WORK_DIR
        "${CMAKE_CURRENT_BINARY_DIR}/gate_f_contextual_levelb_ws")
file(GENERATE OUTPUT "${GATE_F_CONTEXTUAL_LEVELB_SCRIPT}" CONTENT
"file(MAKE_DIRECTORY \"${GATE_F_CONTEXTUAL_LEVELB_WORK_DIR}\")
execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\"
          -o contextual
          \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/gate_f_contextual_levelb.crexx\"
  WORKING_DIRECTORY \"${GATE_F_CONTEXTUAL_LEVELB_WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Level B contextual-word rxc failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxas>\" -o contextual.rxbin contextual
  WORKING_DIRECTORY \"${GATE_F_CONTEXTUAL_LEVELB_WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Level B contextual-word rxas failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxlink>\" -o contextual_linked.rxbin
          contextual.rxbin \"${CMAKE_BINARY_DIR}/bin/library.rxbin\"
  WORKING_DIRECTORY \"${GATE_F_CONTEXTUAL_LEVELB_WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Level B contextual-word rxlink failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxbvm>\" contextual_linked.rxbin
  WORKING_DIRECTORY \"${GATE_F_CONTEXTUAL_LEVELB_WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Level B contextual-word runtime failure:\\n\${out}\\n\${err}\")
endif()
")
add_test(NAME gate_f_contextual_levelb
         COMMAND ${CMAKE_COMMAND} -P "${GATE_F_CONTEXTUAL_LEVELB_SCRIPT}")
set_tests_properties(gate_f_contextual_levelb PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        LABELS "compiler;levelb;concurrency;gate_f;f1f;contextual")
crexx_label_concurrency_tests(SP06 gate_f_contextual_levelb)

# Gate F's compiler-generated task controller uses block-scoped signal
# handlers.  Prove the shared control-transfer emitter independently so a
# future task-lowering change cannot accidentally mask broken RETURN/LEAVE
# cleanup.
function(add_signal_control_unwind_test RUNNER MODE)
    set(TEST_NAME "signal_control_unwind_${RUNNER}_${MODE}")
    set(BASE "${TEST_NAME}")
    set(WORK_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_ws")
    set(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.cmake")
    if(MODE STREQUAL "noopt")
        set(RXC_FLAGS "-n")
        set(RXAS_FLAGS "-n")
    else()
        set(RXC_FLAGS "")
        set(RXAS_FLAGS "")
    endif()

    file(GENERATE OUTPUT "${SCRIPT}" CONTENT
"file(MAKE_DIRECTORY \"${WORK_DIR}\")
execute_process(
  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${RXC_FLAGS}
          -o \"${BASE}\"
          \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/signal_control_unwind.crexx\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Signal control unwind rxc failure:\\n\${out}\\n\${err}\")
endif()
file(READ \"${WORK_DIR}/${BASE}.rxas\" rxas)
foreach(shape IN ITEMS
        \"sigpop \\\"OTHER\\\"[\\r\\n]+[ ]+br l[0-9]+doend\"
        \"sigpop \\\"OTHER\\\"[\\r\\n]+[ ]+br l[0-9]+bexprend\")
  string(REGEX MATCH \"\${shape}\" unwind_match \"\${rxas}\")
  if(unwind_match STREQUAL \"\")
    message(FATAL_ERROR \"Missing signal control unwind shape: \${shape}\")
  endif()
endforeach()
if(\"${MODE}\" STREQUAL \"noopt\")
  string(REGEX MATCH
         \"sigpop \\\"OTHER\\\"[\\r\\n]+[ ]+ret 7\"
         return_unwind \"\${rxas}\")
  if(return_unwind STREQUAL \"\")
    message(FATAL_ERROR \"RETURN did not unwind its signal handler\")
  endif()
else()
  string(REGEX MATCHALL
         \"sigpop \\\"OTHER\\\"[\\r\\n]+[ ]+br l[0-9]+bexprend\"
         inlined_unwinds \"\${rxas}\")
  list(LENGTH inlined_unwinds inlined_unwind_count)
  if(inlined_unwind_count LESS 2)
    message(FATAL_ERROR
            \"Optimized inlined RETURN/LEAVE WITH shapes lost signal unwinds\")
  endif()
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:rxas>\" ${RXAS_FLAGS}
          -o \"${BASE}.rxbin\" \"${BASE}\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Signal control unwind rxas failure:\\n\${out}\\n\${err}\")
endif()
execute_process(
  COMMAND \"$<TARGET_FILE:${RUNNER}>\" \"${BASE}.rxbin\"
  WORKING_DIRECTORY \"${WORK_DIR}\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
set(full_out \"\${out}\${err}\")
if(NOT res EQUAL 0 OR
   NOT full_out MATCHES \"PASS: signal control unwind\" OR
   full_out MATCHES \"BAD:|FAIL:\")
  message(FATAL_ERROR \"Signal control unwind ${RUNNER} failure:\\n\${full_out}\")
endif()
")

    add_test(NAME ${TEST_NAME} COMMAND ${CMAKE_COMMAND} -P "${SCRIPT}")
    set_tests_properties(${TEST_NAME} PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            LABELS "compiler;signals;control_flow;gate_f;${MODE};${RUNNER}"
            TIMEOUT 60)
    crexx_label_concurrency_tests(SP06 ${TEST_NAME})
    crexx_label_concurrency_tests(SP08 ${TEST_NAME})
endfunction()

add_signal_control_unwind_test(rxbvm noopt)
add_signal_control_unwind_test(rxbvm opt)
if(CREXX_THREADED_VM_SUPPORTED)
    add_signal_control_unwind_test(rxtvm noopt)
    add_signal_control_unwind_test(rxtvm opt)
endif()

add_crexx_isolated_run_test(interface_named_factory_same_module rexx_src/interface_named_factory_same_module.crexx)
add_crexx_isolated_run_test(interface_named_factory_tiebreak rexx_src/interface_named_factory_tiebreak.crexx)
add_crexx_isolated_run_fail_test(interface_named_factory_no_impl rexx_src/interface_named_factory_no_impl.crexx)
add_crexx_interface_import_run_test(interface_default_source_import rexx_src/interface_default_main.crexx rexx_src/interface_default_dep_contract.crexx source)
add_crexx_interface_import_run_test(interface_default_binary_import rexx_src/interface_default_binary_main.crexx rexx_src/interface_default_dep_contract.crexx binary)
add_crexx_interface_import_run_test(interface_source_import rexx_src/interface_source_import_main.crexx rexx_src/interface_dep_contract.crexx source)
add_crexx_interface_import_run_test(interface_binary_import rexx_src/interface_binary_import_main.crexx rexx_src/interface_dep_contract.crexx binary)
add_crexx_interface_import_run_test(interface_match_source_import rexx_src/interface_match_main.crexx rexx_src/interface_match_dep_contract.crexx source)
add_crexx_interface_import_run_test(interface_match_binary_import rexx_src/interface_match_main.crexx rexx_src/interface_match_dep_contract.crexx binary)
add_crexx_interface_import_run_test(interface_named_factory_source_import rexx_src/interface_named_factory_main.crexx rexx_src/interface_named_factory_dep_contract.crexx source)
add_crexx_interface_import_run_test(interface_named_factory_binary_import rexx_src/interface_named_factory_main.crexx rexx_src/interface_named_factory_dep_contract.crexx binary)
add_crexx_dual_import_run_test(interface_import_provider_source rexx_src/interface_import_provider_main.crexx rexx_src/interface_import_provider_contract.crexx rexx_src/interface_import_provider_impl.crexx source)
add_crexx_dual_import_run_test(interface_import_provider_binary rexx_src/interface_import_provider_main.crexx rexx_src/interface_import_provider_contract.crexx rexx_src/interface_import_provider_impl.crexx binary)
add_crexx_dual_import_run_test(interface_split_same_namespace_source rexx_src/interface_split_same_namespace_main.crexx rexx_src/interface_split_same_namespace_contract.crexx rexx_src/interface_split_same_namespace_provider.crexx source)
add_crexx_dual_import_run_test(interface_split_same_namespace_binary rexx_src/interface_split_same_namespace_main.crexx rexx_src/interface_split_same_namespace_contract.crexx rexx_src/interface_split_same_namespace_provider.crexx binary)
add_crexx_dual_import_run_test(interface_binary_covariant rexx_src/interface_binary_covariant_main.crexx rexx_src/interface_binary_covariant_contract.crexx rexx_src/interface_binary_covariant_provider.crexx binary)
add_crexx_dual_import_run_test(qualified_interface_import rexx_src/qualified_interface_main.crexx rexx_src/qualified_interface_dep_a.crexx rexx_src/qualified_interface_dep_b.crexx source)
add_crexx_dual_import_run_test(qualified_interface_binary_import rexx_src/qualified_interface_main.crexx rexx_src/qualified_interface_dep_a.crexx rexx_src/qualified_interface_dep_b.crexx binary)

add_test(
        NAME imported_record_return_contract
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXVM=${CREXX_RXVM_EXECUTABLE}
                -DRXBVM=$<TARGET_FILE:rxbvm>
                -DLIBRARY=${CMAKE_BINARY_DIR}/bin
                -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/imported_record_return_contract_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/imported_record_return_contract.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(imported_record_return_contract PROPERTIES
        LABELS "compiler;imports;runtime;optimizer")

add_test(
        NAME do_forever_return_contract
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXVM=${CREXX_RXVM_EXECUTABLE}
                -DRXBVM=$<TARGET_FILE:rxbvm>
                -DLIBRARY=${CMAKE_BINARY_DIR}/bin
                -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/do_forever_return_contract_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/do_forever_return_contract.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(do_forever_return_contract PROPERTIES
        LABELS "compiler;control-flow;runtime;optimizer"
        TIMEOUT 300)

add_test(
        NAME parse_levelg_contract
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXVM=${CREXX_RXVM_EXECUTABLE}
                -DRXBVM=$<TARGET_FILE:rxbvm>
                -DLIBRARY=${CMAKE_BINARY_DIR}/bin
                -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/parse_levelg_contract_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/parse_levelg_contract.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(parse_levelg_contract PROPERTIES
        LABELS "compiler;exits;parse;runtime;optimizer"
        TIMEOUT 120)

set(INLINE_CROSS_FILE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_cross_file_binary_opt.cmake")
set(INLINE_CROSS_FILE_CONTENT "set(full_out \"\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_cross_file_binary_opt_ws\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline dependency\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline main\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.inc\\\"\" dep_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \"=\\\".inline\\\" \\\"I6;c,1,\" dep_inline_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.classify\\\"\" dep_if_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.scoped\\\"\" dep_scoped_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.nested\\\"\" dep_nested_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.sumto\\\"\" dep_sum_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.countuntil\\\"\" dep_count_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.buildarray\\\"\" dep_array_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.renderarray\\\"\" dep_render_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.identitybox\\\"\" dep_object_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.refbump\\\"\" dep_ref_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.optadd\\\"\" dep_opt_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.safelength\\\"\" dep_safe_asm_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.typebump\\\"\" dep_type_bump_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.vargtotal\\\"\" dep_varg_total_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.vargpick\\\"\" dep_varg_pick_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.residualfunctiondependency\\\"\" dep_residual_func_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.residualcalldependency\\\"\" dep_residual_call_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.residualcalltarget\\\"\" dep_residual_target_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.box.getname\\\"\" dep_box_getname_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.box.setname\\\"\" dep_box_setname_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.aliasblocked\\\"\" dep_alias_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.refvargdynamicblocked\\\"\" dep_ref_varg_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.residualcallblocked\\\"\" dep_residual_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.membercalldependency\\\"\" dep_member_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_meta_pos EQUAL -1 OR dep_if_meta_pos EQUAL -1 OR dep_scoped_meta_pos EQUAL -1 OR dep_nested_meta_pos EQUAL -1 OR dep_sum_meta_pos EQUAL -1 OR dep_count_meta_pos EQUAL -1 OR dep_array_meta_pos EQUAL -1 OR dep_render_meta_pos EQUAL -1 OR dep_object_meta_pos EQUAL -1 OR dep_ref_meta_pos EQUAL -1 OR dep_opt_meta_pos EQUAL -1 OR dep_safe_asm_meta_pos EQUAL -1 OR dep_type_bump_meta_pos EQUAL -1 OR dep_varg_total_meta_pos EQUAL -1 OR dep_varg_pick_meta_pos EQUAL -1 OR dep_residual_func_meta_pos EQUAL -1 OR dep_residual_call_meta_pos EQUAL -1 OR dep_residual_target_meta_pos EQUAL -1 OR dep_box_getname_meta_pos EQUAL -1 OR dep_box_setname_meta_pos EQUAL -1 OR dep_inline_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not export expected inline metadata records\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_alias_meta_pos EQUAL -1 OR dep_ref_varg_meta_pos EQUAL -1 OR dep_residual_meta_pos EQUAL -1 OR dep_member_meta_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not emit expected callable signatures\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.safelength\\\"=\\\".inline\\\" \\\"I6;c,1,\" dep_safe_asm_inline_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_safe_asm_inline_pos EQUAL -1)\n  message(FATAL_ERROR \"Safe assembler helper did not export inline metadata\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \";d,\" dep_dependency_record_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_dependency_record_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not export residual-call dependency records\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(expected_inline_name typebump vargtotal residualfunctiondependency residualcalldependency box.getname box.setname)\n  string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.\${expected_inline_name}\\\"=\\\".inline\\\" \\\"I6;c,1,\" dep_expected_inline_pos)\n  if(dep_expected_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"Dependency did not export I6 callable summary metadata for \${expected_inline_name}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.membercalldependency\\\"=\\\".inline\\\" \\\"I7;c,1,\" dep_member_inline_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_member_inline_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not export I7 residual member-call metadata\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(blocked_name aliasblocked refvargdynamicblocked vargpick residualcalltarget residualcallblocked)\n  string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.\${blocked_name}\\\"=\\\".inline\\\" \\\"I6\" dep_blocked_inline_pos)\n  if(NOT dep_blocked_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"Fail-closed callable \${blocked_name} unexpectedly exported inline metadata\")\n  endif()\nendforeach()\n")
# Evidence-opening regression: remove the legacy ARG is_const_arg hint from the
# serialized body.  The independently reconstructed I6 read-only summary must
# still permit direct scalar binding; the I5/mismatch controls below must not.
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REPLACE \";>,0,0,3,3,3,0,0,-,-,-,-,-,-,4,-1\" \";>,0,0,3,3,3,0,0,-,-,-,-,-,-,0,-1\" dep_without_legacy_hint \"\${dep_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_without_legacy_hint STREQUAL dep_rxas)\n  message(FATAL_ERROR \"Failed to remove legacy scalar-formal hint for I6 evidence-opening regression\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${dep_without_legacy_hint}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxdas>\" -o \"inline_cross_file_dep.roundtrip.rxas\" \"inline_cross_file_dep.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxdas failed on inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(READ \"\${work}/inline_cross_file_dep.roundtrip.rxas\" dep_roundtrip_rxas)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_roundtrip_rxas}\" \";d,\" dep_roundtrip_dependency_record_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_roundtrip_dependency_record_pos EQUAL -1)\n  message(FATAL_ERROR \"RXDAS roundtrip lost residual-call dependency records\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(roundtrip_inline_name inc typebump vargtotal residualfunctiondependency residualcalldependency box.getname box.setname)\n  string(FIND \"\${dep_roundtrip_rxas}\" \".meta \\\"inline_cross_file_dep.\${roundtrip_inline_name}\\\"=\\\".inline\\\" \\\"I6;c,1,\" dep_roundtrip_inline_pos)\n  if(dep_roundtrip_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"RXDAS roundtrip lost I6 callable summary metadata for \${roundtrip_inline_name}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_roundtrip_rxas}\" \".meta \\\"inline_cross_file_dep.membercalldependency\\\"=\\\".inline\\\" \\\"I7;c,1,\" dep_roundtrip_member_inline_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_roundtrip_member_inline_pos EQUAL -1)\n  message(FATAL_ERROR \"RXDAS roundtrip lost I7 residual member-call metadata\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(blocked_name aliasblocked refvargdynamicblocked vargpick residualcalltarget residualcallblocked)\n  string(FIND \"\${dep_roundtrip_rxas}\" \".meta \\\"inline_cross_file_dep.\${blocked_name}\\\"=\\\".inline\\\" \\\"I6\" dep_roundtrip_blocked_inline_pos)\n  if(NOT dep_roundtrip_blocked_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"RXDAS roundtrip unexpectedly gained inline metadata for \${blocked_name}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.roundtrip\" \"inline_cross_file_dep.roundtrip.rxas\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on rxdas inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.rxbin\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(RENAME \"\${work}/inline_cross_file_dep.roundtrip.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\" \"\${work}/inline_cross_file_dep.roundtrip.rxas\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${main_rxas}\" \"\\\"inline_cross_file_dep.crexx\\\" 70 3 16 \\\"  result = .int\\\"\" main_dep_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${main_rxas}\" \"\\\"inline_cross_file_dep.crexx\\\" 71 13 32 \\\"  assembler strlen result,value\\\"\" main_dep_asm_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_dep_source_pos EQUAL -1 OR main_dep_asm_source_pos EQUAL -1)\n  message(FATAL_ERROR \"Optimized binary import did not preserve inlined dependency source metadata:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inc\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls inc():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*classify\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls classify():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*scoped\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls scoped():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*nested\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls nested():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*sumto\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls sumTo():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*countuntil\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls countUntil():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*buildarray\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls buildArray():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*renderarray\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls renderArray():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*identitybox\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls identityBox():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*refbump\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls refBump():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*optadd\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls optAdd():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*residualfunctiondependency\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls residualFunctionDependency():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*residualcalldependency\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls residualCallDependency():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.getname\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls box.getName():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.setname\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls box.setName():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(inlined_name inc classify scoped nested sumto countuntil buildarray renderarray identitybox refbump optadd safelength typebump vargtotal residualfunctiondependency residualcalldependency)\n  if(main_rxas MATCHES \"call [^\\n]*inline_cross_file_dep\\\\.\${inlined_name}\\\\(\\\\)\")\n    message(FATAL_ERROR \"Optimized binary import still calls \${inlined_name}():\\n\${main_rxas}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REPLACE \"\\n\" \"|\" flat_main_rxas \"\${main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REGEX MATCH [=[\\.traceevent \"V\"[^|]*\"r\" ([0-9]+)[^|]*\"shared\"[^|]*\\|[ \\t]*\\.traceevent \"A\"[^|]*\"r\" ([0-9]+)[^|]*\"value\"]=] shared_binding \"\${flat_main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "set(shared_source_reg \"\${CMAKE_MATCH_1}\")\nset(shared_formal_reg \"\${CMAKE_MATCH_2}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(shared_binding STREQUAL \"\" OR NOT shared_source_reg STREQUAL shared_formal_reg)\n  message(FATAL_ERROR \"Binary-import callable summary did not preserve direct scalar binding equivalence:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REGEX MATCH [=[\\.traceevent \"V\"[^|]*\"r\" ([0-9]+)[^|]*\"same\"[^|]*\\|[ \\t]*\\.traceevent \"A\"[^|]*\"r\" ([0-9]+)[^|]*\"§this\"]=] receiver_binding \"\${flat_main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "set(receiver_source_reg \"\${CMAKE_MATCH_1}\")\nset(receiver_this_reg \"\${CMAKE_MATCH_2}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(receiver_binding STREQUAL \"\" OR NOT receiver_source_reg STREQUAL receiver_this_reg)\n  message(FATAL_ERROR \"Binary-import method summary did not preserve direct receiver equivalence:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.vargpick\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import unexpectedly inlined dynamic vargPick():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.residualcalltarget\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import did not preserve residual call dependency:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT full_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected inline output, got: \${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_CROSS_FILE_SCRIPT}" CONTENT "${INLINE_CROSS_FILE_CONTENT}")

add_test(NAME inline_cross_file_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_CROSS_FILE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_cross_file_binary_opt PROPERTIES LABELS "linked_opt")

# Review regression: a body payload without the exact callable-summary schema
# must remain a normal call.  Downgrade only the I6 body header while retaining
# the c-record so the importer exercises its fail-closed version/shape path.
set(INLINE_SUMMARY_VERSION_FALLBACK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_summary_version_fallback_binary_opt.cmake")
set(INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_summary_version_fallback_binary_opt_ws\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "string(REPLACE \"\\\"I6;c,1,\" \"\\\"I5;c,1,\" malformed_rxas \"\${dep_rxas}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "string(FIND \"\${malformed_rxas}\" \"\\\"I5;c,1,\" malformed_pos)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(malformed_pos EQUAL -1)\n  message(FATAL_ERROR \"Failed to construct old-version callable-summary payload\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${malformed_rxas}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on old-version summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on old-version summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.inc\\(\\)]=])\n  message(FATAL_ERROR \"Old-version summary payload did not retain inc() call:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on old-version summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on old-version summary fallback: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_SUMMARY_VERSION_FALLBACK_CONTENT "if(NOT run_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Old-version summary fallback output mismatch: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_SUMMARY_VERSION_FALLBACK_SCRIPT}" CONTENT "${INLINE_SUMMARY_VERSION_FALLBACK_CONTENT}")

add_test(NAME inline_summary_version_fallback_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_SUMMARY_VERSION_FALLBACK_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_summary_version_fallback_binary_opt PROPERTIES LABELS "linked_opt")

# Review regression: a same-version summary whose result shape disagrees with
# the independently parsed callable declaration must remain a normal call.
set(INLINE_SUMMARY_SHAPE_FALLBACK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_summary_shape_fallback_binary_opt.cmake")
set(INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_summary_shape_fallback_binary_opt_ws\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "string(REPLACE \"I6;c,1,1,3,0,4,0,7,5,0,0,0,0,3,0,400;\" \"I6;c,1,1,6,0,4,0,7,5,0,0,0,0,3,0,400;\" malformed_rxas \"\${dep_rxas}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(malformed_rxas STREQUAL dep_rxas)\n  message(FATAL_ERROR \"Failed to construct mismatched callable-summary payload\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${malformed_rxas}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on mismatched summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on mismatched summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.inc\\(\\)]=])\n  message(FATAL_ERROR \"Mismatched summary payload did not retain inc() call:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on mismatched summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on mismatched summary fallback: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT "if(NOT run_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Mismatched summary fallback output mismatch: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_SUMMARY_SHAPE_FALLBACK_SCRIPT}" CONTENT "${INLINE_SUMMARY_SHAPE_FALLBACK_CONTENT}")

add_test(NAME inline_summary_shape_fallback_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_SUMMARY_SHAPE_FALLBACK_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_summary_shape_fallback_binary_opt PROPERTIES LABELS "linked_opt")

# Review regression: summary formal effects cannot promote a writable
# reference formal into the exact read-only scalar case. The imported body and
# declaration independently reconstruct the real by-ref/write/escape facts,
# so contradictory payload evidence must retain the ordinary call.
set(INLINE_SUMMARY_FORMAL_FALLBACK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_summary_formal_fallback_binary_opt.cmake")
set(INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_summary_formal_fallback_binary_opt_ws\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on formal-effect dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "string(REPLACE \".meta \\\"inline_cross_file_dep.refbump\\\"=\\\".inline\\\" \\\"I6;c,1,1,3,0,4,0,7,8,1,0,0,0,3,0,481;\" \".meta \\\"inline_cross_file_dep.refbump\\\"=\\\".inline\\\" \\\"I6;c,1,1,3,0,4,0,7,8,1,0,0,0,3,0,400;\" malformed_rxas \"\${dep_rxas}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(malformed_rxas STREQUAL dep_rxas)\n  message(FATAL_ERROR \"Failed to construct contradictory formal-effect summary\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${malformed_rxas}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on contradictory formal-effect dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on contradictory formal-effect consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.refbump\\(\\)]=])\n  message(FATAL_ERROR \"Contradictory formal-effect summary did not retain refbump() call:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on contradictory formal-effect consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on contradictory formal-effect fallback: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT "if(NOT run_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Contradictory formal-effect fallback output mismatch: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_SUMMARY_FORMAL_FALLBACK_SCRIPT}" CONTENT "${INLINE_SUMMARY_FORMAL_FALLBACK_CONTENT}")

add_test(NAME inline_summary_formal_fallback_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_SUMMARY_FORMAL_FALLBACK_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_summary_formal_fallback_binary_opt PROPERTIES LABELS "linked_opt")

# Review regression: receiver effects are trusted only when the imported I6
# control facts exactly match an independent reconstruction of the method body.
# Forging setName as a non-mutating receiver must retain the ordinary call.
set(INLINE_RECEIVER_SUMMARY_FALLBACK_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_receiver_summary_fallback_binary_opt.cmake")
set(INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_summary_fallback_binary_opt_ws\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_summary_main.crexx\" \"\${work}/inline_receiver_summary_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o inline_cross_file_dep \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on receiver-summary dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "string(REPLACE \".meta \\\"inline_cross_file_dep.box.setname\\\"=\\\".inline\\\" \\\"I6;c,1,1,1,0,4,9,7,5,1,0,0,0,6,0,400;\" \".meta \\\"inline_cross_file_dep.box.setname\\\"=\\\".inline\\\" \\\"I6;c,1,1,1,0,4,1,7,5,1,0,0,0,6,0,400;\" malformed_rxas \"\${dep_rxas}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(malformed_rxas STREQUAL dep_rxas)\n  message(FATAL_ERROR \"Failed to construct contradictory receiver-effect summary\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(WRITE \"\${work}/inline_cross_file_dep.rxas\" \"\${malformed_rxas}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o inline_cross_file_dep.rxbin inline_cross_file_dep WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on contradictory receiver summary: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.rxas\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o inline_receiver_summary_main \"\${work}/inline_receiver_summary_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on contradictory receiver-summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "file(READ \"\${work}/inline_receiver_summary_main.rxas\" main_rxas)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.setname\\(\\)]=])\n  message(FATAL_ERROR \"Contradictory receiver summary did not retain setName() call:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o inline_receiver_summary_main.rxbin inline_receiver_summary_main WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on receiver-summary consumer: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_receiver_summary_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT "if(NOT res EQUAL 0 OR NOT run_out STREQUAL \"after\\n\")\n  message(FATAL_ERROR \"Receiver-summary fallback runtime mismatch: \${run_out}\${err}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_RECEIVER_SUMMARY_FALLBACK_SCRIPT}" CONTENT "${INLINE_RECEIVER_SUMMARY_FALLBACK_CONTENT}")

add_test(NAME inline_receiver_summary_fallback_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_RECEIVER_SUMMARY_FALLBACK_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_receiver_summary_fallback_binary_opt PROPERTIES LABELS "linked_opt")

set(INLINE_REF_ARRAY_COUNT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_ref_array_count_binary_opt.cmake")
set(INLINE_REF_ARRAY_COUNT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_ref_array_count_binary_opt_ws\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_ref_array_count_dep.crexx\" \"\${work}/inline_ref_array_count_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline ref-array-count dependency\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_ref_array_count_main.crexx\" \"\${work}/inline_ref_array_count_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline ref-array-count main\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_ref_array_count_dep\" \"\${work}/inline_ref_array_count_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline ref-array-count dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_ref_array_count_dep.rxbin\" \"inline_ref_array_count_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline ref-array-count dependency: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "file(REMOVE \"\${work}/inline_ref_array_count_dep.crexx\" \"\${work}/inline_ref_array_count_dep.rxas\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_ref_array_count_main\" \"\${work}/inline_ref_array_count_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline ref-array-count main: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "file(READ \"\${work}/inline_ref_array_count_main.rxas\" main_rxas)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*firstor\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import still calls firstOr():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(main_rxas MATCHES [=[slt [^\\n]*\\\"1\\\"]=])\n  message(FATAL_ERROR \"Optimized binary import lowered array[0] count compare as string compare:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT main_rxas MATCHES [=[ilt [^\\n]*,1]=])\n  message(FATAL_ERROR \"Optimized binary import did not lower array[0] count compare as integer compare:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_ref_array_count_main.rxbin\" \"inline_ref_array_count_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline ref-array-count main: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_ref_array_count_dep.rxbin\" \"\${work}/inline_ref_array_count_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on inline ref-array-count binary import: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_REF_ARRAY_COUNT_CONTENT "if(NOT run_out STREQUAL \"?\\nA\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected ? and A, got: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_REF_ARRAY_COUNT_SCRIPT}" CONTENT "${INLINE_REF_ARRAY_COUNT_CONTENT}")

add_test(NAME inline_ref_array_count_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_REF_ARRAY_COUNT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_ref_array_count_binary_opt PROPERTIES LABELS "linked_opt")

set(INLINE_RHS_BLOCK_EXPR_CONCAT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_rhs_block_expr_concat_binary_opt.cmake")
set(INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_rhs_block_expr_concat_binary_opt_ws\")\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_rhs_block_expr_concat.crexx\" \"\${work}/inline_rhs_block_expr_concat.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline RHS block-expression concat source\")\nendif()\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_rhs_block_expr_concat\" \"\${work}/inline_rhs_block_expr_concat.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline RHS block-expression concat: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_rhs_block_expr_concat.rxbin\" \"inline_rhs_block_expr_concat\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline RHS block-expression concat: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_rhs_block_expr_concat.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on inline RHS block-expression concat: \${run_out}\${err}\")\nendif()\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT "if(NOT run_out STREQUAL \"prefix:abc:def\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected prefix:abc:def, got: \${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_RHS_BLOCK_EXPR_CONCAT_SCRIPT}" CONTENT "${INLINE_RHS_BLOCK_EXPR_CONCAT_CONTENT}")

add_test(NAME inline_rhs_block_expr_concat_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_RHS_BLOCK_EXPR_CONCAT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_rhs_block_expr_concat_binary_opt PROPERTIES
        LABELS "linked_opt")

set(INLINE_NESTED_BINARY_OWNER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_nested_binary_block_owner_shape.cmake")
set(INLINE_NESTED_BINARY_OWNER_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_nested_binary_block_owner_shape_ws\")\n")
string(APPEND INLINE_NESTED_BINARY_OWNER_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_NESTED_BINARY_OWNER_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_NESTED_BINARY_OWNER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -o \"inline_nested_binary_block_owner\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_nested_binary_block_owner.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_NESTED_BINARY_OWNER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on nested binary owner shape: \${out}\${err}\")\nendif()\n")
string(APPEND INLINE_NESTED_BINARY_OWNER_CONTENT "file(READ \"\${work}/inline_nested_binary_block_owner.rxas\" generated_rxas)\n")
string(APPEND INLINE_NESTED_BINARY_OWNER_CONTENT "if(generated_rxas MATCHES [=[call[0-9]* [^\\n]*(wrap_node|wrap_document)\\(\\)]=])\n  message(FATAL_ERROR \"Supported nested binary calls were not inlined:\\n\${generated_rxas}\")\nendif()\n")
string(APPEND INLINE_NESTED_BINARY_OWNER_CONTENT "if(generated_rxas MATCHES [=[r-1]=])\n  message(FATAL_ERROR \"Nested inline emitted an unset register:\\n\${generated_rxas}\")\nendif()\n")
string(APPEND INLINE_NESTED_BINARY_OWNER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_nested_binary_block_owner.rxbin\" \"inline_nested_binary_block_owner\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND INLINE_NESTED_BINARY_OWNER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas rejected nested binary owner shape: \${out}\${err}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_NESTED_BINARY_OWNER_SCRIPT}" CONTENT "${INLINE_NESTED_BINARY_OWNER_CONTENT}")

add_test(NAME inline_nested_binary_block_owner_shape_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_NESTED_BINARY_OWNER_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_nested_binary_block_owner_shape_opt PROPERTIES
        LABELS "compiler;assembler;inlining;binary")

set(INLINE_CROSS_FILE_SOURCE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_cross_file_source_opt.cmake")
set(INLINE_CROSS_FILE_SOURCE_CONTENT "set(full_out \"\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_cross_file_source_opt_ws\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.crexx\" \"\${work}/inline_cross_file_dep.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline dependency\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.crexx\" \"\${work}/inline_cross_file_main.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline main\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(FIND \"\${main_rxas}\" \"\\\"inline_cross_file_dep.crexx\\\" 70 3 16 \\\"  result = .int\\\"\" main_dep_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(FIND \"\${main_rxas}\" \"\\\"inline_cross_file_dep.crexx\\\" 71 13 32 \\\"  assembler strlen result,value\\\"\" main_dep_asm_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_dep_source_pos EQUAL -1 OR main_dep_asm_source_pos EQUAL -1)\n  message(FATAL_ERROR \"Optimized source import did not preserve inlined dependency source metadata:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inc\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls inc():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*classify\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls classify():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*scoped\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls scoped():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*nested\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls nested():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*sumto\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls sumTo():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*countuntil\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls countUntil():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*buildarray\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls buildArray():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*renderarray\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls renderArray():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*identitybox\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls identityBox():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*refbump\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls refBump():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*optadd\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls optAdd():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*residualfunctiondependency\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls residualFunctionDependency():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*residualcalldependency\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls residualCallDependency():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.getname\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls box.getName():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.box\\.setname\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import still calls box.setName():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "foreach(inlined_name inc classify scoped nested sumto countuntil buildarray renderarray identitybox refbump optadd safelength typebump vargtotal residualfunctiondependency residualcalldependency)\n  if(main_rxas MATCHES \"call [^\\n]*inline_cross_file_dep\\\\.\${inlined_name}\\\\(\\\\)\")\n    message(FATAL_ERROR \"Optimized source import still calls \${inlined_name}():\\n\${main_rxas}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(REPLACE \"\\n\" \"|\" flat_main_rxas \"\${main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(REGEX MATCH [=[\\.traceevent \"V\"[^|]*\"r\" ([0-9]+)[^|]*\"shared\"[^|]*\\|[ \\t]*\\.traceevent \"A\"[^|]*\"r\" ([0-9]+)[^|]*\"value\"]=] shared_binding \"\${flat_main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "set(shared_source_reg \"\${CMAKE_MATCH_1}\")\nset(shared_formal_reg \"\${CMAKE_MATCH_2}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(shared_binding STREQUAL \"\" OR NOT shared_source_reg STREQUAL shared_formal_reg)\n  message(FATAL_ERROR \"Source-import callable summary did not preserve direct scalar binding equivalence:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(REGEX MATCH [=[\\.traceevent \"V\"[^|]*\"r\" ([0-9]+)[^|]*\"same\"[^|]*\\|[ \\t]*\\.traceevent \"A\"[^|]*\"r\" ([0-9]+)[^|]*\"§this\"]=] receiver_binding \"\${flat_main_rxas}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "set(receiver_source_reg \"\${CMAKE_MATCH_1}\")\nset(receiver_this_reg \"\${CMAKE_MATCH_2}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(receiver_binding STREQUAL \"\" OR NOT receiver_source_reg STREQUAL receiver_this_reg)\n  message(FATAL_ERROR \"Source-import method summary did not preserve direct receiver equivalence:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.vargpick\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import unexpectedly inlined dynamic vargPick():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.residualcalltarget\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import did not preserve residual call dependency:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT full_out STREQUAL \"42\\n7\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n21\\n21\\n1\\n22\\n.int\\n23\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected inline output, got: \${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_CROSS_FILE_SOURCE_SCRIPT}" CONTENT "${INLINE_CROSS_FILE_SOURCE_CONTENT}")

add_test(NAME inline_cross_file_source_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_CROSS_FILE_SOURCE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_cross_file_source_opt PROPERTIES LABELS "linked_opt")

set(SOURCE_IMPORT_DOTTED_DECIMAL_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_import_dotted_floats_decimal_opt.cmake")
set(SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_import_dotted_floats_decimal_opt_ws\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "file(WRITE \"\${work}/hello.aap.noot.mies\" \"options levelb\\nnamespace main\\nimport dep_ns\\nsay dep()\\n\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "file(WRITE \"\${work}/dep.aap.noot.mies.rexx\" \"options levelb floats_decimal\\nnamespace dep_ns expose dep\\ndep: procedure = .decimal\\n  return 1.1 + 2.2\\n\")\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/hello.aap.noot.mies\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on dotted floats_decimal source import: \${out}\")\nendif()\n")
string(APPEND SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT "if(NOT EXISTS \"\${work}/main.rxas\")\n  message(FATAL_ERROR \"Expected main.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_IMPORT_DOTTED_DECIMAL_SCRIPT}" CONTENT "${SOURCE_IMPORT_DOTTED_DECIMAL_CONTENT}")

add_test(NAME source_import_dotted_floats_decimal_opt
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_IMPORT_DOTTED_DECIMAL_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_import_dotted_floats_decimal_opt PROPERTIES LABELS "compiler;imports;linked_opt")

set(SOURCE_IMPORT_CONTRACT_STUB_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_import_contract_stub_implements.cmake")
set(SOURCE_IMPORT_CONTRACT_STUB_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_import_contract_stub_implements_ws\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/iface.crexx\" \"options levelb\\nnamespace contract_stub_iface expose contractiface\\ncontractiface: interface\\n  value: method = .int\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/provider.crexx\" \"options levelb\\nnamespace contract_stub_provider expose contractbox\\nimport contract_stub_iface\\ncontractbox: class implements .contractiface\\n  *: factory\\n    return\\n  value: method = .int\\n    return 7\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/main.crexx\" \"options levelb\\nnamespace contract_stub_consumer\\nimport contract_stub_provider\\nmain: procedure = .int\\n  box = .contractbox()\\n  if box.value() <> 7 then return 1\\n  return 0\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import contract stub implements test: \${out}\")\nendif()\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CONTENT "if(NOT EXISTS \"\${work}/main.rxas\")\n  message(FATAL_ERROR \"Expected main.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_IMPORT_CONTRACT_STUB_SCRIPT}" CONTENT "${SOURCE_IMPORT_CONTRACT_STUB_CONTENT}")

add_test(NAME source_import_contract_stub_implements
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_IMPORT_CONTRACT_STUB_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_import_contract_stub_implements PROPERTIES LABELS "compiler;imports")

set(SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_import_optional_factory_arg.cmake")
set(SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_import_optional_factory_arg_ws\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "file(WRITE \"\${work}/provider.crexx\" \"options levelb\\nnamespace optional_factory_provider expose optionalbox\\noptionalbox: class\\n  *: factory\\n    arg expected = 1024\\n    return\\n  value: method = .int\\n    return 7\\n\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "file(WRITE \"\${work}/main.crexx\" \"options levelb\\nnamespace optional_factory_consumer\\nimport optional_factory_provider\\nmain: procedure = .int\\n  box = .optionalbox()\\n  if box.value() <> 7 then return 1\\n  return 0\\n\")\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import optional factory arg test: \${out}\")\nendif()\n")
string(APPEND SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT "if(NOT EXISTS \"\${work}/main.rxas\")\n  message(FATAL_ERROR \"Expected main.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_SCRIPT}" CONTENT "${SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_CONTENT}")

add_test(NAME source_import_optional_factory_arg
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_IMPORT_OPTIONAL_FACTORY_ARG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_import_optional_factory_arg PROPERTIES LABELS "compiler;imports")

add_test(NAME source_import_srcmap_factory
        COMMAND ${CMAKE_COMMAND} -DRXC=$<TARGET_FILE:rxc>
                -DLIBRARY_DIR=${CMAKE_BINARY_DIR}/bin
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/source_import_srcmap_factory_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/run_source_import_srcmap.cmake)
set_tests_properties(source_import_srcmap_factory PROPERTIES LABELS "compiler;imports;srcmap;regression")
crexx_register_test_prep_targets(TESTS source_import_srcmap_factory TARGETS rxc library)

set(SOURCE_IMPORT_CONTRACT_STUB_CYCLE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_import_contract_stub_covariant_cycle.cmake")
set(SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_import_contract_stub_covariant_cycle_ws\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(WRITE \"\${work}/iface.crexx\" \"options levelb\\nnamespace contract_stub_cycle_iface expose cycleiterator cycleiterable\\ncycleiterator: interface\\n  next: method = .int\\ncycleiterable: interface\\n  iterator: method = .cycleiterator\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(WRITE \"\${work}/list.crexx\" \"options levelb\\nnamespace contract_stub_cycle_provider expose cyclelist\\nimport contract_stub_cycle_iface\\ncyclelist: class implements .cycleiterable\\n  *: factory\\n    return\\n  iterator: method = .cyclelistiterator\\n    return .cyclelistiterator(reference self)\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(WRITE \"\${work}/iterator.crexx\" \"options levelb\\nnamespace contract_stub_cycle_provider expose cyclelistiterator\\nimport contract_stub_cycle_iface\\ncyclelistiterator: class implements .cycleiterator\\n  owner = reference .cyclelist\\n  *: factory\\n    arg source = reference .cyclelist\\n    owner = source\\n    return\\n  next: method = .int\\n    return 7\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "file(WRITE \"\${work}/main.crexx\" \"options levelb\\nnamespace contract_stub_cycle_main\\nimport contract_stub_cycle_provider\\nmain: procedure = .int\\n  list = .cyclelist()\\n  iterator = list.iterator()\\n  if iterator.next() <> 7 then return 1\\n  return 0\\n\")\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on recursive source-import covariant contract stub test: \${out}\")\nendif()\n")
string(APPEND SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT "if(NOT EXISTS \"\${work}/main.rxas\")\n  message(FATAL_ERROR \"Expected main.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_IMPORT_CONTRACT_STUB_CYCLE_SCRIPT}" CONTENT "${SOURCE_IMPORT_CONTRACT_STUB_CYCLE_CONTENT}")

add_test(NAME source_import_contract_stub_covariant_cycle
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_IMPORT_CONTRACT_STUB_CYCLE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_import_contract_stub_covariant_cycle PROPERTIES LABELS "compiler;imports")

set(MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_mixed_aggregate_source_contract_stub_covariant.cmake")
set(MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/mixed_aggregate_source_contract_stub_covariant_ws\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(MAKE_DIRECTORY \"\${work}/src\" \"\${work}/binroot\" \"\${work}/build\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/src/Iterator.crexx\" \"options levelb\\nnamespace aggregate_iterator expose Iterator\\n\\nIterator: interface\\n  next: method = .string\\n\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/src/Iterable.crexx\" \"options levelb\\nnamespace aggregate_iterable expose Iterable\\nimport aggregate_iterator\\n\\nIterable: interface\\n  iterator: method = .Iterator\\n\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/src/BinaryIterator.crexx\" \"options levelb\\nnamespace aggregate_provider expose BinaryIterator\\nimport aggregate_iterator\\n\\nBinaryIterator: class implements .Iterator\\n  *: factory\\n    return\\n  next: method = .string\\n    return \\\"ok\\\"\\n\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/src/Provider.crexx\" \"options levelb\\nnamespace aggregate_provider expose Provider\\nimport aggregate_iterable\\n\\nProvider: class implements .Iterable\\n  *: factory\\n    return\\n  iterator: method = .BinaryIterator\\n    return .BinaryIterator()\\n\")\n")
foreach(_mixed_source IN ITEMS Iterator Iterable BinaryIterator)
    string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"\${work}/build/${_mixed_source}\" \"\${work}/src/${_mixed_source}.crexx\" WORKING_DIRECTORY \"\${work}/build\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on aggregate dependency ${_mixed_source}: \${out}\")\nendif()\n")
    string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"\${work}/build/${_mixed_source}.rxbin\" \"\${work}/build/${_mixed_source}\" WORKING_DIRECTORY \"\${work}/build\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on aggregate dependency ${_mixed_source}: \${out}\")\nendif()\n")
endforeach()
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "file(WRITE \"\${work}/build/aggregate.ctl\" \"INPUT \${work}/build/Iterator.rxbin\\nINPUT \${work}/build/Iterable.rxbin\\nINPUT \${work}/build/BinaryIterator.rxbin\\nINCLUDE Iterator\\nINCLUDE Iterable\\nINCLUDE BinaryIterator\\nOUTPUT \${work}/binroot/aggregate\\n\")\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxlink>\" -c \"\${work}/build/aggregate.ctl\" WORKING_DIRECTORY \"\${work}/build\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxlink failed on aggregate contract fixture: \${out}\")\nendif()\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}/binroot\" -o \"\${work}/build/Provider\" \"\${work}/src/Provider.crexx\" WORKING_DIRECTORY \"\${work}/build\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on mixed aggregate/source covariant contract test: \${out}\")\nendif()\n")
string(APPEND MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT "if(NOT EXISTS \"\${work}/build/Provider.rxas\")\n  message(FATAL_ERROR \"Expected Provider.rxas was not produced\")\nendif()\n")
file(GENERATE OUTPUT "${MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_SCRIPT}" CONTENT "${MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_CONTENT}")

add_test(NAME mixed_aggregate_source_contract_stub_covariant
         COMMAND ${CMAKE_COMMAND} -P "${MIXED_AGGREGATE_SOURCE_CONTRACT_STUB_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(mixed_aggregate_source_contract_stub_covariant PROPERTIES LABELS "compiler;imports")

set(SOURCE_EXTENSION_DIRECT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_extension_direct_defaults.cmake")
set(SOURCE_EXTENSION_DIRECT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_extension_direct_defaults_ws\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(WRITE \"\${work}/direct.crexx\" \"namespace direct_crexx\\nmain: procedure\\n  say 'direct-crexx'\\n  return\\n\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(WRITE \"\${work}/direct.crx\" \"namespace direct_crx\\nmain: procedure\\n  say 'direct-crx'\\n  return\\n\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(WRITE \"\${work}/direct.the\" \"namespace direct_the\\nmain: procedure\\n  say 'direct-the'\\n  return\\n\")\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "file(WRITE \"\${work}/direct.rexx\" \"namespace direct_rexx\\nmain: procedure\\n  say 'direct-rexx'\\n  return\\n\")\n")
foreach(_source_ext IN ITEMS crexx crx the)
    string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"direct_${_source_ext}\" \"\${work}/direct.${_source_ext}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed for direct .${_source_ext} default source: \${out}\")\nendif()\n")
    string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT EXISTS \"\${work}/direct_${_source_ext}.rxas\")\n  message(FATAL_ERROR \"Expected direct_${_source_ext}.rxas was not produced\")\nendif()\n")
endforeach()
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"direct_extensionless\" \"\${work}/direct\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed to fall back to .crexx for extensionless source: \${out}\")\nendif()\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT EXISTS \"\${work}/direct_extensionless.rxas\")\n  message(FATAL_ERROR \"Expected direct_extensionless.rxas was not produced\")\nendif()\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"direct_rexx\" \"\${work}/direct.rexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(res EQUAL 0)\n  message(FATAL_ERROR \"Headerless .rexx source unexpectedly compiled as CREXX source\")\nendif()\n")
string(APPEND SOURCE_EXTENSION_DIRECT_CONTENT "if(NOT out MATCHES \"Level C\")\n  message(FATAL_ERROR \"Headerless .rexx should default to Level C; got: \${out}\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_EXTENSION_DIRECT_SCRIPT}" CONTENT "${SOURCE_EXTENSION_DIRECT_CONTENT}")

add_test(NAME source_extension_direct_defaults
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_EXTENSION_DIRECT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_extension_direct_defaults PROPERTIES LABELS "compiler;imports;extensions")

set(SOURCE_EXTENSION_STANDARD_IMPORT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_extension_standard_imports.cmake")
set(SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_extension_standard_imports_ws\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(WRITE \"\${work}/main.crexx\" \"namespace source_extension_standard\\nimport extdepcrexx\\nimport extdepcrx\\nimport extdeprexx\\nsay dep_crexx() || ':' || dep_crx() || ':' || dep_rexx()\\n\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(WRITE \"\${work}/dep_crexx.crexx\" \"namespace extdepcrexx expose dep_crexx\\ndep_crexx: procedure = .string\\n  return 'crexx'\\n\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(WRITE \"\${work}/dep_crx.crx\" \"namespace extdepcrx expose dep_crx\\ndep_crx: procedure = .string\\n  return 'crx'\\n\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "file(WRITE \"\${work}/dep_rexx.rexx\" \"options levelg\\nnamespace extdeprexx expose dep_rexx\\ndep_rexx: procedure = .string\\n  return 'rexx'\\n\")\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed to discover standard source extensions: \${out}\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_EXTENSION_STANDARD_IMPORT_SCRIPT}" CONTENT "${SOURCE_EXTENSION_STANDARD_IMPORT_CONTENT}")

add_test(NAME source_extension_standard_imports
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_EXTENSION_STANDARD_IMPORT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_extension_standard_imports PROPERTIES LABELS "compiler;imports;extensions")

set(SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_extension_rexx_import_default.cmake")
set(SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_extension_rexx_import_default_ws\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "file(WRITE \"\${work}/main.crexx\" \"namespace source_extension_rexx_default\\nimport extdeprexx\\nsay dep_rexx()\\n\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "file(WRITE \"\${work}/dep_rexx.rexx\" \"namespace extdeprexx expose dep_rexx\\ndep_rexx: procedure = .string\\n  return 'rexx'\\n\")\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "if(res EQUAL 0)\n  message(FATAL_ERROR \"Headerless .rexx import unexpectedly inherited the .crexx Level G default\")\nendif()\n")
string(APPEND SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT "if(NOT out MATCHES \"FUNCTION_NOT_FOUND\")\n  message(FATAL_ERROR \"Expected headerless .rexx import to remain unavailable as Level C source; got: \${out}\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_SCRIPT}" CONTENT "${SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_CONTENT}")

add_test(NAME source_extension_rexx_import_default
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_EXTENSION_REXX_IMPORT_DEFAULT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_extension_rexx_import_default PROPERTIES LABELS "compiler;imports;extensions")

set(SOURCE_EXTENSION_ARBITRARY_IMPORT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_source_extension_arbitrary_imports.cmake")
set(SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/source_extension_arbitrary_imports_ws\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "file(WRITE \"\${work}/main.the\" \"namespace source_extension_the\\nimport thedep\\nsay the_value()\\n\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "file(WRITE \"\${work}/dep.the\" \"namespace thedep expose the_value\\nthe_value: procedure = .string\\n  return 'from-the'\\n\")\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"main\" \"\${work}/main.the\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed to discover arbitrary initial source extension: \${out}\")\nendif()\n")
file(GENERATE OUTPUT "${SOURCE_EXTENSION_ARBITRARY_IMPORT_SCRIPT}" CONTENT "${SOURCE_EXTENSION_ARBITRARY_IMPORT_CONTENT}")

add_test(NAME source_extension_arbitrary_imports
         COMMAND ${CMAKE_COMMAND} -P "${SOURCE_EXTENSION_ARBITRARY_IMPORT_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(source_extension_arbitrary_imports PROPERTIES LABELS "compiler;imports;extensions")

set(INLINE_LOCAL_MEMBER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_local_member_scalar_opt.cmake")
set(INLINE_LOCAL_MEMBER_CONTENT "set(full_out \"\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_local_member_scalar_opt_ws\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_local_member_scalar.crexx\" \"\${work}/inline_local_member_scalar.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage local member inline test\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_local_member_scalar\" \"\${work}/inline_local_member_scalar.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on local member inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "file(READ \"\${work}/inline_local_member_scalar.rxas\" main_rxas)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(REGEX MATCH [=[main\\(\\) \\.locals=([0-9]+)]=] main_header \"\${main_rxas}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_header STREQUAL \"\" OR CMAKE_MATCH_1 GREATER 9)\n  message(FATAL_ERROR \"Direct getter/setter receiver placement used more than 9 main locals:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(REGEX MATCHALL [=[\\n[ \\t]*copy [^\\n]+]=] main_copies \"\${main_rxas}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "list(LENGTH main_copies main_copy_count)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_copy_count GREATER 2)\n  message(FATAL_ERROR \"Direct getter/setter receiver placement retained object copies (count=\${main_copy_count}):\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_local_member_scalar\\.box\\.§factory\\(\\)]=])\n  message(FATAL_ERROR \"Optimized local member test still calls Box factory:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_local_member_scalar\\.box\\.get\\(\\)]=])\n  message(FATAL_ERROR \"Optimized local member test still calls get():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_local_member_scalar\\.box\\.set\\(\\)]=])\n  message(FATAL_ERROR \"Optimized local member test still calls set():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_local_member_scalar.rxbin\" \"inline_local_member_scalar\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on local member inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_local_member_scalar.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on local member inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT full_out STREQUAL \"alpha\\nbeta\\ngamma\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected local member inline output, got: \${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_LOCAL_MEMBER_SCRIPT}" CONTENT "${INLINE_LOCAL_MEMBER_CONTENT}")

add_test(NAME inline_local_member_scalar_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_LOCAL_MEMBER_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_local_member_scalar_opt PROPERTIES LABELS "linked_opt")

add_test(NAME inline_receiver_nested_this_direct_placement_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_test_mutating_method_scalar_attr_return.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_nested_this_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_receiver_nested_this_opt.cmake)
set_tests_properties(inline_receiver_nested_this_direct_placement_opt PROPERTIES
        LABELS "linked_opt;receiver")

add_test(NAME inline_receiver_detached_guard_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DLOCAL_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_guard_local.crexx
                 -DDEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_guard_dep.crexx
                 -DMAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_guard_main.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_detached_guard_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_receiver_guard_opt.cmake)
set_tests_properties(inline_receiver_detached_guard_opt PROPERTIES
        LABELS "linked_opt;receiver")

add_test(NAME inline_receiver_production_ladder_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_production_ladder.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_production_ladder_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_receiver_production_ladder.cmake)
set_tests_properties(inline_receiver_production_ladder_opt PROPERTIES
        LABELS "linked_opt;receiver")

add_test(NAME inline_receiver_reference_alias_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_reference_alias.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_reference_alias_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_receiver_reference_alias_opt.cmake)
set_tests_properties(inline_receiver_reference_alias_opt PROPERTIES
        LABELS "linked_opt;reference")

add_test(NAME inline_reference_member_accessors_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_reference_member_accessors.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_reference_member_accessors_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_reference_member_accessors_opt.cmake)
set_tests_properties(inline_reference_member_accessors_opt PROPERTIES
        LABELS "linked_opt;reference;receiver")

add_test(NAME inline_reference_accessors_import_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DDEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_reference_accessors_dep.crexx
                 -DMAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_reference_accessors_main.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_reference_accessors_import_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_reference_accessors_import_opt.cmake)
set_tests_properties(inline_reference_accessors_import_opt PROPERTIES
        LABELS "linked_opt;reference;receiver;imports")

add_test(NAME inline_scalar_accessors_import_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DDEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_scalar_accessors_dep.crexx
                 -DMAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_scalar_accessors_main.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_scalar_accessors_import_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_scalar_accessors_import_opt.cmake)
set_tests_properties(inline_scalar_accessors_import_opt PROPERTIES
        LABELS "linked_opt;scalar-access;receiver;signals;interfaces;imports")

add_test(NAME inline_packed_accessors_import_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DDEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_packed_accessors_dep.crexx
                 -DMAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_packed_accessors_main.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_packed_accessors_import_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_packed_accessors_import_opt.cmake)
set_tests_properties(inline_packed_accessors_import_opt PROPERTIES
        LABELS "linked_opt;packed;binary;scalar-access;receiver;imports;rxdas")

add_test(NAME inline_receiver_storage_import_opt
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DDEP_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_storage_dep.crexx
                 -DMAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_storage_main.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_storage_import_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_receiver_storage_import_opt.cmake)
set_tests_properties(inline_receiver_storage_import_opt PROPERTIES
        LABELS "linked_opt;level-l;receiver;objects;arrays;binary;imports;rxdas")

set(INLINE_COMPUTED_RECEIVER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_test_computed_receiver_copyback_opt_rewrites.cmake")
set(INLINE_COMPUTED_RECEIVER_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_test_computed_receiver_copyback_opt_ws\")\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_test_computed_receiver_copyback.crexx\" \"\${work}/inline_test_computed_receiver_copyback.crexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage computed receiver inline test\")\nendif()\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_test_computed_receiver_copyback\" \"\${work}/inline_test_computed_receiver_copyback.crexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on computed receiver inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "file(READ \"\${work}/inline_test_computed_receiver_copyback.rxas\" main_rxas)\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "if(main_rxas MATCHES [=[call[0-9]* [^\\n]*inline_test_computed_receiver_copyback\\.box\\.setandreport\\(\\)]=])\n  message(FATAL_ERROR \"Optimized computed receiver test still calls setAndReport():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_COMPUTED_RECEIVER_CONTENT "if(main_rxas MATCHES [=[call[0-9]* [^\\n]*inline_test_computed_receiver_copyback\\.box\\.setname\\(\\)]=])\n  message(FATAL_ERROR \"Optimized postfix CALL test still calls setName():\\n\${main_rxas}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_COMPUTED_RECEIVER_SCRIPT}" CONTENT "${INLINE_COMPUTED_RECEIVER_CONTENT}")

add_test(NAME inline_test_computed_receiver_copyback_opt_rewrites
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_COMPUTED_RECEIVER_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_test_computed_receiver_copyback_opt_rewrites PROPERTIES
        LABELS "compiler;parser;levelb;call-postfix;inlining;linked_opt")

add_test(NAME inline_loop_bound_early_return_cleanup_opt_rewrites
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_test_loop_bound_early_return_cleanup.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_loop_bound_early_return_cleanup_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_loop_bound_early_return_cleanup.cmake)
set_tests_properties(inline_loop_bound_early_return_cleanup_opt_rewrites PROPERTIES
        LABELS "compiler;inlining;correctness;linked_opt")

add_test(NAME structured_exit_crossed_loop_cleanup_opt_rewrites
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/structured_exit_crossed_loop_cleanup.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/structured_exit_crossed_loop_cleanup_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_structured_exit_crossed_loop_cleanup.cmake)
set_tests_properties(structured_exit_crossed_loop_cleanup_opt_rewrites PROPERTIES
        LABELS "compiler;flow;correctness;linked_opt")

add_test(NAME signal_crossed_loop_cleanup_opt_rewrites
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/signal_crossed_loop_cleanup.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/signal_crossed_loop_cleanup_opt_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_signal_crossed_loop_cleanup.cmake)
set_tests_properties(signal_crossed_loop_cleanup_opt_rewrites PROPERTIES
        LABELS "compiler;flow;signals;correctness;linked_opt")

set(_inline_receiver_threaded_vm)
# Compiler tests are declared before the interpreter targets. Use the shared
# capability decision, not TARGET existence at this point in configuration.
if(CREXX_THREADED_VM_SUPPORTED)
    set(_inline_receiver_threaded_vm "-DRXTVM=$<TARGET_FILE:rxtvm>")
endif()
add_test(NAME inline_receiver_signal_cleanup
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXDAS=$<TARGET_FILE:rxdas>
                 -DRXLINK=$<TARGET_FILE:rxlink>
                 ${_inline_receiver_threaded_vm}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DCLASSLIB=${CMAKE_BINARY_DIR}/bin/classlib.rxbin
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_receiver_signal_cleanup.crexx
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_receiver_signal_cleanup_ws
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/run_inline_receiver_signal_cleanup.cmake)
set_tests_properties(inline_receiver_signal_cleanup PROPERTIES
        LABELS "compiler;flow;signals;receiver;inlining;correctness;linked_opt")

add_crexx_run_args_test(test_parse_arg_filename rexx_src/test_parse_arg_filename.crexx demo.rexx)
add_crexx_run_args_test(implicit_main_arg_compat rexx_src/implicit_main_arg_compat.crexx alpha beta)
add_crexx_run_args_test(arg_compat_no_vargs rexx_src/arg_compat_no_vargs.crexx alpha beta)

set(CRXC_NOARGS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crxc_noargs.cmake")
set(CRXC_NOARGS_CONTENT "set(full_out \"\")\n")
string(APPEND CRXC_NOARGS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" --level levelb --import rxfnsb -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_NOOPT_FLAG} -o \"crxc_noargs_run\" \"${CMAKE_SOURCE_DIR}/bin/crxc.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
string(APPEND CRXC_NOARGS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"crxc_noargs_run.rxbin\" \"crxc_noargs_run\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
string(APPEND CRXC_NOARGS_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"crxc_noargs_run.rxbin\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT res EQUAL 99)\n  message(FATAL_ERROR \"rxvm noargs expected rc 99, got \${res}: \${out}\")\nendif()\n")
string(APPEND CRXC_NOARGS_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/crxc_noargs.out\" golden_out)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${CRXC_NOARGS_SCRIPT}" CONTENT "${CRXC_NOARGS_CONTENT}")

add_test(NAME crxc_noargs_run
         COMMAND ${CMAKE_COMMAND} -P "${CRXC_NOARGS_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(STRUCTURED_EXIT_DEBUG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_structured_exit_debug.cmake")
set(STRUCTURED_EXIT_DEBUG_CONTENT "set(run_out \"\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "set(debug_log \"${CMAKE_CURRENT_BINARY_DIR}/structured_exit_debug_d2.log\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "file(REMOVE \"\${debug_log}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"structured_exit_debug_bundle\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/structured_exit_debug_bundle.crexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed building structured exit bundle: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"structured_exit_debug_bundle\" \"structured_exit_debug_bundle\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed building structured exit bundle: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E env \"RXCP_EXIT_MODULE=structured_exit_debug_bundle\" \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_BINARY_DIR}\" -i \"${CMAKE_BINARY_DIR}/bin\" -d2 -n -o \"structured_exit_debug_main\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/structured_exit_debug_main.crexx\" OUTPUT_FILE \"\${debug_log}\" ERROR_FILE \"\${debug_log}\" RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc -d2 failed for structured exit debug case. See \${debug_log}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"structured_exit_debug_main.rxbin\" \"structured_exit_debug_main\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed for structured exit main: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"structured_exit_debug_main.rxbin\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed for structured exit main: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/structured_exit_debug.out\" golden_out)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" run_out \"\${run_out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT run_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${run_out}\")\nendif()\n")
file(GENERATE OUTPUT "${STRUCTURED_EXIT_DEBUG_SCRIPT}" CONTENT "${STRUCTURED_EXIT_DEBUG_CONTENT}")

add_test(NAME structured_exit_debug_run
         COMMAND ${CMAKE_COMMAND} -P "${STRUCTURED_EXIT_DEBUG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_crexx_run_test(test_obj_prop rexx_src/test_obj_prop.crexx)
add_crexx_run_test(test_meth_vs_proc rexx_src/test_meth_vs_proc.crexx)
add_crexx_run_test(function_arg_method_call rexx_src/function_arg_method_call.crexx)
add_crexx_run_test(test_class_adv rexx_src/test_class_adv.crexx)
add_crexx_run_test(test_array_edge rexx_src/test_array_edge.crexx)
add_crexx_run_test(test_utf_math rexx_src/test_utf_math.crexx)
add_crexx_run_test(binary_literal_load rexx_src/binary_literal_load.crexx)
add_crexx_run_test(scalar_type_casts rexx_src/scalar_type_casts.crexx)
add_crexx_run_test(decimal_literal_context rexx_src/decimal_literal_context.crexx)
add_crexx_run_test(typed_constructor_builtin rexx_src/typed_constructor_builtin.crexx)
add_crexx_run_test(test_lib_boundary rexx_src/test_lib_boundary.crexx)
add_crexx_run_test(div_ops_common rexx_src/div_ops_common.crexx)
add_crexx_run_test(div_ops_classic rexx_src/div_ops_classic.crexx)
add_crexx_run_test(classic_xor rexx_src/classic_xor.crexx)
add_crexx_run_test(named_operators rexx_src/named_operators.crexx)

# Shadowing behavior in simple DO blocks
add_crexx_run_test(do_scope_typed_shadows rexx_src/do_scope_typed_shadows.crexx)

# Hoisting and effective scopes tests
add_crexx_run_test(test_global_subscopes rexx_src/test_global_subscopes.crexx)

# Inlining tests
add_crexx_run_test(inline_test1 rexx_src/inline_test1.crexx)
add_crexx_run_test(inline_test2 rexx_src/inline_test2.crexx)
add_crexx_run_test(inline_test_call rexx_src/inline_test_call.crexx)
add_crexx_run_test(inline_test_expr rexx_src/inline_test_expr.crexx)
add_crexx_run_test(inline_test_expr_negative rexx_src/inline_test_expr_negative.crexx)
add_crexx_run_test(inline_test_nested_call_expr rexx_src/inline_test_nested_call_expr.crexx)
add_crexx_run_test(inline_nested_binary_block_owner rexx_src/inline_nested_binary_block_owner.crexx)
add_crexx_run_test(inline_test_concat_expr rexx_src/inline_test_concat_expr.crexx)
add_crexx_run_test(inline_test_say_expr rexx_src/inline_test_say_expr.crexx)
add_crexx_run_test(inline_test_say_expr_negative rexx_src/inline_test_say_expr_negative.crexx)
add_crexx_run_test(inline_test_return_expr rexx_src/inline_test_return_expr.crexx)
add_crexx_run_test(inline_test_unary_expr rexx_src/inline_test_unary_expr.crexx)
add_crexx_run_test(inline_test_compare_expr rexx_src/inline_test_compare_expr.crexx)
add_crexx_run_test(inline_test_call_arg_expr rexx_src/inline_test_call_arg_expr.crexx)
add_crexx_run_test(inline_test_call_like_arg_expr rexx_src/inline_test_call_like_arg_expr.crexx)
add_crexx_run_test(inline_test_short_circuit_expr rexx_src/inline_test_short_circuit_expr.crexx)
add_crexx_run_test(inline_test_composed_expr_contexts rexx_src/inline_test_composed_expr_contexts.crexx)
add_crexx_run_test(inline_test_byvalue_arg_reuse rexx_src/inline_test_byvalue_arg_reuse.crexx)
add_crexx_run_test(inline_test_stateful_assembler rexx_src/inline_test_stateful_assembler.crexx)
add_crexx_run_test(assembler_wide_operands rexx_src/assembler_wide_operands.crexx)
add_crexx_run_test(inline_test_condition_contexts rexx_src/inline_test_condition_contexts.crexx)
add_crexx_run_test(inline_test_loop_bound_expr rexx_src/inline_test_loop_bound_expr.crexx)
add_crexx_run_test(inline_test_member_receiver_expr rexx_src/inline_test_member_receiver_expr.crexx)
add_crexx_run_test(inline_test_named_factory_selector rexx_src/inline_test_named_factory_selector.crexx)
add_crexx_run_test(inline_test_factory_arg_scope rexx_src/inline_test_factory_arg_scope.crexx)
add_crexx_run_test(san003_inline_scoped_array_actual
        rexx_src/san003_inline_scoped_array_actual.crexx)
add_test(NAME san003_inline_scoped_array_actual_compile
        COMMAND $<TARGET_FILE:rxc>
                -x --no-exe-import --import-rxas
                -i ${CMAKE_BINARY_DIR}/bin
                -o ${CMAKE_CURRENT_BINARY_DIR}/san003_inline_scoped_array_actual_compile
                ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/san003_inline_scoped_array_actual.crexx)
set_tests_properties(
        san003_inline_scoped_array_actual_compile
        san003_inline_scoped_array_actual_run_noopt
        san003_inline_scoped_array_actual_run_opt
        PROPERTIES LABELS "sanitizer;san-003;compiler;inlining;regression")
add_test(NAME san004_task_pending_object_result_compile
        COMMAND $<TARGET_FILE:rxc>
                -x --no-exe-import --import-rxas
                -i ${CMAKE_BINARY_DIR}/bin
                -o ${CMAKE_CURRENT_BINARY_DIR}/san004_task_pending_object_result_compile
                ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/san004_task_pending_object_result.crexx)
set_tests_properties(san004_task_pending_object_result_compile PROPERTIES
        LABELS "sanitizer;san-004;compiler;levelg;concurrency;regression")
add_test(NAME san006_import_context_lifetime
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DCLASS_AGGREGATE=${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin
                -DCLASS_MEMBER=${CREXX_CLASSLIB_NATIVE_MEMBER_ROOT}/KeyDB/KeyDB.rxbin
                -DCLASS_SOURCE=${CMAKE_SOURCE_DIR}/lib/classlib/KeyDB.crexx
                -DMAIN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/san006_import_context_main.crexx
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/san006_import_context_lifetime_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/san006_import_context_lifetime.cmake)
set_tests_properties(san006_import_context_lifetime PROPERTIES
        LABELS "sanitizer;san-006;compiler;imports;regression"
        TIMEOUT 60)
add_test(NAME san007_imported_inline_payload_lifetime
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                -DSOURCE=${CMAKE_SOURCE_DIR}/lib/rxfnsg/rexx/httpcore.crexx
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/san007_imported_inline_payload_lifetime_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/san007_imported_inline_payload_lifetime.cmake)
set_tests_properties(san007_imported_inline_payload_lifetime PROPERTIES
        LABELS "sanitizer;san-007;compiler;imports;inlining;regression"
        TIMEOUT 60)
add_crexx_run_test(inline_test_computed_receiver_copyback rexx_src/inline_test_computed_receiver_copyback.crexx)
add_crexx_run_test(call_postfix_surface rexx_src/call_postfix_surface.crexx)
set_tests_properties(inline_test_computed_receiver_copyback_run_noopt PROPERTIES
        LABELS "compiler;parser;levelb;call-postfix;inlining")
set_tests_properties(inline_test_computed_receiver_copyback_run_opt PROPERTIES
        LABELS "compiler;parser;levelb;call-postfix;inlining;linked_opt")
set_tests_properties(call_postfix_surface_run_noopt PROPERTIES
        LABELS "compiler;parser;levelb;call-postfix")
set_tests_properties(call_postfix_surface_run_opt PROPERTIES
        LABELS "compiler;parser;levelb;call-postfix;linked_opt")
add_crexx_run_test(inline_test_loop_bound_early_return_cleanup rexx_src/inline_test_loop_bound_early_return_cleanup.crexx)
add_crexx_run_test(structured_exit_crossed_loop_cleanup rexx_src/structured_exit_crossed_loop_cleanup.crexx)
add_crexx_run_test(signal_crossed_loop_cleanup rexx_src/signal_crossed_loop_cleanup.crexx)
add_crexx_run_test(signal_nested_handler_unwind rexx_src/signal_nested_handler_unwind.crexx)
add_crexx_run_test(inline_test_mutating_method_scalar_attr_return rexx_src/inline_test_mutating_method_scalar_attr_return.crexx)
add_test(NAME inline_late_profitability_guard
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DRXVM=${CREXX_RXVM_EXECUTABLE}
                 -DRXBVM=$<TARGET_FILE:rxbvm>
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_late_profitability_guard.crexx
                 -DINCLUDE_DIR=${CMAKE_BINARY_DIR}/bin
                 -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library.rxbin
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_late_profitability_guard_ws
                 -P ${CMAKE_SOURCE_DIR}/cmake/CheckInlineLateProfitability.cmake)
set_tests_properties(inline_late_profitability_guard PROPERTIES
        LABELS "compiler;inlining;performance;regression")
add_crexx_run_test(flow_copy_fixed_point_inline_result rexx_src/flow_copy_fixed_point_inline_result.crexx)
add_test(
        NAME flow_copy_fixed_point_inline_result_contract
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
                -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/flow_copy_fixed_point_inline_result.crexx
                -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/flow_copy_fixed_point_inline_result_contract_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/flow_copy_fixed_point_inline_result_contract.cmake
)
set_tests_properties(flow_copy_fixed_point_inline_result_contract
                     PROPERTIES LABELS "compiler;flow;performance")
add_crexx_run_test(inline_test_ref_opt rexx_src/inline_test_ref_opt.crexx)
add_crexx_run_test(inline_test_ref_indexed rexx_src/inline_test_ref_indexed.crexx)
add_crexx_run_test(inline_test_ref_stem rexx_src/inline_test_ref_stem.crexx)
add_crexx_run_test(inline_test_ref_computed rexx_src/inline_test_ref_computed.crexx)
add_crexx_run_test(inline_test_ref_vararg rexx_src/inline_test_ref_vararg.crexx)
add_crexx_run_test(inline_test_ref_vararg_runtime rexx_src/inline_test_ref_vararg_runtime.crexx)
add_crexx_run_test(inline_test_ref_negative rexx_src/inline_test_ref_negative.crexx)
add_crexx_run_test(inline_test_bool_expr_negative rexx_src/inline_test_bool_expr_negative.crexx)
add_crexx_run_test(inline_test_vararg rexx_src/inline_test_vararg.crexx)
add_crexx_run_test(inline_test_vararg_dynamic rexx_src/inline_test_vararg_dynamic.crexx)
add_crexx_run_test(inline_test_vararg_dynamic_loop rexx_src/inline_test_vararg_dynamic_loop.crexx)
add_crexx_run_test(inline_test_vararg_negative rexx_src/inline_test_vararg_negative.crexx)
add_crexx_run_test(issue_680_vararg_call_argument
        rexx_src/issue_680_vararg_call_argument.crexx)
set_tests_properties(issue_680_vararg_call_argument_run_noopt
        issue_680_vararg_call_argument_run_opt PROPERTIES
        LABELS "compiler;inlining;vararg;regression;issue-680")
add_crexx_run_test(inline_test_exposed_nested_call rexx_src/inline_test_exposed_nested_call.crexx)
add_crexx_run_test(inline_test_nested_scope_do rexx_src/inline_test_nested_scope_do.crexx)
add_crexx_run_test(inline_test_nested_scope_if_do rexx_src/inline_test_nested_scope_if_do.crexx)
add_crexx_run_test(inline_test_nested_scope_exposed_call rexx_src/inline_test_nested_scope_exposed_call.crexx)
add_crexx_run_test(inline_test_multi_return_assign rexx_src/inline_test_multi_return_assign.crexx)
add_crexx_run_test(inline_test_multi_return_expr rexx_src/inline_test_multi_return_expr.crexx)
add_crexx_run_test(inline_test_multi_return_call rexx_src/inline_test_multi_return_call.crexx)
add_crexx_run_test(inline_test_void_fallthrough_call rexx_src/inline_test_void_fallthrough_call.crexx)
add_crexx_run_test(inline_test_array_return_assign rexx_src/inline_test_array_return_assign.crexx)
add_crexx_run_test(inline_test_array_return_expr rexx_src/inline_test_array_return_expr.crexx)
add_crexx_run_test(inline_test_array_multi_return_expr rexx_src/inline_test_array_multi_return_expr.crexx)
add_crexx_run_test(inline_test_array_multi_return_assign rexx_src/inline_test_array_multi_return_assign.crexx)
add_crexx_run_test(inline_test_object_return_assign rexx_src/inline_test_object_return_assign.crexx)
add_crexx_run_test(inline_test_object_return_expr rexx_src/inline_test_object_return_expr.crexx)
add_crexx_run_test(inline_test_object_const_arg_mutation rexx_src/inline_test_object_const_arg_mutation.crexx)
add_crexx_run_test(inline_test_object_writable_arg rexx_src/inline_test_object_writable_arg.crexx)
add_crexx_run_test(inline_test_array_expr_arg rexx_src/inline_test_array_expr_arg.crexx)
add_crexx_run_test(inline_test_object_expr_arg rexx_src/inline_test_object_expr_arg.crexx)
add_crexx_run_test(inline_test_array_return_temp rexx_src/inline_test_array_return_temp.crexx)
add_crexx_run_test(inline_test_block_expr_live_sibling rexx_src/inline_test_block_expr_live_sibling.crexx)
add_crexx_run_test(inline_test_imported_bif_block_expr_lifetime rexx_src/inline_test_imported_bif_block_expr_lifetime.crexx)
add_crexx_run_test(attr_array_call_result_lifetime rexx_src/attr_array_call_result_lifetime.crexx)
add_crexx_run_test(deferred_register_outer_if_lifetime
        rexx_src/deferred_register_outer_if_lifetime.crexx)
add_crexx_run_test(short_circuit_linked_left
        rexx_src/short_circuit_linked_left.crexx)
set_tests_properties(short_circuit_linked_left_run_noopt
        short_circuit_linked_left_run_opt PROPERTIES
        LABELS "compiler;register-lifetime;regression")
set_tests_properties(deferred_register_outer_if_lifetime_run_noopt
        deferred_register_outer_if_lifetime_run_opt PROPERTIES
        LABELS "compiler;register-lifetime;regression")
add_crexx_run_test(inline_indexed_attr_factory_lifetime
        rexx_src/inline_indexed_attr_factory_lifetime.crexx)
add_test(NAME inline_indexed_attr_factory_lifetime_shape
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_indexed_attr_factory_lifetime.crexx
                 -DINCLUDE_DIR=${CMAKE_BINARY_DIR}/bin
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_indexed_attr_factory_lifetime_shape_ws
                 -P ${CMAKE_SOURCE_DIR}/cmake/CheckInlineIndexedAttrFactoryLifetime.cmake)
set_tests_properties(inline_indexed_attr_factory_lifetime_shape PROPERTIES
        LABELS "compiler;inlining;register-lifetime;regression")
add_crexx_run_test(inline_test_complex_lhs_assign rexx_src/inline_test_complex_lhs_assign.crexx)
add_crexx_run_test(inline_test_binary_arg_return rexx_src/inline_test_binary_arg_return.crexx)
add_crexx_run_args_test(inline_binary_formal_string_conversion
        rexx_src/inline_binary_formal_string_conversion.crexx ABC)
add_test(NAME inline_binary_formal_string_conversion_shape
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_binary_formal_string_conversion.crexx
                 -DINCLUDE_DIR=${CMAKE_BINARY_DIR}/bin
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_binary_formal_string_conversion_shape_ws
                 -P ${CMAKE_SOURCE_DIR}/cmake/CheckInlineBinaryFormalConversion.cmake)
set_tests_properties(inline_binary_formal_string_conversion_shape PROPERTIES
        LABELS "compiler;inlining;binary;conversion;regression")
add_crexx_run_test(inline_test_binary_class_attr_assign rexx_src/inline_test_binary_class_attr_assign.crexx)
add_test(NAME inline_test_binary_class_attr_assign_shape
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_test_binary_class_attr_assign.crexx
                 -DINCLUDE_DIR=${CMAKE_BINARY_DIR}/bin
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/inline_test_binary_class_attr_assign_shape_ws
                 -P ${CMAKE_SOURCE_DIR}/cmake/CheckInlineBinaryClassAttribute.cmake)
set_tests_properties(inline_test_binary_class_attr_assign_shape PROPERTIES
        LABELS "compiler;inlining;binary;regression")
add_crexx_run_test(gate_f_signal_names rexx_src/gate_f_signal_names.crexx)
add_crexx_run_test(inline_test_class_methods rexx_src/inline_test_class_methods.crexx)

# New Phase 2 Scoping Tests
add_crexx_run_test(test_simple_do_subscope rexx_src/test_simple_do_subscope.crexx)
add_crexx_run_test(test_if_do_subscope rexx_src/test_if_do_subscope.crexx)
add_crexx_run_test(test_iterative_do_subscope rexx_src/test_iterative_do_subscope.crexx)
add_crexx_run_test(test_if_no_subscope rexx_src/test_if_no_subscope.crexx)
add_crexx_run_test(test_untyped_subscopes rexx_src/test_untyped_subscopes.crexx)
add_crexx_run_test(test_untyped_subscope_2 rexx_src/test_untyped_subscope_2.crexx)
add_crexx_run_test(test_if_else_nested_subscopes rexx_src/test_if_else_nested_subscopes.crexx)
add_crexx_run_test(test_sub_sub_scopes rexx_src/test_sub_sub_scopes.crexx)
add_crexx_run_test(test_sub_sub_scopes_untyped rexx_src/test_sub_sub_scopes_untyped.crexx)
add_crexx_run_test(test_global_nested_complex rexx_src/test_global_nested_complex.crexx)
add_crexx_run_test(test_function_shadowing rexx_src/test_function_shadowing.crexx)
add_crexx_run_test(test_disjoint_detailed rexx_src/test_disjoint_detailed.crexx)

# New test for detailed warning
set(WARN_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_warn_test.cmake")
set(WARN_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o test_disjoint_detailed_warn \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/test_disjoint_detailed.crexx\" ERROR_VARIABLE err_out OUTPUT_VARIABLE std_out RESULT_VARIABLE res)\n")
string(APPEND WARN_CONTENT "if(NOT err_out MATCHES \"NOT_IN_SAME_SCOPE line=\\\"4\\\" column=\\\"5\\\"\")\n  message(FATAL_ERROR \"Expected warning not found in output\")\nendif()\n")
file(GENERATE OUTPUT "${WARN_SCRIPT}" CONTENT "${WARN_CONTENT}")

add_test(NAME test_disjoint_detailed_warn
         COMMAND ${CMAKE_COMMAND} -P "${WARN_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME cri18_scope_warning
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/cri18_scope_warning_ws
                 -DCRI18_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/cri18_disjoint_scope_warning.crexx
                 -DASSIGN_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/disjoint_assignment_warning.crexx
                 -DEXPLICIT_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/disjoint_explicit_intent.crexx
                 -DLEXICAL_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/disjoint_lexical_paths.crexx
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/cri18_scope_warning.cmake)
set_tests_properties(cri18_scope_warning PROPERTIES
        LABELS "compiler;diagnostics;scope;cri-18")

add_test(NAME cri18_exit_scratch_warning
         COMMAND ${CMAKE_COMMAND}
                 -DRXC=$<TARGET_FILE:rxc>
                 -DIMPORT_DIR=${CMAKE_BINARY_DIR}/bin
                 -DWORK=${CMAKE_CURRENT_BINARY_DIR}/cri18_exit_scratch_warning_ws
                 -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/cri18_exit_scratch_explicit.crexx
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/cri18_exit_scratch_warning.cmake)
set_tests_properties(cri18_exit_scratch_warning PROPERTIES
        LABELS "compiler;compiler-exit;diagnostics;scope;cri-18")


# Parsing Tests
add_crexx_parsing_test(expr_abut_01 parsing_src/expr_abut_01.crexx)
add_crexx_parsing_test(expr_abut_02 parsing_src/expr_abut_02.crexx)
add_crexx_parsing_test(expr_abut_03 parsing_src/expr_abut_03.crexx)
add_crexx_parsing_test(expr_abut_04 parsing_src/expr_abut_04.crexx)
add_crexx_parsing_test(expr_ambig_01 parsing_src/expr_ambig_01.crexx)
add_crexx_parsing_test(expr_ambig_02 parsing_src/expr_ambig_02.crexx)
add_crexx_parsing_test(expr_ambig_03 parsing_src/expr_ambig_03.crexx)
add_crexx_parsing_test(binary_intrinsics_parse parsing_src/binary_intrinsics_parse.crexx)
add_crexx_parsing_test(err_struct_01 parsing_src/err_struct_01.crexx)
add_crexx_parsing_test(err_struct_02 parsing_src/err_struct_02.crexx)
add_crexx_parsing_test(err_struct_03 parsing_src/err_struct_03.crexx)
add_crexx_parsing_test(err_namespace_class_anchor parsing_src/err_namespace_class_anchor.crexx)
add_crexx_parsing_test(address_exit_extended parsing_src/address_exit_extended.crexx)
add_crexx_parsing_test(address_inline_then parsing_src/address_inline_then.crexx)
# DO constructs parsing coverage
add_crexx_parsing_test(do_simple_parse parsing_src/do_simple_parse.crexx)
add_crexx_parsing_test(do_counted_parse parsing_src/do_counted_parse.crexx)
add_crexx_parsing_test(do_while_parse parsing_src/do_while_parse.crexx)
add_crexx_parsing_test(do_until_parse parsing_src/do_until_parse.crexx)
add_crexx_parsing_test(block_expr_parse parsing_src/block_expr_parse.crexx)

# SELECT constructs parsing coverage
add_crexx_parsing_test(select_classic_parse parsing_src/select_classic_parse.crexx)
add_crexx_parsing_test(select_c_parse parsing_src/select_c_parse.crexx)

if (ENABLE_PARSER_MODE)
    add_crexx_syntax_highlight_test(
            syntaxhighlight_root_file
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/16_classes.crexx
            MUST_MATCH
            "Node: type=71 \\(PARSE_TREE_FILE\\)"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_syntax_error
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_02_syntax.crexx
            MUST_MATCH
            "SYNTAX_ERROR"
            "Message:"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_srcmap_generated
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_srcmap_generated.crexx
            MUST_MATCH
            "Node: type=37 \\(LEXER_PREPROCESSOR\\), pos=[0-9]+, len=3, sev=48, text=\"@1c\""
            "Node: type=37 \\(LEXER_PREPROCESSOR\\), pos=[0-9]+, len=7, sev=48, text=\"@\\+4\\+16\\{\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=57 \\(LEXER_CONSTANT_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"totl\""
            MUST_NOT_MATCH
            "text=\"emo\""
            "text=\"xpp"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_compound_names
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/repro_dotted_index.crexx
            MUST_MATCH
            "Node: type=46 \\(LEXER_SEPARATOR\\), pos=[0-9]+, len=1, sev=48, text=\"\\.\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"lino\""
            "Node: type=40 \\(LEXER_NUMBER_LITERAL\\), pos=[0-9]+, len=1, sev=48, text=\"1\""
            MUST_NOT_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=[0-9]+, len=1, sev=48, text=\"\\.\""
            "Node: type=46 \\(LEXER_SEPARATOR\\), pos=[0-9]+, len=1, sev=48, text=\"[A-Za-z0-9]\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_structure_nodes
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/16_classes.crexx
            MUST_MATCH
            "PARSE_TREE_STRUCTURE"
            "PARSE_TREE_FUNCTION"
            "PARSE_TREE_CODEBLOCK"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_exit_keyword
            ${CMAKE_SOURCE_DIR}/compiler/exits/fsay/test_fsay.crexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"fsay\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_address_keywords
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_address_keywords.crexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"address\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"input\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"output\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"error\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"expose\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_parse_keywords
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_parse_keywords.crexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"parse\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"upper\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"value\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"trim\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"into\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"var\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_trace_keywords
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_trace_keywords.crexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"trace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"off\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"normal\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"rexx\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"asm\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_top_level_statements
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/01_assign.crexx
            MUST_MATCH
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "PARSE_TREE_FUNCTION"
    )
    set_tests_properties(syntaxhighlight_top_level_statements PROPERTIES LABELS "compiler;syntax_highlighting;stage1")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_tracer
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_tracer.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=2, sev=48, text=\"if\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=56 \\(LEXER_FUNCTION_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"label:\""
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "SYNTAX_ERROR"
    )
    set_tests_properties(syntaxhighlight_levelc_tracer PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_keyword_variables
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_keyword_variables.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=2, sev=48, text=\"if\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "PARSE_TREE_EXPR"
            MUST_NOT_MATCH
            "SYNTAX_ERROR"
    )
    set_tests_properties(syntaxhighlight_levelc_keyword_variables PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_simple_instructions
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_simple_instructions.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"address\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"arg\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"call\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"drop\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"exit\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"interpret\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"nop\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"numeric\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"procedure\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"pull\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"push\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"queue\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"return\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"signal\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"trace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"value\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"with\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"output\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"stream\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"replace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"on\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"off\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"error\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"halt\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"name\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"digits\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"form\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=10, sev=48, text=\"scientific\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"fuzz\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"expose\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"novalue\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"syntax\""
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_simple_instructions PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_command_instructions
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_command_instructions.rexx
            MUST_MATCH
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=14, sev=48, text=\"'host command'\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=50, text=\"host\""
            "PARSE_TREE_EXPR"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"if\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"select\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"otherwise\""
            "RXC-LC-IMPLICIT_ADDRESS"
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_command_instructions PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_implicit_command_warning
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_implicit_command_warning.rexx
            MUST_MATCH
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'listfiles'\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=3, sev=50, text=\"SEY\""
            "RXC-LC-IMPLICIT_ADDRESS"
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"after\""
            MUST_NOT_MATCH
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=50, text=\"'listfiles'\""
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_implicit_command_warning PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_instruction_bad_forms
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_instruction_bad_forms.rexx
            MUST_MATCH
            "RXC-LC-20\\.1"
            "RXC-LC-19\\.2"
            "RXC-LC-19\\.4"
            "RXC-LC-25\\.15"
            "RXC-LC-25\\.11"
            "RXC-LC-25\\.5"
            "RXC-LC-35\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_instruction_bad_forms PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_instruction_recovery_boundaries
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_instruction_recovery_boundaries.rexx
            MUST_MATCH
            "RXC-LC-25\\.15 keywords=\"DIGITS FORM FUZZ\" token=\"10\""
            "RXC-LC-25\\.17 keywords=\"EXPOSE\" token=\"10\""
            "RXC-LC-20\\.1 token=\"10\""
            "RXC-LC-25\\.12 keywords=\"ARG PULL SOURCE LINEIN VERSION VALUE VAR\" token=\"10\""
            "RXC-LC-25\\.13 keywords=\"ARG PULL SOURCE LINEIN VERSION VALUE VAR\" token=\"10\""
            "RXC-LC-19\\.2 token=\"\\+\""
            "RXC-LC-19\\.4 token=\"10\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"arg\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=48, text=\"a\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=11, sev=48, text=\"afterSignal\""
            MUST_NOT_MATCH
            "token=\"a\""
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_instruction_recovery_boundaries PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_instruction_tail_bad_forms
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_instruction_tail_bad_forms.rexx
            MUST_MATCH
            "RXC-LC-25\\.1 keywords=\"ERROR FAILURE HALT NOTREADY\" token=\"end-of-clause\""
            "RXC-LC-25\\.1 keywords=\"ERROR FAILURE HALT NOTREADY\" token=\"syntax\""
            "RXC-LC-25\\.2 keywords=\"ERROR FAILURE HALT NOTREADY\" token=\"10\""
            "RXC-LC-19\\.3 token=\"end-of-clause\""
            "RXC-LC-19\\.3 token=\"10\""
            "RXC-LC-25\\.3 keywords=\"ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS\" token=\"end-of-clause\""
            "RXC-LC-25\\.3 keywords=\"ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS\" token=\"banana\""
            "RXC-LC-25\\.4 keywords=\"ERROR FAILURE HALT NOTREADY NOVALUE SYNTAX LOSTDIGITS\" token=\"10\""
            "RXC-LC-19\\.1 token=\"end-of-clause\""
            "RXC-LC-25\\.5 keywords=\"INPUT OUTPUT ERROR\" token=\"banana\""
            "RXC-LC-25\\.5 keywords=\"INPUT OUTPUT ERROR\" token=\"10\""
            "RXC-LC-25\\.11 keywords=\"ENGINEERING SCIENTIFIC VALUE\" token=\"10\""
            "RXC-LC-35\\.1 token=\"end-of-clause\""
            "RXC-LC-20\\.1 token=\"end-of-clause\""
            "RXC-LC-20\\.1 token=\"\\)\""
            "RXC-LC-20\\.1 token=\"10\""
            "RXC-LC-38\\.1 token=\"\\)\""
            "RXC-LC-38\\.1"
            "RXC-LC-19\\.4 token=\"end-of-clause\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=24, sev=48, text=\"afterAddressValueBadExpr\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_instruction_tail_bad_forms PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_labels_validation
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_labels_validation.rexx
            MUST_MATCH
            "RXC-LC-16\\.2 name=\"INNER\""
            "RXC-LC-16\\.3 name=\"INNER\""
            "RXC-LC-17\\.1"
            "Node: type=56 \\(LEXER_FUNCTION_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"good:\""
            "Node: type=56 \\(LEXER_FUNCTION_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"inner:\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"procedure\""
            MUST_NOT_MATCH
            "RXC-LC-16\\.1 name=\"MISSING\""
            "RXC-LC-16\\.1 name=\"GOOD\""
            "RXC-LC-16\\.3 name=\"DATE\""
            "RXC-LC-16\\.3 name=\"EXTERNAL_PROC\""
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_labels_validation PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_numeric_validation
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_numeric_validation.rexx
            MUST_MATCH
            "RXC-LC-26\\.5 value=\"0\""
            "RXC-LC-26\\.5 value=\"buzz\""
            "RXC-LC-6\\.3"
            "RXC-LC-26\\.6 value=\"-1\""
            "RXC-LC-26\\.6 value=\"buzz\""
            "RXC-LC-6\\.2"
            "RXC-LC-33\\.6 value=\"bad\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"numeric\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=13, sev=48, text=\"'engineering'\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=12, sev=48, text=\"'scientific'\""
            MUST_NOT_MATCH
            "RXC-LC-26\\.5 value=\"9\""
            "RXC-LC-26\\.6 value=\"0\""
            "RXC-LC-33\\.6 value=\"engineering\""
            "RXC-LC-33\\.6 value=\"scientific\""
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_numeric_validation PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_deep_validation
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_deep_validation.rexx
            MUST_MATCH
            "RXC-LC-24\\.1 value=\"B\""
            "RXC-LC-19\\.6 token=\"\\+\""
            "RXC-LC-20\\.1 token=\"10\""
            "RXC-LC-31\\.1 token=\"10\""
            "RXC-LC-25\\.6 keywords=\"STREAM STEM NORMAL\" token=\"end-of-clause\""
            "RXC-LC-53\\.1 token=\"end-of-clause\""
            "RXC-LC-53\\.1 token=\"10\""
            "RXC-LC-25\\.8 keywords=\"STREAM STEM\" token=\"normal\""
            "RXC-LC-53\\.3 name=\"out\""
            "RXC-LC-15\\.3 char=\"G\""
            "RXC-LC-15\\.4 char=\"2\""
            "RXC-LC-15\\.1"
            "RXC-LC-15\\.2"
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_deep_validation PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_deep_validation_ok
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_deep_validation_ok.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"trace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"address\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=4, sev=48, text=\"'F'X\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=12, sev=48, text=\"'0000 1111'B\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "INVALID_HEX"
            "INVALID_BIN"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_deep_validation_ok PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_simple_instruction_keyword_variables
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_simple_instruction_keyword_variables.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=7, sev=48, text=\"address\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"call\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"return\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"trace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_simple_instruction_keyword_variables PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_parse_templates
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_parse_templates.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"parse\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"upper\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"pull\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"source\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"linein\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"version\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"value\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"with\""
            "Node: type=46 \\(LEXER_SEPARATOR\\), pos=[0-9]+, len=1, sev=48, text=\"\\.\""
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_parse_templates PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_parse_bad_forms
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_parse_bad_forms.rexx
            MUST_MATCH
            "RXC-LC-25\\.12"
            "RXC-LC-25\\.13"
            "RXC-LC-38\\.3"
            "RXC-LC-38\\.2"
            "RXC-LC-38\\.1"
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_parse_bad_forms PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_if_else
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_if_else.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"if\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"else\""
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "SYNTAX_ERROR"
    )
    set_tests_properties(syntaxhighlight_levelc_if_else PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_labels_literals
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_labels_literals.rexx
            MUST_MATCH
            "Node: type=56 \\(LEXER_FUNCTION_IDENTIFIER\\), pos=0, len=6, sev=48, text=\"start:\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=8, sev=48, text=\"\"double\"\""
            "Node: type=40 \\(LEXER_NUMBER_LITERAL\\), pos=[0-9]+, len=3, sev=48, text=\"123\""
            MUST_NOT_MATCH
            "SYNTAX_ERROR"
    )
    set_tests_properties(syntaxhighlight_levelc_labels_literals PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_constant_symbols
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_slice45_constant_symbols.rexx
            MUST_MATCH
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=5, sev=48, text=\"1.2.3\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=4, sev=48, text=\"1e2x\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_constant_symbols PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_then_boundary
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_then_boundary.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=51, text=\"then\""
            "RXC-LC-35\\.1 token=\"then\""
            "RXC-LC-8\\.1"
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_then_boundary PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_if_missing_then
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_if_missing_then.rexx
            MUST_MATCH
            "RXC-LC-18\\.1 linenumber=\"1\" token=\"end-of-clause\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_if_missing_then PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_then
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_then.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=4, sev=51, text=\"then\""
            "RXC-LC-8\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_then PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_else
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_else.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=4, sev=51, text=\"else\""
            "RXC-LC-8\\.2"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_else PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_multiline_if_else
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_multiline_if_else.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"if\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"then\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"else\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_multiline_if_else PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_when_otherwise
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_when_otherwise.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"select\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"when\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"otherwise\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=72 \\(PARSE_TREE_CODEBLOCK\\), pos=[0-9]+, len=[0-9]+, sev=48\n[ ]+Node: type=73 \\(PARSE_TREE_STATEMENT\\), pos=[0-9]+, len=[0-9]+, sev=48\n[ ]+Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_when_otherwise PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_do_otherwise
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_do_otherwise.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"select\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"when\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"do\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"otherwise\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_do_otherwise PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_do_otherwise_upper
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_do_otherwise_upper.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"SELECT\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"WHEN\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"DO\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=9, sev=48, text=\"OTHERWISE\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"END\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_do_otherwise_upper PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_keyword_variables
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_keyword_variables.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=2, sev=48, text=\"do\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=2, sev=48, text=\"to\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=2, sev=48, text=\"by\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=3, sev=48, text=\"for\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"while\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"until\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=7, sev=48, text=\"forever\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"leave\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=7, sev=48, text=\"iterate\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_keyword_variables PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_control
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_control.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"do\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"to\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=48, text=\"by\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"for\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_control PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_conditions_leave
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_conditions_leave.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"while\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"until\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"forever\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"leave\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"iterate\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_conditions_leave PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_end_bad_symbol
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_end_bad_symbol.rexx
            MUST_MATCH
            "RXC-LC-10\\.2 linenumber=\"1\" token=\"j\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=51, text=\"j\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_end_bad_symbol PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_end_uncontrolled_symbol
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_end_uncontrolled_symbol.rexx
            MUST_MATCH
            "RXC-LC-10\\.3 linenumber=\"1\" token=\"i\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=51, text=\"i\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_end_uncontrolled_symbol PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_end_symbol
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_end_symbol.rexx
            MUST_MATCH
            "RXC-LC-10\\.4 linenumber=\"1\" token=\"i\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=51, text=\"i\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_end_symbol PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_leave_outside_loop
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_leave_outside_loop.rexx
            MUST_MATCH
            "RXC-LC-28\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=5, sev=51, text=\"leave\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_leave_outside_loop PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_iterate_bad_target
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_iterate_bad_target.rexx
            MUST_MATCH
            "RXC-LC-28\\.4 token=\"k\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=1, sev=51, text=\"k\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_iterate_bad_target PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_forever_bad_tail
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_forever_bad_tail.rexx
            MUST_MATCH
            "RXC-LC-25\\.16 keywords=\"WHILE UNTIL\" token=\"to\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=51, text=\"to\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_forever_bad_tail PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_invalid_initial_to
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_invalid_initial_to.rexx
            MUST_MATCH
            "RXC-LC-27\\.1 token=\"to\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=2, sev=51, text=\"to\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_invalid_initial_to PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_missing_to
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_missing_to.rexx
            MUST_MATCH
            "RXC-LC-35\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_missing_to PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_do_duplicate_to
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_do_duplicate_to.rexx
            MUST_MATCH
            "RXC-LC-27\\.1 token=\"to\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_duplicate_to PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_keyword_variables
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_keyword_variables.rexx
            MUST_MATCH
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=0, len=6, sev=48, text=\"select\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=4, sev=48, text=\"when\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=9, sev=48, text=\"otherwise\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_keyword_variables PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_missing_when
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_missing_when.rexx
            MUST_MATCH
            "RXC-LC-7\\.1 linenumber=\"1\" token=\"end\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=51, text=\"end\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_missing_when PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_select_missing_end
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_missing_end.rexx
            MUST_MATCH
            "RXC-LC-14\\.2"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=6, sev=51, text=\"select\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"when\""
            MUST_NOT_MATCH
            "MISSING_END"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_select_missing_end PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_when
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_when.rexx
            MUST_MATCH
            "RXC-LC-9\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=4, sev=51, text=\"when\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_when PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_otherwise
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_otherwise.rexx
            MUST_MATCH
            "RXC-LC-9\\.2"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=9, sev=51, text=\"otherwise\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_otherwise PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_loose_end
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_loose_end.rexx
            MUST_MATCH
            "RXC-LC-10\\.1"
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=0, len=3, sev=51, text=\"end\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_loose_end PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_when_missing_then
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_when_missing_then.rexx
            MUST_MATCH
            "RXC-LC-18\\.2 linenumber=\"2\" token=\"end\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=51, text=\"when\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"end\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_when_missing_then PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_precedence
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_precedence.rexx
            MUST_MATCH
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=2, sev=48, text=\"\\*\\*\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=48, text=\"%\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=2, sev=48, text=\"//\""
            "Node: type=42 \\(LEXER_OPERATOR\\), pos=[0-9]+, len=2, sev=48, text=\"\\|\\|\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=48, text=\"&\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=48, text=\"\\|\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"&&\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_precedence PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_comparisons
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_comparisons.rexx
            MUST_MATCH
            "Node: type=43 \\(LEXER_OPERATOR_ASSIGN\\), pos=[0-9]+, len=1, sev=48, text=\"=\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"<>\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"><\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"==\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\">>\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=48, text=\"<<\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\">>=\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\"<<=\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\"\\\\==\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\"\\\\>>\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=3, sev=48, text=\"\\\\<<\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_comparisons PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_comma
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_comma.rexx
            MUST_MATCH
            "RXC-LC-37\\.1"
            "Node: type=46 \\(LEXER_SEPARATOR\\), pos=[0-9]+, len=1, sev=51, text=\",\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=7, sev=48, text=\"'after'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_comma PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_rparen
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_rparen.rexx
            MUST_MATCH
            "RXC-LC-37\\.2"
            "Node: type=53 \\(LEXER_RH_EXPR\\), pos=[0-9]+, len=1, sev=51, text=\"\\)\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=7, sev=48, text=\"'after'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_rparen PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_trailing_compare
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_trailing_compare.rexx
            MUST_MATCH
            "RXC-LC-35\\.1 token=\"end-of-clause\""
            "Node: type=43 \\(LEXER_OPERATOR_ASSIGN\\), pos=[0-9]+, len=1, sev=51, text=\"=\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=7, sev=48, text=\"'after'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_trailing_compare PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_trailing_operator
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_trailing_operator.rexx
            MUST_MATCH
            "RXC-LC-35\\.1 token=\"end-of-clause\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\+\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\*\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=2, sev=51, text=\"\\*\\*\""
            "Node: type=42 \\(LEXER_OPERATOR\\), pos=[0-9]+, len=2, sev=51, text=\"\\|\\|\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=51, text=\"&\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=51, text=\"\\|\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=51, text=\"&&\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'after-xor'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_trailing_operator PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_prefix_operator
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_prefix_operator.rexx
            MUST_MATCH
            "RXC-LC-35\\.1 token=\"end-of-clause\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\+\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"-\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=51, text=\"\\\\\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'after-not'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_prefix_operator PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_leading_operator
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_leading_operator.rexx
            MUST_MATCH
            "RXC-LC-35\\.1 token=\"\\*\""
            "RXC-LC-35\\.1 token=\"/\""
            "RXC-LC-35\\.1 token=\"&\""
            "RXC-LC-35\\.1 token=\"<>\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\*\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"/\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=1, sev=51, text=\"&\""
            "Node: type=45 \\(LEXER_OPERATOR_LOGICAL\\), pos=[0-9]+, len=2, sev=51, text=\"<>\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'after-neq'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_leading_operator PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_levelc_expression_bad_paren
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_expression_bad_paren.rexx
            MUST_MATCH
            "RXC-LC-36"
            "RXC-LC-35\\.1 token=\"\\)\""
            "Node: type=52 \\(LEXER_LH_EXPR\\), pos=[0-9]+, len=1, sev=51, text=\"\\(\""
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=51, text=\"\\+\""
            "Node: type=39 \\(LEXER_STRING_LITERAL\\), pos=[0-9]+, len=11, sev=48, text=\"'after-rhs'\""
            MUST_NOT_MATCH
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_expression_bad_paren PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_block_comment
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_block_comment.crexx
            MUST_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=[0-9]+, len=[0-9]+, sev=48"
            MUST_NOT_MATCH
            "LEXER_OPERATOR_ARITHMETIC"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_comment_only
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_comment_only.crexx
            MUST_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=0, len=13, sev=48, text=\"/\\* comment \\*/\""
            MUST_NOT_MATCH
            "LEXER_OPERATOR_ARITHMETIC"
            "PARSE_FAILURE"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_empty
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_empty.crexx
            MUST_MATCH
            "Node: type=71 \\(PARSE_TREE_FILE\\), pos=0"
            MUST_NOT_MATCH
            "LEXER_TOKEN"
            "PARSE_FAILURE"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_comments_dash
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_comments_dash.crexx
            MUST_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=[0-9]+, len=[0-9]+, sev=48, text=\"-- dash line comment\""
            MUST_NOT_MATCH
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=48, text=\"-\""
    )
endif()

function(add_levelc_unsupported_compile_test NAME SOURCE_FILE)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE out\n"
           "    ERROR_VARIABLE err\n"
           "    RESULT_VARIABLE res\n"
           ")\n"
           "set(all \"${RXAS_DIAG_DOLLAR}{out}${RXAS_DIAG_DOLLAR}{err}\")\n"
           "if(res EQUAL 0)\n"
           "  message(FATAL_ERROR \"Level C compile unexpectedly succeeded for ${NAME}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
           "endif()\n"
           "if(NOT all MATCHES \"REXX Level C \\\\(Classic REXX\\\\) compilation does not yet support this program shape:\")\n"
           "  message(FATAL_ERROR \"Missing Level C unsupported diagnostic for ${NAME}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
           "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

add_levelc_unsupported_compile_test(levelc_compile_unsupported
        rexx_src/levelc_unsupported.rexx)

add_levelc_unsupported_compile_test(levelc_slice4_omitted_call_unsupported
        rexx_src/levelc_slice4_omitted_call_unsupported.rexx)

add_levelc_unsupported_compile_test(levelc_slice6_procedure_parse_unsupported
        rexx_src/levelc_slice6_procedure_parse_unsupported.rexx)

function(add_levelc_invalid_compile_test NAME SOURCE_FILE)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE out\n"
           "    ERROR_VARIABLE err\n"
           "    RESULT_VARIABLE res\n"
           ")\n"
           "set(all \"${RXAS_DIAG_DOLLAR}{out}${RXAS_DIAG_DOLLAR}{err}\")\n"
           "if(res EQUAL 0)\n"
           "  message(FATAL_ERROR \"Invalid Level C compile unexpectedly succeeded for ${NAME}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
           "endif()\n"
           "if(all MATCHES \"REXX Level C \\\\(Classic REXX\\\\) compilation does not yet support this program shape\")\n"
           "  message(FATAL_ERROR \"Invalid Level C compile fell through to the unsupported lowering diagnostic for ${NAME}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
           "endif()\n")

    foreach(REQUIRED_PATTERN IN LISTS ARGN)
        string(APPEND SCRIPT_CONTENT
               "if(NOT all MATCHES \"${REQUIRED_PATTERN}\")\n"
               "  message(FATAL_ERROR \"Invalid Level C compile for ${NAME} is missing required pattern ${REQUIRED_PATTERN}.\\nSTDOUT:\\n${RXAS_DIAG_DOLLAR}{out}\\nSTDERR:\\n${RXAS_DIAG_DOLLAR}{err}\")\n"
               "endif()\n")
    endforeach()

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

add_levelc_invalid_compile_test(levelc_return_invalid_expr
        rexx_src/levelc_return_invalid_expr.rexx "RXC-LC-35.1" "@ 2:18")
add_levelc_invalid_compile_test(levelc_return_invalid_comma
        rexx_src/levelc_return_invalid_comma.rexx "RXC-LC-35.1" "@ 2:9")
add_levelc_invalid_compile_test(levelc_return_invalid_close
        rexx_src/levelc_return_invalid_close.rexx "RXC-LC-35.1" "@ 2:10")
add_levelc_invalid_compile_test(levelc_return_invalid_trailing_comma
        rexx_src/levelc_return_invalid_trailing_comma.rexx "RXC-LC-35.1" "@ 2:9")

add_levelc_invalid_compile_test(levelc_compile_invalid_if_missing_then
        rexx_src/levelc_if_missing_then.rexx
        "RXC-LC-18\\.1")
add_levelc_invalid_compile_test(levelc_arg_malformed
        rexx_src/levelc_arg_malformed.rexx
        "RXC-LC-38\\.1" "levelc_arg_malformed\\.rexx @ 2:7")
add_levelc_invalid_compile_test(levelc_arg_missing_positions
        rexx_src/levelc_arg_missing_positions.rexx
        "levelc_arg_missing_positions\\.rexx @ 2:7 - #RXC-LC-38\\.1"
        "levelc_arg_missing_positions\\.rexx @ 3:7 - #RXC-LC-38\\.1"
        "levelc_arg_missing_positions\\.rexx @ 4:7 - #RXC-LC-38\\.1"
        "levelc_arg_missing_positions\\.rexx @ 5:8 - #RXC-LC-38\\.1")
add_levelc_invalid_compile_test(levelc_procedure_nested_invalid
        rexx_src/levelc_procedure_nested_invalid.rexx
        "RXC-LC-17\\.1" "levelc_procedure_nested_invalid\\.rexx @ 5:11")
add_levelc_invalid_compile_test(levelc_procedure_bad_forms
        rexx_src/levelc_procedure_bad_forms.rexx
        "levelc_procedure_bad_forms\\.rexx @ 4:14 - #RXC-LC-20\\.1"
        "levelc_procedure_bad_forms\\.rexx @ 6:22 - #RXC-LC-20\\.1"
        "levelc_procedure_bad_forms\\.rexx @ 8:21 - #RXC-LC-20\\.1"
        "levelc_procedure_bad_forms\\.rexx @ 10:14 - #RXC-LC-25\\.17")
add_levelc_invalid_compile_test(levelc_call_expression_malformed
        rexx_src/levelc_call_expression_malformed.rexx
        "RXC-LC-35\\.1" "levelc_call_expression_malformed\\.rexx @ 2:15")
add_levelc_invalid_compile_test(levelc_call_surplus_policy_tails
        rexx_src/levelc_call_surplus_policy_tails.rexx
        "levelc_call_surplus_policy_tails\\.rexx @ 2:16 - #RXC-LC-21\\.1"
        "levelc_call_surplus_policy_tails\\.rexx @ 3:24 - #RXC-LC-19\\.3"
        "levelc_call_surplus_policy_tails\\.rexx @ 4:15 - #RXC-LC-21\\.1"
        "levelc_call_surplus_policy_tails\\.rexx @ 5:28 - #RXC-LC-19\\.3")
add_levelc_invalid_compile_test(levelc_call_diagnostic_matrix
        rexx_src/levelc_call_diagnostic_matrix.rexx
        "levelc_call_diagnostic_matrix\\.rexx @ 2:1 - #RXC-LC-19\\.2.*end-of-clause"
        "levelc_call_diagnostic_matrix\\.rexx @ 3:6 - #RXC-LC-19\\.2"
        "levelc_call_diagnostic_matrix\\.rexx @ 4:8 - #RXC-LC-25\\.1"
        "levelc_call_diagnostic_matrix\\.rexx @ 5:9 - #RXC-LC-25\\.1"
        "levelc_call_diagnostic_matrix\\.rexx @ 6:9 - #RXC-LC-25\\.2"
        "levelc_call_diagnostic_matrix\\.rexx @ 7:10 - #RXC-LC-25\\.2"
        "levelc_call_diagnostic_matrix\\.rexx @ 8:15 - #RXC-LC-19\\.3"
        "levelc_call_diagnostic_matrix\\.rexx @ 9:20 - #RXC-LC-19\\.3"
        "levelc_call_diagnostic_matrix\\.rexx @ 10:20 - #RXC-LC-19\\.3"
        "levelc_call_diagnostic_matrix\\.rexx @ 11:20 - #RXC-LC-19\\.3")
add_levelc_invalid_compile_test(levelc_binary_grouping_invalid
        rexx_src/levelc_binary_grouping_invalid.rexx
        "levelc_binary_grouping_invalid\\.rexx @ 2:6 - #RXC-LC-15\\.2"
        "levelc_binary_grouping_invalid\\.rexx @ 3:5 - #RXC-LC-15\\.2")
add_levelc_invalid_compile_test(levelc_if_missing_condition
        rexx_src/levelc_if_missing_condition.rexx
        "RXC-LC-35\\.1" "1 error")
add_levelc_invalid_compile_test(levelc_if_stray_else
        rexx_src/levelc_if_stray_else.rexx
        "RXC-LC-8\\.2" "1 error")
add_levelc_invalid_compile_test(levelc_if_nested_missing_then
        rexx_src/levelc_if_nested_missing_then.rexx
        "RXC-LC-18\\.1" "1 error")
add_levelc_invalid_compile_test(levelc_if_then_missing_arm
        rexx_src/levelc_if_then_missing_arm.rexx
        "RXC-LC-14\\.3" "1 error")
add_levelc_invalid_compile_test(levelc_if_then_label_missing_arm
        rexx_src/levelc_if_then_label_missing_arm.rexx
        "RXC-LC-14\\.3" "1 error")
add_levelc_invalid_compile_test(levelc_if_then_separator_missing_arm
        rexx_src/levelc_if_then_separator_missing_arm.rexx
        "RXC-LC-14\\.3" "1 error")
add_levelc_invalid_compile_test(levelc_if_else_missing_arm
        rexx_src/levelc_if_else_missing_arm.rexx
        "RXC-LC-14\\.4" "1 error")

foreach(kind IN ITEMS to by for)
    add_levelc_invalid_compile_test(levelc_do_duplicate_${kind}
            rexx_src/levelc_do_duplicate_${kind}.rexx
            "RXC-LC-27\\.1")
endforeach()
foreach(kind IN ITEMS to by for while until)
    add_levelc_invalid_compile_test(levelc_do_missing_${kind}
            rexx_src/levelc_do_missing_${kind}.rexx
            "RXC-LC-35\\.1")
endforeach()
add_levelc_invalid_compile_test(levelc_do_missing_end
        rexx_src/levelc_do_missing_end.rexx
        "RXC-LC-14\\.1")
add_levelc_invalid_compile_test(levelc_do_group_named_end
        rexx_src/levelc_do_group_named_end.rexx
        "RXC-LC-10\\.3")

add_levelc_invalid_compile_test(levelc_compile_invalid_expression_trailing_operator
        rexx_src/levelc_expression_bad_trailing_operator.rexx
        "RXC-LC-35\\.1")

add_levelc_invalid_compile_test(levelc_compile_invalid_do_end_symbol
        rexx_src/levelc_do_end_bad_symbol.rexx
        "RXC-LC-10\\.2")

add_levelc_invalid_compile_test(levelc_compile_invalid_select_missing_when
        rexx_src/levelc_select_missing_when.rexx
        "RXC-LC-7\\.1")
add_levelc_invalid_compile_test(levelc_select_only_otherwise
        rexx_src/levelc_select_only_otherwise.rexx
        "RXC-LC-7\\.2" "1 error")
add_levelc_invalid_compile_test(levelc_select_missing_condition
        rexx_src/levelc_select_missing_condition.rexx
        "RXC-LC-35\\.1" "1 error")
add_levelc_invalid_compile_test(levelc_select_missing_arm
        rexx_src/levelc_select_missing_arm.rexx
        "RXC-LC-14\\.3" "1 error")
add_levelc_invalid_compile_test(levelc_select_separator_missing_arm
        rexx_src/levelc_select_separator_missing_arm.rexx
        "RXC-LC-14\\.3" "1 error")
add_levelc_invalid_compile_test(levelc_select_stray_when
        rexx_src/levelc_select_stray_when.rexx
        "RXC-LC-9\\.1" "1 error")
add_levelc_invalid_compile_test(levelc_select_stray_otherwise
        rexx_src/levelc_select_stray_otherwise.rexx
        "RXC-LC-9\\.2" "1 error")
add_levelc_invalid_compile_test(levelc_select_when_after_otherwise
        rexx_src/levelc_select_when_after_otherwise.rexx
        "RXC-LC-9\\.1" "1 error")
add_levelc_invalid_compile_test(levelc_select_duplicate_otherwise
        rexx_src/levelc_select_duplicate_otherwise.rexx
        "RXC-LC-9\\.2" "1 error")
add_levelc_invalid_compile_test(levelc_select_named_end
        rexx_src/levelc_select_end_symbol.rexx
        "RXC-LC-10\\.4" "1 error")
add_levelc_invalid_compile_test(levelc_select_missing_end
        rexx_src/levelc_select_missing_end.rexx
        "RXC-LC-14\\.2" "1 error")
add_levelc_invalid_compile_test(levelc_select_missing_then
        rexx_src/levelc_when_missing_then.rexx
        "RXC-LC-18\\.2" "1 error")
add_levelc_invalid_compile_test(levelc_select_bad_header
        rexx_src/levelc_select_bad_header.rexx
        "RXC-LC-21\\.1" "@ 2:8" "1 error")
add_levelc_invalid_compile_test(levelc_select_same_clause_when
        rexx_src/levelc_select_same_clause_when.rexx
        "RXC-LC-21\\.1" "@ 2:8" "1 error")
foreach(position IN ITEMS before between after)
    add_levelc_invalid_compile_test(levelc_select_unexpected_${position}_when
            rexx_src/levelc_select_unexpected_${position}_when.rexx
            "RXC-LC-7\\.2" "1 error")
endforeach()

add_levelc_invalid_compile_test(levelc_drop_invalid
        rexx_src/levelc_drop_invalid.rexx
        "RXC-LC-20\\.1"
        "@ 2:1"
        "@ 3:7"
        "@ 4:6")

add_levelc_invalid_compile_test(levelc_assignment_invalid
        rexx_src/levelc_assignment_invalid.rexx
        "RXC-LC-31\\.1"
        "@ 2:1"
        "@ 3:1")

add_levelc_invalid_compile_test(levelc_nop_invalid
        rexx_src/levelc_nop_invalid.rexx
        "RXC-LC-21\\.1"
        "@ 2:5"
        "@ 3:5")

add_levelc_invalid_compile_test(levelc_options_numeric_common
        rexx_src/levelc_options_numeric_common.rexx
        "INCOMPATIBLE_OPTIONS"
        "numeric_common")
add_levelc_invalid_compile_test(levelc_options_comments_slash
        rexx_src/levelc_options_comments_slash.rexx
        "INCOMPATIBLE_OPTIONS"
        "comments_slash")
add_levelc_invalid_compile_test(levelc_options_conflicting_comments
        rexx_src/levelc_options_conflicting_comments.rexx
        "INCOMPATIBLE_OPTIONS"
        "comments_nohash")
add_levelc_invalid_compile_test(levelc_options_invalid_expression
        rexx_src/levelc_options_invalid_expression.rexx
        "RXC-LC-35\\.1"
        "@ 2:12")

add_levelc_invalid_compile_test(levelc_slice13_named_leave_invalid
        rexx_src/levelc_slice13_named_leave_invalid.rexx
        "RXC-LC-28\\.3")
add_levelc_invalid_compile_test(levelc_leave_named_outside
        rexx_src/levelc_leave_named_outside.rexx
        "RXC-LC-28\\.1" "@ 2:1" "1 error")
add_levelc_invalid_compile_test(levelc_leave_dead_default
        rexx_src/levelc_leave_dead_default.rexx
        "RXC-LC-28\\.1")
add_levelc_invalid_compile_test(levelc_leave_inactive_caller
        rexx_src/levelc_leave_inactive_caller.rexx
        "RXC-LC-28\\.1" "@ 7:1" "1 error")
add_levelc_invalid_compile_test(levelc_leave_malformed_target
        rexx_src/levelc_leave_malformed_target.rexx
        "RXC-LC-20\\.1" "@ 3:9" "1 error")
add_levelc_invalid_compile_test(levelc_leave_extra_target
        rexx_src/levelc_leave_extra_target.rexx
        "RXC-LC-21\\.1" "@ 3:11" "1 error")
add_levelc_invalid_compile_test(levelc_strictc_late_option
        rexx_src/levelc_strictc_late_option.rexx
        "RXC-LC-28\\.1")
add_levelc_invalid_compile_test(levelc_strictc_leave_extra_target
        rexx_src/levelc_strictc_leave_extra_target.rexx
        "RXC-LC-21\\.1")
add_levelc_invalid_compile_test(levelc_strictc_leave_malformed_target
        rexx_src/levelc_strictc_leave_malformed_target.rexx
        "RXC-LC-20\\.1")
add_levelc_invalid_compile_test(levelc_slice29_named_leave_invalid
        rexx_src/levelc_slice29_named_leave_invalid.rexx
        "RXC-LC-28\\.3")
add_levelc_invalid_compile_test(levelc_slice29_named_iterate_invalid
        rexx_src/levelc_slice29_named_iterate_invalid.rexx
        "RXC-LC-28\\.4")
add_levelc_invalid_compile_test(levelc_iterate_dead_default
        rexx_src/levelc_iterate_dead_default.rexx
        "RXC-LC-28\\.2")
add_levelc_invalid_compile_test(levelc_iterate_named_outside
        rexx_src/levelc_iterate_named_outside.rexx
        "RXC-LC-28\\.2")
add_levelc_invalid_compile_test(levelc_iterate_inactive_caller
        rexx_src/levelc_iterate_inactive_caller.rexx
        "RXC-LC-28\\.2")
add_levelc_invalid_compile_test(levelc_iterate_extra_target
        rexx_src/levelc_iterate_extra_target.rexx
        "RXC-LC-21\\.1")
add_levelc_invalid_compile_test(levelc_iterate_malformed_target
        rexx_src/levelc_iterate_malformed_target.rexx
        "RXC-LC-20\\.1")
add_levelc_invalid_compile_test(levelc_strictc_iterate_extra_target
        rexx_src/levelc_strictc_iterate_extra_target.rexx
        "RXC-LC-21\\.1")
add_levelc_invalid_compile_test(levelc_strictc_iterate_malformed_target
        rexx_src/levelc_strictc_iterate_malformed_target.rexx
        "RXC-LC-20\\.1")

function(add_levelc_runtime_slice_test NAME SOURCE_FILE EXPECTED_OUTPUT)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${ARGN} -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxc_out\n"
           "    ERROR_VARIABLE rxc_err\n"
           "    RESULT_VARIABLE rxc_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
           "if(NOT rxc_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n")
    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}.rxbin\" \"${NAME}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxas_out\n"
           "    ERROR_VARIABLE rxas_err\n"
           "    RESULT_VARIABLE rxas_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{full_out}${RXAS_DIAG_DOLLAR}{rxas_out}${RXAS_DIAG_DOLLAR}{rxas_err}\")\n"
           "if(NOT rxas_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxas failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n")
    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${NAME}.rxbin\" \"${CMAKE_BINARY_DIR}/bin/library\" \"${CMAKE_BINARY_DIR}/bin/classlib\" \"${CMAKE_BINARY_DIR}/bin/rxfnsc\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxvm_out\n"
           "    ERROR_VARIABLE rxvm_err\n"
           "    RESULT_VARIABLE rxvm_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{full_out}${RXAS_DIAG_DOLLAR}{rxvm_out}${RXAS_DIAG_DOLLAR}{rxvm_err}\")\n"
           "string(REPLACE \"\\r\\n\" \"\\n\" rxvm_out \"${RXAS_DIAG_DOLLAR}{rxvm_out}\")\n"
           "if(NOT rxvm_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxvm failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n"
           "if(NOT rxvm_out STREQUAL \"${EXPECTED_OUTPUT}\")\n"
           "  message(FATAL_ERROR \"Unexpected runtime output for ${NAME}.\\nExpected:\\n${EXPECTED_OUTPUT}\\nActual:\\n${RXAS_DIAG_DOLLAR}{rxvm_out}\\nFull output:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

function(add_runtime_bytes_test NAME SOURCE_FILE EXPECTED_HEX LEVEL)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")
    string(APPEND SCRIPT_CONTENT
           "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${ARGN} -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" RESULT_VARIABLE result ERROR_VARIABLE error)\n"
           "if(NOT result EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: ${RXAS_DIAG_DOLLAR}{error}\")\nendif()\n"
           "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}.rxbin\" \"${NAME}\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" RESULT_VARIABLE result ERROR_VARIABLE error)\n"
           "if(NOT result EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: ${RXAS_DIAG_DOLLAR}{error}\")\nendif()\n"
           "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${NAME}.rxbin\" \"${CMAKE_BINARY_DIR}/bin/library\" \"${CMAKE_BINARY_DIR}/bin/classlib\" \"${CMAKE_BINARY_DIR}/bin/rxfnsc\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" OUTPUT_FILE \"${NAME}.out\" RESULT_VARIABLE result ERROR_VARIABLE error)\n"
           "if(NOT result EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: ${RXAS_DIAG_DOLLAR}{error}\")\nendif()\n"
           "file(READ \"${CMAKE_CURRENT_BINARY_DIR}/${NAME}.out\" actual HEX)\n"
           "if(NOT actual STREQUAL \"${EXPECTED_HEX}\")\n  message(FATAL_ERROR \"Expected ${EXPECTED_HEX}; got ${RXAS_DIAG_DOLLAR}{actual}\")\nendif()\n")
    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME} COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}")
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;${LEVEL}")
endfunction()

function(add_levelc_runtime_syntax_signal_test NAME SOURCE_FILE EXPECTED_CODE EXPECTED_OUTPUT EXPECTED_SOURCE_LOCATION)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")
    string(APPEND SCRIPT_CONTENT
           "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" ${ARGN} -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" RESULT_VARIABLE status ERROR_VARIABLE diagnostic)\n"
           "if(NOT status EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: ${RXAS_DIAG_DOLLAR}{diagnostic}\")\nendif()\n"
           "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}.rxbin\" \"${NAME}\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" RESULT_VARIABLE status ERROR_VARIABLE diagnostic)\n"
           "if(NOT status EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: ${RXAS_DIAG_DOLLAR}{diagnostic}\")\nendif()\n"
           "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${NAME}.rxbin\" \"${CMAKE_BINARY_DIR}/bin/library\" \"${CMAKE_BINARY_DIR}/bin/classlib\" \"${CMAKE_BINARY_DIR}/bin/rxfnsc\"\n"
           "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" OUTPUT_VARIABLE output ERROR_VARIABLE diagnostic RESULT_VARIABLE status)\n"
           "string(FIND \"${RXAS_DIAG_DOLLAR}{diagnostic}\" \"source: ${EXPECTED_SOURCE_LOCATION}\" source_location)\n"
           "if(status EQUAL 0 OR NOT diagnostic MATCHES \"CLASSIC_SYNTAX\" OR NOT diagnostic MATCHES \"RXC-LC-${EXPECTED_CODE}\" OR NOT output STREQUAL \"${EXPECTED_OUTPUT}\" OR source_location EQUAL -1)\n"
           "  message(FATAL_ERROR \"Expected Classic SYNTAX ${EXPECTED_CODE}, authored source ${EXPECTED_SOURCE_LOCATION}, and exact prior output; status=${RXAS_DIAG_DOLLAR}{status}, output=${RXAS_DIAG_DOLLAR}{output}, diagnostic=${RXAS_DIAG_DOLLAR}{diagnostic}\")\nendif()\n")
    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME} COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}")
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

add_levelc_runtime_slice_test(levelc_bare_stem_read
        rexx_src/levelc_bare_stem_read.rexx
        "ITEMS.\n")
add_levelc_runtime_slice_test(levelc_options_instruction
        rexx_src/levelc_options_instruction.rexx "6\n1\n")
add_levelc_runtime_slice_test(levelc_options_instruction_noopt
        rexx_src/levelc_options_instruction.rexx "6\n1\n" -n)
add_levelc_runtime_slice_test(levelc_options_dynamic_first
        rexx_src/levelc_options_dynamic_first.rexx "1\n" --level levelc)
add_levelc_runtime_slice_test(levelc_options_dynamic_first_noopt
        rexx_src/levelc_options_dynamic_first.rexx "1\n" --level levelc -n)
add_levelc_runtime_slice_test(levelc_options_source
        rexx_src/levelc_options_source.rexx "1\n" --level levelb)
add_levelc_runtime_slice_test(levelc_options_source_noopt
        rexx_src/levelc_options_source.rexx "1\n" --level levelb -n)
add_runtime_bytes_test(levelc_say_bytes
        rexx_src/levelc_say_bytes.rexx
        "410042c280c3bf0ae6bca2f09f99820ac280c3bf0ac383c2a90ac3a90a0a" levelc)
add_runtime_bytes_test(levelc_say_bytes_noopt
        rexx_src/levelc_say_bytes.rexx
        "410042c280c3bf0ae6bca2f09f99820ac280c3bf0ac383c2a90ac3a90a0a" levelc -n)
add_runtime_bytes_test(levelc_arg_exact_bytes
        rexx_src/levelc_arg_exact_bytes.rexx
        "6100620a4100420a" levelc)
add_runtime_bytes_test(levelc_arg_exact_bytes_noopt
        rexx_src/levelc_arg_exact_bytes.rexx
        "6100620a4100420a" levelc -n)
set_tests_properties(levelc_arg_exact_bytes levelc_arg_exact_bytes_noopt PROPERTIES
        CREXX_PREP_TARGETS "rxc;rxas;rxvm;library;classlib;rxfnsc")
add_runtime_bytes_test(levelg_say_bytes
        rexx_src/levelg_say_bytes.crexx
        "4100420ac3a90a" levelg)
add_runtime_bytes_test(levelg_say_bytes_noopt
        rexx_src/levelg_say_bytes.crexx
        "4100420ac3a90a" levelg -n)
add_levelc_runtime_syntax_signal_test(levelc_bif_missing_required
        rexx_src/levelc_bif_missing_required.rexx "40.3" ""
        "levelc_bif_missing_required.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_latin1_out_of_range
        rexx_src/levelc_latin1_out_of_range.rexx "23.1" "before\n"
        "levelc_latin1_out_of_range.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_latin1_out_of_range_noopt
        rexx_src/levelc_latin1_out_of_range.rexx "23.1" "before\n"
        "levelc_latin1_out_of_range.rexx:3:" -n)
add_levelc_runtime_syntax_signal_test(levelc_bif_syntax_signal
        rexx_src/levelc_bif_syntax_signal.rexx "40.14" "before\n"
        "levelc_bif_syntax_signal.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_bif_syntax_signal_noopt
        rexx_src/levelc_bif_syntax_signal.rexx "40.14" "before\n"
        "levelc_bif_syntax_signal.rexx:3:" -n)
add_levelc_runtime_syntax_signal_test(levelc_bif_syntax_local
        rexx_src/levelc_bif_syntax_local.rexx "40.14" "before\n"
        "levelc_bif_syntax_local.rexx:8:")
add_levelc_runtime_syntax_signal_test(levelc_bif_syntax_local_noopt
        rexx_src/levelc_bif_syntax_local.rexx "40.14" "before\n"
        "levelc_bif_syntax_local.rexx:8:" -n)
add_levelc_runtime_syntax_signal_test(levelc_arg_bif_bad_index
        rexx_src/levelc_arg_bif_bad_index.rexx "40.14" "before\n"
        "levelc_arg_bif_bad_index.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_arg_bif_bad_option
        rexx_src/levelc_arg_bif_bad_option.rexx "40.28" "before\n"
        "levelc_arg_bif_bad_option.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_arg_bad_dynamic_position
        rexx_src/levelc_arg_bad_dynamic_position.rexx "26.4" "before\n"
        "levelc_arg_bad_dynamic_position.rexx:9:")
add_levelc_runtime_syntax_signal_test(levelc_arg_bad_dynamic_position_noopt
        rexx_src/levelc_arg_bad_dynamic_position.rexx "26.4" "before\n"
        "levelc_arg_bad_dynamic_position.rexx:9:" -n)
add_levelc_runtime_syntax_signal_test(levelc_parse_bad_dynamic_position
        rexx_src/levelc_parse_bad_dynamic_position.rexx "26.4" "before\n"
        "levelc_parse_bad_dynamic_position.rexx:4:")
add_levelc_runtime_syntax_signal_test(levelc_parse_bad_dynamic_position_noopt
        rexx_src/levelc_parse_bad_dynamic_position.rexx "26.4" "before\n"
        "levelc_parse_bad_dynamic_position.rexx:4:" -n)
add_levelc_runtime_slice_test(levelc_bif_inventory
        rexx_src/levelc_bif_inventory.rexx "inventory\n")
add_levelc_runtime_slice_test(levelc_latin1_ordinals
        rexx_src/levelc_latin1_ordinals.rexx "0\nFF\n255\nFF\n16\nFEFF00\n1\n")
add_levelc_runtime_slice_test(levelc_latin1_ordinals_noopt
        rexx_src/levelc_latin1_ordinals.rexx "0\nFF\n255\nFF\n16\nFEFF00\n1\n" -n)
add_levelc_runtime_slice_test(levelc_unicode_character_bifs
        rexx_src/levelc_unicode_character_bifs.rexx
        "length=4\nsubstr=é🙂\npos=3\nlastpos=3\nreverse=B🙂éA\nleft=Aé\nright=🙂B\ndelstr=AB\ninsert=A漢B\noverlay=A漢B\nchangestr=漢🙂漢\ncountstr=2\ncompare=2\nverify=2\nabbrev=1\nwords=2\nword=🙂\nwordindex=3\nwordlength=1\nwordpos=2\nsubword=🙂\ndelword=🙂\nspace=é|🙂\nstrip=é\ntranslate=É🙂\nupper=É🙂\nlower=é🙂\ncopies=é🙂é🙂\ncenter=.é.\nleftpad=é.\nrightpad=.é\n")
add_levelc_runtime_slice_test(levelc_unicode_character_bifs_noopt
        rexx_src/levelc_unicode_character_bifs.rexx
        "length=4\nsubstr=é🙂\npos=3\nlastpos=3\nreverse=B🙂éA\nleft=Aé\nright=🙂B\ndelstr=AB\ninsert=A漢B\noverlay=A漢B\nchangestr=漢🙂漢\ncountstr=2\ncompare=2\nverify=2\nabbrev=1\nwords=2\nword=🙂\nwordindex=3\nwordlength=1\nwordpos=2\nsubword=🙂\ndelword=🙂\nspace=é|🙂\nstrip=é\ntranslate=É🙂\nupper=É🙂\nlower=é🙂\ncopies=é🙂é🙂\ncenter=.é.\nleftpad=é.\nrightpad=.é\n" -n)
add_levelc_runtime_slice_test(levelc_translate_implicit
        rexx_src/levelc_translate_implicit.rexx
        "mapped=AB.Ā🙂\nempty=   Ā🙂\noutside=Ā🙂\n")
add_levelc_runtime_slice_test(levelc_translate_implicit_noopt
        rexx_src/levelc_translate_implicit.rexx
        "mapped=AB.Ā🙂\nempty=   Ā🙂\noutside=Ā🙂\n" -n)
add_levelc_runtime_slice_test(levelc_literal_ordinals
        rexx_src/levelc_literal_ordinals.rexx
        "FF\nFF\nC3A9\n2\n00FF\n0F\n07\nC3A9\n80\n80\nAB\nAB\n")
add_levelc_runtime_slice_test(levelc_literal_ordinals_noopt
        rexx_src/levelc_literal_ordinals.rexx
        "FF\nFF\nC3A9\n2\n00FF\n0F\n07\nC3A9\n80\n80\nAB\nAB\n" -n)
add_levelc_runtime_slice_test(levelc_say_bif_family
        rexx_src/levelc_say_bif_family.rexx
        "length=2\nnested=3\nsubstr=bcd\nomitted=bc\npos=2\nword=blue\nvalue=MiXeD\nlocal=iXeD\nchange=MiXeD\nafter=next\nsymbol=VAR\nx2d=255\ncenter=-A-\nleft=ab..\nmin=2\nmax=4\n")
add_levelc_runtime_slice_test(levelc_say_bif_family_noopt
        rexx_src/levelc_say_bif_family.rexx
        "length=2\nnested=3\nsubstr=bcd\nomitted=bc\npos=2\nword=blue\nvalue=MiXeD\nlocal=iXeD\nchange=MiXeD\nafter=next\nsymbol=VAR\nx2d=255\ncenter=-A-\nleft=ab..\nmin=2\nmax=4\n" -n)
add_levelc_runtime_slice_test(levelc_bif_shadow
        rexx_src/levelc_bif_shadow.rexx "local\n")
add_levelc_runtime_slice_test(levelc_bif_shadow_noopt
        rexx_src/levelc_bif_shadow.rexx "local\n" -n)
add_levelc_runtime_slice_test(levelc_bif_config_lifecycle
        rexx_src/levelc_bif_config_lifecycle.rexx "1\n1\n1\n1\n1\n1\n")
add_levelc_runtime_slice_test(levelc_bif_config_lifecycle_noopt
        rexx_src/levelc_bif_config_lifecycle.rexx "1\n1\n1\n1\n1\n1\n" -n)

function(add_levelc_lowered_tree_shape_test NAME SOURCE_FILE)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -d1 -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxc_out\n"
           "    ERROR_VARIABLE rxc_err\n"
           "    RESULT_VARIABLE rxc_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
           "if(NOT rxc_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc debug probe failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n"
           "string(FIND \"${RXAS_DIAG_DOLLAR}{full_out}\" \"STAGE_LEVELC_LOWERED\" lowered_pos)\n"
           "if(lowered_pos EQUAL -1)\n"
           "  message(FATAL_ERROR \"Missing STAGE_LEVELC_LOWERED debug stage for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n"
           "string(LENGTH \"STAGE_LEVELC_LOWERED\" marker_len)\n"
           "math(EXPR lowered_start \"${RXAS_DIAG_DOLLAR}{lowered_pos} + ${RXAS_DIAG_DOLLAR}{marker_len}\")\n"
           "string(SUBSTRING \"${RXAS_DIAG_DOLLAR}{full_out}\" ${RXAS_DIAG_DOLLAR}{lowered_start} -1 lowered)\n"
           "if(lowered MATCHES \"LEVELC_\")\n"
           "  message(FATAL_ERROR \"Lowered tree still contains Level C-only nodes for ${NAME}.\\nLowered debug output:\\n${RXAS_DIAG_DOLLAR}{lowered}\")\n"
           "endif()\n"
           "if(lowered MATCHES \"\\\\(INSTRUCTIONS\\\\)\")\n"
           "  message(FATAL_ERROR \"Lowered tree kept the builder-only INSTRUCTIONS wrapper for ${NAME}.\\nLowered debug output:\\n${RXAS_DIAG_DOLLAR}{lowered}\")\n"
           "endif()\n")

    foreach(REQUIRED_PATTERN IN LISTS ARGN)
        string(APPEND SCRIPT_CONTENT
               "if(NOT lowered MATCHES \"${REQUIRED_PATTERN}\")\n"
               "  message(FATAL_ERROR \"Lowered tree for ${NAME} is missing required pattern ${REQUIRED_PATTERN}.\\nLowered debug output:\\n${RXAS_DIAG_DOLLAR}{lowered}\")\n"
               "endif()\n")
    endforeach()

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

function(add_levelc_no_unused_generated_import_warning_test NAME SOURCE_FILE)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")

    string(APPEND SCRIPT_CONTENT
           "execute_process(\n"
           "    COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\"\n"
           "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
           "    OUTPUT_VARIABLE rxc_out\n"
           "    ERROR_VARIABLE rxc_err\n"
           "    RESULT_VARIABLE rxc_res\n"
           ")\n"
           "set(full_out \"${RXAS_DIAG_DOLLAR}{rxc_out}${RXAS_DIAG_DOLLAR}{rxc_err}\")\n"
           "if(NOT rxc_res EQUAL 0)\n"
           "  message(FATAL_ERROR \"rxc failed for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n"
           "if(full_out MATCHES \"UNUSED_IMPORT\")\n"
           "  message(FATAL_ERROR \"Generated Level C imports produced an unused-import warning for ${NAME}.\\nOutput:\\n${RXAS_DIAG_DOLLAR}{full_out}\")\n"
           "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")
    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

add_levelc_runtime_slice_test(levelc_slice1_target_shape
        rexx_src/levelc_slice1_target_shape.crexx
        "3\n")

add_levelc_runtime_slice_test(levelc_slice1_pool_say
        rexx_src/levelc_slice1_pool_say.rexx
        "3\n")

add_levelc_no_unused_generated_import_warning_test(levelc_generated_imports_no_unused_warning
        rexx_src/levelc_slice1_pool_say.rexx)
add_levelc_no_unused_generated_import_warning_test(levelc_slice40_parse_upper_imports
        rexx_src/levelc_slice40_parse_upper_import.rexx)

add_levelc_runtime_slice_test(levelc_slice2_procedure_expose_target_shape
        rexx_src/levelc_slice2_procedure_expose_target_shape.crexx
        "3\n")

add_levelc_runtime_slice_test(levelc_slice2_procedure_expose
        rexx_src/levelc_slice2_procedure_expose.rexx
        "3\n")

add_levelc_runtime_slice_test(levelc_frame_fallthrough
        rexx_src/levelc_frame_fallthrough.rexx
        "first\nalpha\nbeta\n")
add_levelc_runtime_slice_test(levelc_frame_fallthrough_noopt
        rexx_src/levelc_frame_fallthrough.rexx
        "first\nalpha\nbeta\n" -n)
add_levelc_runtime_slice_test(levelc_frame_shared_pool
        rexx_src/levelc_frame_shared_pool.rexx
        "changed-inner\n")
add_levelc_runtime_slice_test(levelc_frame_shared_pool_noopt
        rexx_src/levelc_frame_shared_pool.rexx
        "changed-inner\n" -n)
add_levelc_runtime_slice_test(levelc_frame_called_fallthrough
        rexx_src/levelc_frame_called_fallthrough.rexx
        "first-second\n")
add_levelc_runtime_slice_test(levelc_frame_called_fallthrough_noopt
        rexx_src/levelc_frame_called_fallthrough.rexx
        "first-second\n" -n)
add_levelc_runtime_slice_test(levelc_frame_private_pool
        rexx_src/levelc_frame_private_pool.rexx
        "private\nouter\n")
add_levelc_runtime_slice_test(levelc_frame_private_pool_noopt
        rexx_src/levelc_frame_private_pool.rexx
        "private\nouter\n" -n)
add_levelc_runtime_slice_test(levelc_frame_recursive_call
        rexx_src/levelc_frame_recursive_call.rexx
        "3\n")
add_levelc_runtime_slice_test(levelc_frame_recursive_call_noopt
        rexx_src/levelc_frame_recursive_call.rexx
        "3\n" -n)

add_levelc_runtime_slice_test(levelc_frame_empty_label_procedure
        rexx_src/levelc_frame_empty_label_procedure.rexx
        "private\n")
add_levelc_runtime_slice_test(levelc_frame_empty_label_procedure_noopt
        rexx_src/levelc_frame_empty_label_procedure.rexx
        "private\n" -n)
add_levelc_runtime_slice_test(levelc_signal_direct
        rexx_src/levelc_signal_direct.rexx
        "before\ndone=3\n")
add_levelc_runtime_slice_test(levelc_signal_direct_noopt
        rexx_src/levelc_signal_direct.rexx
        "before\ndone=3\n" -n)
add_levelc_runtime_slice_test(levelc_signal_if_reached
        rexx_src/levelc_signal_if_reached.rexx
        "jump=3\n")
add_levelc_runtime_slice_test(levelc_signal_if_reached_noopt
        rexx_src/levelc_signal_if_reached.rexx
        "jump=3\n" -n)
add_levelc_runtime_slice_test(levelc_signal_quoted_local
        rexx_src/levelc_signal_quoted_local.rexx
        "start\nlater=8\nmain\n")
add_levelc_runtime_slice_test(levelc_signal_quoted_local_noopt
        rexx_src/levelc_signal_quoted_local.rexx
        "start\nlater=8\nmain\n" -n)
add_levelc_runtime_slice_test(levelc_signal_crossed_control
        rexx_src/levelc_signal_crossed_control.rexx
        "branch=1|1|1|7\ncaught=40|15|SYNTAX|OFF\nlater=1\nlater=2\n")
add_levelc_runtime_slice_test(levelc_signal_crossed_control_noopt
        rexx_src/levelc_signal_crossed_control.rexx
        "branch=1|1|1|7\ncaught=40|15|SYNTAX|OFF\nlater=1\nlater=2\n" -n)
add_levelc_runtime_slice_test(levelc_signal_unicode_labels
        rexx_src/levelc_signal_unicode_labels.rexx
        "direct=2\ncall\nback\nvalue=8\n")
add_levelc_runtime_slice_test(levelc_signal_unicode_labels_noopt
        rexx_src/levelc_signal_unicode_labels.rexx
        "direct=2\ncall\nback\nvalue=8\n" -n)
add_levelc_runtime_slice_test(levelc_signal_duplicate_labels
        rexx_src/levelc_signal_duplicate_labels.rexx
        "first\nsecond\nfirst\nsecond\nmain\nfirst\nsecond\n")
add_levelc_runtime_slice_test(levelc_signal_duplicate_labels_noopt
        rexx_src/levelc_signal_duplicate_labels.rexx
        "first\nsecond\nfirst\nsecond\nmain\nfirst\nsecond\n" -n)
add_levelc_runtime_syntax_signal_test(levelc_signal_missing
        rexx_src/levelc_signal_missing.rexx "16.1" "before\n"
        "levelc_signal_missing.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_signal_missing_noopt
        rexx_src/levelc_signal_missing.rexx "16.1" "before\n"
        "levelc_signal_missing.rexx:3:" -n)
add_levelc_runtime_slice_test(levelc_signal_value_once
        rexx_src/levelc_signal_value_once.rexx
        "count=1 sigl=3\n")
add_levelc_runtime_slice_test(levelc_signal_value_once_noopt
        rexx_src/levelc_signal_value_once.rexx
        "count=1 sigl=3\n" -n)
add_levelc_runtime_slice_test(levelc_signal_value_second_label
        rexx_src/levelc_signal_value_second_label.rexx
        "before\nbeta=3\n")
add_levelc_runtime_slice_test(levelc_signal_value_second_label_noopt
        rexx_src/levelc_signal_value_second_label.rexx
        "before\nbeta=3\n" -n)
add_levelc_runtime_slice_test(levelc_signal_implicit_value
        rexx_src/levelc_signal_implicit_value.rexx
        "paren=4\n")
add_levelc_runtime_slice_test(levelc_signal_implicit_value_noopt
        rexx_src/levelc_signal_implicit_value.rexx
        "paren=4\n" -n)
foreach(form plus minus not)
    add_levelc_runtime_syntax_signal_test(levelc_signal_implicit_value_${form}_missing
            rexx_src/levelc_signal_implicit_value_${form}_missing.rexx "16.1" "before\n"
            "levelc_signal_implicit_value_${form}_missing.rexx:3:")
    add_levelc_runtime_syntax_signal_test(levelc_signal_implicit_value_${form}_missing_noopt
            rexx_src/levelc_signal_implicit_value_${form}_missing.rexx "16.1" "before\n"
            "levelc_signal_implicit_value_${form}_missing.rexx:3:" -n)
endforeach()
add_levelc_invalid_compile_test(levelc_signal_implicit_value_missing_expr
        rexx_src/levelc_signal_implicit_value_missing_expr.rexx
        "RXC-LC-35\\.1" "@ 2:8" "1 error")
add_levelc_runtime_slice_test(levelc_signal_on_syntax
        rexx_src/levelc_signal_on_syntax.rexx
        "before\ncaught=40|4\n")
add_levelc_runtime_slice_test(levelc_signal_on_syntax_noopt
        rexx_src/levelc_signal_on_syntax.rexx
        "before\ncaught=40|4\n" -n)
add_levelc_runtime_slice_test(levelc_signal_quoted_handler
        rexx_src/levelc_signal_quoted_handler.rexx
        "caught=40|3\n")
add_levelc_runtime_slice_test(levelc_signal_quoted_handler_noopt
        rexx_src/levelc_signal_quoted_handler.rexx
        "caught=40|3\n" -n)
add_levelc_runtime_slice_test(levelc_signal_on_default
        rexx_src/levelc_signal_on_default.rexx
        "before\ncaught=40|4\n")
add_levelc_runtime_slice_test(levelc_signal_on_default_noopt
        rexx_src/levelc_signal_on_default.rexx
        "before\ncaught=40|4\n" -n)
add_levelc_runtime_slice_test(levelc_signal_on_nested
        rexx_src/levelc_signal_on_nested.rexx
        "inner|40|10\nafter\n")
add_levelc_runtime_slice_test(levelc_signal_on_nested_noopt
        rexx_src/levelc_signal_on_nested.rexx
        "inner|40|10\nafter\n" -n)
add_levelc_runtime_slice_test(levelc_signal_on_child_override
        rexx_src/levelc_signal_on_child_override.rexx
        "inner=40|10\nparent\nouter=40|5\n")
add_levelc_runtime_slice_test(levelc_signal_on_child_override_noopt
        rexx_src/levelc_signal_on_child_override.rexx
        "inner=40|10\nparent\nouter=40|5\n" -n)
add_levelc_runtime_slice_test(levelc_signal_child_off
        rexx_src/levelc_signal_child_off.rexx "40|4\n")
add_levelc_runtime_slice_test(levelc_signal_child_off_noopt
        rexx_src/levelc_signal_child_off.rexx "40|4\n" -n)
add_levelc_runtime_slice_test(levelc_signal_on_recursive
        rexx_src/levelc_signal_on_recursive.rexx
        "caught=40|14\nup=0\nup=0\nafter\n")
add_levelc_runtime_slice_test(levelc_signal_on_recursive_noopt
        rexx_src/levelc_signal_on_recursive.rexx
        "caught=40|14\nup=0\nup=0\nafter\n" -n)
add_levelc_runtime_slice_test(levelc_signal_policy_names
        rexx_src/levelc_signal_policy_names.rexx "all names\n")
add_levelc_runtime_slice_test(levelc_signal_policy_names_noopt
        rexx_src/levelc_signal_policy_names.rexx "all names\n" -n)
add_levelc_runtime_slice_test(levelc_condition_state
        rexx_src/levelc_condition_state.rexx
        "pre=|||||\ncaught=SIGNAL|NOVALUE|MISSING||SIGNAL|OFF\nnested=NOVALUE|MISSING|OFF\nrestored=NOVALUE|MISSING|OFF\nenabled=ON\n")
add_levelc_runtime_slice_test(levelc_condition_state_noopt
        rexx_src/levelc_condition_state.rexx
        "pre=|||||\ncaught=SIGNAL|NOVALUE|MISSING||SIGNAL|OFF\nnested=NOVALUE|MISSING|OFF\nrestored=NOVALUE|MISSING|OFF\nenabled=ON\n" -n)
add_levelc_runtime_slice_test(levelc_condition_syntax_state
        rexx_src/levelc_condition_syntax_state.rexx
        "name=SYNTAX\ncode=40.14\ndescription=Error 40.14\ninstruction=SIGNAL\nstate=OFF\nrc=40\n")
add_levelc_runtime_slice_test(levelc_condition_syntax_state_noopt
        rexx_src/levelc_condition_syntax_state.rexx
        "name=SYNTAX\ncode=40.14\ndescription=Error 40.14\ninstruction=SIGNAL\nstate=OFF\nrc=40\n" -n)
add_levelc_runtime_slice_test(levelc_condition_syntax_description
        rexx_src/levelc_condition_syntax_description.rexx
        "1\n")
add_levelc_runtime_slice_test(levelc_condition_syntax_description_noopt
        rexx_src/levelc_condition_syntax_description.rexx
        "1\n" -n)
add_levelc_runtime_slice_test(levelc_condition_syntax_description_braces
        rexx_src/levelc_condition_syntax_description_braces.rexx
        "1\n")
add_levelc_runtime_slice_test(levelc_condition_syntax_description_braces_noopt
        rexx_src/levelc_condition_syntax_description_braces.rexx
        "1\n" -n)
add_levelc_runtime_slice_test(levelc_condition_syntax_description_escapes
        rexx_src/levelc_condition_syntax_description_escapes.rexx
        "1\n")
add_levelc_runtime_slice_test(levelc_condition_syntax_description_escapes_noopt
        rexx_src/levelc_condition_syntax_description_escapes.rexx
        "1\n" -n)
add_levelc_runtime_slice_test(levelc_condition_syntax_description_empty
        rexx_src/levelc_condition_syntax_description_empty.rexx
        "1\n")
add_levelc_runtime_slice_test(levelc_condition_syntax_description_empty_noopt
        rexx_src/levelc_condition_syntax_description_empty.rexx
        "1\n" -n)
add_levelc_runtime_slice_test(levelc_condition_syntax_description_random
        rexx_src/levelc_condition_syntax_description_random.rexx
        "1\n")
add_levelc_runtime_slice_test(levelc_condition_syntax_description_random_noopt
        rexx_src/levelc_condition_syntax_description_random.rexx
        "1\n" -n)
add_levelc_runtime_slice_test(levelc_condition_syntax_description_date
        rexx_src/levelc_condition_syntax_description_date.rexx
        "1\n")
add_levelc_runtime_slice_test(levelc_condition_syntax_description_date_noopt
        rexx_src/levelc_condition_syntax_description_date.rexx
        "1\n" -n)
add_levelc_runtime_slice_test(levelc_condition_nested_restore
        rexx_src/levelc_condition_nested_restore.rexx
        "outer=NOVALUE|MISSING\ninner=SYNTAX|40.14\nafter=NOVALUE|MISSING|OFF\n")
add_levelc_runtime_slice_test(levelc_condition_nested_restore_noopt
        rexx_src/levelc_condition_nested_restore.rexx
        "outer=NOVALUE|MISSING\ninner=SYNTAX|40.14\nafter=NOVALUE|MISSING|OFF\n" -n)
add_levelc_runtime_syntax_signal_test(levelc_condition_bad_option
        rexx_src/levelc_condition_bad_option.rexx "40.28" "before\n"
        "levelc_condition_bad_option.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_condition_bad_option_noopt
        rexx_src/levelc_condition_bad_option.rexx "40.28" "before\n"
        "levelc_condition_bad_option.rexx:3:" -n)
add_levelc_runtime_syntax_signal_test(levelc_condition_bad_empty
        rexx_src/levelc_condition_bad_empty.rexx "40.21" "before\n"
        "levelc_condition_bad_empty.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_condition_bad_empty_noopt
        rexx_src/levelc_condition_bad_empty.rexx "40.21" "before\n"
        "levelc_condition_bad_empty.rexx:3:" -n)
add_levelc_runtime_syntax_signal_test(levelc_condition_bad_arity
        rexx_src/levelc_condition_bad_arity.rexx "40.4" "before\n"
        "levelc_condition_bad_arity.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_condition_bad_arity_noopt
        rexx_src/levelc_condition_bad_arity.rexx "40.4" "before\n"
        "levelc_condition_bad_arity.rexx:3:" -n)
add_levelc_runtime_slice_test(levelc_signal_novalue_lifecycle
        rexx_src/levelc_signal_novalue_lifecycle.rexx
        "before\ncaught=4|RC\nAFTER_FIRST\nagain=11|RC\n")
add_levelc_runtime_slice_test(levelc_signal_novalue_lifecycle_noopt
        rexx_src/levelc_signal_novalue_lifecycle.rexx
        "before\ncaught=4|RC\nAFTER_FIRST\nagain=11|RC\n" -n)
add_levelc_runtime_slice_test(levelc_signal_novalue_default_off
        rexx_src/levelc_signal_novalue_default_off.rexx
        "UNBOUND\ndefault=6\n")
add_levelc_runtime_slice_test(levelc_signal_novalue_default_off_noopt
        rexx_src/levelc_signal_novalue_default_off.rexx
        "UNBOUND\ndefault=6\n" -n)
add_levelc_runtime_slice_test(levelc_signal_novalue_nested
        rexx_src/levelc_signal_novalue_nested.rexx
        "inner=8|RC\nafter\n")
add_levelc_runtime_slice_test(levelc_signal_novalue_nested_noopt
        rexx_src/levelc_signal_novalue_nested.rexx
        "inner=8|RC\nafter\n" -n)
add_levelc_runtime_slice_test(levelc_signal_novalue_nested_override
        rexx_src/levelc_signal_novalue_nested_override.rexx
        "UNBOUND\ninner=11|RC\nouter=4|RC\n")
add_levelc_runtime_slice_test(levelc_signal_novalue_nested_override_noopt
        rexx_src/levelc_signal_novalue_nested_override.rexx
        "UNBOUND\ninner=11|RC\nouter=4|RC\n" -n)
add_levelc_runtime_slice_test(levelc_signal_novalue_compound
        rexx_src/levelc_signal_novalue_compound.rexx
        "0\ncaught=5\n")
add_levelc_runtime_slice_test(levelc_signal_novalue_compound_noopt
        rexx_src/levelc_signal_novalue_compound.rexx
        "0\ncaught=5\n" -n)
add_levelc_runtime_syntax_signal_test(levelc_signal_novalue_missing
        rexx_src/levelc_signal_novalue_missing.rexx "16.1" "before\n"
        "levelc_signal_novalue_missing.rexx:4:")
add_levelc_runtime_syntax_signal_test(levelc_signal_novalue_missing_noopt
        rexx_src/levelc_signal_novalue_missing.rexx "16.1" "before\n"
        "levelc_signal_novalue_missing.rexx:4:" -n)
add_levelc_runtime_syntax_signal_test(levelc_signal_on_missing
        rexx_src/levelc_signal_on_missing.rexx "16.1" "before\n"
        "levelc_signal_on_missing.rexx:4:")
add_levelc_runtime_syntax_signal_test(levelc_signal_on_missing_noopt
        rexx_src/levelc_signal_on_missing.rexx "16.1" "before\n"
        "levelc_signal_on_missing.rexx:4:" -n)
add_levelc_runtime_syntax_signal_test(levelc_signal_quoted_handler_missing
        rexx_src/levelc_signal_quoted_handler_missing.rexx "16.1" "before\n"
        "levelc_signal_quoted_handler_missing.rexx:4:")
add_levelc_runtime_syntax_signal_test(levelc_signal_quoted_handler_missing_noopt
        rexx_src/levelc_signal_quoted_handler_missing.rexx "16.1" "before\n"
        "levelc_signal_quoted_handler_missing.rexx:4:" -n)
add_levelc_runtime_syntax_signal_test(levelc_signal_off_syntax
        rexx_src/levelc_signal_off_syntax.rexx "40.14" "before\n"
        "levelc_signal_off_syntax.rexx:5:")
add_levelc_runtime_syntax_signal_test(levelc_signal_on_once
        rexx_src/levelc_signal_on_once.rexx "40.14" "caught=40|3\n"
        "levelc_signal_on_once.rexx:6:")
add_levelc_runtime_syntax_signal_test(levelc_signal_value_missing
        rexx_src/levelc_signal_value_missing.rexx "16.1" "before\n"
        "levelc_signal_value_missing.rexx:3:")
add_levelc_runtime_syntax_signal_test(levelc_signal_value_missing_noopt
        rexx_src/levelc_signal_value_missing.rexx "16.1" "before\n"
        "levelc_signal_value_missing.rexx:3:" -n)
add_levelc_lowered_tree_shape_test(levelc_signal_direct_frame_tree
        rexx_src/levelc_signal_direct.rexx
        "FRAME_BRANCH"
        "FRAME_LABEL"
        "setSymbolValue")
add_levelc_lowered_tree_shape_test(levelc_signal_value_frame_tree
        rexx_src/levelc_signal_value_once.rexx
        "FRAME_BRANCH"
        "rexxclassicbif_translate"
        "CLASSIC_SYNTAX")

add_levelc_runtime_slice_test(levelc_slice3_bif_length_target_shape
        rexx_src/levelc_slice3_bif_length_target_shape.crexx
        "6\n")

add_levelc_runtime_slice_test(levelc_slice3_bif_length
        rexx_src/levelc_slice3_bif_length.rexx
        "6\n")

add_levelc_runtime_slice_test(levelc_slice4_bif_substr_target_shape
        rexx_src/levelc_slice4_bif_substr_target_shape.crexx
        "bcd\nef.\nbcdef\n")

add_levelc_runtime_slice_test(levelc_slice4_bif_substr
        rexx_src/levelc_slice4_bif_substr.rexx
        "bcd\nef.\nbcdef\n")

add_levelc_runtime_slice_test(levelc_slice4_procedure_args_return_target_shape
        rexx_src/levelc_slice4_procedure_args_return_target_shape.crexx
        "5\n")

add_levelc_runtime_slice_test(levelc_slice4_procedure_args_return
        rexx_src/levelc_slice4_procedure_args_return.rexx
        "5\n")

add_levelc_runtime_slice_test(levelc_slice4_call_args_expose
        rexx_src/levelc_slice4_call_args_expose.rexx
        "3\n")

add_levelc_runtime_slice_test(levelc_if_parse_dynamic_branch
        rexx_src/levelc_if_parse_dynamic_branch.rexx "left|right\n")
add_levelc_runtime_slice_test(levelc_if_parse_dynamic_branch_noopt
        rexx_src/levelc_if_parse_dynamic_branch.rexx "left|right\n" -n)
add_levelc_runtime_slice_test(levelc_do_parse_dynamic_body
        rexx_src/levelc_do_parse_dynamic_body.rexx "left|right\n")
add_levelc_runtime_slice_test(levelc_do_parse_dynamic_body_noopt
        rexx_src/levelc_do_parse_dynamic_body.rexx "left|right\n" -n)

add_levelc_runtime_slice_test(levelc_arg_frame_calls
        rexx_src/levelc_arg_frame_calls.rexx
        "A /  / C\nA\n / B / \n\n / B / \n\n:B\nA:\n")
add_levelc_runtime_slice_test(levelc_arg_frame_calls_noopt
        rexx_src/levelc_arg_frame_calls.rexx
        "A /  / C\nA\n / B / \n\n / B / \n\n:B\nA:\n" -n)
add_levelc_runtime_slice_test(levelc_arg_frame_lifecycle
        rexx_src/levelc_arg_frame_lifecycle.rexx
        "outer=A B||C D|0\ninner=X||Z\nrepeat=A B|0\nmain=X\n")
add_levelc_runtime_slice_test(levelc_arg_frame_lifecycle_noopt
        rexx_src/levelc_arg_frame_lifecycle.rexx
        "outer=A B||C D|0\ninner=X||Z\nrepeat=A B|0\nmain=X\n" -n)
add_levelc_runtime_slice_test(levelc_arg_recursive_shared
        rexx_src/levelc_arg_recursive_shared.rexx
        "enter=3:A B||Z|0\nenter=2:A BX||Z|0\nenter=1:A BXX||Z|0\nleave=1:A BXX|0\nleave=1:A BX|0\nleave=1:A B|0\n")
add_levelc_runtime_slice_test(levelc_arg_recursive_shared_noopt
        rexx_src/levelc_arg_recursive_shared.rexx
        "enter=3:A B||Z|0\nenter=2:A BX||Z|0\nenter=1:A BXX||Z|0\nleave=1:A BXX|0\nleave=1:A BX|0\nleave=1:A B|0\n" -n)
add_levelc_runtime_slice_test(levelc_arg_recursive_private
        rexx_src/levelc_arg_recursive_private.rexx
        "enter=3:A B||Z|0\nenter=2:A BX||Z|0\nenter=1:A BXX||Z|0\nleave=1:A BXX|0\nleave=2:A BX|0\nleave=3:A B|0\n")
add_levelc_runtime_slice_test(levelc_arg_recursive_private_noopt
        rexx_src/levelc_arg_recursive_private.rexx
        "enter=3:A B||Z|0\nenter=2:A BX||Z|0\nenter=1:A BXX||Z|0\nleave=1:A BXX|0\nleave=2:A BX|0\nleave=3:A B|0\n" -n)
add_levelc_runtime_slice_test(levelc_function_result_lifecycle
        rexx_src/levelc_function_result_lifecycle.rexx
        "empty=[]\nfall=Xy\nrecursive=321\ndone\n")
add_levelc_runtime_slice_test(levelc_function_result_lifecycle_noopt
        rexx_src/levelc_function_result_lifecycle.rexx
        "empty=[]\nfall=Xy\nrecursive=321\ndone\n" -n)
add_levelc_runtime_slice_test(levelc_return_whole
        rexx_src/levelc_return_whole.rexx
        "keyword=8\nshared=雪2;root=雪2\nprivate=inside;secret=SECRET\nbare=RESULT\nempty=VAR;len=0\ncounted=1;count=1\nfunction=E900;chars=2\nloop=1\n")
add_levelc_runtime_slice_test(levelc_return_whole_noopt
        rexx_src/levelc_return_whole.rexx
        "keyword=8\nshared=雪2;root=雪2\nprivate=inside;secret=SECRET\nbare=RESULT\nempty=VAR;len=0\ncounted=1;count=1\nfunction=E900;chars=2\nloop=1\n" -n)
add_levelc_runtime_slice_test(levelc_call_expression_actuals
        rexx_src/levelc_call_expression_actuals.rexx
        "enter=AÉ🙂|2\nenter=AÉ🙂|1\nenter=AÉ🙂|0\nrelay=A||B\nleave=AÉ🙂|0\nrelay=A||B\nleave=AÉ🙂|1\nrelay=A||B\nleave=AÉ🙂|2\norder=ABABAB\nrelay=É|Ÿ|3\nedges=2||C|\norder=ABABABC\nempty=0\n")
add_levelc_runtime_slice_test(levelc_call_expression_actuals_noopt
        rexx_src/levelc_call_expression_actuals.rexx
        "enter=AÉ🙂|2\nenter=AÉ🙂|1\nenter=AÉ🙂|0\nrelay=A||B\nleave=AÉ🙂|0\nrelay=A||B\nleave=AÉ🙂|1\nrelay=A||B\nleave=AÉ🙂|2\norder=ABABAB\nrelay=É|Ÿ|3\nedges=2||C|\norder=ABABABC\nempty=0\n" -n)
set(_levelc_call_resolution_expected
        "${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_resolution_result.expected")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${_levelc_call_resolution_expected}")
file(READ "${_levelc_call_resolution_expected}" _levelc_call_resolution_output)
add_levelc_runtime_slice_test(levelc_call_resolution_result
        rexx_src/levelc_call_resolution_result.rexx
        "${_levelc_call_resolution_output}")
add_levelc_runtime_slice_test(levelc_call_resolution_result_noopt
        rexx_src/levelc_call_resolution_result.rexx
        "${_levelc_call_resolution_output}" -n)
foreach(_mode IN ITEMS opt noopt)
    add_test(NAME levelc_call_external_${_mode}
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_external.py
                --rxc $<TARGET_FILE:rxc>
                --rxas $<TARGET_FILE:rxas>
                --rxlink $<TARGET_FILE:rxlink>
                --rxvm ${CREXX_RXVM_EXECUTABLE}
                --bindir ${CMAKE_BINARY_DIR}/bin
                --sourcedir ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                --workdir ${CMAKE_CURRENT_BINARY_DIR}/levelc_call_external
                --mode ${_mode})
    set_tests_properties(levelc_call_external_${_mode} PROPERTIES
            LABELS "compiler;levelc"
            CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc"
            RUN_SERIAL TRUE
            TIMEOUT 300)
endforeach()
foreach(_mode IN ITEMS opt noopt)
    add_test(NAME levelc_call_missing_external_${_mode}
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_missing_external.py
                --rxc $<TARGET_FILE:rxc>
                --rxas $<TARGET_FILE:rxas>
                --rxlink $<TARGET_FILE:rxlink>
                --rxvm ${CREXX_RXVM_EXECUTABLE}
                --bindir ${CMAKE_BINARY_DIR}/bin
                --sourcedir ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
                --workdir ${CMAKE_CURRENT_BINARY_DIR}/levelc_call_missing_external
                --mode ${_mode})
    set_tests_properties(levelc_call_missing_external_${_mode} PROPERTIES
            LABELS "compiler;levelc"
            CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc"
            RUN_SERIAL TRUE
            TIMEOUT 300)
endforeach()
foreach(_mode IN ITEMS opt noopt)
    add_test(NAME levelc_call_delayed_handler_${_mode}
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_delayed_injection.py
                --rxc $<TARGET_FILE:rxc>
                --rxas $<TARGET_FILE:rxas>
                --rxvm ${CREXX_RXVM_EXECUTABLE}
                --bindir ${CMAKE_BINARY_DIR}/bin
                --source ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_delayed_handler.rexx
                --workdir ${CMAKE_CURRENT_BINARY_DIR}
                --mode ${_mode})
    set_tests_properties(levelc_call_delayed_handler_${_mode} PROPERTIES
            LABELS "compiler;levelc"
            RUN_SERIAL TRUE
            TIMEOUT 180)
    add_test(NAME levelc_call_condition_matrix_${_mode}
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_delayed_injection.py
                --rxc $<TARGET_FILE:rxc>
                --rxas $<TARGET_FILE:rxas>
                --rxvm ${CREXX_RXVM_EXECUTABLE}
                --bindir ${CMAKE_BINARY_DIR}/bin
                --source ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_condition_matrix.rexx
                --workdir ${CMAKE_CURRENT_BINARY_DIR}
                --mode ${_mode}
                --scenario conditions)
    set_tests_properties(levelc_call_condition_matrix_${_mode} PROPERTIES
            LABELS "compiler;levelc"
            RUN_SERIAL TRUE
            TIMEOUT 180)
    add_test(NAME levelc_call_clause_boundaries_${_mode}
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_delayed_injection.py
                --rxc $<TARGET_FILE:rxc>
                --rxas $<TARGET_FILE:rxas>
                --rxvm ${CREXX_RXVM_EXECUTABLE}
                --bindir ${CMAKE_BINARY_DIR}/bin
                --source ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_clause_boundaries.rexx
                --workdir ${CMAKE_CURRENT_BINARY_DIR}
                --mode ${_mode}
                --scenario boundaries)
    set_tests_properties(levelc_call_clause_boundaries_${_mode} PROPERTIES
            LABELS "compiler;levelc"
            RUN_SERIAL TRUE
            TIMEOUT 180)
    add_test(NAME levelc_call_transfer_boundaries_${_mode}
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_delayed_injection.py
                --rxc $<TARGET_FILE:rxc>
                --rxas $<TARGET_FILE:rxas>
                --rxvm ${CREXX_RXVM_EXECUTABLE}
                --bindir ${CMAKE_BINARY_DIR}/bin
                --source ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_transfer_boundaries.rexx
                --workdir ${CMAKE_CURRENT_BINARY_DIR}
                --mode ${_mode}
                --scenario transfers)
    set_tests_properties(levelc_call_transfer_boundaries_${_mode} PROPERTIES
            LABELS "compiler;levelc"
            RUN_SERIAL TRUE
            TIMEOUT 180)
    add_test(NAME levelc_call_policy_lifecycle_${_mode}
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_delayed_injection.py
                --rxc $<TARGET_FILE:rxc>
                --rxas $<TARGET_FILE:rxas>
                --rxvm ${CREXX_RXVM_EXECUTABLE}
                --bindir ${CMAKE_BINARY_DIR}/bin
                --source ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_policy_lifecycle.rexx
                --workdir ${CMAKE_CURRENT_BINARY_DIR}
                --mode ${_mode}
                --scenario lifecycle)
    set_tests_properties(levelc_call_policy_lifecycle_${_mode} PROPERTIES
            LABELS "compiler;levelc"
            RUN_SERIAL TRUE
            TIMEOUT 180)
    add_test(NAME levelc_call_buffered_halt_${_mode}
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_delayed_injection.py
                --rxc $<TARGET_FILE:rxc>
                --rxas $<TARGET_FILE:rxas>
                --rxvm ${CREXX_RXVM_EXECUTABLE}
                --bindir ${CMAKE_BINARY_DIR}/bin
                --source ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_buffered_halt.rexx
                --workdir ${CMAKE_CURRENT_BINARY_DIR}
                --mode ${_mode}
                --scenario buffered_halt)
    set_tests_properties(levelc_call_buffered_halt_${_mode} PROPERTIES
            LABELS "compiler;levelc"
            RUN_SERIAL TRUE
            TIMEOUT 300)
    add_test(NAME levelc_call_missing_delayed_handler_${_mode}
            COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_delayed_injection.py
                --rxc $<TARGET_FILE:rxc>
                --rxas $<TARGET_FILE:rxas>
                --rxvm ${CREXX_RXVM_EXECUTABLE}
                --bindir ${CMAKE_BINARY_DIR}/bin
                --source ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_missing_delayed_handler.rexx
                --workdir ${CMAKE_CURRENT_BINARY_DIR}
                --mode ${_mode}
                --scenario missing)
    set_tests_properties(levelc_call_missing_delayed_handler_${_mode} PROPERTIES
            LABELS "compiler;levelc"
            RUN_SERIAL TRUE
            TIMEOUT 180)
endforeach()
add_levelc_runtime_slice_test(levelc_arg_static_templates
        rexx_src/levelc_arg_static_templates.rexx
        "first=AB|CD|ONE|TWO|\nrepeat=AB|CD|ONE TWO|\nmissing=|\nexposed=TAIL|\n")
add_levelc_runtime_slice_test(levelc_arg_static_templates_noopt
        rexx_src/levelc_arg_static_templates.rexx
        "first=AB|CD|ONE|TWO|\nrepeat=AB|CD|ONE TWO|\nmissing=|\nexposed=TAIL|\n" -n)
add_levelc_runtime_slice_test(levelc_arg_dynamic_templates
        rexx_src/levelc_arg_dynamic_templates.rexx
        "parse=mn|op|\nliteral=AB|CD|\nposition=AB|CDEF|\ncapture=X|B:TAIL||\n")
add_levelc_runtime_slice_test(levelc_arg_dynamic_templates_noopt
        rexx_src/levelc_arg_dynamic_templates.rexx
        "parse=mn|op|\nliteral=AB|CD|\nposition=AB|CDEF|\ncapture=X|B:TAIL||\n" -n)
add_levelc_runtime_slice_test(levelc_arg_unicode
        rexx_src/levelc_arg_unicode.rexx
        "fields=AÉ|🙂B|Ÿ\nraw=aé🙂b|FF\nrepeat=AÉ🙂B\ndot=🙂\nfunction=É🙂\n")
add_levelc_runtime_slice_test(levelc_arg_unicode_noopt
        rexx_src/levelc_arg_unicode.rexx
        "fields=AÉ|🙂B|Ÿ\nraw=aé🙂b|FF\nrepeat=AÉ🙂B\ndot=🙂\nfunction=É🙂\n" -n)
add_levelc_runtime_slice_test(levelc_arg_patterns
        rexx_src/levelc_arg_patterns.rexx
        "pattern AB CD \nleading LEFT RIGHT\ntrailing AB:CD\nbare\n")
add_levelc_runtime_slice_test(levelc_arg_patterns_noopt
        rexx_src/levelc_arg_patterns.rexx
        "pattern AB CD \nleading LEFT RIGHT\ntrailing AB:CD\nbare\n" -n)
add_levelc_runtime_slice_test(levelc_arg_nested_long
        rexx_src/levelc_arg_nested_long.rexx
        "ONE|SIX|TWELVE\nONE TWO THREE FOUR FIVE SIX SEVEN EIGHT NINE TEN ELEVEN TWELVE\n")
add_levelc_runtime_slice_test(levelc_arg_nested_long_noopt
        rexx_src/levelc_arg_nested_long.rexx
        "ONE|SIX|TWELVE\nONE TWO THREE FOUR FIVE SIX SEVEN EIGHT NINE TEN ELEVEN TWELVE\n" -n)
add_levelc_runtime_slice_test(levelc_arg_bif
        rexx_src/levelc_arg_bif.rexx
        "main=0\ncount=3\nvalues=a|||\nexists=1010\nomitted=0101\nagain=a|A\nfunction=1:xy\n")
add_levelc_runtime_slice_test(levelc_arg_bif_noopt
        rexx_src/levelc_arg_bif.rexx
        "main=0\ncount=3\nvalues=a|||\nexists=1010\nomitted=0101\nagain=a|A\nfunction=1:xy\n" -n)
add_levelc_runtime_slice_test(levelc_arg_instruction_invocation
        rexx_src/levelc_arg_instruction_invocation.rexx
        "initial=A|B||0|1\nrepeat=A B|A B|A B\nnoargs=0|\nfunction=mixed\nadjacency=Y\nmain=0\n")
add_levelc_runtime_slice_test(levelc_arg_instruction_invocation_noopt
        rexx_src/levelc_arg_instruction_invocation.rexx
        "initial=A|B||0|1\nrepeat=A B|A B|A B\nnoargs=0|\nfunction=mixed\nadjacency=Y\nmain=0\n" -n)
add_test(NAME levelc_arg_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_arg_dynamic_templates.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_arg_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
add_test(NAME levelc_arg_frame_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_arg_frame_lifecycle.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DNAME=levelc_arg_frame_linked
        -DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_arg_frame_lifecycle.expected
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_arg_frame_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
add_test(NAME levelc_call_resolution_result_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_resolution_result.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DNAME=levelc_call_resolution_result_linked
        -DEXPECTED_FILE=${_levelc_call_resolution_expected}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_call_resolution_result_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_procedure_whole.expected"
        "${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_procedure_lifecycle.expected"
        "${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_procedure_unicode.expected")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_procedure_whole.expected"
        LEVELC_PROCEDURE_WHOLE_EXPECTED)
add_levelc_runtime_slice_test(levelc_procedure_whole
        rexx_src/levelc_procedure_whole.rexx
        "${LEVELC_PROCEDURE_WHOLE_EXPECTED}")
add_levelc_runtime_slice_test(levelc_procedure_whole_noopt
        rexx_src/levelc_procedure_whole.rexx
        "${LEVELC_PROCEDURE_WHOLE_EXPECTED}" -n)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_procedure_lifecycle.expected"
        LEVELC_PROCEDURE_LIFECYCLE_EXPECTED)
add_levelc_runtime_slice_test(levelc_procedure_lifecycle
        rexx_src/levelc_procedure_lifecycle.rexx
        "${LEVELC_PROCEDURE_LIFECYCLE_EXPECTED}")
add_levelc_runtime_slice_test(levelc_procedure_lifecycle_noopt
        rexx_src/levelc_procedure_lifecycle.rexx
        "${LEVELC_PROCEDURE_LIFECYCLE_EXPECTED}" -n)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_procedure_unicode.expected"
        LEVELC_PROCEDURE_UNICODE_EXPECTED)
add_levelc_runtime_slice_test(levelc_procedure_unicode
        rexx_src/levelc_procedure_unicode.rexx
        "${LEVELC_PROCEDURE_UNICODE_EXPECTED}")
add_levelc_runtime_slice_test(levelc_procedure_unicode_noopt
        rexx_src/levelc_procedure_unicode.rexx
        "${LEVELC_PROCEDURE_UNICODE_EXPECTED}" -n)
add_test(NAME levelc_procedure_whole_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_procedure_whole.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DNAME=levelc_procedure_whole_linked
        -DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_procedure_whole.expected
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_procedure_whole_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
add_test(NAME levelc_procedure_source_tree
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_procedure_whole.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_procedure_source_tree.cmake)
set_tests_properties(levelc_procedure_source_tree PROPERTIES LABELS "compiler;levelc")
add_test(NAME levelc_return_source_tree
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_return_whole.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_return_source_tree.cmake)
set_tests_properties(levelc_return_source_tree PROPERTIES LABELS "compiler;levelc")
add_test(NAME levelc_function_result_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_function_result_lifecycle.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DNAME=levelc_function_result_linked
        -DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_function_result_lifecycle.expected
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_function_result_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
foreach(_return_mode IN ITEMS opt noopt)
    set(_return_noopt_arg)
    if(_return_mode STREQUAL "noopt")
        set(_return_noopt_arg -DNOOPT=1)
    endif()
    add_test(NAME levelc_return_whole_linked_${_return_mode}
            COMMAND ${CMAKE_COMMAND}
            -DRXC=$<TARGET_FILE:rxc>
            -DRXAS=$<TARGET_FILE:rxas>
            -DRXLINK=$<TARGET_FILE:rxlink>
            -DRXVM=${CREXX_RXVM_EXECUTABLE}
            -DBINDIR=${CMAKE_BINARY_DIR}/bin
            -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_return_whole.rexx
            -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DNAME=levelc_return_whole_linked_${_return_mode}
            -DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_return_whole.expected
            ${_return_noopt_arg}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
    set_tests_properties(levelc_return_whole_linked_${_return_mode} PROPERTIES
            LABELS "compiler;levelc"
            CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
endforeach()
set(_return_status_cases seven text bare continuation fallthrough decimal huge overflow exponent_huge args)
if(UNIX)
    list(APPEND _return_status_cases nearmax negative)
endif()
foreach(_return_case IN LISTS _return_status_cases)
    set(_return_extra_args)
    if(_return_case STREQUAL "seven")
        set(_return_source levelc_return_status7.rexx)
        set(_return_status 7)
    elseif(_return_case STREQUAL "text")
        set(_return_source levelc_return_status_text.rexx)
        set(_return_status 0)
    elseif(_return_case STREQUAL "continuation")
        set(_return_source levelc_return_continuation.rexx)
        set(_return_status 3)
    elseif(_return_case STREQUAL "fallthrough")
        set(_return_source levelc_return_main_fallthrough.rexx)
        set(_return_status 7)
    elseif(_return_case STREQUAL "decimal")
        set(_return_source levelc_return_status_decimal.rexx)
        set(_return_status 3)
    elseif(_return_case STREQUAL "huge")
        set(_return_source levelc_return_status_huge.rexx)
        set(_return_status 0)
    elseif(_return_case STREQUAL "nearmax")
        set(_return_source levelc_return_status_nearmax.rexx)
        set(_return_status 255)
    elseif(_return_case STREQUAL "overflow")
        set(_return_source levelc_return_status_overflow.rexx)
        set(_return_status 0)
    elseif(_return_case STREQUAL "negative")
        set(_return_source levelc_return_status_negative.rexx)
        set(_return_status 249)
    elseif(_return_case STREQUAL "exponent_huge")
        set(_return_source levelc_return_status_exponent_huge.rexx)
        set(_return_status 0)
    elseif(_return_case STREQUAL "args")
        set(_return_source levelc_return_status_args.rexx)
        set(_return_status 7)
        set(_return_extra_args
                "-DARG_ONE=blue green"
                "-DARG_TWO=red"
                "-DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_return_status_args.expected")
    else()
        set(_return_source levelc_return_status_bare.rexx)
        set(_return_status 0)
    endif()
    foreach(_return_mode IN ITEMS opt noopt)
        set(_return_noopt_arg)
        if(_return_mode STREQUAL "noopt")
            set(_return_noopt_arg -DNOOPT=1)
        endif()
        add_test(NAME levelc_return_status_${_return_case}_${_return_mode}
                COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXVM=${CREXX_RXVM_EXECUTABLE}
                -DBINDIR=${CMAKE_BINARY_DIR}/bin
                -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/${_return_source}
                -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
                -DNAME=levelc_return_status_${_return_case}_${_return_mode}
                -DEXPECTED_STATUS=${_return_status}
                ${_return_extra_args}
                ${_return_noopt_arg}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_return_status.cmake)
        set_tests_properties(levelc_return_status_${_return_case}_${_return_mode} PROPERTIES
                LABELS "compiler;levelc"
                CREXX_PREP_TARGETS "rxc;rxas;rxvm;library;classlib;rxfnsc")
    endforeach()
endforeach()
add_test(NAME levelc_signal_crossed_control_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_crossed_control.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DNAME=levelc_signal_crossed_control_linked
        -DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_crossed_control.expected
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_signal_crossed_control_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
add_test(NAME levelc_signal_unicode_labels_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_unicode_labels.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DNAME=levelc_signal_unicode_labels_linked
        -DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_unicode_labels.expected
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_signal_unicode_labels_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
add_test(NAME levelc_signal_duplicate_labels_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_duplicate_labels.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DNAME=levelc_signal_duplicate_labels_linked
        -DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_duplicate_labels.expected
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_signal_duplicate_labels_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
add_test(NAME levelc_signal_quoted_handler_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_quoted_handler.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DNAME=levelc_signal_quoted_handler_linked
        -DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_quoted_handler.expected
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_signal_quoted_handler_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
add_test(NAME levelc_signal_implicit_value_linked
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_implicit_value.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -DNAME=levelc_signal_implicit_value_linked
        -DEXPECTED_FILE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_implicit_value.expected
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_linked.cmake)
set_tests_properties(levelc_signal_implicit_value_linked PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "rxc;rxas;rxlink;rxvm;library;classlib;rxfnsc")
add_levelc_runtime_slice_test(levelc_slice4_procedure_missing_arg
        rexx_src/levelc_slice4_procedure_missing_arg.rexx "2|\n")
add_levelc_runtime_slice_test(levelc_slice6_local_function_omitted_arg
        rexx_src/levelc_slice6_local_function_omitted_arg.rexx "|2\n")
add_levelc_runtime_slice_test(levelc_slice40_parse_dynamic
        rexx_src/levelc_slice40_parse_dynamic.rexx "a|b\n")
foreach(variant IN ITEMS opt noopt)
    set(arg_main_name "levelc_arg_frame_main_${variant}")
    add_test(NAME ${arg_main_name}
            COMMAND ${CMAKE_COMMAND}
            -DRXC=$<TARGET_FILE:rxc>
            -DRXAS=$<TARGET_FILE:rxas>
            -DRXVM=${CREXX_RXVM_EXECUTABLE}
            -DBINDIR=${CMAKE_BINARY_DIR}/bin
            -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_arg_frame_main.rexx
            -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DNAME=${arg_main_name}
            -DNOOPT=$<STREQUAL:${variant},noopt>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_frame_main.cmake)
    set_tests_properties(${arg_main_name} PROPERTIES
            LABELS "compiler;levelc"
            CREXX_PREP_TARGETS "rxc;rxas;rxvm;library;classlib;rxfnsc")
endforeach()

add_levelc_runtime_slice_test(levelc_slice5_stem_expose_target_shape
        rexx_src/levelc_slice5_stem_expose_target_shape.crexx
        "after\n")

add_levelc_runtime_slice_test(levelc_slice5_stem_expose
        rexx_src/levelc_slice5_stem_expose.rexx
        "after\n")

add_levelc_runtime_slice_test(levelc_slice6_expressions_target_shape
        rexx_src/levelc_slice6_expressions_target_shape.crexx
        "11\n5\n24\n4\n2\n2\n8\n-5\n6\nab c\n1\n1\n0\n1\n0\n1\n1\n0\n1\n1\n2\n1\n3\n1\n1\n3\n")

add_levelc_runtime_slice_test(levelc_slice6_expressions
        rexx_src/levelc_slice6_expressions.rexx
        "11\n5\n24\n4\n2\n2\n8\n-5\n6\nab c\n1\n1\n0\n1\n0\n1\n1\n0\n1\n1\n2\n1\n3\n1\n1\n3\n")

add_levelc_runtime_slice_test(levelc_slice7_if_else_target_shape
        rexx_src/levelc_slice7_if_else_target_shape.crexx
        "yes\nno\n")
add_levelc_runtime_slice_test(levelc_slice7_if_else
        rexx_src/levelc_slice7_if_else.rexx
        "yes\nno\n")
add_levelc_runtime_slice_test(levelc_slice7_if_else_noopt
        rexx_src/levelc_slice7_if_else.rexx
        "yes\nno\n" -n)
add_levelc_runtime_slice_test(levelc_slice7_if_nested
        rexx_src/levelc_slice7_if_nested.rexx
        "true\nfalse\nnested\nafter\nonce\n1\n1\n")
add_levelc_runtime_slice_test(levelc_slice7_if_nested_noopt
        rexx_src/levelc_slice7_if_nested.rexx
        "true\nfalse\nnested\nafter\nonce\n1\n1\n" -n)
add_levelc_runtime_slice_test(levelc_if_instruction
        rexx_src/levelc_if_instruction.rexx
        "red blue new\n1\n2\nselect-if\nlocal\nlabel\nouter\n")
add_levelc_runtime_slice_test(levelc_if_instruction_noopt
        rexx_src/levelc_if_instruction.rexx
        "red blue new\n1\n2\nselect-if\nlocal\nlabel\nouter\n" -n)

add_levelc_runtime_slice_test(levelc_slice8_do_block_target_shape
        rexx_src/levelc_slice8_do_block_target_shape.crexx
        "before\none\ntwo\nafter\n")
add_levelc_runtime_slice_test(levelc_slice8_do_block
        rexx_src/levelc_slice8_do_block.rexx
        "before\none\ntwo\nafter\n")
add_levelc_runtime_slice_test(levelc_slice8_do_block_noopt
        rexx_src/levelc_slice8_do_block.rexx
        "before\none\ntwo\nafter\n" -n)
add_levelc_runtime_slice_test(levelc_slice8_do_nested
        rexx_src/levelc_slice8_do_nested.rexx
        "before\nnested\n2\n2\narm\n3\n")
add_levelc_runtime_slice_test(levelc_slice8_do_nested_noopt
        rexx_src/levelc_slice8_do_nested.rexx
        "before\nnested\n2\n2\narm\n3\n" -n)

add_levelc_runtime_slice_test(levelc_slice9_nop
        rexx_src/levelc_slice9_nop.rexx
        "before\nmiddle\nafter\n")
add_levelc_runtime_slice_test(levelc_slice9_nop_noopt
        rexx_src/levelc_slice9_nop.rexx
        "before\nmiddle\nafter\n" -n)

add_levelc_runtime_slice_test(levelc_slice10_scalar_drop
        rexx_src/levelc_slice10_scalar_drop.rexx
        "MISSING\nA\ntwo\nB C\nA\nA\nLOCAL\nSHARED\nSHARED\n")
add_levelc_runtime_slice_test(levelc_slice10_scalar_drop_noopt
        rexx_src/levelc_slice10_scalar_drop.rexx
        "MISSING\nA\ntwo\nB C\nA\nA\nLOCAL\nSHARED\nSHARED\n" -n)

add_levelc_runtime_slice_test(levelc_slice11_select
        rexx_src/levelc_slice11_select.rexx
        "first\n0\nsecond\nnested\n1\narm\nprocedure\n")
add_levelc_runtime_slice_test(levelc_slice11_select_noopt
        rexx_src/levelc_slice11_select.rexx
        "first\n0\nsecond\nnested\n1\narm\nprocedure\n" -n)
add_levelc_runtime_slice_test(levelc_select_instruction
        rexx_src/levelc_select_instruction.rexx
        "new 1\nempty\ninline\nred blue\n2\nnested\n2\nlabel\nlocal\nVALUE\n")
add_levelc_runtime_slice_test(levelc_select_instruction_noopt
        rexx_src/levelc_select_instruction.rexx
        "new 1\nempty\ninline\nred blue\n2\nnested\n2\nlabel\nlocal\nVALUE\n" -n)

add_levelc_runtime_slice_test(levelc_slice12_counted_do
        rexx_src/levelc_slice12_counted_do.rexx
        "before\nouter\ninner\ninner\nouter\ninner\ninner\nafter\nif\nif\n3\n")
add_levelc_runtime_slice_test(levelc_slice12_counted_do_noopt
        rexx_src/levelc_slice12_counted_do.rexx
        "before\nouter\ninner\ninner\nouter\ninner\ninner\nafter\nif\nif\n3\n" -n)

add_levelc_runtime_slice_test(levelc_slice13_loop_transfer
        rexx_src/levelc_slice13_loop_transfer.rexx
        "leave 1\nafter-leave 2\niterate 3\nafter-select-leave\nafter-select-iterate\nouter\nouter\n2\n")
add_levelc_runtime_slice_test(levelc_slice13_loop_transfer_noopt
        rexx_src/levelc_slice13_loop_transfer.rexx
        "leave 1\nafter-leave 2\niterate 3\nafter-select-leave\nafter-select-iterate\nouter\nouter\n2\n" -n)
add_levelc_runtime_slice_test(levelc_leave_instruction
        rexx_src/levelc_leave_instruction.rexx
        "count 2\nforever\nwhile 1\nuntil 1\ninner 1 1\nouter-state 1 2\nsame-inner 1\nsame-final 1\ncompound STEM.A STEM.B\nlimit-body 1\nlimit-state 1\nlocal 1\n")
add_levelc_runtime_slice_test(levelc_leave_instruction_noopt
        rexx_src/levelc_leave_instruction.rexx
        "count 2\nforever\nwhile 1\nuntil 1\ninner 1 1\nouter-state 1 2\nsame-inner 1\nsame-final 1\ncompound STEM.A STEM.B\nlimit-body 1\nlimit-state 1\nlocal 1\n" -n)
set(LEVELC_ITERATE_EXPECTED
        "counted 1\ncounted 3\ncounted 4\ncounted-state 4\nforever 1\nforever 3\nforever-state 4\nwhile 1\nwhile 3\nwhile-state 3\nuntil 1\nuntil 3\nuntil-state 3\ncontrolled 1\ncontrolled 5\ncontrolled-state 7\nfor 1\nfor 3\nfor-state 4\nuntil-step 3\nuntil-step-state 3\nwhile-step 1\nwhile-step 3\nwhile-step-state 4\nfor-until 2\nfor-until-state 2\nouter-visit 1\nouter-visit 2\nouter-visit 3\nouter-state 4 1\nsame-inner 2\nsame-outer 3\nsame-state 4\ncompound 2\ncompound 3\ncompound-state 4\nselect-done\nlocal 4\n")
add_levelc_runtime_slice_test(levelc_iterate_instruction
        rexx_src/levelc_iterate_instruction.rexx
        "${LEVELC_ITERATE_EXPECTED}")
add_levelc_runtime_slice_test(levelc_iterate_instruction_noopt
        rexx_src/levelc_iterate_instruction.rexx
        "${LEVELC_ITERATE_EXPECTED}" -n)
add_levelc_runtime_slice_test(levelc_strictc_transfer_dead
        rexx_src/levelc_strictc_transfer_dead.rexx
        "body 1\nafter 2\n")
add_levelc_runtime_slice_test(levelc_strictc_transfer_dead_noopt
        rexx_src/levelc_strictc_transfer_dead.rexx
        "body 1\nafter 2\n" -n)

add_levelc_runtime_slice_test(levelc_slice14_forever
        rexx_src/levelc_slice14_forever.rexx
        "1\n3\ncount 4\nafter-simple\nafter-select\nouter\nouter\n3\n")
add_levelc_runtime_slice_test(levelc_slice14_forever_noopt
        rexx_src/levelc_slice14_forever.rexx
        "1\n3\ncount 4\nafter-simple\nafter-select\nouter\nouter\n3\n" -n)

add_levelc_runtime_slice_test(levelc_slice15_while
        rexx_src/levelc_slice15_while.rexx
        "zero\ncount 1\ncount 2\ncount 3\nbody 1\nbody 3\nafter 4\ninner 1\ninner 2\nouter\ninner 1\ninner 2\nouter\n2\n")
add_levelc_runtime_slice_test(levelc_slice15_while_noopt
        rexx_src/levelc_slice15_while.rexx
        "zero\ncount 1\ncount 2\ncount 3\nbody 1\nbody 3\nafter 4\ninner 1\ninner 2\nouter\ninner 1\ninner 2\nouter\n2\n" -n)

add_levelc_runtime_slice_test(levelc_slice16_until
        rexx_src/levelc_slice16_until.rexx
        "once 1\nafter-once\nafter-iterate 1\nbody 2\nafter 3\ninner 1\ninner 2\nouter\ninner 1\ninner 2\nouter\n2\n")
add_levelc_runtime_slice_test(levelc_slice16_until_noopt
        rexx_src/levelc_slice16_until.rexx
        "once 1\nafter-once\nafter-iterate 1\nbody 2\nafter 3\ninner 1\ninner 2\nouter\ninner 1\ninner 2\nouter\n2\n" -n)

add_levelc_runtime_slice_test(levelc_slice17_combined_do
        rexx_src/levelc_slice17_combined_do.rexx
        "while 1\nwhile 2\nwhile 3\nafter-while 3\nzero-while\nuntil 2\nuntil 3\nafter-until 3\nzero-until\nafter-iterate 1\nafter-leave\n2\n")
add_levelc_runtime_slice_test(levelc_slice17_combined_do_noopt
        rexx_src/levelc_slice17_combined_do.rexx
        "while 1\nwhile 2\nwhile 3\nafter-while 3\nzero-while\nuntil 2\nuntil 3\nafter-until 3\nzero-until\nafter-iterate 1\nafter-leave\n2\n" -n)

add_levelc_runtime_slice_test(levelc_slice19_logical_eager
        rexx_src/levelc_slice19_logical_eager.rexx
        "0\ncalls 1\n1\ncalls 2\n1\ncalls 3\n1\ncalls 4\n1\nflag 0\n1\nflag 0\n1\ncalls 6\n")
add_levelc_runtime_slice_test(levelc_slice19_logical_eager_noopt
        rexx_src/levelc_slice19_logical_eager.rexx
        "0\ncalls 1\n1\ncalls 2\n1\ncalls 3\n1\ncalls 4\n1\nflag 0\n1\nflag 0\n1\ncalls 6\n" -n)
add_levelc_runtime_slice_test(levelc_slice18_while_setup
        rexx_src/levelc_slice18_while_setup.rexx
        "and 1\nand 2\nand 3\nafter-and 3\ntick-loop 1\ntick-loop 2\ncalls 3\nsubstr 2\nafter-substr 2\ntail 2\nafter-tail 2\nbody 1\nbody 3\nafter-body 4\n2\n")
add_levelc_runtime_slice_test(levelc_slice18_while_setup_noopt
        rexx_src/levelc_slice18_while_setup.rexx
        "and 1\nand 2\nand 3\nafter-and 3\ntick-loop 1\ntick-loop 2\ncalls 3\nsubstr 2\nafter-substr 2\ntail 2\nafter-tail 2\nbody 1\nbody 3\nafter-body 4\n2\n" -n)
add_levelc_runtime_slice_test(levelc_slice20_until_setup
        rexx_src/levelc_slice20_until_setup.rexx
        "once\nfirst-calls 1\ntick-body 2\ncalls 3\nsubstr 1\nsubstr 2\nafter-substr 2\ntail 1\ntail 2\nafter-tail 2\nbody 2\nafter-body 3\n2\n")
add_levelc_runtime_slice_test(levelc_slice20_until_setup_noopt
        rexx_src/levelc_slice20_until_setup.rexx
        "once\nfirst-calls 1\ntick-body 2\ncalls 3\nsubstr 1\nsubstr 2\nafter-substr 2\ntail 1\ntail 2\nafter-tail 2\nbody 2\nafter-body 3\n2\n" -n)
add_levelc_runtime_slice_test(levelc_slice21_combined_setup
        rexx_src/levelc_slice21_combined_setup.rexx
        "zero-while-calls 0\nzero-until-calls 0\nwhile 1\nwhile 2\nafter-while 2 3\ncount-while 1\ncount-while 2\nafter-count-while 2 5\nuntil 2\nafter-until 2 7\ncount-until 1\ncount-until 2\nafter-count-until 2 9\nsubstr 1\nsubstr 2\nafter-substr 2\nbody 2\nafter-body 3 11\n2\n")
add_levelc_runtime_slice_test(levelc_slice21_combined_setup_noopt
        rexx_src/levelc_slice21_combined_setup.rexx
        "zero-while-calls 0\nzero-until-calls 0\nwhile 1\nwhile 2\nafter-while 2 3\ncount-while 1\ncount-while 2\nafter-count-while 2 5\nuntil 2\nafter-until 2 7\ncount-until 1\ncount-until 2\nafter-count-until 2 9\nsubstr 1\nsubstr 2\nafter-substr 2\nbody 2\nafter-body 3 11\n2\n" -n)
add_levelc_runtime_slice_test(levelc_slice22_forever_condition
        rexx_src/levelc_slice22_forever_condition.rexx
        "while 1\nwhile 2\nafter-while 2 3\nafter-zero 4\nuntil 2\nafter-until 2 6\nonce\nafter-once 7\nsubstr 1\nsubstr 2\nafter-substr 2\nbody 2\nafter-body 3\n2\n")
add_levelc_runtime_slice_test(levelc_slice22_forever_condition_noopt
        rexx_src/levelc_slice22_forever_condition.rexx
        "while 1\nwhile 2\nafter-while 2 3\nafter-zero 4\nuntil 2\nafter-until 2 6\nonce\nafter-once 7\nsubstr 1\nsubstr 2\nafter-substr 2\nbody 2\nafter-body 3\n2\n" -n)
add_levelc_runtime_slice_test(levelc_slice23_dynamic_count
        rexx_src/levelc_slice23_dynamic_count.rexx
        "scalar 1\nscalar 2\nscalar 3\nafter-scalar 3 0\nafter-zero\ndecimal 1\ndecimal 2\nexponent 1\nexponent 2\ncall 1\ncall 2\nafter-call 2 1\nlength 1\nlength 2\nlength 3\nsubstr-count 1\nsubstr-count 2\nbody 2\nafter-body 3\n2\n")
add_levelc_runtime_slice_test(levelc_slice23_dynamic_count_noopt
        rexx_src/levelc_slice23_dynamic_count.rexx
        "scalar 1\nscalar 2\nscalar 3\nafter-scalar 3 0\nafter-zero\ndecimal 1\ndecimal 2\nexponent 1\nexponent 2\ncall 1\ncall 2\nafter-call 2 1\nlength 1\nlength 2\nlength 3\nsubstr-count 1\nsubstr-count 2\nbody 2\nafter-body 3\n2\n" -n)
add_levelc_runtime_slice_test(levelc_slice24_dynamic_combined
        rexx_src/levelc_slice24_dynamic_combined.rexx
        "after-zero 0\nwhile 1\nwhile 2\nafter-while 2 0 3\nlimited-while 1\nlimited-while 2\nafter-limited-while 2 5\nuntil 2\nafter-until 2 7\nlimited-until 1\nlimited-until 2\nafter-limited-until 2 9\nsubstr 1\nsubstr 2\nafter-substr 2\ncall 1\ncall 2\nafter-call 2 1 11\nbody 2\nafter-body 3 13\n2\n")
add_levelc_runtime_slice_test(levelc_slice24_dynamic_combined_noopt
        rexx_src/levelc_slice24_dynamic_combined.rexx
        "after-zero 0\nwhile 1\nwhile 2\nafter-while 2 0 3\nlimited-while 1\nlimited-while 2\nafter-limited-while 2 5\nuntil 2\nafter-until 2 7\nlimited-until 1\nlimited-until 2\nafter-limited-until 2 9\nsubstr 1\nsubstr 2\nafter-substr 2\ncall 1\ncall 2\nafter-call 2 1 11\nbody 2\nafter-body 3 13\n2\n" -n)
add_levelc_runtime_slice_test(levelc_slice25_controlled_to
        rexx_src/levelc_slice25_controlled_to.rexx
        "basic 1\nbasic 2\nbasic 3\nafter-basic 4\nafter-zero 3\nchanged 2\nchanged 3\nchanged 4\nafter-changed 5\niter 1\niter 3\nafter-iter 4\nleft 1\nafter-left 2\nnested 1 2\nnested 2 2\nafter-nested 3 3\n3\n")
add_levelc_runtime_slice_test(levelc_slice25_controlled_to_noopt
        rexx_src/levelc_slice25_controlled_to.rexx
        "basic 1\nbasic 2\nbasic 3\nafter-basic 4\nafter-zero 3\nchanged 2\nchanged 3\nchanged 4\nafter-changed 5\niter 1\niter 3\nafter-iter 4\nleft 1\nafter-left 2\nnested 1 2\nnested 2 2\nafter-nested 3 3\n3\n" -n)
add_levelc_runtime_slice_test(levelc_slice26_controlled_by
        rexx_src/levelc_slice26_controlled_by.rexx
        "down 3\ndown 2\ndown 1\nafter-down 0\nup 1\nup 3\nafter-up 5\nchanged 3\nchanged 2\nchanged 1\nafter-changed 0\nzero 1\nafter-zero 1\nafter-bad-up 2\nafter-bad-down 1\nnested 2\nafter-nested 1\n")
add_levelc_runtime_slice_test(levelc_slice26_controlled_by_noopt
        rexx_src/levelc_slice26_controlled_by.rexx
        "down 3\ndown 2\ndown 1\nafter-down 0\nup 1\nup 3\nafter-up 5\nchanged 3\nchanged 2\nchanged 1\nafter-changed 0\nzero 1\nafter-zero 1\nafter-bad-up 2\nafter-bad-down 1\nnested 2\nafter-nested 1\n" -n)
add_levelc_runtime_slice_test(levelc_slice27_controlled_for
        rexx_src/levelc_slice27_controlled_for.rexx
        "simple 1\nsimple 2\nafter-simple 3\nstep 1\nstep 3\nafter-step 5\ndown 3\ndown 2\nafter-down 1\nafter-zero 1\nchanged 2\nchanged 3\nafter-changed 4\niter 1\niter 3\nafter-iter 4\nleft 1\nafter-left 2\n3\n")
add_levelc_runtime_slice_test(levelc_slice27_controlled_for_noopt
        rexx_src/levelc_slice27_controlled_for.rexx
        "simple 1\nsimple 2\nafter-simple 3\nstep 1\nstep 3\nafter-step 5\ndown 3\ndown 2\nafter-down 1\nafter-zero 1\nchanged 2\nchanged 3\nafter-changed 4\niter 1\niter 3\nafter-iter 4\nleft 1\nafter-left 2\n3\n" -n)
add_levelc_runtime_slice_test(levelc_slice28_controlled_for_only
        rexx_src/levelc_slice28_controlled_for_only.rexx
        "plain 1\nplain 2\nafter-plain 3\ndown 3\ndown 2\nafter-down 1\nup 1\nup 3\nafter-up 5\nafter-zero 1\nchanged 2\nchanged 3\nafter-changed 4\niter 1\niter 3\nafter-iter 4\nleft 1\nafter-left 2\n3\n")
add_levelc_runtime_slice_test(levelc_slice28_controlled_for_only_noopt
        rexx_src/levelc_slice28_controlled_for_only.rexx
        "plain 1\nplain 2\nafter-plain 3\ndown 3\ndown 2\nafter-down 1\nup 1\nup 3\nafter-up 5\nafter-zero 1\nchanged 2\nchanged 3\nafter-changed 4\niter 1\niter 3\nafter-iter 4\nleft 1\nafter-left 2\n3\n" -n)
add_levelc_runtime_slice_test(levelc_slice29_named_transfer
        rexx_src/levelc_slice29_named_transfer.rexx
        "outer-iter 1 1\nouter-iter 2 1\nouter-iter 3 1\nafter-outer-iter 4 2\nouter-leave 1 1\nafter-outer-leave 1 2\ninner-iter 1 2\ninner-iter 1 3\ninner-iter 2 2\ninner-iter 2 3\nafter-inner-iter 3 4\nafter-select 2\n3\n")
add_levelc_runtime_slice_test(levelc_slice29_named_transfer_noopt
        rexx_src/levelc_slice29_named_transfer.rexx
        "outer-iter 1 1\nouter-iter 2 1\nouter-iter 3 1\nafter-outer-iter 4 2\nouter-leave 1 1\nafter-outer-leave 1 2\ninner-iter 1 2\ninner-iter 1 3\ninner-iter 2 2\ninner-iter 2 3\nafter-inner-iter 3 4\nafter-select 2\n3\n" -n)
add_levelc_runtime_slice_test(levelc_slice30_controlled_while
        rexx_src/levelc_slice30_controlled_while.rexx
        "to 1 1\nto 2 2\nafter-to 3 2\nafter-zero-to 3 0\nafter-zero-for 1 0\nfor 1 1\nfor 3 2\nfor 5 3\nafter-for 7 3\ndown 3 1\ndown 1 3\nafter-down 0 3\nleft 1 1\nafter-left 2 2\nboth 1 1\nboth 2 2\nafter-both 3 2\nlocal 1 0\nlocal 2 1\n3\n")
add_levelc_runtime_slice_test(levelc_slice30_controlled_while_noopt
        rexx_src/levelc_slice30_controlled_while.rexx
        "to 1 1\nto 2 2\nafter-to 3 2\nafter-zero-to 3 0\nafter-zero-for 1 0\nfor 1 1\nfor 3 2\nfor 5 3\nafter-for 7 3\ndown 3 1\ndown 1 3\nafter-down 0 3\nleft 1 1\nafter-left 2 2\nboth 1 1\nboth 2 2\nafter-both 3 2\nlocal 1 0\nlocal 2 1\n3\n" -n)
add_levelc_runtime_slice_test(levelc_slice31_controlled_until
        rexx_src/levelc_slice31_controlled_until.rexx
        "to 1 0\nto 2 1\nafter-to 2 2\nafter-zero-to 3 0\nafter-zero-for 1 0\nfor 1 0\nfor 3 1\nafter-for 3 2\ncounted 1 0\ncounted 2 1\nafter-counted 3 2\ndown 2 1\nafter-down 2 2\nafter-left 1 0\nboth 1 0\nboth 2 1\nafter-both 2 2\nlocal 1 0\n1\n")
add_levelc_runtime_slice_test(levelc_slice31_controlled_until_noopt
        rexx_src/levelc_slice31_controlled_until.rexx
        "to 1 0\nto 2 1\nafter-to 2 2\nafter-zero-to 3 0\nafter-zero-for 1 0\nfor 1 0\nfor 3 1\nafter-for 3 2\ncounted 1 0\ncounted 2 1\nafter-counted 3 2\ndown 2 1\nafter-down 2 2\nafter-left 1 0\nboth 1 0\nboth 2 1\nafter-both 2 2\nlocal 1 0\n1\n" -n)
add_levelc_runtime_slice_test(levelc_slice32_unbounded_control
        rexx_src/levelc_slice32_unbounded_control.rexx
        "plain 1\nplain 2\nafter-plain 3\nnext 3\nnext 5\nafter-next 7\ndown 3\ndown 2\ndown 1\nafter-down 0\nchanged 3\nchanged 4\nafter-changed 5\nwhile 1 1\nwhile 2 2\nafter-while 3 3\nuntil 1 0\nuntil 2 1\nafter-until 2 2\n2\n")
add_levelc_runtime_slice_test(levelc_slice32_unbounded_control_noopt
        rexx_src/levelc_slice32_unbounded_control.rexx
        "plain 1\nplain 2\nafter-plain 3\nnext 3\nnext 5\nafter-next 7\ndown 3\ndown 2\ndown 1\nafter-down 0\nchanged 3\nchanged 4\nafter-changed 5\nwhile 1 1\nwhile 2 2\nafter-while 3 3\nuntil 1 0\nuntil 2 1\nafter-until 2 2\n2\n" -n)
set_tests_properties(levelc_slice32_unbounded_control
        levelc_slice32_unbounded_control_noopt PROPERTIES TIMEOUT 60)
add_levelc_runtime_slice_test(levelc_slice33_dynamic_to
        rexx_src/levelc_slice33_dynamic_to.rexx
        "fixed 1 3\nfixed 2 1\nfixed 3 1\nafter-fixed 4 1\nendpoint 10\nafter-zero 2 1\ndown 3 1\ndown 2 9\ndown 1 9\nafter-down 0 9\nwhile 1\nwhile 2\nafter-while 3\nuntil 1\nuntil 2\nafter-until 2\nafter-outer 4\n")
add_levelc_runtime_slice_test(levelc_slice33_dynamic_to_noopt
        rexx_src/levelc_slice33_dynamic_to.rexx
        "fixed 1 3\nfixed 2 1\nfixed 3 1\nafter-fixed 4 1\nendpoint 10\nafter-zero 2 1\ndown 3 1\ndown 2 9\ndown 1 9\nafter-down 0 9\nwhile 1\nwhile 2\nafter-while 3\nuntil 1\nuntil 2\nafter-until 2\nafter-outer 4\n" -n)
add_levelc_runtime_slice_test(levelc_slice34_dynamic_by
        rexx_src/levelc_slice34_dynamic_by.rexx
        "to 10 1\nby 10 2\nfirst-after 1 2\nby 10 1\nto 10 2\nsecond-after 1 2\nup 1 2\nup 3 1\nup 5 1\nafter-up 7 1\ndown 3 -1\ndown 2 1\ndown 1 1\nafter-down 0 1\nzero 1\nafter-zero 1\nno-to 1\nno-to 3\nno-to 5\nafter-no-to 7\nwhile 1\nwhile 2\nafter-while 3\nuntil 1\nuntil 2\nafter-until 2\nafter-outer 4\nlocal 1\nlocal 3\n5\n")
add_levelc_runtime_slice_test(levelc_slice34_dynamic_by_noopt
        rexx_src/levelc_slice34_dynamic_by.rexx
        "to 10 1\nby 10 2\nfirst-after 1 2\nby 10 1\nto 10 2\nsecond-after 1 2\nup 1 2\nup 3 1\nup 5 1\nafter-up 7 1\ndown 3 -1\ndown 2 1\ndown 1 1\nafter-down 0 1\nzero 1\nafter-zero 1\nno-to 1\nno-to 3\nno-to 5\nafter-no-to 7\nwhile 1\nwhile 2\nafter-while 3\nuntil 1\nuntil 2\nafter-until 2\nafter-outer 4\nlocal 1\nlocal 3\n5\n" -n)
set_tests_properties(levelc_slice34_dynamic_by
        levelc_slice34_dynamic_by_noopt PROPERTIES TIMEOUT 60)
add_levelc_runtime_slice_test(levelc_call_window_loop_carried
        rexx_src/levelc_call_window_loop_carried.rexx
        "first-after 1\nall 1\nall 2\nafter-all 3\n")
add_levelc_runtime_slice_test(levelc_call_window_loop_carried_noopt
        rexx_src/levelc_call_window_loop_carried.rexx
        "first-after 1\nall 1\nall 2\nafter-all 3\n" -n)
set_tests_properties(levelc_call_window_loop_carried
        levelc_call_window_loop_carried_noopt PROPERTIES TIMEOUT 60)
add_levelc_runtime_slice_test(levelc_slice35_dynamic_for
        rexx_src/levelc_slice35_dynamic_for.rexx
        "for 10 1\nto 10 2\nby 10 3\nfirst-after 1 3\nby 10 1\nto 10 2\nfor 10 3\nsecond-after 1 3\nby 10 1\nto 10 2\nfor-two 10 3\nall 1 3\nall 2 3\nafter-all 3 3\ncounted 1 3\ncounted 2 1\ncounted 3 1\nafter-counted 4 1\ndecimal 1\ndecimal 2\nafter-decimal 3\nuntil 1\nuntil 2\nafter-until 2 9\nafter-zero 1 1 0\nafter-zero-end 1 1 0\nafter-outer 3\nlocal 1\nlocal 2\n3\n")
add_levelc_runtime_slice_test(levelc_slice35_dynamic_for_noopt
        rexx_src/levelc_slice35_dynamic_for.rexx
        "for 10 1\nto 10 2\nby 10 3\nfirst-after 1 3\nby 10 1\nto 10 2\nfor 10 3\nsecond-after 1 3\nby 10 1\nto 10 2\nfor-two 10 3\nall 1 3\nall 2 3\nafter-all 3 3\ncounted 1 3\ncounted 2 1\ncounted 3 1\nafter-counted 4 1\ndecimal 1\ndecimal 2\nafter-decimal 3\nuntil 1\nuntil 2\nafter-until 2 9\nafter-zero 1 1 0\nafter-zero-end 1 1 0\nafter-outer 3\nlocal 1\nlocal 2\n3\n" -n)
set_tests_properties(levelc_slice35_dynamic_for
        levelc_slice35_dynamic_for_noopt PROPERTIES TIMEOUT 60)
add_levelc_runtime_slice_test(levelc_slice36_dynamic_start
        rexx_src/levelc_slice36_dynamic_start.rexx
        "start 10 1\nto 10 2\nby 10 3\nfor 10 4\nafter-zero 1 4\nchanged 2 2\nchanged 3 9\nafter-changed 4 9\nfractional -1.0\nfractional 0\nfractional 1\nafter-fractional 2 bad\nwhile 1\nwhile 2\nafter-while 3\nuntil 1\nuntil 2\nafter-until 2\nafter-outer 4\nlocal 1\nlocal 2\n3\n")
add_levelc_runtime_slice_test(levelc_slice36_dynamic_start_noopt
        rexx_src/levelc_slice36_dynamic_start.rexx
        "start 10 1\nto 10 2\nby 10 3\nfor 10 4\nafter-zero 1 4\nchanged 2 2\nchanged 3 9\nafter-changed 4 9\nfractional -1.0\nfractional 0\nfractional 1\nafter-fractional 2 bad\nwhile 1\nwhile 2\nafter-while 3\nuntil 1\nuntil 2\nafter-until 2\nafter-outer 4\nlocal 1\nlocal 2\n3\n" -n)
set_tests_properties(levelc_slice36_dynamic_start
        levelc_slice36_dynamic_start_noopt PROPERTIES TIMEOUT 60)
add_levelc_runtime_slice_test(levelc_do_compound_whole
        rexx_src/levelc_do_compound_whole.rexx
        "body=1\nafter=1|11\nmulti=1\nmulti-after=1|8\nheader-before 90 80 a\nheader-body 90 1 b\nheader-body 90 2 b\nheader-after 90 3 b\n")
add_levelc_runtime_slice_test(levelc_do_compound_whole_noopt
        rexx_src/levelc_do_compound_whole.rexx
        "body=1\nafter=1|11\nmulti=1\nmulti-after=1|8\nheader-before 90 80 a\nheader-body 90 1 b\nheader-body 90 2 b\nheader-after 90 3 b\n" -n)
add_levelc_runtime_slice_test(levelc_do_large_count
        rexx_src/levelc_do_large_count.rexx
        "dynamic\nliteral\nfor 1\nexponent 1\nafter 1 1\n")
add_levelc_runtime_slice_test(levelc_do_large_count_noopt
        rexx_src/levelc_do_large_count.rexx
        "dynamic\nliteral\nfor 1\nexponent 1\nafter 1 1\n" -n)
add_levelc_runtime_slice_test(levelc_do_numeric_scale
        rexx_src/levelc_do_numeric_scale.rexx
        "scale 1.00\nscale 2.00\nafter-scale 3.00\nleading 1\nleading 2\nplus 1\nplus 2\nexponent 10\nexponent 11\nadjusted 12.0\nadjusted 13.0\nfraction 0.500\nfraction 1.000\nfraction 1.500\nnegative -1.0\nnegative 0\nnegative 1\n")
add_levelc_runtime_slice_test(levelc_do_numeric_scale_noopt
        rexx_src/levelc_do_numeric_scale.rexx
        "scale 1.00\nscale 2.00\nafter-scale 3.00\nleading 1\nleading 2\nplus 1\nplus 2\nexponent 10\nexponent 11\nadjusted 12.0\nadjusted 13.0\nfraction 0.500\nfraction 1.000\nfraction 1.500\nnegative -1.0\nnegative 0\nnegative 1\n" -n)
add_levelc_runtime_slice_test(levelc_do_modifier_order
        rexx_src/levelc_do_modifier_order.rexx
        "TBF 1 STBF\nafter-TBF 2 STBF\nTFB 1 STFB\nafter-TFB 2 STFB\nBTF 1 SBTF\nafter-BTF 2 SBTF\nBFT 1 SBFT\nafter-BFT 2 SBFT\nFTB 1 SFTB\nafter-FTB 2 SFTB\nFBT 1 SFBT\nafter-FBT 2 SFBT\n")
add_levelc_runtime_slice_test(levelc_do_modifier_order_noopt
        rexx_src/levelc_do_modifier_order.rexx
        "TBF 1 STBF\nafter-TBF 2 STBF\nTFB 1 STFB\nafter-TFB 2 STFB\nBTF 1 SBTF\nafter-BTF 2 SBTF\nBFT 1 SBFT\nafter-BFT 2 SBFT\nFTB 1 SFTB\nafter-FTB 2 SFTB\nFBT 1 SFBT\nafter-FBT 2 SFBT\n" -n)
add_levelc_runtime_slice_test(levelc_slice37_empty_string
        rexx_src/levelc_slice37_empty_string.rexx
        "\n\nlength 0 0\njoined leftright\ncall 0\ncompare 1\n")
add_levelc_runtime_slice_test(levelc_slice37_empty_string_noopt
        rexx_src/levelc_slice37_empty_string.rexx
        "\n\nlength 0 0\njoined leftright\ncall 0\ncompare 1\n" -n)
add_levelc_runtime_slice_test(levelc_slice38_adjacent_call
        rexx_src/levelc_slice38_adjacent_call.rexx
        "literal 3\nx 2\nnested 2\nempty 0\nlocal ZZ\nspaced LENGTH abc\n")
add_levelc_runtime_slice_test(levelc_slice38_adjacent_call_noopt
        rexx_src/levelc_slice38_adjacent_call.rexx
        "literal 3\nx 2\nnested 2\nempty 0\nlocal ZZ\nspaced LENGTH abc\n" -n)
add_levelc_runtime_slice_test(levelc_slice39_arg_upper
        rexx_src/levelc_slice39_arg_upper.rexx
        "inside ABC DEF aBc\nresult ABC/DEF\nafter aBc dEf\ninside A B C D aBc\nspaces A B/C D\nempty 0\n")
add_levelc_runtime_slice_test(levelc_slice39_arg_upper_noopt
        rexx_src/levelc_slice39_arg_upper.rexx
        "inside ABC DEF aBc\nresult ABC/DEF\nafter aBc dEf\ninside A B C D aBc\nspaces A B/C D\nempty 0\n" -n)
add_levelc_runtime_slice_test(levelc_slice40_parse_single
        rexx_src/levelc_slice40_parse_single.rexx
        "plain aBc dEf aBc dEf\nupper ABC DEF aBc dEf\nvalue aBc dEf! aBc dEf\nonce new 1\nself MIX ED\nsame MIX ED\nnested ABC DEF\nlocal MIX/mIx\n")
add_levelc_runtime_slice_test(levelc_slice40_parse_single_noopt
        rexx_src/levelc_slice40_parse_single.rexx
        "plain aBc dEf aBc dEf\nupper ABC DEF aBc dEf\nvalue aBc dEf! aBc dEf\nonce new 1\nself MIX ED\nsame MIX ED\nnested ABC DEF\nlocal MIX/mIx\n" -n)
add_levelc_runtime_slice_test(levelc_slice40_parse_upper_import
        rexx_src/levelc_slice40_parse_upper_import.rexx
        "MIX\n")
add_levelc_runtime_slice_test(levelc_slice41_parse_three
        rexx_src/levelc_slice41_parse_three.rexx
        "a|b|  c  |  a  b   c  \na||\n  c  |\nleft|right|tail here\nx|y|z  q|1\nA|B|  C |\nlocal a|b|  c \n")
add_levelc_runtime_slice_test(levelc_slice41_parse_three_noopt
        rexx_src/levelc_slice41_parse_three.rexx
        "a|b|  c  |  a  b   c  \na||\n  c  |\nleft|right|tail here\nx|y|z  q|1\nA|B|  C |\nlocal a|b|  c \n" -n)
add_levelc_runtime_slice_test(levelc_slice46_parse_two
        rexx_src/levelc_slice46_parse_two.rexx
        "a| b   c  |  a  b   c  \na|\n b   c  |\nleft|right tail here\nx|y z  q|1\nA|  B   C |\nlocal a| b   c \n")
add_levelc_runtime_slice_test(levelc_slice46_parse_two_noopt
        rexx_src/levelc_slice46_parse_two.rexx
        "a| b   c  |  a  b   c  \na|\n b   c  |\nleft|right tail here\nx|y z  q|1\nA|  B   C |\nlocal a| b   c \n" -n)
add_levelc_runtime_slice_test(levelc_slice47_parse_dot
        rexx_src/levelc_slice47_parse_dot.rexx
        "a|b|c|  a  b   c d e  \na|b|\nz|\nleft|right|third\nm|n|o|1\nA|B|C|\nlocal a|b|c\n")
add_levelc_runtime_slice_test(levelc_slice47_parse_dot_noopt
        rexx_src/levelc_slice47_parse_dot.rexx
        "a|b|c|  a  b   c d e  \na|b|\nz|\nleft|right|third\nm|n|o|1\nA|B|C|\nlocal a|b|c\n" -n)
add_levelc_runtime_slice_test(levelc_slice48_parse_placeholders
        rexx_src/levelc_slice48_parse_placeholders.rexx
        "final2=a|  a  b   c d  |\ninitial2= b   c d  |\nmiddle3=a|  c d  |\ninitial3=b|  c d  |\ndouble3=  c d  |\nfinal3=a|b|\ndoublefinal3=a|\nshort=||\nalias=left|tail more|\nrepeat=z q|\nvalue=n|o p|1\nupper=B|  C D |\nlocal=a|  c d |\n")
add_levelc_runtime_slice_test(levelc_slice48_parse_placeholders_noopt
        rexx_src/levelc_slice48_parse_placeholders.rexx
        "final2=a|  a  b   c d  |\ninitial2= b   c d  |\nmiddle3=a|  c d  |\ninitial3=b|  c d  |\ndouble3=  c d  |\nfinal3=a|b|\ndoublefinal3=a|\nshort=||\nalias=left|tail more|\nrepeat=z q|\nvalue=n|o p|1\nupper=B|  C D |\nlocal=a|  c d |\n" -n)
add_levelc_runtime_slice_test(levelc_slice49_parse_all_dots
        rexx_src/levelc_slice49_parse_all_dots.rexx
        "one=  a  b c |0\ntwo=  a  b c |0\nthree=  a  b c |0\nvalue1=1\nvalue2=2\nvalue3=3\nlocal=done|4\n")
add_levelc_runtime_slice_test(levelc_slice49_parse_all_dots_noopt
        rexx_src/levelc_slice49_parse_all_dots.rexx
        "one=  a  b c |0\ntwo=  a  b c |0\nthree=  a  b c |0\nvalue1=1\nvalue2=2\nvalue3=3\nlocal=done|4\n" -n)
add_levelc_runtime_slice_test(levelc_slice50_parse_four
        rexx_src/levelc_slice50_parse_four.rexx
        "four=a|b|c| d e  |\ninternal=a|c| d e  |\nmultiple=b| d e  |\nfinal= d e  |\nfinaldrop=a|c|\nalldrop=  a  b   c  d e  |\nshort=a|b|c||\nalias=left|right|third|tail more|\nrepeat=z q|\nvalue=m|n|o|p q|1\nupper=A|C|D E |\nlocal=a|b|c|d e |\n")
add_levelc_runtime_slice_test(levelc_slice50_parse_four_noopt
        rexx_src/levelc_slice50_parse_four.rexx
        "four=a|b|c| d e  |\ninternal=a|c| d e  |\nmultiple=b| d e  |\nfinal= d e  |\nfinaldrop=a|c|\nalldrop=  a  b   c  d e  |\nshort=a|b|c||\nalias=left|right|third|tail more|\nrepeat=z q|\nvalue=m|n|o|p q|1\nupper=A|C|D E |\nlocal=a|b|c|d e |\n" -n)
add_levelc_runtime_slice_test(levelc_slice51_parse_generic
        rexx_src/levelc_slice51_parse_generic.rexx
        "five=a|b|c|d|e  |\ndots=b|d|\nalldots=  a  b   c  d e  |\nshort=a|b|||||||\nalias=left|one|two|three|four|five six|\nrepeat=r|\nvalue=m|n|o|p|q|r s|1\nupper=A|C|E|F G |\nlocal=a|b|c|d|e|f |\n")
add_levelc_runtime_slice_test(levelc_slice51_parse_generic_noopt
        rexx_src/levelc_slice51_parse_generic.rexx
        "five=a|b|c|d|e  |\ndots=b|d|\nalldots=  a  b   c  d e  |\nshort=a|b|||||||\nalias=left|one|two|three|four|five six|\nrepeat=r|\nvalue=m|n|o|p|q|r s|1\nupper=A|C|E|F G |\nlocal=a|b|c|d|e|f |\n" -n)
add_levelc_runtime_slice_test(levelc_slice52_parse_static
        rexx_src/levelc_slice52_parse_static.rexx
        "patterns=ab|cd|ef|\ndots=cd|ab,cd,ef|\nalldots=ab,cd,ef|\nedges=||end||\nmissing=ab,cd,ef||\nalias=ab|cd,ef|\nlong=w|x|y|z|q|r|\nabsolute=ab|cdef|\nrelative=abc|de|f|\nbackward=cd|ef|def|\nzero=abcdef|abcdef|\nfar=abcdef||\nspelling=ab|c|cdef|\nhex=a|b|\nbinary=a|b|\nquote=|,c|\ndoublequote=|,c|\nutf8=a|bc|\nleading=cd,ef|\ntrailing=ab|\nadjacent=ab|ef|\nstartabs=,cd,ef|\nadjpos=ab|d,ef|\nstartrel=ab,cd,ef|\nonce=m|n|1|\nupper=AB|CD|\nlocal=left|right|\n")
add_levelc_runtime_slice_test(levelc_slice52_parse_static_noopt
        rexx_src/levelc_slice52_parse_static.rexx
        "patterns=ab|cd|ef|\ndots=cd|ab,cd,ef|\nalldots=ab,cd,ef|\nedges=||end||\nmissing=ab,cd,ef||\nalias=ab|cd,ef|\nlong=w|x|y|z|q|r|\nabsolute=ab|cdef|\nrelative=abc|de|f|\nbackward=cd|ef|def|\nzero=abcdef|abcdef|\nfar=abcdef||\nspelling=ab|c|cdef|\nhex=a|b|\nbinary=a|b|\nquote=|,c|\ndoublequote=|,c|\nutf8=a|bc|\nleading=cd,ef|\ntrailing=ab|\nadjacent=ab|ef|\nstartabs=,cd,ef|\nadjpos=ab|d,ef|\nstartrel=ab,cd,ef|\nonce=m|n|1|\nupper=AB|CD|\nlocal=left|right|\n" -n)
add_levelc_runtime_slice_test(levelc_slice53_drop_direct
        rexx_src/levelc_slice53_drop_direct.rexx
        "before=one|two|lower|\ntail=ITEMS.A|two|\nderived=ITEMS.b|two|\nrestore=again|\norder=NAME|ITEMS.NAME|again|\nstem=ITEMS.A|ITEMS.B|ITEMS.b|\nnumeric=ITEMS.1|\nlocal=ITEMS.A/inside|ITEMS.A|\nif=ITEMS.B|\ndo=ITEMS.B|\n")
add_levelc_runtime_slice_test(levelc_slice53_drop_direct_noopt
        rexx_src/levelc_slice53_drop_direct.rexx
        "before=one|two|lower|\ntail=ITEMS.A|two|\nderived=ITEMS.b|two|\nrestore=again|\norder=NAME|ITEMS.NAME|again|\nstem=ITEMS.A|ITEMS.B|ITEMS.b|\nnumeric=ITEMS.1|\nlocal=ITEMS.A/inside|ITEMS.A|\nif=ITEMS.B|\ndo=ITEMS.B|\n" -n)
add_levelc_runtime_slice_test(levelc_slice54_drop_indirect
        rexx_src/levelc_slice54_drop_indirect.rexx
        "pair=A|B|a b|\ninvalid=A|B|\ncompound=ITEMS.b|ITEMS.B|\norder=KEY|ITEMS.KEY|\nretained=lower2|upper2|\nstem=ITEMS.b|ITEMS.B|\nouter=NAMES|held|\nreference=A|\nlocal=ITEMS.B|ITEMS.B|\nif=A|\ndo=A|\nunicode=A|B|\n")
add_levelc_runtime_slice_test(levelc_slice54_drop_indirect_noopt
        rexx_src/levelc_slice54_drop_indirect.rexx
        "pair=A|B|a b|\ninvalid=A|B|\ncompound=ITEMS.b|ITEMS.B|\norder=KEY|ITEMS.KEY|\nretained=lower2|upper2|\nstem=ITEMS.b|ITEMS.B|\nouter=NAMES|held|\nreference=A|\nlocal=ITEMS.B|ITEMS.B|\nif=A|\ndo=A|\nunicode=A|B|\n" -n)
add_levelc_runtime_slice_test(levelc_drop_instruction
        rexx_src/levelc_drop_instruction.rexx
        "list=A|B|C|D|E|\nbefore=first\ndirect=ITEMS.part.Tail.last\nindirect=ITEMS.part.Tail.last\norder=third|ITEMS.X.Tail.last\nlocal=ITEMS.part.Tail.last|ITEMS.part.Tail.last\nstem=ITEMS.part.Tail.last\n")
add_levelc_runtime_slice_test(levelc_drop_instruction_noopt
        rexx_src/levelc_drop_instruction.rexx
        "list=A|B|C|D|E|\nbefore=first\ndirect=ITEMS.part.Tail.last\nindirect=ITEMS.part.Tail.last\norder=third|ITEMS.X.Tail.last\nlocal=ITEMS.part.Tail.last|ITEMS.part.Tail.last\nstem=ITEMS.part.Tail.last\n" -n)
add_levelc_runtime_slice_test(levelc_assignment_instruction
        rexx_src/levelc_assignment_instruction.rexx
        "scalar=second\nkeyword=keyword\nbytes=3\ncompound=first\nreset=default\ncaptured=default|captured\nchanged=new\nnested=do\nexposed=local\nstem=again\nempty-scalar=0\nempty-compound=0\nempty-stem=0|0\nempty-nested=0\nempty-local=0\nunicode-scalar=410042\nunicode-tail=漢🙂\nnul-tail=🌍\nunicode-order=漢|old-value\nunicode-local=🙂\nunicode-exposed=🙂\n")
add_levelc_runtime_slice_test(levelc_assignment_instruction_noopt
        rexx_src/levelc_assignment_instruction.rexx
        "scalar=second\nkeyword=keyword\nbytes=3\ncompound=first\nreset=default\ncaptured=default|captured\nchanged=new\nnested=do\nexposed=local\nstem=again\nempty-scalar=0\nempty-compound=0\nempty-stem=0|0\nempty-nested=0\nempty-local=0\nunicode-scalar=410042\nunicode-tail=漢🙂\nnul-tail=🌍\nunicode-order=漢|old-value\nunicode-local=🙂\nunicode-exposed=🙂\n" -n)
add_levelc_runtime_slice_test(levelc_say_instruction
        rexx_src/levelc_say_instruction.rexx
        "a\n\n\nb\n\n\ncall=ok\n\ninside=1\nexpr=1\ncount=1\ninside=2\nexpr=2\ncount=2\nz\n")
add_levelc_runtime_slice_test(levelc_say_instruction_noopt
        rexx_src/levelc_say_instruction.rexx
        "a\n\n\nb\n\n\ncall=ok\n\ninside=1\nexpr=1\ncount=1\ninside=2\nexpr=2\ncount=2\nz\n" -n)
add_levelc_runtime_slice_test(levelc_say_pool_reads
        rexx_src/levelc_say_pool_reads.rexx
        "missing=BAG.MiX.b\nsingle=single\nbound=multi\nstem=BAG.\nproc=multi\nlocal=BAG.\ncall=MULTI\ndropped-stem=BAG.\ndropped-compound=BAG.MiX.b\n")
add_levelc_runtime_slice_test(levelc_say_pool_reads_noopt
        rexx_src/levelc_say_pool_reads.rexx
        "missing=BAG.MiX.b\nsingle=single\nbound=multi\nstem=BAG.\nproc=multi\nlocal=BAG.\ncall=MULTI\ndropped-stem=BAG.\ndropped-compound=BAG.MiX.b\n" -n)
add_levelc_runtime_slice_test(levelc_slice42_abuttal
        rexx_src/levelc_slice42_abuttal.rexx
        "literal [a]\nsymbol a!\ngroup-left ab\ncall-left 3x\ncall-right x1\nspaced [ a ]\ncomment [a]\ncomment-spaced [ a ]\nexplicit [a]\nparse [a]\nparse-spaced [ a ]\n")
add_levelc_runtime_slice_test(levelc_slice42_abuttal_noopt
        rexx_src/levelc_slice42_abuttal.rexx
        "literal [a]\nsymbol a!\ngroup-left ab\ncall-left 3x\ncall-right x1\nspaced [ a ]\ncomment [a]\ncomment-spaced [ a ]\nexplicit [a]\nparse [a]\nparse-spaced [ a ]\n" -n)
add_levelc_runtime_slice_test(levelc_slice43_decimal_fraction
        rexx_src/levelc_slice43_decimal_fraction.rexx
        "literal 1.5 1. .5\nvalue 1.5\narithmetic 3.5 1.5\nparse 1.5\nabutted [1.5]\n")
add_levelc_runtime_slice_test(levelc_slice43_decimal_fraction_noopt
        rexx_src/levelc_slice43_decimal_fraction.rexx
        "literal 1.5 1. .5\nvalue 1.5\narithmetic 3.5 1.5\nparse 1.5\nabutted [1.5]\n" -n)
add_levelc_runtime_slice_test(levelc_slice44_exponent
        rexx_src/levelc_slice44_exponent.rexx
        "literal 1E2 1E+2 1E-2 1.5E2 1.E2 .5E+1\nmath 101 1.01\nparse 1E2\nabutted [1E2]\nwhole 1\n")
add_levelc_runtime_slice_test(levelc_slice44_exponent_noopt
        rexx_src/levelc_slice44_exponent.rexx
        "literal 1E2 1E+2 1E-2 1.5E2 1.E2 .5E+1\nmath 101 1.01\nparse 1E2\nabutted [1E2]\nwhole 1\n" -n)
add_levelc_runtime_slice_test(levelc_slice45_constant_symbols
        rexx_src/levelc_slice45_constant_symbols.rexx
        "literal 1X 1E 1.2.3 1A.B 123ABC 1E2X\npool 1X 1E\nabutted [1X]\nparse 1A.B\n")
add_levelc_runtime_slice_test(levelc_slice45_constant_symbols_noopt
        rexx_src/levelc_slice45_constant_symbols.rexx
        "literal 1X 1E 1.2.3 1A.B 123ABC 1E2X\npool 1X 1E\nabutted [1X]\nparse 1A.B\n" -n)

function(add_levelc_invalid_runtime_test NAME SOURCE_FILE EXPECTED_DIAGNOSTIC)
    set(noopt OFF)
    set(expected_order "${ARGN}")
    if("${ARGN}" STREQUAL "-n")
        set(noopt ON)
        set(expected_order "")
    endif()
    add_test(NAME ${NAME}
            COMMAND ${CMAKE_COMMAND}
            -DRXC=$<TARGET_FILE:rxc>
            -DRXAS=$<TARGET_FILE:rxas>
            -DRXVM=${CREXX_RXVM_EXECUTABLE}
            -DBINDIR=${CMAKE_BINARY_DIR}/bin
            -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
            -DNAME=${NAME}
            -DEXPECTED_DIAGNOSTIC=${EXPECTED_DIAGNOSTIC}
            -DEXPECTED_OUTPUT_ORDER=${expected_order}
            -DNOOPT=${noopt}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_invalid_runtime.cmake)
    set_tests_properties(${NAME} PROPERTIES LABELS "compiler;levelc")
endfunction()

add_levelc_invalid_runtime_test(levelc_frame_main_procedure_17_1
        rexx_src/levelc_slice2_procedure_expose_unsupported.rexx RXC-LC-17.1 "3")
add_levelc_invalid_runtime_test(levelc_frame_main_procedure_17_1_noopt
        rexx_src/levelc_slice2_procedure_expose_unsupported.rexx RXC-LC-17.1 -n)
add_levelc_invalid_runtime_test(levelc_frame_second_procedure_17_1
        rexx_src/levelc_frame_second_procedure_17_1.rexx RXC-LC-17.1 "one")
add_levelc_invalid_runtime_test(levelc_frame_second_procedure_17_1_noopt
        rexx_src/levelc_frame_second_procedure_17_1.rexx RXC-LC-17.1 -n)
add_levelc_invalid_runtime_test(levelc_frame_called_late_procedure_17_1
        rexx_src/levelc_frame_called_late_procedure_17_1.rexx RXC-LC-17.1 "before")
add_levelc_invalid_runtime_test(levelc_frame_called_late_procedure_17_1_noopt
        rexx_src/levelc_frame_called_late_procedure_17_1.rexx RXC-LC-17.1 -n)
add_levelc_invalid_runtime_test(levelc_function_bare_return_45_1
        rexx_src/levelc_function_bare_return_45_1.rexx RXC-LC-45.1 "before")
add_levelc_invalid_runtime_test(levelc_function_bare_return_45_1_noopt
        rexx_src/levelc_function_bare_return_45_1.rexx RXC-LC-45.1 -n)
set_tests_properties(levelc_function_bare_return_45_1
        levelc_function_bare_return_45_1_noopt PROPERTIES
        ENVIRONMENT "LEVELC_EXPECTED_FRAGMENT=source: levelc_function_bare_return_45_1.rexx:7:")
add_levelc_invalid_runtime_test(levelc_function_falloff_44_1
        rexx_src/levelc_function_falloff_44_1.rexx RXC-LC-44.1 "before\ninside=ARG")
add_levelc_invalid_runtime_test(levelc_function_falloff_44_1_noopt
        rexx_src/levelc_function_falloff_44_1.rexx RXC-LC-44.1 -n)
set_tests_properties(levelc_function_falloff_44_1
        levelc_function_falloff_44_1_noopt PROPERTIES
        ENVIRONMENT "LEVELC_EXPECTED_FRAGMENT=source: levelc_function_falloff_44_1.rexx:3:")
set_tests_properties(levelc_frame_main_procedure_17_1
        levelc_frame_main_procedure_17_1_noopt PROPERTIES
        ENVIRONMENT "LEVELC_EXPECTED_FRAGMENT=source: levelc_slice2_procedure_expose_unsupported.rexx:8:")
set_tests_properties(levelc_frame_second_procedure_17_1
        levelc_frame_second_procedure_17_1_noopt PROPERTIES
        ENVIRONMENT "LEVELC_EXPECTED_FRAGMENT=source: levelc_frame_second_procedure_17_1.rexx:9:")
set_tests_properties(levelc_frame_called_late_procedure_17_1
        levelc_frame_called_late_procedure_17_1_noopt PROPERTIES
        ENVIRONMENT "LEVELC_EXPECTED_FRAGMENT=source: levelc_frame_called_late_procedure_17_1.rexx:8:")

add_levelc_invalid_runtime_test(levelc_strictc_leave_outside
        rexx_src/levelc_strictc_leave_outside.rexx RXC-LC-28.1)
add_levelc_invalid_runtime_test(levelc_strictc_leave_outside_noopt
        rexx_src/levelc_strictc_leave_outside.rexx RXC-LC-28.1 -n)
add_levelc_invalid_runtime_test(levelc_strictc_leave_unknown
        rexx_src/levelc_strictc_leave_unknown.rexx RXC-LC-28.3)
add_levelc_invalid_runtime_test(levelc_strictc_leave_unknown_noopt
        rexx_src/levelc_strictc_leave_unknown.rexx RXC-LC-28.3 -n)
add_levelc_invalid_runtime_test(levelc_strictc_leave_inactive_caller
        rexx_src/levelc_strictc_leave_inactive_caller.rexx RXC-LC-28.1)
add_levelc_invalid_runtime_test(levelc_strictc_leave_inactive_caller_noopt
        rexx_src/levelc_strictc_leave_inactive_caller.rexx RXC-LC-28.1 -n)
add_levelc_invalid_runtime_test(levelc_strictc_iterate_outside
        rexx_src/levelc_strictc_iterate_outside.rexx RXC-LC-28.2)
add_levelc_invalid_runtime_test(levelc_strictc_iterate_outside_noopt
        rexx_src/levelc_strictc_iterate_outside.rexx RXC-LC-28.2 -n)
add_levelc_invalid_runtime_test(levelc_strictc_iterate_unknown
        rexx_src/levelc_strictc_iterate_unknown.rexx RXC-LC-28.4)
add_levelc_invalid_runtime_test(levelc_strictc_iterate_unknown_noopt
        rexx_src/levelc_strictc_iterate_unknown.rexx RXC-LC-28.4 -n)
add_levelc_invalid_runtime_test(levelc_strictc_iterate_inactive_caller
        rexx_src/levelc_strictc_iterate_inactive_caller.rexx RXC-LC-28.2)
add_levelc_invalid_runtime_test(levelc_strictc_iterate_inactive_caller_noopt
        rexx_src/levelc_strictc_iterate_inactive_caller.rexx RXC-LC-28.2 -n)

add_levelc_invalid_runtime_test(levelc_slice6_invalid_logical
        rexx_src/levelc_slice6_invalid_logical.rexx RXC-LC-34)
add_levelc_invalid_runtime_test(levelc_slice7_if_invalid_logical
        rexx_src/levelc_slice7_if_invalid_logical.rexx RXC-LC-34.1)
add_levelc_invalid_runtime_test(levelc_slice11_select_invalid_logical
        rexx_src/levelc_slice11_select_invalid_logical.rexx RXC-LC-34.2)
add_levelc_invalid_runtime_test(levelc_select_invalid_logical_noopt
        rexx_src/levelc_slice11_select_invalid_logical.rexx RXC-LC-34.2 -n)
add_levelc_invalid_runtime_test(levelc_slice11_select_no_match
        rexx_src/levelc_slice11_select_no_match.rexx RXC-LC-7.3)
add_levelc_invalid_runtime_test(levelc_select_no_match_noopt
        rexx_src/levelc_slice11_select_no_match.rexx RXC-LC-7.3 -n)
set_tests_properties(levelc_slice11_select_invalid_logical
        levelc_select_invalid_logical_noopt PROPERTIES
        ENVIRONMENT "LEVELC_EXPECTED_FRAGMENT=source: levelc_slice11_select_invalid_logical.rexx:3:3")
set_tests_properties(levelc_slice11_select_no_match
        levelc_select_no_match_noopt PROPERTIES
        ENVIRONMENT "LEVELC_EXPECTED_FRAGMENT=SELECT on line 2")
add_levelc_invalid_runtime_test(levelc_slice15_while_invalid_logical
        rexx_src/levelc_slice15_while_invalid_logical.rexx RXC-LC-34.3)
add_levelc_invalid_runtime_test(levelc_slice16_until_invalid_logical
        rexx_src/levelc_slice16_until_invalid_logical.rexx RXC-LC-34.4)
add_levelc_invalid_runtime_test(levelc_slice17_combined_while_invalid_logical
        rexx_src/levelc_slice17_combined_while_invalid_logical.rexx RXC-LC-34.3)
add_levelc_invalid_runtime_test(levelc_slice17_combined_until_invalid_logical
        rexx_src/levelc_slice17_combined_until_invalid_logical.rexx RXC-LC-34.4)
add_levelc_invalid_runtime_test(levelc_slice19_logical_left_invalid
        rexx_src/levelc_slice19_logical_left_invalid.rexx RXC-LC-34.5)
add_levelc_invalid_runtime_test(levelc_slice19_logical_right_and_invalid
        rexx_src/levelc_slice19_logical_right_and_invalid.rexx RXC-LC-34.6)
add_levelc_invalid_runtime_test(levelc_slice19_logical_right_or_invalid
        rexx_src/levelc_slice19_logical_right_or_invalid.rexx RXC-LC-34.6)
add_levelc_invalid_runtime_test(levelc_slice18_while_setup_invalid_logical
        rexx_src/levelc_slice18_while_setup_invalid_logical.rexx RXC-LC-34.3)
add_levelc_invalid_runtime_test(levelc_slice20_until_setup_invalid_logical
        rexx_src/levelc_slice20_until_setup_invalid_logical.rexx RXC-LC-34.4)
add_levelc_invalid_runtime_test(levelc_slice21_combined_while_invalid_logical
        rexx_src/levelc_slice21_combined_while_invalid_logical.rexx RXC-LC-34.3)
add_levelc_invalid_runtime_test(levelc_slice21_combined_until_invalid_logical
        rexx_src/levelc_slice21_combined_until_invalid_logical.rexx RXC-LC-34.4)
add_levelc_invalid_runtime_test(levelc_slice22_forever_while_invalid_logical
        rexx_src/levelc_slice22_forever_while_invalid_logical.rexx RXC-LC-34.3)
add_levelc_invalid_runtime_test(levelc_slice22_forever_until_invalid_logical
        rexx_src/levelc_slice22_forever_until_invalid_logical.rexx RXC-LC-34.4)
add_levelc_invalid_runtime_test(levelc_slice12_negative_do_invalid
        rexx_src/levelc_slice12_negative_do_invalid.rexx RXC-LC-26.2)
add_levelc_invalid_runtime_test(levelc_slice23_dynamic_count_invalid_text
        rexx_src/levelc_slice23_dynamic_count_invalid_text.rexx RXC-LC-26.2)
add_levelc_invalid_runtime_test(levelc_slice23_dynamic_count_invalid_fraction
        rexx_src/levelc_slice23_dynamic_count_invalid_fraction.rexx RXC-LC-26.2)
add_levelc_invalid_runtime_test(levelc_slice24_dynamic_combined_invalid_count
        rexx_src/levelc_slice24_dynamic_combined_invalid_count.rexx RXC-LC-26.2)
add_levelc_invalid_runtime_test(levelc_slice24_dynamic_combined_invalid_while
        rexx_src/levelc_slice24_dynamic_combined_invalid_while.rexx RXC-LC-34.3)
add_levelc_invalid_runtime_test(levelc_slice24_dynamic_combined_invalid_until
        rexx_src/levelc_slice24_dynamic_combined_invalid_until.rexx RXC-LC-34.4)
add_levelc_invalid_runtime_test(levelc_slice25_controlled_to_invalid_value
        rexx_src/levelc_slice25_controlled_to_invalid_value.rexx RXC-LC-41.6)
add_levelc_invalid_runtime_test(levelc_slice30_controlled_while_invalid
        rexx_src/levelc_slice30_controlled_while_invalid.rexx RXC-LC-34.3)
add_levelc_invalid_runtime_test(levelc_slice31_controlled_until_invalid
        rexx_src/levelc_slice31_controlled_until_invalid.rexx RXC-LC-34.4)
add_levelc_invalid_runtime_test(levelc_slice33_dynamic_to_invalid
        rexx_src/levelc_slice33_dynamic_to_invalid.rexx RXC-LC-41.4)
add_levelc_invalid_runtime_test(levelc_slice34_dynamic_by_invalid
        rexx_src/levelc_slice34_dynamic_by_invalid.rexx RXC-LC-41.5)
add_levelc_invalid_runtime_test(levelc_slice35_dynamic_for_invalid
        rexx_src/levelc_slice35_dynamic_for_invalid.rexx RXC-LC-26.3)
add_levelc_invalid_runtime_test(levelc_slice35_dynamic_for_fraction
        rexx_src/levelc_slice35_dynamic_for_fraction.rexx RXC-LC-26.3)
add_levelc_invalid_runtime_test(levelc_slice44_exponent_for_fraction
        rexx_src/levelc_slice44_exponent_for_fraction.rexx RXC-LC-26.3)
add_levelc_invalid_runtime_test(levelc_slice45_constant_for_invalid
        rexx_src/levelc_slice45_constant_for_invalid.rexx RXC-LC-26.3)
add_levelc_invalid_runtime_test(levelc_slice35_dynamic_for_negative
        rexx_src/levelc_slice35_dynamic_for_negative.rexx RXC-LC-26.3)
add_levelc_invalid_runtime_test(levelc_slice36_dynamic_start_invalid
        rexx_src/levelc_slice36_dynamic_start_invalid.rexx RXC-LC-41.6)
add_levelc_invalid_runtime_test(levelc_slice36_dynamic_start_invalid_after_clauses
        rexx_src/levelc_slice36_dynamic_start_invalid_after_clauses.rexx
        RXC-LC-41.6 "to 10|by 10|for 10")
add_levelc_invalid_runtime_test(levelc_slice36_dynamic_start_clause_error
        rexx_src/levelc_slice36_dynamic_start_clause_error.rexx
        RXC-LC-41.4 "to 10")
add_levelc_invalid_runtime_test(levelc_slice37_empty_string_numeric_invalid
        rexx_src/levelc_slice37_empty_string_numeric_invalid.rexx RXC-LC-41.4)

add_levelc_lowered_tree_shape_test(levelc_options_tree
        rexx_src/levelc_options_instruction.rexx
        "applyOptions"
        "__rxcp_levelc_options_config_"
        "RexxClassicConfig")

add_levelc_lowered_tree_shape_test(levelc_literal_ordinals_tree
        rexx_src/levelc_literal_ordinals.rexx
        "RexxValue"
        "parseplan")

add_levelc_lowered_tree_shape_test(levelc_slice2_procedure_expose_tree_shape
        rexx_src/levelc_slice2_procedure_expose.rexx
        "PROCEDURE"
        "ARGS"
        "OP_REFERENCE"
        "OP_DEREFERENCE"
        "RexxVariablePool"
        "exposeSymbol"
        "setSymbolValue"
        "symbolValue"
        "add"
        "asString")

add_levelc_lowered_tree_shape_test(levelc_slice3_bif_length_tree_shape
        rexx_src/levelc_slice3_bif_length.rexx
        "FUNCTION"
        "rexxclassicbif_length"
        "rexxclassicbifs"
        "RexxVariablePool"
        "RexxValue"
        "setSymbolValue"
        "symbolValue"
        "asString")

add_levelc_lowered_tree_shape_test(levelc_slice4_bif_substr_tree_shape
        rexx_src/levelc_slice4_bif_substr.rexx
        "RexxBifCallContext"
        "setArguments"
        "setCallerPool"
        "rexxclassicbifsubstr.rexxclassicbif_substr"
        "errorDetail"
        "__rxcp_levelc_bif_args"
        "__rxcp_levelc_bif_exists"
        ".RexxValue"
        ".int")

add_levelc_lowered_tree_shape_test(levelc_slice4_procedure_args_return_tree_shape
        rexx_src/levelc_slice4_procedure_args_return.rexx
        "__rxcp_levelc_body"
        "__rxcp_levelc_activation"
        "RexxActivationArguments"
        "append"
        "argument"
        ".RexxValue"
        "parseplan"
        "setSymbolValue"
        "RETURN"
        "add")

add_levelc_lowered_tree_shape_test(levelc_slice5_stem_expose_tree_shape
        rexx_src/levelc_slice5_stem_expose.rexx
        "RexxVariablePool"
        "setSymbolValue"
        "symbolValue"
        "exposeSymbol"
        "ITEMS.I")

add_levelc_lowered_tree_shape_test(levelc_nop_instruction_tree_shape
        rexx_src/levelc_slice9_nop.rexx
        "NOP : .*\\(2:0\\)"
        "NOP : .*\\(4:5\\)"
        "NOP : .*\\(8:2\\)"
        "NOP : .*\\(16:0\\)")

add_levelc_lowered_tree_shape_test(levelc_slice6_expressions_tree_shape
        rexx_src/levelc_slice6_expressions.rexx
        "subtract"
        "multiply"
        "divide"
        "integerDivide"
        "remainder"
        "power"
        "spaceConcat"
        "compareEqual"
        "strictCompareEqual"
        "logicalNot"
        "logicalXor"
        "logicalAnd"
        "logicalOr"
        "RexxBifCallContext")

add_levelc_lowered_tree_shape_test(levelc_slice7_if_else_tree_shape
        rexx_src/levelc_slice7_if_else.rexx
        "logicalIfValue"
        "IF"
        "DO"
        "RexxVariablePool")
add_levelc_lowered_tree_shape_test(levelc_if_instruction_tree
        rexx_src/levelc_if_instruction.rexx
        "logicalIfValue"
        "IF"
        "DO"
        "applyOptions"
        "RexxVariablePool")

add_levelc_lowered_tree_shape_test(levelc_slice8_do_block_tree_shape
        rexx_src/levelc_slice8_do_block.rexx
        "DO"
        "REPEAT"
        "FOR"
        "RexxValue")

add_levelc_lowered_tree_shape_test(levelc_slice10_scalar_drop_tree_shape
        rexx_src/levelc_slice10_scalar_drop.rexx
        "symbolValue"
        "drop"
        "RexxVariablePool")

add_levelc_lowered_tree_shape_test(levelc_slice11_select_tree_shape
        rexx_src/levelc_slice11_select.rexx
        "logicalWhenValue"
        "IF"
        "DO"
        "RexxVariablePool")
add_levelc_lowered_tree_shape_test(levelc_select_instruction_tree
        rexx_src/levelc_select_instruction.rexx
        "logicalWhenValue"
        "IF"
        "DO"
        "RexxVariablePool"
        "applyOptions")
add_test(NAME levelc_select_source_tree
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_select_instruction.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_select_source_tree.cmake)
set_tests_properties(levelc_select_source_tree PROPERTIES LABELS "compiler;levelc")

add_test(NAME levelc_arg_template_source_tree
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_arg_template_source_tree.cmake)
set_tests_properties(levelc_arg_template_source_tree PROPERTIES LABELS "compiler;levelc")
add_test(NAME levelc_signal_source_tree
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_signal_source_forms.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_signal_source_tree.cmake)
set_tests_properties(levelc_signal_source_tree PROPERTIES LABELS "compiler;levelc")

add_test(NAME levelc_call_expression_source_tree
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_expression_actuals.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_expression_source_tree.cmake)
set_tests_properties(levelc_call_expression_source_tree PROPERTIES LABELS "compiler;levelc")
add_test(NAME levelc_call_resolution_source_tree
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_resolution_result.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_call_resolution_source_tree.cmake)
set_tests_properties(levelc_call_resolution_source_tree PROPERTIES LABELS "compiler;levelc")

add_levelc_lowered_tree_shape_test(levelc_slice12_counted_do_tree_shape
        rexx_src/levelc_slice12_counted_do.rexx
        "DO"
        "REPEAT"
        "RexxDoState"
        "setRepeatCount"
        "entryAllowed"
        "advance")

add_levelc_lowered_tree_shape_test(levelc_slice13_loop_transfer_tree_shape
        rexx_src/levelc_slice13_loop_transfer.rexx
        "__rxcp_levelc_loop_"
        "LEAVE"
        "ITERATE"
        "IF")
add_levelc_lowered_tree_shape_test(levelc_leave_instruction_tree
        rexx_src/levelc_leave_instruction.rexx
        "__rxcp_levelc_loop_"
        "LEAVE"
        "IF"
        "RexxDoState")
add_levelc_lowered_tree_shape_test(levelc_strictc_transfer_dead_tree
        rexx_src/levelc_strictc_transfer_dead.rexx
        "LEAVE"
        "rexxdostate_invalid_transfer"
        "IF")
add_levelc_lowered_tree_shape_test(levelc_iterate_instruction_tree
        rexx_src/levelc_iterate_instruction.rexx
        "ITERATE"
        "__rxcp_levelc_loop_"
        "RexxDoState"
        "IF")
add_test(NAME levelc_strictc_transfer_source_tree
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DBINDIR=${CMAKE_BINARY_DIR}/bin
        -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_strictc_transfer_dead.rexx
        -DBUILD_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/levelc_strictc_transfer_tree.cmake)
set_tests_properties(levelc_strictc_transfer_source_tree PROPERTIES LABELS "compiler;levelc")

add_levelc_lowered_tree_shape_test(levelc_slice14_forever_tree_shape
        rexx_src/levelc_slice14_forever.rexx
        "__rxcp_levelc_loop_"
        "LEAVE"
        "ITERATE"
        "DO")

add_levelc_lowered_tree_shape_test(levelc_slice15_while_tree_shape
        rexx_src/levelc_slice15_while.rexx
        "__rxcp_levelc_loop_"
        "WHILE"
        "logicalWhileValue"
        "LEAVE"
        "ITERATE")

add_levelc_lowered_tree_shape_test(levelc_slice16_until_tree_shape
        rexx_src/levelc_slice16_until.rexx
        "__rxcp_levelc_loop_"
        "UNTIL"
        "logicalUntilValue"
        "LEAVE"
        "ITERATE")

add_levelc_lowered_tree_shape_test(levelc_slice17_combined_do_tree_shape
        rexx_src/levelc_slice17_combined_do.rexx
        "__rxcp_levelc_loop_"
        "RexxDoState"
        "setRepeatCount"
        "entryAllowed"
        "advance"
        "WHILE"
        "UNTIL"
        "LEAVE"
        "ITERATE")

add_levelc_lowered_tree_shape_test(levelc_slice19_logical_eager_tree_shape
        rexx_src/levelc_slice19_logical_eager.rexx
        "__rxcp_levelc_expr_"
        "logicalAnd"
        "logicalOr"
        "RexxVariablePool")
add_levelc_lowered_tree_shape_test(levelc_slice18_while_setup_tree_shape
        rexx_src/levelc_slice18_while_setup.rexx
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "logicalWhileValue"
        "RexxBifCallContext"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice20_until_setup_tree_shape
        rexx_src/levelc_slice20_until_setup.rexx
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "logicalUntilValue"
        "RexxBifCallContext"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice21_combined_setup_tree_shape
        rexx_src/levelc_slice21_combined_setup.rexx
        "RexxDoState"
        "setRepeatCount"
        "entryAllowed"
        "advance"
        "WHILE"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "RexxBifCallContext"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice22_forever_condition_tree_shape
        rexx_src/levelc_slice22_forever_condition.rexx
        "REPEAT"
        "WHILE"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice23_dynamic_count_tree_shape
        rexx_src/levelc_slice23_dynamic_count.rexx
        "RexxDoState"
        "setRepeatCount"
        "entryAllowed"
        "advance"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "RexxBifCallContext"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice24_dynamic_combined_tree_shape
        rexx_src/levelc_slice24_dynamic_combined.rexx
        "RexxDoState"
        "setRepeatCount"
        "entryAllowed"
        "advance"
        "WHILE"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "RexxBifCallContext"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice25_controlled_to_tree_shape
        rexx_src/levelc_slice25_controlled_to.rexx
        "RexxDoState"
        "setStart"
        "setTo"
        "entryAllowed"
        "advance"
        "WHILE"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice26_controlled_by_tree_shape
        rexx_src/levelc_slice26_controlled_by.rexx
        "RexxDoState"
        "setStart"
        "setTo"
        "setBy"
        "entryAllowed"
        "advance"
        "WHILE"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice27_controlled_for_tree_shape
        rexx_src/levelc_slice27_controlled_for.rexx
        "RexxDoState"
        "setStart"
        "setTo"
        "setBy"
        "setForCount"
        "entryAllowed"
        "advance"
        "WHILE"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice28_controlled_for_only_tree_shape
        rexx_src/levelc_slice28_controlled_for_only.rexx
        "RexxDoState"
        "setStart"
        "setForCount"
        "entryAllowed"
        "advance"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice29_named_transfer_tree_shape
        rexx_src/levelc_slice29_named_transfer.rexx
        "RexxDoState"
        "__rxcp_levelc_loop_"
        "entryAllowed"
        "advance"
        "LEAVE"
        "ITERATE"
        "BLOCK_EXPR")
add_levelc_lowered_tree_shape_test(levelc_slice30_controlled_while_tree_shape
        rexx_src/levelc_slice30_controlled_while.rexx
        "RexxDoState"
        "setStart"
        "entryAllowed"
        "advance"
        "logicalWhileValue"
        "WHILE"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH")
add_levelc_lowered_tree_shape_test(levelc_slice31_controlled_until_tree_shape
        rexx_src/levelc_slice31_controlled_until.rexx
        "RexxDoState"
        "setStart"
        "entryAllowed"
        "advance"
        "logicalUntilValue"
        "WHILE"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH")
add_levelc_lowered_tree_shape_test(levelc_slice32_unbounded_control_tree_shape
        rexx_src/levelc_slice32_unbounded_control.rexx
        "RexxDoState"
        "setStart"
        "entryAllowed"
        "advance"
        "logicalUntilValue"
        "REPEAT"
        "UNTIL"
        "BLOCK_EXPR"
        "LEAVE_WITH"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice33_dynamic_to_tree_shape
        rexx_src/levelc_slice33_dynamic_to.rexx
        "RexxDoState"
        "setTo"
        "entryAllowed"
        "advance"
        "BLOCK_EXPR"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice34_dynamic_by_tree_shape
        rexx_src/levelc_slice34_dynamic_by.rexx
        "RexxDoState"
        "setBy"
        "setTo"
        "entryAllowed"
        "advance"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice35_dynamic_for_tree_shape
        rexx_src/levelc_slice35_dynamic_for.rexx
        "RexxDoState"
        "setForCount"
        "setTo"
        "setBy"
        "entryAllowed"
        "advance"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice36_dynamic_start_tree_shape
        rexx_src/levelc_slice36_dynamic_start.rexx
        "RexxDoState"
        "setStart"
        "setTo"
        "setBy"
        "setForCount"
        "entryAllowed"
        "advance"
        "ITERATE")
add_levelc_lowered_tree_shape_test(levelc_slice37_empty_string_tree_shape
        rexx_src/levelc_slice37_empty_string.rexx
        "FACTORY_CALL"
        "RexxValue"
        "STRING"
        "strictCompareEqual")
add_levelc_lowered_tree_shape_test(levelc_slice38_adjacent_call_tree_shape
        rexx_src/levelc_slice38_adjacent_call.rexx
        "rexxclassicbif_length"
        "rexxclassicbifsubstr.rexxclassicbif_substr"
        "errorDetail"
        "__rxcp_levelc_body"
        "spaceConcat")
add_levelc_lowered_tree_shape_test(levelc_slice39_arg_upper_tree_shape
        rexx_src/levelc_slice39_arg_upper.rexx
        "rexxclassicbiftranslate"
        "rexxclassicbif_translate"
        "setArguments"
        "setCallerPool"
        "setSymbolValue")
add_levelc_lowered_tree_shape_test(levelc_arg_dynamic_tree_shape
        rexx_src/levelc_arg_dynamic_templates.rexx
        "RexxActivationArguments"
        "parseplan"
        "rexxclassicbif_translate"
        "setSymbolValue")
add_levelc_lowered_tree_shape_test(levelc_slice40_parse_single_tree_shape
        rexx_src/levelc_slice40_parse_single.rexx
        "rexxclassicbif_translate"
        "symbolValue"
        "setSymbolValue"
        "__rxcp_levelc_body")
add_levelc_lowered_tree_shape_test(levelc_slice41_parse_three_tree_shape
        rexx_src/levelc_slice41_parse_three.rexx
        "parseplan"
        "__rxcp_levelc_parse_fields_"
        "setSymbolValue"
        "rexxclassicbif_translate")
add_levelc_lowered_tree_shape_test(levelc_slice46_parse_two_tree_shape
        rexx_src/levelc_slice46_parse_two.rexx
        "parseplan"
        "__rxcp_levelc_parse_fields_"
        "setSymbolValue"
        "rexxclassicbif_translate")
add_levelc_lowered_tree_shape_test(levelc_slice47_parse_dot_tree_shape
        rexx_src/levelc_slice47_parse_dot.rexx
        "parseplan"
        "__rxcp_levelc_parse_fields_"
        "setSymbolValue"
        "rexxclassicbif_translate")
add_levelc_lowered_tree_shape_test(levelc_slice48_parse_placeholders_tree_shape
        rexx_src/levelc_slice48_parse_placeholders.rexx
        "parseplan"
        "__rxcp_levelc_parse_fields_"
        "setSymbolValue"
        "rexxclassicbif_translate")
add_levelc_lowered_tree_shape_test(levelc_slice49_parse_all_dots_tree_shape
        rexx_src/levelc_slice49_parse_all_dots.rexx
        "parseplan"
        "__rxcp_levelc_parse_fields_"
        "rexxclassicbif_translate")
add_levelc_lowered_tree_shape_test(levelc_slice50_parse_four_tree_shape
        rexx_src/levelc_slice50_parse_four.rexx
        "parseplan"
        "__rxcp_levelc_parse_fields_"
        "setSymbolValue"
        "rexxclassicbif_translate")
add_levelc_lowered_tree_shape_test(levelc_slice51_parse_generic_tree_shape
        rexx_src/levelc_slice51_parse_generic.rexx
        "parseplan"
        "__rxcp_levelc_parse_fields_"
        "setSymbolValue"
        "rexxclassicbif_translate")
add_levelc_lowered_tree_shape_test(levelc_slice52_parse_static_tree_shape
        rexx_src/levelc_slice52_parse_static.rexx
        "parseplan"
        "__rxcp_levelc_parse_source_"
        "__rxcp_levelc_parse_fields_"
        "setSymbolValue"
        "rexxclassicbif_translate")
add_levelc_lowered_tree_shape_test(levelc_slice53_drop_direct_tree_shape
        rexx_src/levelc_slice53_drop_direct.rexx
        "symbolValue"
        "dropSymbol"
        "setSymbolValue")
add_levelc_lowered_tree_shape_test(levelc_assignment_instruction_tree_shape
        rexx_src/levelc_assignment_instruction.rexx
        "setSymbolValue.*\\(16:0\\)"
        "ITEMS.KEY.PART"
        "FRAME_LABEL.*\\(16:15\\)"
        "setSymbolValue.*\\(32:0\\)"
        "STRING : .*\\(32:0\\)"
        "setSymbolValue.*\\(58:0\\)"
        "RexxVariablePool")
add_levelc_lowered_tree_shape_test(levelc_slice54_drop_indirect_tree_shape
        rexx_src/levelc_slice54_drop_indirect.rexx
        "dropIndirectList"
        "dropSymbol"
        "symbolValue")
add_levelc_lowered_tree_shape_test(levelc_drop_instruction_tree_shape
        rexx_src/levelc_drop_instruction.rexx
        "dropIndirectList"
        "dropSymbol")
add_levelc_lowered_tree_shape_test(levelc_say_instruction_tree_shape
        rexx_src/levelc_say_instruction.rexx
        "SAY : .*\\(2:0\\)"
        "STRING : .*\\(2:0\\)"
        "SAY : .*\\(3:10\\)"
        "SAY : .*\\(12:0\\)"
        "SAY : .*\\(20:0\\)"
        "STRING : .*\\(20:0\\)")
add_levelc_lowered_tree_shape_test(levelc_say_pool_reads_tree_shape
        rexx_src/levelc_say_pool_reads.rexx
        "SAY : .*\\(3:0\\)"
        "SAY : .*\\(9:0\\)"
        "SAY : .*\\(19:0\\)"
        "symbolValue")
add_levelc_lowered_tree_shape_test(levelc_bif_syntax_signal_tree_shape
        rexx_src/levelc_bif_syntax_signal.rexx
        "SAY : .*\\(2:0\\)"
        "ASSEMBLER : .*signal.*\\(2:4\\)"
        "rexxclassicbifsubstr.rexxclassicbif_substr.*\\(2:4\\)")
add_levelc_lowered_tree_shape_test(levelc_slice42_abuttal_tree_shape
        rexx_src/levelc_slice42_abuttal.rexx
        "concat"
        "spaceConcat"
        "rexxclassicbif_length")
add_levelc_lowered_tree_shape_test(levelc_slice43_decimal_fraction_tree_shape
        rexx_src/levelc_slice43_decimal_fraction.rexx
        "STRING : .*1[.]5"
        "RexxValue"
        "add")
add_levelc_lowered_tree_shape_test(levelc_slice44_exponent_tree_shape
        rexx_src/levelc_slice44_exponent.rexx
        "STRING : .*1E2"
        "RexxValue"
        "add")
add_levelc_lowered_tree_shape_test(levelc_slice45_constant_symbols_tree_shape
        rexx_src/levelc_slice45_constant_symbols.rexx
        "STRING : .*1X"
        "RexxValue"
        "concat")

# Bridge Connectivity Test
add_custom_command(
        OUTPUT stub_plugin.rxbin
        COMMAND $<TARGET_FILE:rxc> -n -o stub_plugin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/stub_plugin.crexx
        COMMAND $<TARGET_FILE:rxas> -o stub_plugin stub_plugin
        DEPENDS rxc rxas compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/stub_plugin.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling stub_plugin.crexx ..."
)

add_custom_target(stub_plugin_bin DEPENDS stub_plugin.rxbin)

add_executable(test_bridge src/test_bridge.c)
target_link_libraries(test_bridge PRIVATE rxvml rxpashim)
target_include_directories(test_bridge PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_bridge PRIVATE
        CREXX_TEST_LIBRARY_MODULE="${CMAKE_BINARY_DIR}/bin/library.rxbin"
        CREXX_TEST_RXCEXITS_MODULE="${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin")
add_dependencies(test_bridge stub_plugin_bin library)

add_executable(test_address_bridge src/test_address_bridge.c)
target_link_libraries(test_address_bridge PRIVATE rxvml rxpashim)
target_include_directories(test_address_bridge PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_address_bridge PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_CMS_ADDRESS_MODULE="${CMAKE_CURRENT_BINARY_DIR}/address_cms_provider.rxbin"
        CREXX_TEST_CMS_ADDRESS_HOST_MODULE="${CMAKE_CURRENT_BINARY_DIR}/address_cms_host.rxbin")

set(CREXX_CMS_DEMO_DIR "${CMAKE_SOURCE_DIR}/demos/cms")
set(CREXX_LLM_DEMO_DIR "${CMAKE_SOURCE_DIR}/demos/llm")
set(CREXX_NATIVE_KV_DEMO_DIR "${CMAKE_SOURCE_DIR}/demos/native/kv")
add_custom_command(
        OUTPUT address_cms_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_cms_provider ${CREXX_CMS_DEMO_DIR}/cms_address_environment.crexx
        COMMAND $<TARGET_FILE:rxas> address_cms_provider
        DEPENDS rxc rxas library compiler_exit_bin ${CREXX_CMS_DEMO_DIR}/cms_address_environment.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling cms_address_environment.crexx ..."
)

add_custom_command(
        OUTPUT address_cms_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_cms_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_cms_host.crexx
        COMMAND $<TARGET_FILE:rxas> address_cms_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_cms_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling address_cms_host.crexx ..."
)

add_custom_command(
        OUTPUT address_cms_demo.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_cms_demo ${CREXX_CMS_DEMO_DIR}/cms_address_demo.crexx
        COMMAND $<TARGET_FILE:rxas> address_cms_demo
        DEPENDS rxc rxas library compiler_exit_bin ${CREXX_CMS_DEMO_DIR}/cms_address_demo.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling cms_address_demo.crexx ..."
)

add_custom_target(address_cms_fixture_bin DEPENDS address_cms_provider.rxbin address_cms_host.rxbin address_cms_demo.rxbin)
add_custom_target(address_cms_demo_bin DEPENDS address_cms_provider.rxbin address_cms_demo.rxbin)
add_dependencies(test_address_bridge address_cms_fixture_bin library)

add_custom_command(
        OUTPUT address_llm_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_llm_provider ${CREXX_LLM_DEMO_DIR}/llm_address_environment.crexx
        COMMAND $<TARGET_FILE:rxas> address_llm_provider
        DEPENDS rxc rxas library rxfnsg compiler_exit_bin ${CREXX_LLM_DEMO_DIR}/llm_address_environment.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling llm_address_environment.crexx ..."
)

add_custom_command(
        OUTPUT address_llm_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_llm_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_llm_host.crexx
        COMMAND $<TARGET_FILE:rxas> address_llm_host
        DEPENDS rxc rxas library rxfnsg compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_llm_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling address_llm_host.crexx ..."
)

# The Level G providers now own bounded task/endpoint resources. Link this
# integration fixture through the normal library/classlib/rxfnsg toolchain so
# interface providers and teardown match deployed images.
add_custom_command(
        OUTPUT address_llm_linked.rxbin
        COMMAND $<TARGET_FILE:rxlink> -o address_llm_linked
                address_llm_provider.rxbin
                address_llm_host.rxbin
                ${CMAKE_BINARY_DIR}/bin/library.rxbin
                ${CMAKE_BINARY_DIR}/bin/classlib.rxbin
                ${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin
        DEPENDS rxlink address_llm_provider.rxbin address_llm_host.rxbin library classlib rxfnsg
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Linking Level G LLM ADDRESS fixture with its concurrency classes ..."
)

add_custom_target(address_llm_fixture_bin ALL DEPENDS address_llm_linked.rxbin)

add_custom_command(
        OUTPUT native_kv_address_demo.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o native_kv_address_demo ${CREXX_NATIVE_KV_DEMO_DIR}/kv_address_demo.crexx
        COMMAND $<TARGET_FILE:rxas> native_kv_address_demo
        DEPENDS rxc rxas library compiler_exit_bin ${CREXX_NATIVE_KV_DEMO_DIR}/kv_address_demo.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling kv_address_demo.crexx ..."
)

add_custom_target(native_kv_address_demo_bin DEPENDS native_kv_address_demo.rxbin)

add_executable(native_kv_address_demo ${CREXX_NATIVE_KV_DEMO_DIR}/kv_address_demo.c)
target_link_libraries(native_kv_address_demo PRIVATE rxvml rxpashim)
target_include_directories(native_kv_address_demo PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(native_kv_address_demo PRIVATE
        CREXX_NATIVE_KV_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_NATIVE_KV_DEMO_MODULE="${CMAKE_CURRENT_BINARY_DIR}/native_kv_address_demo.rxbin")
add_dependencies(native_kv_address_demo native_kv_address_demo_bin library)

add_custom_command(
        OUTPUT address_callback_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_callback_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_callback_host.crexx
        COMMAND $<TARGET_FILE:rxas> address_callback_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_callback_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling address_callback_host.crexx ..."
)

add_custom_target(address_callback_host_bin DEPENDS address_callback_host.rxbin)

add_executable(test_address_callback_host src/test_address_callback_host.c)
target_link_libraries(test_address_callback_host PRIVATE rxvml rxpashim)
target_include_directories(test_address_callback_host PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_address_callback_host PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_ADDRESS_CALLBACK_MODULE="${CMAKE_CURRENT_BINARY_DIR}/address_callback_host.rxbin")
add_dependencies(test_address_callback_host address_callback_host_bin library)

add_custom_command(
        OUTPUT e2_active_context.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o e2_active_context ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/e2_active_context.crexx
        COMMAND $<TARGET_FILE:rxas> e2_active_context
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/e2_active_context.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling e2_active_context.crexx ..."
)

add_custom_target(e2_active_context_bin DEPENDS e2_active_context.rxbin)

add_executable(test_e2_active_context src/test_e2_active_context.c)
target_link_libraries(test_e2_active_context PRIVATE rxvml rxpashim Threads::Threads)
target_include_directories(test_e2_active_context PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_e2_active_context PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_E2_ACTIVE_MODULE="${CMAKE_CURRENT_BINARY_DIR}/e2_active_context.rxbin"
        CREXX_TEST_E2_DIRECTORY_A="${CMAKE_SOURCE_DIR}"
        CREXX_TEST_E2_DIRECTORY_B="${CMAKE_BINARY_DIR}")
add_dependencies(test_e2_active_context e2_active_context_bin library)

add_custom_command(
        OUTPUT say_bytes_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o say_bytes_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/say_bytes_host.crexx
        COMMAND $<TARGET_FILE:rxas> say_bytes_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/say_bytes_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling say_bytes_host.crexx ...")
add_custom_target(say_bytes_host_bin DEPENDS say_bytes_host.rxbin)
add_executable(test_say_bytes_host src/test_say_bytes_host.c)
target_link_libraries(test_say_bytes_host PRIVATE rxvml rxpashim)
target_include_directories(test_say_bytes_host PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_say_bytes_host PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_SAY_BYTES_MODULE="${CMAKE_CURRENT_BINARY_DIR}/say_bytes_host.rxbin")
add_dependencies(test_say_bytes_host say_bytes_host_bin library)

add_custom_command(
        OUTPUT levelc_say_host_output.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o levelc_say_host_output ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_say_bytes.rexx
        COMMAND $<TARGET_FILE:rxas> levelc_say_host_output
        DEPENDS rxc rxas library rxfnsc compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_say_bytes.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling Level C SAY host output fixture ...")
add_custom_target(levelc_say_host_output_bin DEPENDS levelc_say_host_output.rxbin)
add_executable(test_levelc_say_host_output src/test_levelc_say_host_output.c)
target_link_libraries(test_levelc_say_host_output PRIVATE rxvml rxpashim)
target_include_directories(test_levelc_say_host_output PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_levelc_say_host_output PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_CLASSLIB_PATH="${CMAKE_BINARY_DIR}/bin/classlib"
        CREXX_TEST_RXFNSC_PATH="${CMAKE_BINARY_DIR}/bin/rxfnsc"
        CREXX_TEST_SAY_MODULE="${CMAKE_CURRENT_BINARY_DIR}/levelc_say_host_output.rxbin")
add_dependencies(test_levelc_say_host_output levelc_say_host_output_bin library rxfnsc)

add_custom_command(
        OUTPUT levelc_arg_host_entry.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o levelc_arg_host_entry ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_arg_host_entry.rexx
        COMMAND $<TARGET_FILE:rxas> levelc_arg_host_entry
        DEPENDS rxc rxas library rxfnsc compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_arg_host_entry.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling levelc_arg_host_entry.rexx ...")
add_custom_target(levelc_arg_host_entry_bin DEPENDS levelc_arg_host_entry.rxbin)
add_executable(test_levelc_arg_host_entry src/test_levelc_arg_host_entry.c)
target_link_libraries(test_levelc_arg_host_entry PRIVATE rxvml rxpashim)
target_include_directories(test_levelc_arg_host_entry PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_levelc_arg_host_entry PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_CLASSLIB_PATH="${CMAKE_BINARY_DIR}/bin/classlib"
        CREXX_TEST_RXFNSC_PATH="${CMAKE_BINARY_DIR}/bin/rxfnsc"
        CREXX_TEST_ARG_MODULE="${CMAKE_CURRENT_BINARY_DIR}/levelc_arg_host_entry.rxbin")
add_dependencies(test_levelc_arg_host_entry levelc_arg_host_entry_bin library rxfnsc)

add_custom_command(
        OUTPUT levelc_call_host_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> --levelc-routine -i ${CMAKE_BINARY_DIR}/bin -o levelc_call_host_provider ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_host_provider.rexx
        COMMAND $<TARGET_FILE:rxas> levelc_call_host_provider
        DEPENDS rxc rxas library rxfnsc compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_host_provider.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling Level C CALL host provider ...")
add_custom_command(
        OUTPUT levelc_call_host_entry.rxbin
        COMMAND $<TARGET_FILE:rxc> -s ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src -i ${CMAKE_BINARY_DIR}/bin -o levelc_call_host_entry ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_host_entry.rexx
        COMMAND $<TARGET_FILE:rxas> levelc_call_host_entry
        DEPENDS rxc rxas library rxfnsc compiler_exit_bin levelc_call_host_provider.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_call_host_entry.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling Level C CALL host entry ...")
add_custom_target(levelc_call_host_entry_bin DEPENDS levelc_call_host_provider.rxbin levelc_call_host_entry.rxbin)
add_executable(test_levelc_call_host_entry src/test_levelc_call_host_entry.c)
target_link_libraries(test_levelc_call_host_entry PRIVATE rxvml rxpashim)
target_include_directories(test_levelc_call_host_entry PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_levelc_call_host_entry PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_CLASSLIB_PATH="${CMAKE_BINARY_DIR}/bin/classlib"
        CREXX_TEST_RXFNSC_PATH="${CMAKE_BINARY_DIR}/bin/rxfnsc"
        CREXX_TEST_CALL_PROVIDER="${CMAKE_CURRENT_BINARY_DIR}/levelc_call_host_provider.rxbin"
        CREXX_TEST_CALL_ENTRY="${CMAKE_CURRENT_BINARY_DIR}/levelc_call_host_entry.rxbin")
add_dependencies(test_levelc_call_host_entry levelc_call_host_entry_bin library rxfnsc)

add_custom_command(
        OUTPUT levelc_return_status7.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o levelc_return_status7 ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_return_status7.rexx
        COMMAND $<TARGET_FILE:rxas> levelc_return_status7
        DEPENDS rxc rxas library rxfnsc compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_return_status7.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling Level C RETURN host entry ...")
add_custom_target(levelc_return_status7_bin DEPENDS levelc_return_status7.rxbin)
add_executable(test_levelc_return_host_entry src/test_levelc_return_host_entry.c)
target_link_libraries(test_levelc_return_host_entry PRIVATE rxvml rxpashim)
target_include_directories(test_levelc_return_host_entry PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_levelc_return_host_entry PRIVATE
        CREXX_TEST_BINDIR="${CMAKE_BINARY_DIR}/bin"
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library.rxbin"
        CREXX_TEST_CLASSLIB_PATH="${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
        CREXX_TEST_RXFNSC_PATH="${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
        CREXX_TEST_RETURN_MODULE="${CMAKE_CURRENT_BINARY_DIR}/levelc_return_status7.rxbin")
add_dependencies(test_levelc_return_host_entry levelc_return_status7_bin library rxfnsc)

add_custom_command(
        OUTPUT dynamic_load_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o dynamic_load_provider ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_provider.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_load_provider
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_provider.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic_load_provider.crexx ..."
)

add_custom_command(
        OUTPUT dynamic_load_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_BINARY_DIR} -i ${CMAKE_BINARY_DIR}/bin -o dynamic_load_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_host.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_load_host
        DEPENDS rxc rxas library compiler_exit_bin dynamic_load_provider.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic_load_host.crexx ..."
)

add_custom_target(dynamic_load_fixture_bin DEPENDS dynamic_load_provider.rxbin dynamic_load_host.rxbin)

add_executable(test_dynamic_load src/test_dynamic_load.c)
target_link_libraries(test_dynamic_load PRIVATE rxvml rxpashim)
target_include_directories(test_dynamic_load PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(test_dynamic_load PRIVATE
        CREXX_TEST_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_TEST_DYNAMIC_LOAD_HOST_MODULE="${CMAKE_CURRENT_BINARY_DIR}/dynamic_load_host.rxbin"
        CREXX_TEST_DYNAMIC_LOAD_PROVIDER_MODULE="${CMAKE_CURRENT_BINARY_DIR}/dynamic_load_provider.rxbin")
add_dependencies(test_dynamic_load dynamic_load_fixture_bin library)

add_custom_command(
        OUTPUT dynamic_interface_contract.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o dynamic_interface_contract ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_contract.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_interface_contract
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_contract.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic interface contract ..."
)

add_custom_command(
        OUTPUT dynamic_interface_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_BINARY_DIR} -i ${CMAKE_BINARY_DIR}/bin -o dynamic_interface_provider ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_provider.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_interface_provider
        DEPENDS rxc rxas library compiler_exit_bin dynamic_interface_contract.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_provider.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic interface provider ..."
)

add_custom_command(
        OUTPUT dynamic_interface_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_BINARY_DIR} -i ${CMAKE_BINARY_DIR}/bin -o dynamic_interface_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_host.crexx
        COMMAND $<TARGET_FILE:rxas> dynamic_interface_host
        DEPENDS rxc rxas library compiler_exit_bin dynamic_interface_contract.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic interface host ..."
)

# These files are direct CTest inputs, so the normal build must create them.
add_custom_target(dynamic_interface_fixture_bin ALL DEPENDS
        dynamic_interface_contract.rxbin
        dynamic_interface_provider.rxbin
        dynamic_interface_host.rxbin)
add_dependencies(dynamic_interface_fixture_bin crexx)

set(DYNAMIC_INTERFACE_DRIVER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver.cmake")
file(GENERATE OUTPUT "${DYNAMIC_INTERFACE_DRIVER_SCRIPT}" CONTENT
"file(REMOVE_RECURSE \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver\")
file(MAKE_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver\")
file(COPY \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_interface_host.crexx\" DESTINATION \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver\")
execute_process(
  COMMAND \"$<TARGET_FILE:crexx>\"
          \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver/dynamic_interface_host.crexx\"
          \"-i${CMAKE_CURRENT_BINARY_DIR}\"
          \"-l${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_contract.rxbin\"
          -args \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_provider.rxbin\"
  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_driver\"
  OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
set(all_output \"\${out}\${err}\")
if(NOT res EQUAL 0)
  message(FATAL_ERROR \"Late-loaded factory driver failed (\${res}):\\n\${all_output}\")
endif()
if(NOT all_output MATCHES \"PASS: explicit late-loaded interface factory\")
  message(FATAL_ERROR \"Late-loaded factory driver omitted PASS marker:\\n\${all_output}\")
endif()
")

add_executable(test_rxvml_utf_boundaries src/test_rxvml_utf_boundaries.c)
target_link_libraries(test_rxvml_utf_boundaries PRIVATE rxvml rxpashim)
target_include_directories(test_rxvml_utf_boundaries PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)

add_custom_command(
        OUTPUT crexxsaa_variables_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o crexxsaa_variables_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_variables_host.crexx
        COMMAND $<TARGET_FILE:rxas> crexxsaa_variables_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_variables_host.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling crexxsaa_variables_host.crexx ..."
)

add_custom_target(crexxsaa_variables_host_bin DEPENDS crexxsaa_variables_host.rxbin)

add_executable(test_crexxsaa_variables src/test_crexxsaa_variables.c)
target_link_libraries(test_crexxsaa_variables PRIVATE crexxsaa)
target_include_directories(test_crexxsaa_variables PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_compile_definitions(test_crexxsaa_variables PRIVATE
        CREXX_TEST_LOCATION="${CMAKE_BINARY_DIR}/bin"
        CREXX_TEST_LIBRARY_RXBIN="${CMAKE_BINARY_DIR}/bin/library.rxbin"
        CREXX_TEST_CREXXSAA_VARIABLE_MODULE="${CMAKE_CURRENT_BINARY_DIR}/crexxsaa_variables_host.rxbin")
add_dependencies(test_crexxsaa_variables crexxsaa_variables_host_bin library crexxsaa)

add_executable(test_crexxsaa_status src/test_crexxsaa_status.c)
target_link_libraries(test_crexxsaa_status PRIVATE crexxsaa)
target_include_directories(test_crexxsaa_status PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_compile_definitions(test_crexxsaa_status PRIVATE
        CREXX_TEST_LOCATION="${CMAKE_BINARY_DIR}/bin"
        CREXX_TEST_LIBRARY_RXBIN="${CMAKE_BINARY_DIR}/bin/library.rxbin"
        CREXX_TEST_RXC="$<TARGET_FILE:rxc>"
        CREXX_TEST_RXAS="$<TARGET_FILE:rxas>"
        CREXX_TEST_IMPORT_DIR="${CMAKE_BINARY_DIR}/bin"
        CREXX_TEST_CREXXSAA_EXIT7_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_exit7_hosted.crexx"
        CREXX_TEST_CREXXSAA_RETURN7_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_return7_hosted.crexx")
add_dependencies(test_crexxsaa_status library crexxsaa rxc rxas compiler_exit_bin)

add_custom_command(
        OUTPUT crexxsaa_arg_length_hosted.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o crexxsaa_arg_length_hosted ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_arg_length_hosted.crexx
        COMMAND $<TARGET_FILE:rxas> crexxsaa_arg_length_hosted
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_arg_length_hosted.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling crexxsaa_arg_length_hosted.crexx ...")
add_custom_target(crexxsaa_arg_length_hosted_bin DEPENDS crexxsaa_arg_length_hosted.rxbin)
add_executable(test_crexxsaa_arg_lengths src/test_crexxsaa_arg_lengths.c)
target_link_libraries(test_crexxsaa_arg_lengths PRIVATE crexxsaa)
target_include_directories(test_crexxsaa_arg_lengths PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_compile_definitions(test_crexxsaa_arg_lengths PRIVATE
        CREXX_TEST_LOCATION="${CMAKE_BINARY_DIR}/bin"
        CREXX_TEST_LIBRARY_RXBIN="${CMAKE_BINARY_DIR}/bin/library.rxbin"
        CREXX_TEST_RXC="$<TARGET_FILE:rxc>"
        CREXX_TEST_RXAS="$<TARGET_FILE:rxas>"
        CREXX_TEST_IMPORT_DIR="${CMAKE_BINARY_DIR}/bin"
        CREXX_TEST_ARG_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_arg_length_hosted.crexx"
        CREXX_TEST_ARG_MODULE="${CMAKE_CURRENT_BINARY_DIR}/crexxsaa_arg_length_hosted.rxbin"
        CREXX_TEST_ARG_CACHE="${CMAKE_CURRENT_BINARY_DIR}/crexxsaa-arg-length-cache")
add_dependencies(test_crexxsaa_arg_lengths crexxsaa_arg_length_hosted_bin crexxsaa library rxc rxas compiler_exit_bin)

add_executable(test_source_provenance src/test_source_provenance.c)
target_link_libraries(test_source_provenance PRIVATE rxclib)
target_include_directories(test_source_provenance PRIVATE
        ${CMAKE_SOURCE_DIR}/compiler
        ${CMAKE_BINARY_DIR}/compiler
        ${CMAKE_SOURCE_DIR}/assembler
        ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/utf8
        ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/binutils/include)

add_executable(test_levelc_tree_boundary src/test_levelc_tree_boundary.c)
target_link_libraries(test_levelc_tree_boundary PRIVATE rxclib)
target_include_directories(test_levelc_tree_boundary PRIVATE
        ${CMAKE_SOURCE_DIR}/compiler
        ${CMAKE_BINARY_DIR}/compiler
        ${CMAKE_SOURCE_DIR}/assembler
        ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/utf8
        ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/binutils/include)

if (ENABLE_PARSER_MODE)
    add_executable(test_highlight_cache src/test_highlight_cache.c)
    target_link_libraries(test_highlight_cache PRIVATE rxclib DSLSH::editor)
    target_compile_definitions(test_highlight_cache PRIVATE ENABLE_PARSER_MODE)
    target_include_directories(test_highlight_cache PRIVATE
            ${CMAKE_SOURCE_DIR}/compiler
            ${CMAKE_BINARY_DIR}/compiler
            ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/avl_tree
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/utf8
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/interpreter
            ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
            ${CMAKE_SOURCE_DIR}/binutils/include)

    add_executable(test_source_semantics src/test_source_semantics.c)
    target_link_libraries(test_source_semantics PRIVATE rxclib DSLSH::editor)
    target_compile_definitions(test_source_semantics PRIVATE ENABLE_PARSER_MODE)
    target_include_directories(test_source_semantics PRIVATE
            ${CMAKE_SOURCE_DIR}/compiler
            ${CMAKE_BINARY_DIR}/compiler
            ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/avl_tree
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/utf8
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/interpreter
            ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
            ${CMAKE_SOURCE_DIR}/binutils/include)

    add_executable(test_highlight_editor_diagnostics src/test_highlight_editor_diagnostics.c)
    target_link_libraries(test_highlight_editor_diagnostics PRIVATE rxclib DSLSH::editor)
    target_compile_definitions(test_highlight_editor_diagnostics PRIVATE
            ENABLE_PARSER_MODE
            RXC_SYNTAXHIGHLIGHT_CMD="$<TARGET_FILE:rxc> --syntaxhighlight")
    target_include_directories(test_highlight_editor_diagnostics PRIVATE
            ${CMAKE_SOURCE_DIR}/compiler
            ${CMAKE_BINARY_DIR}/compiler
            ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/avl_tree
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/utf8
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/interpreter
            ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
            ${CMAKE_SOURCE_DIR}/binutils/include)
    add_dependencies(test_highlight_editor_diagnostics rxc)
endif()

# Commented out until fix found for multiple calls to the VM
add_test(NAME BridgeConnectivity COMMAND test_bridge)
add_test(NAME rxvml_utf_boundaries COMMAND test_rxvml_utf_boundaries)
set_tests_properties(rxvml_utf_boundaries PROPERTIES LABELS "compiler;rxvml;utf")
add_test(NAME address_bridge COMMAND test_address_bridge)
set_tests_properties(address_bridge PROPERTIES
        LABELS "compiler;stage3"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_test(NAME address_cms_provider COMMAND
        ${CREXX_RXVM_EXECUTABLE}
        ${CMAKE_BINARY_DIR}/bin/library.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_provider.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_host.rxbin)
set_tests_properties(address_cms_provider PROPERTIES LABELS "compiler;stage3")
add_test(NAME address_cms_demo COMMAND
        ${CREXX_RXVM_EXECUTABLE}
        ${CMAKE_BINARY_DIR}/bin/library.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_provider.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_demo.rxbin)
set_tests_properties(address_cms_demo PROPERTIES LABELS "compiler;stage3")
add_test(NAME address_llm_provider COMMAND
        ${CREXX_RXVM_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/address_llm_linked.rxbin)
set_tests_properties(address_llm_provider PROPERTIES LABELS "compiler;stage3")
set_tests_properties(address_llm_provider PROPERTIES
        PASS_REGULAR_EXPRESSION "PASS: address llm host"
        FAIL_REGULAR_EXPRESSION "address llm host failures")
crexx_label_concurrency_tests(SP07 address_llm_provider)
add_test(NAME address_native_kv_demo COMMAND native_kv_address_demo)
set_tests_properties(address_native_kv_demo PROPERTIES LABELS "compiler;stage3")
add_test(NAME address_callback_host COMMAND test_address_callback_host)
set_tests_properties(address_callback_host PROPERTIES LABELS "compiler;stage3")
add_test(NAME e2_active_context_isolation COMMAND test_e2_active_context)
set_tests_properties(e2_active_context_isolation PROPERTIES LABELS "compiler;stage3")
crexx_label_concurrency_tests(SP01 e2_active_context_isolation)
add_test(NAME say_bytes_host_callbacks COMMAND test_say_bytes_host)
set_tests_properties(say_bytes_host_callbacks PROPERTIES LABELS "compiler;levelc")
add_test(NAME levelc_say_host_output COMMAND test_levelc_say_host_output)
set_tests_properties(levelc_say_host_output PROPERTIES LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "test_levelc_say_host_output")
add_test(NAME levelc_arg_host_entry COMMAND test_levelc_arg_host_entry)
set_tests_properties(levelc_arg_host_entry PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "test_levelc_arg_host_entry")
add_test(NAME levelc_call_host_entry COMMAND test_levelc_call_host_entry)
set_tests_properties(levelc_call_host_entry PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "test_levelc_call_host_entry")
add_test(NAME levelc_return_host_entry COMMAND test_levelc_return_host_entry)
set_tests_properties(levelc_return_host_entry PROPERTIES
        LABELS "compiler;levelc"
        CREXX_PREP_TARGETS "test_levelc_return_host_entry")
add_test(NAME dynamic_load COMMAND test_dynamic_load)
set_tests_properties(dynamic_load PROPERTIES LABELS "compiler;stage3")
foreach(_dynamic_interface_runner IN ITEMS rxvm)
    if(_dynamic_interface_runner STREQUAL "rxvm")
        set(_dynamic_interface_runner_executable "${CREXX_RXVM_EXECUTABLE}")
    else()
        set(_dynamic_interface_runner_executable "$<TARGET_FILE:${_dynamic_interface_runner}>")
    endif()
    add_test(NAME dynamic_interface_load_${_dynamic_interface_runner}
            COMMAND ${_dynamic_interface_runner_executable}
                    ${CMAKE_BINARY_DIR}/bin/library.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_contract.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_host.rxbin
                    -a ${CMAKE_CURRENT_BINARY_DIR}/dynamic_interface_provider.rxbin)
    set_tests_properties(dynamic_interface_load_${_dynamic_interface_runner} PROPERTIES
            LABELS "compiler;stage3;vm;interfaces;late-load"
            PASS_REGULAR_EXPRESSION "PASS: explicit late-loaded interface factory"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:")
endforeach()
add_test(NAME dynamic_interface_load_driver
        COMMAND ${CMAKE_COMMAND} -P ${DYNAMIC_INTERFACE_DRIVER_SCRIPT})
set_tests_properties(dynamic_interface_load_driver PROPERTIES
        LABELS "compiler;stage3;driver;interfaces;late-load"
        ENVIRONMENT "CREXX_HOME=${CMAKE_BINARY_DIR}")
add_test(NAME crexxsaa_variables COMMAND test_crexxsaa_variables)
set_tests_properties(crexxsaa_variables PROPERTIES
        LABELS "compiler;stage3"
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin")
add_test(NAME crexxsaa_status COMMAND test_crexxsaa_status)
add_test(NAME crexxsaa_arg_lengths COMMAND test_crexxsaa_arg_lengths)
set_tests_properties(crexxsaa_status PROPERTIES
        LABELS "compiler;stage3"
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin")
set_tests_properties(crexxsaa_arg_lengths PROPERTIES
        LABELS "compiler;stage3;levelc"
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin")
add_test(NAME source_provenance COMMAND test_source_provenance)
set_tests_properties(source_provenance PROPERTIES LABELS "compiler;stage2")
add_test(NAME levelc_tree_boundary COMMAND test_levelc_tree_boundary)
set_tests_properties(levelc_tree_boundary PROPERTIES LABELS "compiler;stage2;levelc")

add_test(NAME rxc_srcmap
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DWORK=${CMAKE_CURRENT_BINARY_DIR}/rxc_srcmap
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_rxc_srcmap.cmake)
set_tests_properties(rxc_srcmap PROPERTIES LABELS "compiler;diagnostics;srcmap")

add_test(NAME trace_event_metadata
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DWORK=${CMAKE_CURRENT_BINARY_DIR}/trace_event_metadata
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_trace_event_metadata.cmake)
set_tests_properties(trace_event_metadata PROPERTIES LABELS "compiler;stage4")

set(TRACE_STEM_SUGAR_RXVM_TIMEOUT 60)
set(_trace_stem_sugar_sanitizer_flags
        "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _trace_stem_sugar_build_type)
    string(APPEND _trace_stem_sugar_sanitizer_flags
            " ${CMAKE_C_FLAGS_${_trace_stem_sugar_build_type}}"
            " ${CMAKE_CXX_FLAGS_${_trace_stem_sugar_build_type}}"
            " ${CMAKE_EXE_LINKER_FLAGS_${_trace_stem_sugar_build_type}}"
            " ${CMAKE_SHARED_LINKER_FLAGS_${_trace_stem_sugar_build_type}}")
endif()
string(FIND "${_trace_stem_sugar_sanitizer_flags}" "-fsanitize=address" _trace_stem_sugar_asan_flag)
if(NOT _trace_stem_sugar_asan_flag EQUAL -1)
    set(TRACE_STEM_SUGAR_RXVM_TIMEOUT 60)
endif()

add_test(NAME trace_stem_sugar
        COMMAND ${CMAKE_COMMAND}
        -DRXC=$<TARGET_FILE:rxc>
        -DRXAS=$<TARGET_FILE:rxas>
        -DRXVM=${CREXX_RXVM_EXECUTABLE}
        -DBIN_DIR=${CMAKE_BINARY_DIR}/bin
        -DLIBRARY=${CMAKE_BINARY_DIR}/bin/library
        -DWORK=${CMAKE_CURRENT_BINARY_DIR}/trace_stem_sugar
        -DRXVM_TIMEOUT=${TRACE_STEM_SUGAR_RXVM_TIMEOUT}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_trace_stem_sugar.cmake)
set_tests_properties(trace_stem_sugar PROPERTIES LABELS "compiler;stage4")

if (ENABLE_PARSER_MODE)
    add_test(NAME highlight_cache COMMAND test_highlight_cache)
    set_tests_properties(highlight_cache PROPERTIES
            LABELS "compiler;syntax_highlighting;stage4"
            RUN_SERIAL TRUE)

    add_test(NAME source_semantics COMMAND test_source_semantics)
    set_tests_properties(source_semantics PROPERTIES
            LABELS "compiler;syntax_highlighting;stage5"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    add_test(NAME highlight_editor_diagnostics COMMAND test_highlight_editor_diagnostics)
    set_tests_properties(highlight_editor_diagnostics PROPERTIES LABELS "compiler;syntax_highlighting;stage5")
endif()

file(GLOB_RECURSE RXC_DIAGNOSTIC_CATALOG_SCAN_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/compiler/*.c"
        "${CMAKE_SOURCE_DIR}/compiler/*.h"
        "${CMAKE_SOURCE_DIR}/compiler/*.l"
        "${CMAKE_SOURCE_DIR}/compiler/*.y"
        "${CMAKE_SOURCE_DIR}/compiler/exits/*.crexx")
list(SORT RXC_DIAGNOSTIC_CATALOG_SCAN_SOURCES)
set(RXC_DIAGNOSTIC_CATALOG_SOURCE_LIST "${CMAKE_CURRENT_BINARY_DIR}/rxc_diagnostic_catalog_sources.txt")
file(WRITE "${RXC_DIAGNOSTIC_CATALOG_SOURCE_LIST}" "")
foreach(_rxc_diagnostic_catalog_source IN LISTS RXC_DIAGNOSTIC_CATALOG_SCAN_SOURCES)
    file(APPEND "${RXC_DIAGNOSTIC_CATALOG_SOURCE_LIST}" "${_rxc_diagnostic_catalog_source}\n")
endforeach()

set(RXC_DIAGNOSTIC_CATALOG_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_diagnostic_catalogs.cmake")
set(RXC_DIAGNOSTIC_CATALOG_CONTENT "")
string(APPEND RXC_DIAGNOSTIC_CATALOG_CONTENT
       "set(tool_base \"${CMAKE_CURRENT_BINARY_DIR}/check_diagnostic_catalogs\")\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"\${tool_base}\" \"${CMAKE_SOURCE_DIR}/tools/check_diagnostic_catalogs.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n"
       "if(NOT res EQUAL 0)\n"
       "  message(FATAL_ERROR \"diagnostic catalogue checker compile failed with \${res}:\\n\${out}\${err}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"\${tool_base}.rxbin\" \"\${tool_base}\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n"
       "if(NOT res EQUAL 0)\n"
       "  message(FATAL_ERROR \"diagnostic catalogue checker assembly failed with \${res}:\\n\${out}\${err}\")\n"
       "endif()\n"
       "execute_process(\n"
       "  COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${tool_base}.rxbin\" -a \"${CMAKE_SOURCE_DIR}/messages/diagnostics.en_GB.msg\" \"${RXC_DIAGNOSTIC_CATALOG_SOURCE_LIST}\" \"2\" \"${CMAKE_SOURCE_DIR}/messages/diagnostics.de_DE.msg\" \"${CMAKE_SOURCE_DIR}/messages/diagnostics.nl_NL.msg\" \"1\" \"${CMAKE_SOURCE_DIR}/messages/diagnostics.en_US.msg\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n"
       "if(NOT res EQUAL 0)\n"
       "  message(FATAL_ERROR \"diagnostic catalogue checker failed with \${res}:\\n\${out}\${err}\")\n"
       "endif()\n"
       "message(STATUS \"\${out}\")\n")
file(GENERATE OUTPUT "${RXC_DIAGNOSTIC_CATALOG_SCRIPT}" CONTENT "${RXC_DIAGNOSTIC_CATALOG_CONTENT}")
add_test(NAME rxc_diagnostic_catalogs
         COMMAND ${CMAKE_COMMAND} -P "${RXC_DIAGNOSTIC_CATALOG_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_diagnostic_catalogs PROPERTIES LABELS "compiler;diagnostics")

set(RXC_DIAGNOSTIC_LOCALIZATION_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_diagnostic_localization.cmake")
set(RXC_DIAGNOSTIC_LOCALIZATION_CONTENT "")
string(APPEND RXC_DIAGNOSTIC_LOCALIZATION_CONTENT
       "set(unused_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_warning.crexx\")\n"
       "set(address_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/test_implicit_address.crexx\")\n"
       "set(missing_end_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_02_syntax.crexx\")\n"
       "set(binary_memory_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/binary_memory_invalid_storage.crexx\")\n"
       "set(legacy_arg_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/legacy_arg_call_removed.crexx\")\n"
       "set(top_level_constant_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/top_level_constant_removed.crexx\")\n"
       "set(literal_continuation_src \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/literal_continuation_suffix_error.crexx\")\n"
       "function(run_diag label locale source suffix expected unexpected)\n"
       "  execute_process(\n"
       "    COMMAND \"${CMAKE_COMMAND}\" -E env \"CREXX_DIAGNOSTICS=localized\" \"CREXX_DIAGNOSTIC_LOCALE=\${locale}\" \"$<TARGET_FILE:rxc>\" -n -o \"\${label}_\${suffix}_diag\" \"\${source}\"\n"
       "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "    OUTPUT_VARIABLE out\n"
       "    ERROR_VARIABLE err\n"
       "    RESULT_VARIABLE res\n"
       "    ENCODING UTF-8\n"
       "  )\n"
       "  set(full_out \"\${out}\${err}\")\n"
       "  if(NOT res EQUAL 0)\n"
       "    message(FATAL_ERROR \"\${label} localized diagnostic run failed with \${res}:\\n\${full_out}\")\n"
       "  endif()\n"
       "  string(FIND \"\${full_out}\" \"\${expected}\" expected_pos)\n"
       "  if(expected_pos EQUAL -1)\n"
       "    message(FATAL_ERROR \"\${label} localized diagnostic missing '\${expected}':\\n\${full_out}\")\n"
       "  endif()\n"
       "  if(NOT \"\${unexpected}\" STREQUAL \"\")\n"
       "    string(FIND \"\${full_out}\" \"\${unexpected}\" unexpected_pos)\n"
       "    if(NOT unexpected_pos EQUAL -1)\n"
       "      message(FATAL_ERROR \"\${label} localized diagnostic contained unexpected '\${unexpected}':\\n\${full_out}\")\n"
       "    endif()\n"
       "  endif()\n"
       "  string(REPLACE \"\\n\" \" | \" sample \"\${full_out}\")\n"
       "  message(STATUS \"\${label}/\${suffix}: \${sample}\")\n"
       "endfunction()\n"
       "function(run_diag_fail label locale source suffix expected unexpected)\n"
       "  execute_process(\n"
       "    COMMAND \"${CMAKE_COMMAND}\" -E env \"CREXX_DIAGNOSTICS=localized\" \"CREXX_DIAGNOSTIC_LOCALE=\${locale}\" \"$<TARGET_FILE:rxc>\" -n -o \"\${label}_\${suffix}_diag\" \"\${source}\"\n"
       "    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "    OUTPUT_VARIABLE out\n"
       "    ERROR_VARIABLE err\n"
       "    RESULT_VARIABLE res\n"
       "    ENCODING UTF-8\n"
       "  )\n"
       "  set(full_out \"\${out}\${err}\")\n"
       "  if(res EQUAL 0)\n"
       "    message(FATAL_ERROR \"\${label} localized diagnostic unexpectedly succeeded:\\n\${full_out}\")\n"
       "  endif()\n"
       "  string(FIND \"\${full_out}\" \"\${expected}\" expected_pos)\n"
       "  if(expected_pos EQUAL -1)\n"
       "    message(FATAL_ERROR \"\${label} localized diagnostic missing '\${expected}':\\n\${full_out}\")\n"
       "  endif()\n"
       "  if(NOT \"\${unexpected}\" STREQUAL \"\")\n"
       "    string(FIND \"\${full_out}\" \"\${unexpected}\" unexpected_pos)\n"
       "    if(NOT unexpected_pos EQUAL -1)\n"
       "      message(FATAL_ERROR \"\${label} localized diagnostic contained unexpected '\${unexpected}':\\n\${full_out}\")\n"
       "    endif()\n"
       "  endif()\n"
       "  string(REPLACE \"\\n\" \" | \" sample \"\${full_out}\")\n"
       "  message(STATUS \"\${label}/\${suffix}: \${sample}\")\n"
       "endfunction()\n"
       "run_diag(\"en_GB\" \"en_GB\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import is recognised but not used.\" \"recognized\")\n"
       "run_diag(\"en_US\" \"en_US\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import is recognized but not used.\" \"recognised\")\n"
       "run_diag(\"de_DE\" \"de_DE\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import wird erkannt, aber nicht verwendet.\" \"recognised\")\n"
       "run_diag(\"nl_NL\" \"nl_NL\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import wordt herkend, maar niet gebruikt.\" \"recognised\")\n"
       "run_diag(\"en_GB\" \"en_GB\" \"\${address_src}\" \"address\" \"IMPLICIT_ADDRESS: Implicit ADDRESS command\" \"\")\n"
       "run_diag(\"en_US\" \"en_US\" \"\${address_src}\" \"address\" \"IMPLICIT_ADDRESS: Implicit ADDRESS command\" \"\")\n"
       "run_diag(\"de_DE\" \"de_DE\" \"\${address_src}\" \"address\" \"IMPLICIT_ADDRESS: Impliziter ADDRESS-Befehl\" \"\")\n"
       "run_diag(\"nl_NL\" \"nl_NL\" \"\${address_src}\" \"address\" \"IMPLICIT_ADDRESS: Impliciete ADDRESS-opdracht\" \"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${missing_end_src}\" \"missing_end\" \"MISSING_END: Missing END\" \"#MISSING_END, \\\"do\\\"\")\n"
       "run_diag_fail(\"en_US\" \"en_US\" \"\${missing_end_src}\" \"missing_end\" \"MISSING_END: Missing END\" \"#MISSING_END, \\\"do\\\"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${missing_end_src}\" \"missing_end\" \"MISSING_END: Fehlendes END\" \"#MISSING_END, \\\"do\\\"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${missing_end_src}\" \"missing_end\" \"MISSING_END: Ontbrekende END\" \"#MISSING_END, \\\"do\\\"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${binary_memory_src}\" \"binary_memory\" \"BINARY_MEMORY_INVALID_STORAGE_TYPE: Binary memory access has an unsupported storage type.\" \"#BINARY_MEMORY_INVALID_STORAGE_TYPE, \\\"<at..u64>(0) data\\\"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${binary_memory_src}\" \"binary_memory\" \"BINARY_MEMORY_INVALID_STORAGE_TYPE: Binärer Speicherzugriff verwendet einen nicht unterstützten Speichertyp.\" \"#BINARY_MEMORY_INVALID_STORAGE_TYPE, \\\"<at..u64>(0) data\\\"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${binary_memory_src}\" \"binary_memory\" \"BINARY_MEMORY_INVALID_STORAGE_TYPE: Binaire-geheugentoegang gebruikt een niet-ondersteund opslagtype.\" \"#BINARY_MEMORY_INVALID_STORAGE_TYPE, \\\"<at..u64>(0) data\\\"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${legacy_arg_src}\" \"legacy_arg\" \"LEGACY_ARG_CALL_SYNTAX: ARG call syntax has been removed; use arg[], arg[n], or <argexists>(n).\" \"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${legacy_arg_src}\" \"legacy_arg\" \"LEGACY_ARG_CALL_SYNTAX: ARG-Aufrufsyntax wurde entfernt; verwenden Sie arg[], arg[n] oder <argexists>(n).\" \"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${legacy_arg_src}\" \"legacy_arg\" \"LEGACY_ARG_CALL_SYNTAX: ARG-aanroepsyntaxis is verwijderd; gebruik arg[], arg[n] of <argexists>(n).\" \"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${top_level_constant_src}\" \"top_level_constant\" \"CONSTANT_OUTSIDE_ROUTINE: Constant declarations must be inside an explicit routine body.\" \"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${top_level_constant_src}\" \"top_level_constant\" \"CONSTANT_OUTSIDE_ROUTINE: Konstantendeklarationen müssen sich in einem expliziten Routinenrumpf befinden.\" \"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${top_level_constant_src}\" \"top_level_constant\" \"CONSTANT_OUTSIDE_ROUTINE: Constantdeclaraties moeten binnen een expliciete routinebody staan.\" \"\")\n"
       "run_diag_fail(\"en_GB\" \"en_GB\" \"\${literal_continuation_src}\" \"literal_continuation\" \"LITERAL_CONTINUATION_SUFFIX: Only the final chunk of a continued string literal may use an x or b suffix.\" \"\")\n"
       "run_diag_fail(\"de_DE\" \"de_DE\" \"\${literal_continuation_src}\" \"literal_continuation\" \"LITERAL_CONTINUATION_SUFFIX: Nur das letzte Stück eines fortgesetzten Zeichenfolgenliterals darf ein x- oder b-Suffix verwenden.\" \"\")\n"
       "run_diag_fail(\"nl_NL\" \"nl_NL\" \"\${literal_continuation_src}\" \"literal_continuation\" \"LITERAL_CONTINUATION_SUFFIX: Alleen het laatste deel van een voortgezet tekenreeksliteral mag een x- of b-suffix gebruiken.\" \"\")\n"
       "run_diag(\"fallback\" \"fr_FR\" \"\${unused_src}\" \"unused\" \"UNUSED_IMPORT: Import is recognised but not used.\" \"recognized\")\n")
file(GENERATE OUTPUT "${RXC_DIAGNOSTIC_LOCALIZATION_SCRIPT}" CONTENT "${RXC_DIAGNOSTIC_LOCALIZATION_CONTENT}")
add_test(NAME rxc_diagnostic_localization
         COMMAND ${CMAKE_COMMAND} -P "${RXC_DIAGNOSTIC_LOCALIZATION_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_diagnostic_localization PROPERTIES LABELS "compiler;diagnostics")

set(RXC_DIAGNOSTIC_RAW_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/rxc_diagnostic_raw.cmake")
set(RXC_DIAGNOSTIC_RAW_CONTENT "")
string(APPEND RXC_DIAGNOSTIC_RAW_CONTENT
       "execute_process(\n"
       "  COMMAND \"${CMAKE_COMMAND}\" -E env \"CREXX_DIAGNOSTICS=raw\" \"$<TARGET_FILE:rxc>\" -o raw_diag \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/unused_import_warning.crexx\"\n"
       "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
       "  OUTPUT_VARIABLE out\n"
       "  ERROR_VARIABLE err\n"
       "  RESULT_VARIABLE res\n"
       ")\n"
       "set(full_out \"\${out}\${err}\")\n"
       "if(NOT res EQUAL 0)\n"
       "  message(FATAL_ERROR \"raw diagnostic run failed with \${res}:\\n\${full_out}\")\n"
       "endif()\n"
       "string(FIND \"\${full_out}\" \"UNUSED_IMPORT\" code_pos)\n"
       "if(code_pos EQUAL -1)\n"
       "  message(FATAL_ERROR \"raw diagnostic missing code:\\n\${full_out}\")\n"
       "endif()\n"
       "string(FIND \"\${full_out}\" \"recognised\" text_pos)\n"
       "if(NOT text_pos EQUAL -1)\n"
       "  message(FATAL_ERROR \"raw diagnostic unexpectedly used localized text:\\n\${full_out}\")\n"
       "endif()\n")
file(GENERATE OUTPUT "${RXC_DIAGNOSTIC_RAW_SCRIPT}" CONTENT "${RXC_DIAGNOSTIC_RAW_CONTENT}")
add_test(NAME rxc_diagnostic_raw
         COMMAND ${CMAKE_COMMAND} -P "${RXC_DIAGNOSTIC_RAW_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxc_diagnostic_raw PROPERTIES LABELS "compiler;diagnostics")

crexx_label_smoke_tests(
        rxc_help_syntaxhighlight
        rxas_invalid_diagnostic
        rxc_function_lookup_diagnostic
        rxc_constants_levelb
        01_assign_noopt
        03_calc_opt
        07_loops_noopt
        08_select_opt
        10_procs_noopt
        15_imports_opt
        err_01_type_fail
        err_02_syntax_fail
        err_06_missing_method_fail
        unused_import_warn
        repro_opt_args_run_noopt
        basic_arrays_run_noopt
        13_stems_run_noopt
        optional_arg_presence_copy_run_noopt
        interface_runtime_dispatch_multi_run_noopt
        interface_source_import_run_noopt
        source_import_optional_factory_arg
        source_import_contract_stub_implements
        syntaxhighlight_trace_keywords
        levelc_slice1_pool_say
        levelc_slice6_expressions
        address_bridge
        address_callback_host
        dynamic_load
        crexxsaa_variables
        crexxsaa_status
        source_provenance
        rxc_srcmap
        trace_stem_sugar
        source_semantics
        rxc_diagnostic_catalogs
        PREP_TARGETS crexx_test_driver rxc rxas rxvm library classlib rxfnsc
)

# The selected compiler smoke cases exercise the golden driver, compiler and
# assembler scripts, runtime execution, and the two Level C library surfaces.
# Their direct native harnesses own any additional generated fixtures.
crexx_register_test_prep_targets(
        TESTS
            address_bridge
            address_callback_host
            dynamic_load
            crexxsaa_variables
            crexxsaa_status
            crexxsaa_arg_lengths
            source_provenance levelc_tree_boundary
        TARGETS
            test_address_bridge
            test_address_callback_host
            test_dynamic_load
            test_crexxsaa_variables
            test_crexxsaa_status
            test_crexxsaa_arg_lengths
            test_source_provenance test_levelc_tree_boundary)
if(ENABLE_PARSER_MODE)
    crexx_register_test_prep_targets(
            TESTS rxc_help_syntaxhighlight syntaxhighlight_trace_keywords
            TARGETS parser_tester)
    crexx_register_test_prep_targets(
            TESTS source_semantics
            TARGETS test_source_semantics library compiler_exit_bin)
    crexx_register_test_prep_targets(
            TESTS highlight_cache
            TARGETS test_highlight_cache library compiler_exit_bin)
endif()

function(crexx_force_raw_diagnostics_for_directory_tests)
    get_property(_crexx_tests DIRECTORY PROPERTY TESTS)
    foreach(_crexx_test IN LISTS _crexx_tests)
        if(_crexx_test MATCHES "^rxc_diagnostic_(catalogs|localization|raw)$")
            continue()
        endif()
        get_test_property(${_crexx_test} ENVIRONMENT _crexx_env)
        if(NOT _crexx_env OR _crexx_env MATCHES "NOTFOUND$")
            set(_crexx_env)
        endif()
        list(APPEND _crexx_env "CREXX_DIAGNOSTICS=raw")
        set_tests_properties(${_crexx_test} PROPERTIES ENVIRONMENT "${_crexx_env}")
    endforeach()
endfunction()

crexx_force_raw_diagnostics_for_directory_tests()

crexx_finalize_directory_qa_tiers()
