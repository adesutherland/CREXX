cmake_minimum_required(VERSION 3.24)

project(CompilerTests C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

# The test driver itself must be compiled as strict C90
add_executable(crexx_test_driver src/test_driver.c)

# Ensure the driver is built when tests are built
add_dependencies(crexx_test_driver rxc)

if (ENABLE_PARSER_MODE)
    add_executable(crexx_fake_editor src/fake_editor.c)
    target_link_libraries(crexx_fake_editor DSLSH::editor)

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
        set_tests_properties(${NAME} PROPERTIES LABELS "compiler;syntax_highlighting;stage0")
    endfunction()
endif()

enable_testing()
include(${CMAKE_SOURCE_DIR}/cmake/CrexxLinkedRuntime.cmake)

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
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${NAME}.rxbin\" ${DEP_BINS} \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" OUTPUT_VARIABLE vm_out RESULT_VARIABLE res)\n")
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

add_crexx_robustness_test(set1_main set1_main.rexx set1_lib.rexx)
add_crexx_robustness_test(set2_main set2_main.rexx set2_lib.rexx)
add_crexx_robustness_test(set3_main set3_main.rexx set3_lib.rexx)
add_crexx_robustness_test(set4_main set4_main.rexx set4_mid.rexx set4_base.rexx)
add_crexx_robustness_test(set6_main set6_main.rexx Counter.rexx CounterIterator.rexx)

# Set 5 - Encapsulation (Expected to fail compilation)
set(SCRIPT_FILE_SET5 "${CMAKE_CURRENT_BINARY_DIR}/run_robustness_set5.cmake")
set(SCRIPT_CONTENT_SET5 "")
string(APPEND SCRIPT_CONTENT_SET5 "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness\" -o \"set5_main\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/set5_main.rexx\" RESULT_VARIABLE res)\n")
string(APPEND SCRIPT_CONTENT_SET5 "if(res EQUAL 0)\n  message(FATAL_ERROR \"Expected compilation to fail, but it succeeded\")\nendif()\n")
file(GENERATE OUTPUT "${SCRIPT_FILE_SET5}" CONTENT "${SCRIPT_CONTENT_SET5}")

add_test(NAME robustness_set5_encapsulation
         COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE_SET5}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Example tests
add_crexx_compiler_test(01_assign rexx_src/01_assign.rexx)
add_crexx_compiler_test(02_flow rexx_src/02_flow.rexx)
add_crexx_compiler_test(03_calc rexx_src/03_calc.rexx)
add_crexx_compiler_test(04_literals rexx_src/04_literals.rexx)
add_crexx_compiler_test(05_ops_common rexx_src/05_ops_common.rexx)
add_crexx_compiler_test(05_ops_classic rexx_src/05_ops_classic.rexx)
add_crexx_compiler_test(strict_compare_codegen rexx_src/strict_compare_codegen.rexx)
add_crexx_compiler_test(address_expose rexx_src/address_expose.rexx)
add_crexx_compiler_test(div_ops_common rexx_src/div_ops_common.rexx)
add_crexx_compiler_test(div_ops_classic rexx_src/div_ops_classic.rexx)
add_crexx_compiler_test(06_logic rexx_src/06_logic.rexx)
add_crexx_compiler_test(07_loops rexx_src/07_loops.rexx)
add_crexx_compiler_test(08_select rexx_src/08_select.rexx)
add_crexx_compiler_test(test_select_classic rexx_src/test_select_classic.rexx)
add_crexx_compiler_test(test_select_c rexx_src/test_select_c.rexx)
add_crexx_compiler_test(09_nested rexx_src/09_nested.rexx)
add_crexx_compiler_test(10_procs rexx_src/10_procs.rexx)
add_crexx_compiler_test(11_scope rexx_src/11_scope.rexx)
add_crexx_compiler_test(12_arrays_lvlb rexx_src/12_arrays_lvlb.rexx)
add_crexx_compiler_test(13_stems rexx_src/13_stems.rexx)
add_crexx_compiler_test(14_strings rexx_src/14_strings.rexx)
add_crexx_compiler_test(17_string_arrays rexx_src/17_string_arrays.rexx)
add_crexx_compiler_dep_test(18_array_class rexx_src/18_array_class_main.rexx rexx_src/array_class_lib.rexx)
add_crexx_compiler_test(val_magic_01 rexx_src/val_magic_01.rexx)
add_crexx_compiler_test(val_magic_02 rexx_src/val_magic_02.rexx)
add_crexx_compiler_test(opt_decimal_safety rexx_src/opt_decimal_safety.rexx)
add_crexx_compiler_test(opt_numeric_safety rexx_src/opt_numeric_safety.rexx)
add_crexx_compiler_dep_test(15_imports rexx_src/15_imports.rexx rexx_src/mymath.rexx)
add_crexx_compiler_dep_test(class_import rexx_src/class_import_main.rexx rexx_src/class_import_lib.rexx)
add_crexx_parsing_test(16_classes rexx_src/16_classes.rexx)

# Negative tests
add_crexx_compiler_fail_test(err_01_type rexx_src/err_01_type.rexx err_01_type.txt)
add_crexx_compiler_fail_test(err_02_syntax rexx_src/err_02_syntax.rexx err_02_syntax.txt)
add_crexx_compiler_fail_test(err_04_getter rexx_src/err_04_getter.rexx err_04_getter.txt)
add_crexx_compiler_fail_test(err_05_setter rexx_src/err_05_setter.rexx err_05_setter.txt)
add_crexx_compiler_fail_test(err_06_missing_method rexx_src/err_06_missing_method.rexx err_06_missing_method.txt)
add_crexx_compiler_fail_test(err_symbol_boundary_whitespace rexx_src/err_symbol_boundary_whitespace.rexx err_symbol_boundary_whitespace.txt)
add_crexx_compiler_fail_test(err_loop_class rexx_src/err_loop_class.rexx err_loop_class.txt)
add_crexx_compiler_fail_test(err_select_no_when rexx_src/err_select_no_when.rexx err_select_no_when.txt)
add_crexx_compiler_fail_test(err_select_c_no_when rexx_src/err_select_c_no_when.rexx err_select_c_no_when.txt)
add_crexx_compiler_fail_test(err_class_assign rexx_src/err_class_assign.rexx err_class_assign.txt)
add_crexx_compiler_fail_test(err_address_commandless_sandbox rexx_src/err_address_commandless_sandbox.rexx err_address_commandless_sandbox.txt)
add_crexx_compiler_fail_test(err_block_expr_type_mismatch rexx_src/err_block_expr_type_mismatch.rexx err_block_expr_type_mismatch.txt)
add_crexx_compiler_fail_test(err_block_expr_no_leave rexx_src/err_block_expr_no_leave.rexx err_block_expr_no_leave.txt)
add_crexx_compiler_fail_test(err_block_expr_leave_plain rexx_src/err_block_expr_leave_plain.rexx err_block_expr_leave_plain.txt)
add_crexx_compiler_fail_test(err_block_expr_missing_end rexx_src/err_block_expr_missing_end.rexx err_block_expr_missing_end.txt)
add_crexx_compiler_fail_test(err_block_expr_incomplete rexx_src/err_block_expr_incomplete.rexx err_block_expr_incomplete.txt)
add_crexx_compiler_fail_test(err_leave_with_not_in_block rexx_src/err_leave_with_not_in_block.rexx err_leave_with_not_in_block.txt)
add_crexx_compiler_fail_test(interface_missing_impl rexx_src/interface_missing_impl.rexx interface_missing_impl.txt)
add_crexx_compiler_fail_test(interface_match_bad_args rexx_src/interface_match_bad_args.rexx interface_match_bad_args.txt)
add_crexx_compiler_fail_test(interface_match_without_factory rexx_src/interface_match_without_factory.rexx interface_match_without_factory.txt)
add_crexx_compiler_fail_test(interface_default_redefine rexx_src/interface_default_redefine.rexx interface_default_redefine.txt)
add_crexx_compiler_fail_test(interface_factory_return_type rexx_src/interface_factory_return_type.rexx interface_factory_return_type.txt)
add_crexx_compiler_fail_test(qualified_namespace_missing_import rexx_src/qualified_namespace_missing_import.rexx qualified_namespace_missing_import.txt)
add_crexx_compiler_fail_test(type_ops_reserved_keyword rexx_src/type_ops_reserved_keyword.rexx type_ops_reserved_keyword.txt)
add_test(NAME err_address_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_address_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_address_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_address_exit_disabled.rexx)

add_test(NAME err_parse_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_parse_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_parse_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_parse_exit_disabled.rexx)

add_test(NAME err_parse_inline_then_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_parse_inline_then_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_parse_inline_then_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_parse_inline_then_exit_disabled.rexx)

add_test(NAME err_trace_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_trace_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_trace_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_trace_exit_disabled.rexx)

add_crexx_compiler_fail_test(err_parse_missing_arguments rexx_src/err_parse_missing_arguments.rexx err_parse_missing_arguments.txt)

# Warning tests
add_crexx_compiler_warning_test(loop rexx_src/loop.rexx loop.txt)

# Regression tests for rxpp issue
add_crexx_compiler_test(repro_dotted_index rexx_src/repro_dotted_index.rexx)
add_crexx_compiler_test(repro_expand_until rexx_src/repro_expand_until.rexx)
add_crexx_compiler_test(repro_array_arg rexx_src/repro_array_arg.rexx)
add_crexx_compiler_test(repro_opt_args rexx_src/repro_opt_args.rexx)
add_crexx_compiler_test(arg_semantics_scalar rexx_src/arg_semantics_scalar.rexx)
add_crexx_compiler_test(arg_semantics_array rexx_src/arg_semantics_array.rexx)
add_crexx_compiler_test(arg_semantics_object rexx_src/arg_semantics_object.rexx)
add_crexx_compiler_test(object_reference_regression rexx_src/object_reference_regression.rexx)
add_crexx_compiler_test(basic_loops rexx_src/basic_loops.rexx)
add_crexx_compiler_test(basic_strings rexx_src/basic_strings.rexx)
add_crexx_compiler_test(basic_arrays rexx_src/basic_arrays.rexx)
add_crexx_compiler_test(block_expr_basic rexx_src/block_expr_basic.rexx)
add_crexx_compiler_test(block_expr_short_circuit rexx_src/block_expr_short_circuit.rexx)
add_crexx_compiler_test(inline_test1 rexx_src/inline_test1.rexx)
add_crexx_compiler_test(inline_test2 rexx_src/inline_test2.rexx)
add_crexx_compiler_test(inline_test_call rexx_src/inline_test_call.rexx)
add_crexx_compiler_test(inline_test_expr rexx_src/inline_test_expr.rexx)
add_crexx_compiler_test(inline_test_expr_negative rexx_src/inline_test_expr_negative.rexx)
add_crexx_compiler_test(inline_test_nested_call_expr rexx_src/inline_test_nested_call_expr.rexx)
add_crexx_compiler_test(inline_test_concat_expr rexx_src/inline_test_concat_expr.rexx)
add_crexx_compiler_test(inline_test_say_expr rexx_src/inline_test_say_expr.rexx)
add_crexx_compiler_test(inline_test_say_expr_negative rexx_src/inline_test_say_expr_negative.rexx)
add_crexx_compiler_test(inline_test_return_expr rexx_src/inline_test_return_expr.rexx)
add_crexx_compiler_test(inline_test_unary_expr rexx_src/inline_test_unary_expr.rexx)
add_crexx_compiler_test(inline_test_compare_expr rexx_src/inline_test_compare_expr.rexx)
add_crexx_compiler_test(inline_test_call_arg_expr rexx_src/inline_test_call_arg_expr.rexx)
add_crexx_compiler_test(inline_test_call_like_arg_expr rexx_src/inline_test_call_like_arg_expr.rexx)
add_crexx_compiler_test(inline_test_short_circuit_expr rexx_src/inline_test_short_circuit_expr.rexx)
add_crexx_compiler_test(inline_test_composed_expr_contexts rexx_src/inline_test_composed_expr_contexts.rexx)
add_crexx_compiler_test(inline_test_stateful_assembler rexx_src/inline_test_stateful_assembler.rexx)
add_crexx_compiler_test(inline_test_condition_contexts rexx_src/inline_test_condition_contexts.rexx)
add_crexx_compiler_test(inline_test_loop_bound_expr rexx_src/inline_test_loop_bound_expr.rexx)
add_crexx_compiler_test(inline_test_member_receiver_expr rexx_src/inline_test_member_receiver_expr.rexx)
add_crexx_compiler_test(inline_test_named_factory_selector rexx_src/inline_test_named_factory_selector.rexx)
add_crexx_compiler_test(inline_test_ref_opt rexx_src/inline_test_ref_opt.rexx)
add_crexx_compiler_test(inline_test_ref_indexed rexx_src/inline_test_ref_indexed.rexx)
add_crexx_compiler_test(inline_test_ref_stem rexx_src/inline_test_ref_stem.rexx)
add_crexx_compiler_test(inline_test_ref_computed rexx_src/inline_test_ref_computed.rexx)
add_crexx_compiler_test(inline_test_ref_vararg rexx_src/inline_test_ref_vararg.rexx)
add_crexx_compiler_test(inline_test_ref_vararg_runtime rexx_src/inline_test_ref_vararg_runtime.rexx)
add_crexx_compiler_test(inline_test_ref_negative rexx_src/inline_test_ref_negative.rexx)
add_crexx_compiler_test(inline_test_bool_expr_negative rexx_src/inline_test_bool_expr_negative.rexx)
add_crexx_compiler_test(inline_test_vararg rexx_src/inline_test_vararg.rexx)
add_crexx_compiler_test(inline_test_vararg_dynamic rexx_src/inline_test_vararg_dynamic.rexx)
add_crexx_compiler_test(inline_test_vararg_dynamic_loop rexx_src/inline_test_vararg_dynamic_loop.rexx)
add_crexx_compiler_test(inline_test_vararg_negative rexx_src/inline_test_vararg_negative.rexx)
add_crexx_compiler_test(inline_test_exposed_nested_call rexx_src/inline_test_exposed_nested_call.rexx)
add_crexx_compiler_test(inline_test_self_recursive_negative rexx_src/inline_test_self_recursive_negative.rexx)
add_crexx_compiler_test(inline_test_mutual_recursive_negative rexx_src/inline_test_mutual_recursive_negative.rexx)
add_crexx_compiler_test(inline_test_nested_scope_do rexx_src/inline_test_nested_scope_do.rexx)
add_crexx_compiler_test(inline_test_nested_scope_if_do rexx_src/inline_test_nested_scope_if_do.rexx)
add_crexx_compiler_test(inline_test_nested_scope_exposed_call rexx_src/inline_test_nested_scope_exposed_call.rexx)
add_crexx_compiler_test(inline_test_multi_return_assign rexx_src/inline_test_multi_return_assign.rexx)
add_crexx_compiler_test(inline_test_multi_return_expr rexx_src/inline_test_multi_return_expr.rexx)
add_crexx_compiler_test(inline_test_multi_return_call rexx_src/inline_test_multi_return_call.rexx)
add_crexx_compiler_test(inline_test_void_fallthrough_call rexx_src/inline_test_void_fallthrough_call.rexx)
add_crexx_compiler_test(inline_test_array_return_assign rexx_src/inline_test_array_return_assign.rexx)
add_crexx_compiler_test(inline_test_array_return_expr rexx_src/inline_test_array_return_expr.rexx)
add_crexx_compiler_test(inline_test_array_multi_return_expr rexx_src/inline_test_array_multi_return_expr.rexx)
add_crexx_compiler_test(inline_test_array_multi_return_assign rexx_src/inline_test_array_multi_return_assign.rexx)
add_crexx_compiler_test(inline_test_object_return_assign rexx_src/inline_test_object_return_assign.rexx)
add_crexx_compiler_test(inline_test_object_return_expr rexx_src/inline_test_object_return_expr.rexx)
add_crexx_compiler_test(inline_test_object_const_arg_mutation rexx_src/inline_test_object_const_arg_mutation.rexx)
add_crexx_compiler_test(inline_test_object_writable_arg rexx_src/inline_test_object_writable_arg.rexx)
add_crexx_compiler_test(inline_test_array_expr_arg rexx_src/inline_test_array_expr_arg.rexx)
add_crexx_compiler_test(inline_test_object_expr_arg rexx_src/inline_test_object_expr_arg.rexx)
add_crexx_compiler_test(inline_test_array_return_temp rexx_src/inline_test_array_return_temp.rexx)
add_crexx_compiler_test(inline_test_complex_lhs_assign rexx_src/inline_test_complex_lhs_assign.rexx)
add_crexx_compiler_test(inline_test_binary_arg_return rexx_src/inline_test_binary_arg_return.rexx)
add_crexx_compiler_test(inline_test_block_expr_live_sibling rexx_src/inline_test_block_expr_live_sibling.rexx)
add_crexx_compiler_test(inline_test_class_methods rexx_src/inline_test_class_methods.rexx)

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
    
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${NAME}_run_noopt.rxbin\" ${RUNTIME_MODULES} OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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
            RUNNER "$<TARGET_FILE:rxvm>"
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
                    RUNNER "$<TARGET_FILE:rxvm>"
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
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_MODULE}${TEST_SUFFIX}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_MODULE}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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
                    RUNNER "$<TARGET_FILE:rxvm>"
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
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_A_MODULE}${TEST_SUFFIX}.rxbin\" \"\${work}/${DEP_B_MODULE}${TEST_SUFFIX}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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
                string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${DEP_A_MODULE}.rxbin\" \"\${work}/${DEP_B_MODULE}.rxbin\" \"\${work}/main${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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
                    RUNNER "$<TARGET_FILE:rxvm>"
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
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${NAME}${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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
                    RUNNER "$<TARGET_FILE:rxvm>"
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
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/${NAME}${TEST_SUFFIX}.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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

    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${NAME}_run_args_noopt.rxbin\" -a${PROGRAM_ARGS_TEXT} OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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
            RUNNER "$<TARGET_FILE:rxvm>"
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

add_crexx_run_test(repro_dotted_index rexx_src/repro_dotted_index.rexx)
add_crexx_run_test(repro_expand_until rexx_src/repro_expand_until.rexx)
add_crexx_run_test(repro_array_arg rexx_src/repro_array_arg.rexx)
add_crexx_run_test(repro_array_trailing_concat rexx_src/repro_array_trailing_concat.rexx)
add_crexx_run_test(repro_opt_args rexx_src/repro_opt_args.rexx)
add_crexx_run_test(arg_semantics_scalar rexx_src/arg_semantics_scalar.rexx)
add_crexx_run_test(arg_semantics_array rexx_src/arg_semantics_array.rexx)
add_crexx_run_test(arg_semantics_object rexx_src/arg_semantics_object.rexx)
add_crexx_run_test(object_reference_regression rexx_src/object_reference_regression.rexx)
add_crexx_run_test(basic_loops rexx_src/basic_loops.rexx)
add_crexx_run_test(basic_strings rexx_src/basic_strings.rexx)
add_crexx_run_test(basic_arrays rexx_src/basic_arrays.rexx)
add_crexx_run_test(block_expr_basic rexx_src/block_expr_basic.rexx)
add_crexx_run_test(block_expr_short_circuit rexx_src/block_expr_short_circuit.rexx)
add_crexx_run_test(repro_args_parse rexx_src/repro_args_parse.rexx)
add_crexx_run_test(repro_args_alias rexx_src/repro_args_alias.rexx)
add_crexx_run_test(repro_sconcat rexx_src/repro_sconcat.rexx)
add_crexx_run_test(repro_strict_compare rexx_src/repro_strict_compare.rexx)
add_crexx_run_test(08_select rexx_src/08_select.rexx)
add_crexx_run_test(13_stems rexx_src/13_stems.rexx)
add_crexx_run_test(repro_rexx_compare_method rexx_src/repro_rexx_compare_method.rexx "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
add_crexx_run_test(repro_rexx_format_sign_gap rexx_src/repro_rexx_format_sign_gap.rexx "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
add_crexx_run_test(repro_implicit_address_abuttal rexx_src/repro_implicit_address_abuttal.rexx)
add_crexx_run_test(test_explicit_address rexx_src/test_explicit_address.rexx)
add_crexx_run_test(test_address_redirect rexx_src/test_address_redirect.rexx)
add_crexx_run_test(test_implicit_address rexx_src/test_implicit_address.rexx)
add_crexx_run_test(test_parse_statement rexx_src/test_parse_statement.rexx)
add_crexx_isolated_run_test(qualified_namespace_same_module rexx_src/qualified_namespace_same_module.rexx)
add_crexx_isolated_run_test(qualified_bif_direct_namespace rexx_src/qualified_bif_direct_namespace.rexx)
add_crexx_dual_import_run_test(qualified_namespace_import_source rexx_src/qualified_namespace_main.rexx rexx_src/qualified_namespace_dep_a.rexx rexx_src/qualified_namespace_dep_b.rexx source)
add_crexx_dual_import_run_test(qualified_namespace_import_binary rexx_src/qualified_namespace_main.rexx rexx_src/qualified_namespace_dep_a.rexx rexx_src/qualified_namespace_dep_b.rexx binary)
add_crexx_isolated_run_test(interface_tracer rexx_src/interface_tracer.rexx)
add_crexx_isolated_run_test(interface_runtime_dispatch_multi rexx_src/interface_runtime_dispatch_multi.rexx)
add_crexx_isolated_run_test(interface_method_arg_same_module rexx_src/interface_method_arg_same_module.rexx)
add_crexx_isolated_run_test(interface_method_object_arg_same_module rexx_src/interface_method_object_arg_same_module.rexx)
add_crexx_isolated_run_test(interface_factory_tiebreak rexx_src/interface_factory_tiebreak.rexx)
add_crexx_isolated_run_fail_test(interface_no_impl rexx_src/interface_no_impl.rexx)
add_crexx_compiler_test(interface_default_codegen rexx_src/interface_default_codegen.rexx)
add_crexx_isolated_run_test(interface_default_same_module rexx_src/interface_default_same_module.rexx)
add_crexx_isolated_run_test(interface_multi_interface_same_module rexx_src/interface_multi_interface_same_module.rexx)
add_crexx_isolated_run_test(interface_showcase_same_module rexx_src/interface_showcase_same_module.rexx)
add_crexx_isolated_run_test(type_ops_showcase rexx_src/type_ops_showcase.rexx)
add_crexx_isolated_run_test(interface_match_same_module rexx_src/interface_match_same_module.rexx)
add_crexx_isolated_run_fail_test(interface_match_reject_single rexx_src/interface_match_reject_single.rexx)
add_crexx_isolated_run_fail_test(type_ops_fail rexx_src/type_ops_fail.rexx)
add_crexx_isolated_run_test(interface_named_factory_same_module rexx_src/interface_named_factory_same_module.rexx)
add_crexx_isolated_run_test(interface_named_factory_tiebreak rexx_src/interface_named_factory_tiebreak.rexx)
add_crexx_isolated_run_fail_test(interface_named_factory_no_impl rexx_src/interface_named_factory_no_impl.rexx)
add_crexx_interface_import_run_test(interface_default_source_import rexx_src/interface_default_main.rexx rexx_src/interface_default_dep_contract.rexx source)
add_crexx_interface_import_run_test(interface_default_binary_import rexx_src/interface_default_binary_main.rexx rexx_src/interface_default_dep_contract.rexx binary)
add_crexx_interface_import_run_test(interface_source_import rexx_src/interface_source_import_main.rexx rexx_src/interface_dep_contract.rexx source)
add_crexx_interface_import_run_test(interface_binary_import rexx_src/interface_binary_import_main.rexx rexx_src/interface_dep_contract.rexx binary)
add_crexx_interface_import_run_test(interface_match_source_import rexx_src/interface_match_main.rexx rexx_src/interface_match_dep_contract.rexx source)
add_crexx_interface_import_run_test(interface_match_binary_import rexx_src/interface_match_main.rexx rexx_src/interface_match_dep_contract.rexx binary)
add_crexx_interface_import_run_test(interface_named_factory_source_import rexx_src/interface_named_factory_main.rexx rexx_src/interface_named_factory_dep_contract.rexx source)
add_crexx_interface_import_run_test(interface_named_factory_binary_import rexx_src/interface_named_factory_main.rexx rexx_src/interface_named_factory_dep_contract.rexx binary)
add_crexx_dual_import_run_test(interface_import_provider_source rexx_src/interface_import_provider_main.rexx rexx_src/interface_import_provider_contract.rexx rexx_src/interface_import_provider_impl.rexx source)
add_crexx_dual_import_run_test(interface_import_provider_binary rexx_src/interface_import_provider_main.rexx rexx_src/interface_import_provider_contract.rexx rexx_src/interface_import_provider_impl.rexx binary)
add_crexx_dual_import_run_test(interface_split_same_namespace_source rexx_src/interface_split_same_namespace_main.rexx rexx_src/interface_split_same_namespace_contract.rexx rexx_src/interface_split_same_namespace_provider.rexx source)
add_crexx_dual_import_run_test(interface_split_same_namespace_binary rexx_src/interface_split_same_namespace_main.rexx rexx_src/interface_split_same_namespace_contract.rexx rexx_src/interface_split_same_namespace_provider.rexx binary)
add_crexx_dual_import_run_test(qualified_interface_import rexx_src/qualified_interface_main.rexx rexx_src/qualified_interface_dep_a.rexx rexx_src/qualified_interface_dep_b.rexx source)
add_crexx_dual_import_run_test(qualified_interface_binary_import rexx_src/qualified_interface_main.rexx rexx_src/qualified_interface_dep_a.rexx rexx_src/qualified_interface_dep_b.rexx binary)

set(INLINE_CROSS_FILE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_cross_file_binary_opt.cmake")
set(INLINE_CROSS_FILE_CONTENT "set(full_out \"\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_cross_file_binary_opt_ws\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.rexx\" \"\${work}/inline_cross_file_dep.rexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline dependency\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.rexx\" \"\${work}/inline_cross_file_main.rexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline main\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.rexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(READ \"\${work}/inline_cross_file_dep.rxas\" dep_rxas)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.inc\\\"\" dep_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \"=\\\".inline\\\" \\\"I4;\" dep_inline_pos)\n")
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
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.membercallblocked\\\"\" dep_member_meta_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_meta_pos EQUAL -1 OR dep_if_meta_pos EQUAL -1 OR dep_scoped_meta_pos EQUAL -1 OR dep_nested_meta_pos EQUAL -1 OR dep_sum_meta_pos EQUAL -1 OR dep_count_meta_pos EQUAL -1 OR dep_array_meta_pos EQUAL -1 OR dep_render_meta_pos EQUAL -1 OR dep_object_meta_pos EQUAL -1 OR dep_ref_meta_pos EQUAL -1 OR dep_opt_meta_pos EQUAL -1 OR dep_safe_asm_meta_pos EQUAL -1 OR dep_varg_total_meta_pos EQUAL -1 OR dep_varg_pick_meta_pos EQUAL -1 OR dep_residual_func_meta_pos EQUAL -1 OR dep_residual_call_meta_pos EQUAL -1 OR dep_residual_target_meta_pos EQUAL -1 OR dep_box_getname_meta_pos EQUAL -1 OR dep_box_setname_meta_pos EQUAL -1 OR dep_inline_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not export expected inline metadata records\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_alias_meta_pos EQUAL -1 OR dep_ref_varg_meta_pos EQUAL -1 OR dep_residual_meta_pos EQUAL -1 OR dep_member_meta_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not emit expected fail-closed callable signatures\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.safelength\\\"=\\\".inline\\\" \\\"I4\" dep_safe_asm_inline_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_safe_asm_inline_pos EQUAL -1)\n  message(FATAL_ERROR \"Safe assembler helper did not export inline metadata\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_rxas}\" \";d,\" dep_dependency_record_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_dependency_record_pos EQUAL -1)\n  message(FATAL_ERROR \"Dependency did not export residual-call dependency records\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(expected_inline_name vargtotal residualfunctiondependency residualcalldependency box.getname box.setname)\n  string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.\${expected_inline_name}\\\"=\\\".inline\\\" \\\"I4\" dep_expected_inline_pos)\n  if(dep_expected_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"Dependency did not export inline metadata for \${expected_inline_name}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(blocked_name aliasblocked refvargdynamicblocked vargpick residualcalltarget residualcallblocked membercallblocked)\n  string(FIND \"\${dep_rxas}\" \".meta \\\"inline_cross_file_dep.\${blocked_name}\\\"=\\\".inline\\\" \\\"I4\" dep_blocked_inline_pos)\n  if(NOT dep_blocked_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"Fail-closed callable \${blocked_name} unexpectedly exported inline metadata\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxdas>\" -o \"inline_cross_file_dep.roundtrip.rxas\" \"inline_cross_file_dep.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxdas failed on inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(READ \"\${work}/inline_cross_file_dep.roundtrip.rxas\" dep_roundtrip_rxas)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${dep_roundtrip_rxas}\" \";d,\" dep_roundtrip_dependency_record_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(dep_roundtrip_dependency_record_pos EQUAL -1)\n  message(FATAL_ERROR \"RXDAS roundtrip lost residual-call dependency records\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(roundtrip_inline_name inc vargtotal residualfunctiondependency residualcalldependency box.getname box.setname)\n  string(FIND \"\${dep_roundtrip_rxas}\" \".meta \\\"inline_cross_file_dep.\${roundtrip_inline_name}\\\"=\\\".inline\\\" \\\"I4\" dep_roundtrip_inline_pos)\n  if(dep_roundtrip_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"RXDAS roundtrip lost inline metadata for \${roundtrip_inline_name}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(blocked_name aliasblocked refvargdynamicblocked vargpick residualcalltarget residualcallblocked membercallblocked)\n  string(FIND \"\${dep_roundtrip_rxas}\" \".meta \\\"inline_cross_file_dep.\${blocked_name}\\\"=\\\".inline\\\" \\\"I4\" dep_roundtrip_blocked_inline_pos)\n  if(NOT dep_roundtrip_blocked_inline_pos EQUAL -1)\n    message(FATAL_ERROR \"RXDAS roundtrip unexpectedly gained inline metadata for \${blocked_name}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.roundtrip\" \"inline_cross_file_dep.roundtrip.rxas\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on rxdas inline dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.rxbin\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(RENAME \"\${work}/inline_cross_file_dep.roundtrip.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(REMOVE \"\${work}/inline_cross_file_dep.rexx\" \"\${work}/inline_cross_file_dep.rxas\" \"\${work}/inline_cross_file_dep.roundtrip.rxas\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.rexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on inline main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${main_rxas}\" \".srcfile=\\\"inline_cross_file_dep.rexx\\\"\" main_dep_srcfile_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${main_rxas}\" \".src 70:3=\\\"result = .int\\\"\" main_dep_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(FIND \"\${main_rxas}\" \".src 71:13=\\\"strlen result,value\\\"\" main_dep_asm_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(main_dep_srcfile_pos EQUAL -1 OR main_dep_source_pos EQUAL -1 OR main_dep_asm_source_pos EQUAL -1)\n  message(FATAL_ERROR \"Optimized binary import did not preserve inlined dependency source metadata:\\n\${main_rxas}\")\nendif()\n")
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
string(APPEND INLINE_CROSS_FILE_CONTENT "foreach(inlined_name inc classify scoped nested sumto countuntil buildarray renderarray identitybox refbump optadd safelength vargtotal residualfunctiondependency residualcalldependency)\n  if(main_rxas MATCHES \"call [^\\n]*inline_cross_file_dep\\\\.\${inlined_name}\\\\(\\\\)\")\n    message(FATAL_ERROR \"Optimized binary import still calls \${inlined_name}():\\n\${main_rxas}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.vargpick\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import unexpectedly inlined dynamic vargPick():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.residualcalltarget\\(\\)]=])\n  message(FATAL_ERROR \"Optimized binary import did not preserve residual call dependency:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on inline main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND INLINE_CROSS_FILE_CONTENT "if(NOT full_out STREQUAL \"42\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected inline output, got: \${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_CROSS_FILE_SCRIPT}" CONTENT "${INLINE_CROSS_FILE_CONTENT}")

add_test(NAME inline_cross_file_binary_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_CROSS_FILE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_cross_file_binary_opt PROPERTIES LABELS "linked_opt")

set(INLINE_CROSS_FILE_SOURCE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_cross_file_source_opt.cmake")
set(INLINE_CROSS_FILE_SOURCE_CONTENT "set(full_out \"\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_cross_file_source_opt_ws\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_dep.rexx\" \"\${work}/inline_cross_file_dep.rexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline dependency\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_cross_file_main.rexx\" \"\${work}/inline_cross_file_main.rexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage inline main\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"\${work}\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_main\" \"\${work}/inline_cross_file_main.rexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "file(READ \"\${work}/inline_cross_file_main.rxas\" main_rxas)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(FIND \"\${main_rxas}\" \".srcfile=\\\"inline_cross_file_dep.rexx\\\"\" main_dep_srcfile_pos)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(FIND \"\${main_rxas}\" \".src 70:3=\\\"result = .int\\\"\" main_dep_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(FIND \"\${main_rxas}\" \".src 71:13=\\\"strlen result,value\\\"\" main_dep_asm_source_pos)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(main_dep_srcfile_pos EQUAL -1 OR main_dep_source_pos EQUAL -1 OR main_dep_asm_source_pos EQUAL -1)\n  message(FATAL_ERROR \"Optimized source import did not preserve inlined dependency source metadata:\\n\${main_rxas}\")\nendif()\n")
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
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "foreach(inlined_name inc classify scoped nested sumto countuntil buildarray renderarray identitybox refbump optadd safelength vargtotal residualfunctiondependency residualcalldependency)\n  if(main_rxas MATCHES \"call [^\\n]*inline_cross_file_dep\\\\.\${inlined_name}\\\\(\\\\)\")\n    message(FATAL_ERROR \"Optimized source import still calls \${inlined_name}():\\n\${main_rxas}\")\n  endif()\nendforeach()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.vargpick\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import unexpectedly inlined dynamic vargPick():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT main_rxas MATCHES [=[call [^\\n]*inline_cross_file_dep\\.residualcalltarget\\(\\)]=])\n  message(FATAL_ERROR \"Optimized source import did not preserve residual call dependency:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_main.rxbin\" \"inline_cross_file_main\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import main: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_cross_file_dep\" \"\${work}/inline_cross_file_dep.rexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on source-import dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_cross_file_dep.rxbin\" \"inline_cross_file_dep\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on source-import dependency: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_cross_file_dep.rxbin\" \"\${work}/inline_cross_file_main.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND INLINE_CROSS_FILE_SOURCE_CONTENT "if(NOT full_out STREQUAL \"42\\n-1\\n0\\n1\\n20\\n-3\\n42\\n10\\n3\\nred:blue\\nseed\\nafter\\n11\\n11\\n15\\n8\\n6\\n12\\n9\\n10\\ncall:12\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected inline output, got: \${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_CROSS_FILE_SOURCE_SCRIPT}" CONTENT "${INLINE_CROSS_FILE_SOURCE_CONTENT}")

add_test(NAME inline_cross_file_source_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_CROSS_FILE_SOURCE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_cross_file_source_opt PROPERTIES LABELS "linked_opt")

set(INLINE_LOCAL_MEMBER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_inline_local_member_scalar_opt.cmake")
set(INLINE_LOCAL_MEMBER_CONTENT "set(full_out \"\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/inline_local_member_scalar_opt_ws\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/inline_local_member_scalar.rexx\" \"\${work}/inline_local_member_scalar.rexx\" RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage local member inline test\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"inline_local_member_scalar\" \"\${work}/inline_local_member_scalar.rexx\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed on local member inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "file(READ \"\${work}/inline_local_member_scalar.rxas\" main_rxas)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_local_member_scalar\\.box\\.§factory\\(\\)]=])\n  message(FATAL_ERROR \"Optimized local member test still calls Box factory:\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_local_member_scalar\\.box\\.get\\(\\)]=])\n  message(FATAL_ERROR \"Optimized local member test still calls get():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(main_rxas MATCHES [=[call [^\\n]*inline_local_member_scalar\\.box\\.set\\(\\)]=])\n  message(FATAL_ERROR \"Optimized local member test still calls set():\\n\${main_rxas}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"inline_local_member_scalar.rxbin\" \"inline_local_member_scalar\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed on local member inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"\${work}/inline_local_member_scalar.rxbin\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed on local member inline test: \${out}\")\nendif()\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
string(APPEND INLINE_LOCAL_MEMBER_CONTENT "if(NOT full_out STREQUAL \"alpha\\nbeta\\ngamma\\n\")\n  message(FATAL_ERROR \"Output mismatch: expected local member inline output, got: \${full_out}\")\nendif()\n")
file(GENERATE OUTPUT "${INLINE_LOCAL_MEMBER_SCRIPT}" CONTENT "${INLINE_LOCAL_MEMBER_CONTENT}")

add_test(NAME inline_local_member_scalar_opt
         COMMAND ${CMAKE_COMMAND} -P "${INLINE_LOCAL_MEMBER_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(inline_local_member_scalar_opt PROPERTIES LABELS "linked_opt")

add_crexx_run_args_test(test_parse_arg_filename rexx_src/test_parse_arg_filename.rexx demo.rexx)
add_crexx_run_args_test(implicit_main_arg_compat rexx_src/implicit_main_arg_compat.rexx alpha beta)
add_crexx_run_args_test(arg_compat_no_vargs rexx_src/arg_compat_no_vargs.rexx alpha beta)

set(CRXC_NOARGS_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crxc_noargs.cmake")
set(CRXC_NOARGS_CONTENT "set(full_out \"\")\n")
string(APPEND CRXC_NOARGS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" --level levelb --import rxfnsb -i \"${CMAKE_BINARY_DIR}/bin\" ${COMPILER_NOOPT_FLAG} -o \"crxc_noargs_run\" \"${CMAKE_SOURCE_DIR}/bin/crxc.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
string(APPEND CRXC_NOARGS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"crxc_noargs_run.rxbin\" \"crxc_noargs_run\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
string(APPEND CRXC_NOARGS_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"crxc_noargs_run.rxbin\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CRXC_NOARGS_CONTENT "string(APPEND full_out \"\${out}\")\n")
string(APPEND CRXC_NOARGS_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
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
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -n -o \"structured_exit_debug_bundle\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/structured_exit_debug_bundle.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed building structured exit bundle: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"structured_exit_debug_bundle\" \"structured_exit_debug_bundle\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed building structured exit bundle: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E env \"RXCP_EXIT_MODULE=structured_exit_debug_bundle\" \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_CURRENT_BINARY_DIR}\" -i \"${CMAKE_BINARY_DIR}/bin\" -d2 -n -o \"structured_exit_debug_main\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/structured_exit_debug_main.rexx\" OUTPUT_FILE \"\${debug_log}\" ERROR_FILE \"\${debug_log}\" RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc -d2 failed for structured exit debug case. See \${debug_log}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"structured_exit_debug_main.rxbin\" \"structured_exit_debug_main\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "string(APPEND run_out \"\${out}\")\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed for structured exit main: \${out}\")\nendif()\n")
string(APPEND STRUCTURED_EXIT_DEBUG_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"structured_exit_debug_main.rxbin\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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
add_crexx_run_test(test_obj_prop rexx_src/test_obj_prop.rexx)
add_crexx_run_test(test_meth_vs_proc rexx_src/test_meth_vs_proc.rexx)
add_crexx_run_test(test_class_adv rexx_src/test_class_adv.rexx)
add_crexx_run_test(test_array_edge rexx_src/test_array_edge.rexx)
add_crexx_run_test(test_utf_math rexx_src/test_utf_math.rexx)
add_crexx_run_test(test_lib_boundary rexx_src/test_lib_boundary.rexx)
add_crexx_run_test(div_ops_common rexx_src/div_ops_common.rexx)
add_crexx_run_test(div_ops_classic rexx_src/div_ops_classic.rexx)

# Shadowing behavior in simple DO blocks
add_crexx_run_test(do_scope_typed_shadows rexx_src/do_scope_typed_shadows.rexx)

# Hoisting and effective scopes tests
add_crexx_run_test(test_global_subscopes rexx_src/test_global_subscopes.rexx)

# Inlining tests
add_crexx_run_test(inline_test1 rexx_src/inline_test1.rexx)
add_crexx_run_test(inline_test2 rexx_src/inline_test2.rexx)
add_crexx_run_test(inline_test_call rexx_src/inline_test_call.rexx)
add_crexx_run_test(inline_test_expr rexx_src/inline_test_expr.rexx)
add_crexx_run_test(inline_test_expr_negative rexx_src/inline_test_expr_negative.rexx)
add_crexx_run_test(inline_test_nested_call_expr rexx_src/inline_test_nested_call_expr.rexx)
add_crexx_run_test(inline_test_concat_expr rexx_src/inline_test_concat_expr.rexx)
add_crexx_run_test(inline_test_say_expr rexx_src/inline_test_say_expr.rexx)
add_crexx_run_test(inline_test_say_expr_negative rexx_src/inline_test_say_expr_negative.rexx)
add_crexx_run_test(inline_test_return_expr rexx_src/inline_test_return_expr.rexx)
add_crexx_run_test(inline_test_unary_expr rexx_src/inline_test_unary_expr.rexx)
add_crexx_run_test(inline_test_compare_expr rexx_src/inline_test_compare_expr.rexx)
add_crexx_run_test(inline_test_call_arg_expr rexx_src/inline_test_call_arg_expr.rexx)
add_crexx_run_test(inline_test_call_like_arg_expr rexx_src/inline_test_call_like_arg_expr.rexx)
add_crexx_run_test(inline_test_short_circuit_expr rexx_src/inline_test_short_circuit_expr.rexx)
add_crexx_run_test(inline_test_composed_expr_contexts rexx_src/inline_test_composed_expr_contexts.rexx)
add_crexx_run_test(inline_test_stateful_assembler rexx_src/inline_test_stateful_assembler.rexx)
add_crexx_run_test(inline_test_condition_contexts rexx_src/inline_test_condition_contexts.rexx)
add_crexx_run_test(inline_test_loop_bound_expr rexx_src/inline_test_loop_bound_expr.rexx)
add_crexx_run_test(inline_test_member_receiver_expr rexx_src/inline_test_member_receiver_expr.rexx)
add_crexx_run_test(inline_test_named_factory_selector rexx_src/inline_test_named_factory_selector.rexx)
add_crexx_run_test(inline_test_ref_opt rexx_src/inline_test_ref_opt.rexx)
add_crexx_run_test(inline_test_ref_indexed rexx_src/inline_test_ref_indexed.rexx)
add_crexx_run_test(inline_test_ref_stem rexx_src/inline_test_ref_stem.rexx)
add_crexx_run_test(inline_test_ref_computed rexx_src/inline_test_ref_computed.rexx)
add_crexx_run_test(inline_test_ref_vararg rexx_src/inline_test_ref_vararg.rexx)
add_crexx_run_test(inline_test_ref_vararg_runtime rexx_src/inline_test_ref_vararg_runtime.rexx)
add_crexx_run_test(inline_test_ref_negative rexx_src/inline_test_ref_negative.rexx)
add_crexx_run_test(inline_test_bool_expr_negative rexx_src/inline_test_bool_expr_negative.rexx)
add_crexx_run_test(inline_test_vararg rexx_src/inline_test_vararg.rexx)
add_crexx_run_test(inline_test_vararg_dynamic rexx_src/inline_test_vararg_dynamic.rexx)
add_crexx_run_test(inline_test_vararg_dynamic_loop rexx_src/inline_test_vararg_dynamic_loop.rexx)
add_crexx_run_test(inline_test_vararg_negative rexx_src/inline_test_vararg_negative.rexx)
add_crexx_run_test(inline_test_exposed_nested_call rexx_src/inline_test_exposed_nested_call.rexx)
add_crexx_run_test(inline_test_nested_scope_do rexx_src/inline_test_nested_scope_do.rexx)
add_crexx_run_test(inline_test_nested_scope_if_do rexx_src/inline_test_nested_scope_if_do.rexx)
add_crexx_run_test(inline_test_nested_scope_exposed_call rexx_src/inline_test_nested_scope_exposed_call.rexx)
add_crexx_run_test(inline_test_multi_return_assign rexx_src/inline_test_multi_return_assign.rexx)
add_crexx_run_test(inline_test_multi_return_expr rexx_src/inline_test_multi_return_expr.rexx)
add_crexx_run_test(inline_test_multi_return_call rexx_src/inline_test_multi_return_call.rexx)
add_crexx_run_test(inline_test_void_fallthrough_call rexx_src/inline_test_void_fallthrough_call.rexx)
add_crexx_run_test(inline_test_array_return_assign rexx_src/inline_test_array_return_assign.rexx)
add_crexx_run_test(inline_test_array_return_expr rexx_src/inline_test_array_return_expr.rexx)
add_crexx_run_test(inline_test_array_multi_return_expr rexx_src/inline_test_array_multi_return_expr.rexx)
add_crexx_run_test(inline_test_array_multi_return_assign rexx_src/inline_test_array_multi_return_assign.rexx)
add_crexx_run_test(inline_test_object_return_assign rexx_src/inline_test_object_return_assign.rexx)
add_crexx_run_test(inline_test_object_return_expr rexx_src/inline_test_object_return_expr.rexx)
add_crexx_run_test(inline_test_object_const_arg_mutation rexx_src/inline_test_object_const_arg_mutation.rexx)
add_crexx_run_test(inline_test_object_writable_arg rexx_src/inline_test_object_writable_arg.rexx)
add_crexx_run_test(inline_test_array_expr_arg rexx_src/inline_test_array_expr_arg.rexx)
add_crexx_run_test(inline_test_object_expr_arg rexx_src/inline_test_object_expr_arg.rexx)
add_crexx_run_test(inline_test_array_return_temp rexx_src/inline_test_array_return_temp.rexx)
add_crexx_run_test(inline_test_block_expr_live_sibling rexx_src/inline_test_block_expr_live_sibling.rexx)
add_crexx_run_test(inline_test_complex_lhs_assign rexx_src/inline_test_complex_lhs_assign.rexx)
add_crexx_run_test(inline_test_binary_arg_return rexx_src/inline_test_binary_arg_return.rexx)
add_crexx_run_test(inline_test_class_methods rexx_src/inline_test_class_methods.rexx)

# New Phase 2 Scoping Tests
add_crexx_run_test(test_simple_do_subscope rexx_src/test_simple_do_subscope.rexx)
add_crexx_run_test(test_if_do_subscope rexx_src/test_if_do_subscope.rexx)
add_crexx_run_test(test_iterative_do_subscope rexx_src/test_iterative_do_subscope.rexx)
add_crexx_run_test(test_if_no_subscope rexx_src/test_if_no_subscope.rexx)
add_crexx_run_test(test_untyped_subscopes rexx_src/test_untyped_subscopes.rexx)
add_crexx_run_test(test_untyped_subscope_2 rexx_src/test_untyped_subscope_2.rexx)
add_crexx_run_test(test_if_else_nested_subscopes rexx_src/test_if_else_nested_subscopes.rexx)
add_crexx_run_test(test_sub_sub_scopes rexx_src/test_sub_sub_scopes.rexx)
add_crexx_run_test(test_sub_sub_scopes_untyped rexx_src/test_sub_sub_scopes_untyped.rexx)
add_crexx_run_test(test_global_nested_complex rexx_src/test_global_nested_complex.rexx)
add_crexx_run_test(test_function_shadowing rexx_src/test_function_shadowing.rexx)
add_crexx_run_test(test_disjoint_detailed rexx_src/test_disjoint_detailed.rexx)

# New test for detailed warning
set(WARN_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_warn_test.cmake")
set(WARN_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o test_disjoint_detailed_warn \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/test_disjoint_detailed.rexx\" ERROR_VARIABLE err_out OUTPUT_VARIABLE std_out RESULT_VARIABLE res)\n")
string(APPEND WARN_CONTENT "if(NOT err_out MATCHES \"original definition @ 4:5\")\n  message(FATAL_ERROR \"Expected warning not found in output\")\nendif()\n")
file(GENERATE OUTPUT "${WARN_SCRIPT}" CONTENT "${WARN_CONTENT}")

add_test(NAME test_disjoint_detailed_warn
         COMMAND ${CMAKE_COMMAND} -P "${WARN_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})


# Parsing Tests
add_crexx_parsing_test(expr_abut_01 parsing_src/expr_abut_01.rexx)
add_crexx_parsing_test(expr_abut_02 parsing_src/expr_abut_02.rexx)
add_crexx_parsing_test(expr_abut_03 parsing_src/expr_abut_03.rexx)
add_crexx_parsing_test(expr_abut_04 parsing_src/expr_abut_04.rexx)
add_crexx_parsing_test(expr_ambig_01 parsing_src/expr_ambig_01.rexx)
add_crexx_parsing_test(expr_ambig_02 parsing_src/expr_ambig_02.rexx)
add_crexx_parsing_test(expr_ambig_03 parsing_src/expr_ambig_03.rexx)
add_crexx_parsing_test(err_struct_01 parsing_src/err_struct_01.rexx)
add_crexx_parsing_test(err_struct_02 parsing_src/err_struct_02.rexx)
add_crexx_parsing_test(err_struct_03 parsing_src/err_struct_03.rexx)
add_crexx_parsing_test(err_namespace_class_anchor parsing_src/err_namespace_class_anchor.rexx)
add_crexx_parsing_test(address_exit_extended parsing_src/address_exit_extended.rexx)
add_crexx_parsing_test(address_inline_then parsing_src/address_inline_then.rexx)
# DO constructs parsing coverage
add_crexx_parsing_test(do_simple_parse parsing_src/do_simple_parse.rexx)
add_crexx_parsing_test(do_counted_parse parsing_src/do_counted_parse.rexx)
add_crexx_parsing_test(do_while_parse parsing_src/do_while_parse.rexx)
add_crexx_parsing_test(do_until_parse parsing_src/do_until_parse.rexx)
add_crexx_parsing_test(block_expr_parse parsing_src/block_expr_parse.rexx)

# SELECT constructs parsing coverage
add_crexx_parsing_test(select_classic_parse parsing_src/select_classic_parse.rexx)
add_crexx_parsing_test(select_c_parse parsing_src/select_c_parse.rexx)

if (ENABLE_PARSER_MODE)
    add_crexx_syntax_highlight_test(
            syntaxhighlight_root_file
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/16_classes.rexx
            MUST_MATCH
            "Node: type=71 \\(PARSE_TREE_FILE\\)"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_syntax_error
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_02_syntax.rexx
            MUST_MATCH
            "SYNTAX_ERROR"
            "Message:"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_compound_names
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/repro_dotted_index.rexx
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
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/16_classes.rexx
            MUST_MATCH
            "PARSE_TREE_STRUCTURE"
            "PARSE_TREE_FUNCTION"
            "PARSE_TREE_CODEBLOCK"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_exit_keyword
            ${CMAKE_SOURCE_DIR}/compiler/exits/fsay/test_fsay.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"fsay\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_address_keywords
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_address_keywords.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=7, sev=48, text=\"address\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"input\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=6, sev=48, text=\"output\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=5, sev=48, text=\"error\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"expose\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_parse_keywords
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_parse_keywords.rexx
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
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_trace_keywords.rexx
            MUST_MATCH
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=5, sev=48, text=\"trace\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"off\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=6, sev=48, text=\"normal\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=4, sev=48, text=\"rexx\""
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"asm\""
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_top_level_statements
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/01_assign.rexx
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
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
            "Node: type=72 \\(PARSE_TREE_CODEBLOCK\\), pos=[0-9]+, len=[0-9]+, sev=48\n[ ]+Node: type=73 \\(PARSE_TREE_STATEMENT\\), pos=[0-9]+, len=[0-9]+, sev=48\n[ ]+Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
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
            "Node: type=41 \\(LEXER_KEYWORD\\), pos=[0-9]+, len=3, sev=48, text=\"say\""
            "Node: type=54 \\(LEXER_IDENTIFIER\\), pos=[0-9]+, len=2, sev=48, text=\"do\""
            MUST_NOT_MATCH
            "RXC-LC-"
            "PARSE_FAILURE"
    )
    set_tests_properties(syntaxhighlight_levelc_do_keyword_variables PROPERTIES LABELS "compiler;syntax_highlighting;stage1;levelc")

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
            syntaxhighlight_block_comment
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_block_comment.rexx
            MUST_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=[0-9]+, len=[0-9]+, sev=48"
            MUST_NOT_MATCH
            "LEXER_OPERATOR_ARITHMETIC"
    )

    add_crexx_syntax_highlight_test(
            syntaxhighlight_comments_dash
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_comments_dash.rexx
            MUST_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=[0-9]+, len=[0-9]+, sev=48, text=\"-- dash line comment\""
            MUST_NOT_MATCH
            "Node: type=44 \\(LEXER_OPERATOR_ARITHMETIC\\), pos=[0-9]+, len=1, sev=48, text=\"-\""
    )
endif()

set(LEVELC_UNSUPPORTED_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/levelc_compile_unsupported.cmake")
file(GENERATE OUTPUT "${LEVELC_UNSUPPORTED_SCRIPT}" CONTENT
"execute_process(
    COMMAND \"$<TARGET_FILE:rxc>\" \"${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/levelc_unsupported.rexx\"
    WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res
)
set(all \"\${out}\${err}\")
if(res EQUAL 0)
  message(FATAL_ERROR \"Level C compile unexpectedly succeeded.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")
endif()
if(NOT all MATCHES \"REXX Level A/C/D \\\\(cREXX Classic\\\\) - Not supported yet\")
  message(FATAL_ERROR \"Missing Level C unsupported diagnostic.\\nSTDOUT:\\n\${out}\\nSTDERR:\\n\${err}\")
endif()
")
add_test(NAME levelc_compile_unsupported
         COMMAND ${CMAKE_COMMAND} -P "${LEVELC_UNSUPPORTED_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(levelc_compile_unsupported PROPERTIES LABELS "compiler;levelc")

# Bridge Connectivity Test
add_custom_command(
        OUTPUT stub_plugin.rxbin
        COMMAND $<TARGET_FILE:rxc> -n -o stub_plugin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/stub_plugin.rexx
        COMMAND $<TARGET_FILE:rxas> -o stub_plugin stub_plugin
        DEPENDS rxc rxas compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/stub_plugin.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling stub_plugin.rexx ..."
)

add_custom_target(stub_plugin_bin DEPENDS stub_plugin.rxbin)

add_executable(test_bridge src/test_bridge.c)
target_link_libraries(test_bridge PRIVATE rxvml rxpashim)
target_include_directories(test_bridge PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
add_dependencies(test_bridge stub_plugin_bin)

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
set(CREXX_NATIVE_SQLITE_DEMO_DIR "${CMAKE_SOURCE_DIR}/demos/native/sqlite")

add_custom_command(
        OUTPUT address_cms_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_cms_provider ${CREXX_CMS_DEMO_DIR}/cms_address_environment.rexx
        COMMAND $<TARGET_FILE:rxas> address_cms_provider
        DEPENDS rxc rxas library compiler_exit_bin ${CREXX_CMS_DEMO_DIR}/cms_address_environment.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling cms_address_environment.rexx ..."
)

add_custom_command(
        OUTPUT address_cms_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_cms_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_cms_host.rexx
        COMMAND $<TARGET_FILE:rxas> address_cms_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_cms_host.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling address_cms_host.rexx ..."
)

add_custom_command(
        OUTPUT address_cms_demo.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_cms_demo ${CREXX_CMS_DEMO_DIR}/cms_address_demo.rexx
        COMMAND $<TARGET_FILE:rxas> address_cms_demo
        DEPENDS rxc rxas library compiler_exit_bin ${CREXX_CMS_DEMO_DIR}/cms_address_demo.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling cms_address_demo.rexx ..."
)

add_custom_target(address_cms_fixture_bin DEPENDS address_cms_provider.rxbin address_cms_host.rxbin address_cms_demo.rxbin)
add_custom_target(address_cms_demo_bin DEPENDS address_cms_provider.rxbin address_cms_demo.rxbin)
add_dependencies(test_address_bridge address_cms_fixture_bin library)

add_custom_command(
        OUTPUT address_llm_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_llm_provider ${CREXX_LLM_DEMO_DIR}/llm_address_environment.rexx
        COMMAND $<TARGET_FILE:rxas> address_llm_provider
        DEPENDS rxc rxas library rxfnsg compiler_exit_bin ${CREXX_LLM_DEMO_DIR}/llm_address_environment.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling llm_address_environment.rexx ..."
)

add_custom_command(
        OUTPUT address_llm_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_llm_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_llm_host.rexx
        COMMAND $<TARGET_FILE:rxas> address_llm_host
        DEPENDS rxc rxas library rxfnsg compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_llm_host.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling address_llm_host.rexx ..."
)

add_custom_target(address_llm_fixture_bin ALL DEPENDS address_llm_provider.rxbin address_llm_host.rxbin)

add_custom_command(
        OUTPUT native_kv_address_demo.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o native_kv_address_demo ${CREXX_NATIVE_KV_DEMO_DIR}/kv_address_demo.rexx
        COMMAND $<TARGET_FILE:rxas> native_kv_address_demo
        DEPENDS rxc rxas library compiler_exit_bin ${CREXX_NATIVE_KV_DEMO_DIR}/kv_address_demo.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling kv_address_demo.rexx ..."
)

add_custom_target(native_kv_address_demo_bin DEPENDS native_kv_address_demo.rxbin)

add_executable(native_kv_address_demo ${CREXX_NATIVE_KV_DEMO_DIR}/kv_address_demo.c)
target_link_libraries(native_kv_address_demo PRIVATE rxvml rxpashim)
target_include_directories(native_kv_address_demo PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
target_compile_definitions(native_kv_address_demo PRIVATE
        CREXX_NATIVE_KV_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
        CREXX_NATIVE_KV_DEMO_MODULE="${CMAKE_CURRENT_BINARY_DIR}/native_kv_address_demo.rxbin")
add_dependencies(native_kv_address_demo native_kv_address_demo_bin library)

if(CREXX_BUILD_SQLITE_ADDRESS_DEMO)
    include(FetchContent)
    message(STATUS "Fetching SQLite amalgamation ${CREXX_SQLITE_AMALGAMATION_VERSION} for native ADDRESS demo")
    FetchContent_Declare(
            crexx_sqlite_amalgamation
            URL "https://www.sqlite.org/${CREXX_SQLITE_AMALGAMATION_YEAR}/sqlite-amalgamation-${CREXX_SQLITE_AMALGAMATION_VERSION}.zip"
            URL_HASH "SHA3_256=${CREXX_SQLITE_AMALGAMATION_SHA3_256}"
    )
    FetchContent_MakeAvailable(crexx_sqlite_amalgamation)
    FetchContent_GetProperties(crexx_sqlite_amalgamation)

    add_library(crexx_demo_sqlite3 STATIC
            "${crexx_sqlite_amalgamation_SOURCE_DIR}/sqlite3.c")
    target_include_directories(crexx_demo_sqlite3 PUBLIC
            "${crexx_sqlite_amalgamation_SOURCE_DIR}")
    target_compile_definitions(crexx_demo_sqlite3 PRIVATE
            SQLITE_THREADSAFE=0
            SQLITE_OMIT_LOAD_EXTENSION
            SQLITE_DQS=0)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(crexx_demo_sqlite3 PRIVATE -w)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(crexx_demo_sqlite3 PRIVATE /w)
    endif()

    add_custom_command(
            OUTPUT native_sqlite_address_demo.rxbin
            COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o native_sqlite_address_demo ${CREXX_NATIVE_SQLITE_DEMO_DIR}/sqlite_address_demo.rexx
            COMMAND $<TARGET_FILE:rxas> native_sqlite_address_demo
            DEPENDS rxc rxas library compiler_exit_bin ${CREXX_NATIVE_SQLITE_DEMO_DIR}/sqlite_address_demo.rexx
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Compiling sqlite_address_demo.rexx ..."
    )

    add_custom_target(native_sqlite_address_demo_bin DEPENDS native_sqlite_address_demo.rxbin)

    add_executable(native_sqlite_address_demo ${CREXX_NATIVE_SQLITE_DEMO_DIR}/sqlite_address_demo.c)
    target_link_libraries(native_sqlite_address_demo PRIVATE rxvml rxpashim crexx_demo_sqlite3)
    target_include_directories(native_sqlite_address_demo PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
    target_compile_definitions(native_sqlite_address_demo PRIVATE
            CREXX_NATIVE_SQLITE_LIBRARY_PATH="${CMAKE_BINARY_DIR}/bin/library"
            CREXX_NATIVE_SQLITE_DEMO_MODULE="${CMAKE_CURRENT_BINARY_DIR}/native_sqlite_address_demo.rxbin")
    add_dependencies(native_sqlite_address_demo native_sqlite_address_demo_bin library)
endif()

add_custom_command(
        OUTPUT address_callback_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o address_callback_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_callback_host.rexx
        COMMAND $<TARGET_FILE:rxas> address_callback_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/address_callback_host.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling address_callback_host.rexx ..."
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
        OUTPUT dynamic_load_provider.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o dynamic_load_provider ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_provider.rexx
        COMMAND $<TARGET_FILE:rxas> dynamic_load_provider
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_provider.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic_load_provider.rexx ..."
)

add_custom_command(
        OUTPUT dynamic_load_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_BINARY_DIR} -i ${CMAKE_BINARY_DIR}/bin -o dynamic_load_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_host.rexx
        COMMAND $<TARGET_FILE:rxas> dynamic_load_host
        DEPENDS rxc rxas library compiler_exit_bin dynamic_load_provider.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/dynamic_load_host.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling dynamic_load_host.rexx ..."
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
        OUTPUT crexxsaa_variables_host.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o crexxsaa_variables_host ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_variables_host.rexx
        COMMAND $<TARGET_FILE:rxas> crexxsaa_variables_host
        DEPENDS rxc rxas library compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/crexxsaa_variables_host.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling crexxsaa_variables_host.rexx ..."
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
add_test(NAME address_bridge COMMAND test_address_bridge)
set_tests_properties(address_bridge PROPERTIES
        LABELS "compiler;stage3"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_test(NAME address_cms_provider COMMAND
        $<TARGET_FILE:rxvm>
        ${CMAKE_BINARY_DIR}/bin/library.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_provider.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_host.rxbin)
set_tests_properties(address_cms_provider PROPERTIES LABELS "compiler;stage3")
add_test(NAME address_cms_demo COMMAND
        $<TARGET_FILE:rxvm>
        ${CMAKE_BINARY_DIR}/bin/library.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_provider.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_cms_demo.rxbin)
set_tests_properties(address_cms_demo PROPERTIES LABELS "compiler;stage3")
add_test(NAME address_llm_provider COMMAND
        $<TARGET_FILE:rxvm>
        ${CMAKE_BINARY_DIR}/bin/library.rxbin
        ${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_llm_provider.rxbin
        ${CMAKE_CURRENT_BINARY_DIR}/address_llm_host.rxbin)
set_tests_properties(address_llm_provider PROPERTIES LABELS "compiler;stage3")
set_tests_properties(address_llm_provider PROPERTIES
        PASS_REGULAR_EXPRESSION "PASS: address llm host"
        FAIL_REGULAR_EXPRESSION "address llm host failures")
add_test(NAME address_native_kv_demo COMMAND native_kv_address_demo)
set_tests_properties(address_native_kv_demo PROPERTIES LABELS "compiler;stage3")
if(CREXX_BUILD_SQLITE_ADDRESS_DEMO)
    add_test(NAME address_native_sqlite_demo COMMAND native_sqlite_address_demo)
    set_tests_properties(address_native_sqlite_demo PROPERTIES LABELS "compiler;stage3")
endif()
add_test(NAME address_callback_host COMMAND test_address_callback_host)
set_tests_properties(address_callback_host PROPERTIES LABELS "compiler;stage3")
add_test(NAME dynamic_load COMMAND test_dynamic_load)
set_tests_properties(dynamic_load PROPERTIES LABELS "compiler;stage3")
add_test(NAME crexxsaa_variables COMMAND test_crexxsaa_variables)
set_tests_properties(crexxsaa_variables PROPERTIES
        LABELS "compiler;stage3"
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${CMAKE_BINARY_DIR}/bin")
add_test(NAME source_provenance COMMAND test_source_provenance)
set_tests_properties(source_provenance PROPERTIES LABELS "compiler;stage2")

if (ENABLE_PARSER_MODE)
    add_test(NAME highlight_cache COMMAND test_highlight_cache)
    set_tests_properties(highlight_cache PROPERTIES
            LABELS "compiler;syntax_highlighting;stage4"
            RUN_SERIAL TRUE)

    add_test(NAME source_semantics COMMAND test_source_semantics)
    set_tests_properties(source_semantics PROPERTIES LABELS "compiler;syntax_highlighting;stage5")

    add_test(NAME highlight_editor_diagnostics COMMAND test_highlight_editor_diagnostics)
    set_tests_properties(highlight_editor_diagnostics PROPERTIES LABELS "compiler;syntax_highlighting;stage5")
endif()
