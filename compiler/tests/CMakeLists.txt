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
    target_link_libraries(crexx_fake_editor dslsyntax_editor)
    target_include_directories(crexx_fake_editor PRIVATE ${DSL_SH_SOURCE_DIR}/codebuffer)

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

# Compiler flags for optimization levels
# Based on rxcpmain.c: -n is for No Optimising, and it's on by default.
set(COMPILER_NOOPT_FLAG "-n")
set(COMPILER_OPT_FLAG "") # Default is optimized
set(CREXX_TEST_DRIVER_RESOURCE_LOCK "crexx_test_driver_workspace")

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
    set_tests_properties(${NAME}_noopt PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})

    # Test 2 (Opt)
    add_test(NAME ${NAME}_opt
             COMMAND crexx_test_driver
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/opt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt.rxas
             $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin ${COMPILER_OPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
    set_tests_properties(${NAME}_opt PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})
endmacro()

# Macro to add a compiler warning test
macro(add_crexx_compiler_warning_test NAME SOURCE_FILE GOLDEN_FILE)
    add_test(NAME ${NAME}_warn
             COMMAND crexx_test_driver --warnings
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/warnings/${GOLDEN_FILE}
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_warn.txt
             $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
    set_tests_properties(${NAME}_warn PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})
endmacro()

# Macro to add a compiler failure test
macro(add_crexx_compiler_fail_test NAME SOURCE_FILE GOLDEN_FILE)
    add_test(NAME ${NAME}_fail
             COMMAND crexx_test_driver --expect-fail
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/${GOLDEN_FILE}
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_fail.txt
             $<TARGET_FILE:rxc> ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
    set_tests_properties(${NAME}_fail PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})
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
    set_tests_properties(${NAME}_noopt PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})

    # Test 2 (Opt)
    add_test(NAME ${NAME}_opt
             COMMAND crexx_test_driver --copy ${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE}
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/opt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt.rxas
             $<TARGET_FILE:rxc> ${COMPILER_OPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
    set_tests_properties(${NAME}_opt PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})
endmacro()

# Macro to add a parsing test
macro(add_crexx_parsing_test NAME SOURCE_FILE)
    add_test(NAME ${NAME}_parse
             COMMAND crexx_test_driver --ast
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/parsing/${NAME}.txt
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ast.txt
             $<TARGET_FILE:rxc> -dp ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
    set_tests_properties(${NAME}_parse PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})
endmacro()

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
add_crexx_compiler_fail_test(err_block_expr_type_mismatch rexx_src/err_block_expr_type_mismatch.rexx err_block_expr_type_mismatch.txt)
add_crexx_compiler_fail_test(err_block_expr_no_leave rexx_src/err_block_expr_no_leave.rexx err_block_expr_no_leave.txt)
add_crexx_compiler_fail_test(err_block_expr_leave_plain rexx_src/err_block_expr_leave_plain.rexx err_block_expr_leave_plain.txt)
add_crexx_compiler_fail_test(err_block_expr_missing_end rexx_src/err_block_expr_missing_end.rexx err_block_expr_missing_end.txt)
add_crexx_compiler_fail_test(err_block_expr_incomplete rexx_src/err_block_expr_incomplete.rexx err_block_expr_incomplete.txt)
add_crexx_compiler_fail_test(err_leave_with_not_in_block rexx_src/err_leave_with_not_in_block.rexx err_leave_with_not_in_block.txt)
add_test(NAME err_address_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_address_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_address_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_address_exit_disabled.rexx)
set_tests_properties(err_address_exit_disabled_fail PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})

add_test(NAME err_parse_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_parse_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_parse_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_parse_exit_disabled.rexx)
set_tests_properties(err_parse_exit_disabled_fail PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})

add_test(NAME err_parse_inline_then_exit_disabled_fail
         COMMAND crexx_test_driver --expect-fail
         ${CMAKE_CURRENT_SOURCE_DIR}/golden/errors/err_parse_inline_then_exit_disabled.txt
         ${CMAKE_CURRENT_BINARY_DIR}/err_parse_inline_then_exit_disabled_fail.txt
         $<TARGET_FILE:rxc> -x ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/err_parse_inline_then_exit_disabled.rexx)
set_tests_properties(err_parse_inline_then_exit_disabled_fail PROPERTIES RESOURCE_LOCK ${CREXX_TEST_DRIVER_RESOURCE_LOCK})

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
add_crexx_compiler_test(inline_test_condition_contexts rexx_src/inline_test_condition_contexts.rexx)
add_crexx_compiler_test(inline_test_loop_bound_expr rexx_src/inline_test_loop_bound_expr.rexx)
add_crexx_compiler_test(inline_test_member_receiver_expr rexx_src/inline_test_member_receiver_expr.rexx)
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

# Macro to add a run test
macro(add_crexx_run_test NAME SOURCE_FILE)
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
    
    string(APPEND NOOPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${NAME}_run_noopt.rxbin\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
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
    set(OPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_opt.cmake")
    set(OPT_CONTENT "set(full_out \"\")\n")
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}_run_opt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")
    
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_opt.rxbin\" \"${NAME}_run_opt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")
    
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${NAME}_run_opt.rxbin\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")
    
    string(APPEND OPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${OPT_SCRIPT}" CONTENT "${OPT_CONTENT}")

    add_test(NAME ${NAME}_run_opt
             COMMAND ${CMAKE_COMMAND} -P "${OPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

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

    set(OPT_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}_args_opt.cmake")
    set(OPT_CONTENT "set(full_out \"\")\n")
    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"${NAME}_run_args_opt\" \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" -o \"${NAME}_run_args_opt.rxbin\" \"${NAME}_run_args_opt\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${NAME}_run_args_opt.rxbin\" -a${PROGRAM_ARGS_TEXT} OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND OPT_CONTENT "string(APPEND full_out \"\${out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxvm failed: \${out}\")\nendif()\n")

    string(APPEND OPT_CONTENT "file(READ \"${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out\" golden_out)\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"\${full_out}\")\n")
    string(APPEND OPT_CONTENT "string(REPLACE \"\\r\\n\" \"\\n\" golden_out \"\${golden_out}\")\n")
    string(APPEND OPT_CONTENT "if(NOT full_out STREQUAL golden_out)\n  message(FATAL_ERROR \"Output mismatch: \\nExpected:\\n\${golden_out}\\nGot:\\n\${full_out}\")\nendif()\n")
    file(GENERATE OUTPUT "${OPT_SCRIPT}" CONTENT "${OPT_CONTENT}")

    add_test(NAME ${NAME}_run_args_opt
             COMMAND ${CMAKE_COMMAND} -P "${OPT_SCRIPT}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

add_crexx_run_test(repro_dotted_index rexx_src/repro_dotted_index.rexx)
add_crexx_run_test(repro_expand_until rexx_src/repro_expand_until.rexx)
add_crexx_run_test(repro_array_arg rexx_src/repro_array_arg.rexx)
add_crexx_run_test(repro_array_trailing_concat rexx_src/repro_array_trailing_concat.rexx)
add_crexx_run_test(repro_opt_args rexx_src/repro_opt_args.rexx)
add_crexx_run_test(arg_semantics_scalar rexx_src/arg_semantics_scalar.rexx)
add_crexx_run_test(arg_semantics_array rexx_src/arg_semantics_array.rexx)
add_crexx_run_test(arg_semantics_object rexx_src/arg_semantics_object.rexx)
add_crexx_run_test(basic_loops rexx_src/basic_loops.rexx)
add_crexx_run_test(basic_strings rexx_src/basic_strings.rexx)
add_crexx_run_test(basic_arrays rexx_src/basic_arrays.rexx)
add_crexx_run_test(block_expr_basic rexx_src/block_expr_basic.rexx)
add_crexx_run_test(block_expr_short_circuit rexx_src/block_expr_short_circuit.rexx)
add_crexx_run_test(repro_args_parse rexx_src/repro_args_parse.rexx)
add_crexx_run_test(repro_args_alias rexx_src/repro_args_alias.rexx)
add_crexx_run_test(repro_sconcat rexx_src/repro_sconcat.rexx)
add_crexx_run_test(repro_implicit_address_abuttal rexx_src/repro_implicit_address_abuttal.rexx)
add_crexx_run_test(test_explicit_address rexx_src/test_explicit_address.rexx)
add_crexx_run_test(test_address_redirect rexx_src/test_address_redirect.rexx)
add_crexx_run_test(test_implicit_address rexx_src/test_implicit_address.rexx)
add_crexx_run_test(test_parse_statement rexx_src/test_parse_statement.rexx)
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
add_crexx_run_test(inline_test_condition_contexts rexx_src/inline_test_condition_contexts.rexx)
add_crexx_run_test(inline_test_loop_bound_expr rexx_src/inline_test_loop_bound_expr.rexx)
add_crexx_run_test(inline_test_member_receiver_expr rexx_src/inline_test_member_receiver_expr.rexx)
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
add_crexx_run_test(inline_test_complex_lhs_assign rexx_src/inline_test_complex_lhs_assign.rexx)
add_crexx_run_test(inline_test_binary_arg_return rexx_src/inline_test_binary_arg_return.rexx)

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
            syntaxhighlight_top_level_statements
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/01_assign.rexx
            MUST_MATCH
            "PARSE_TREE_STATEMENT"
            MUST_NOT_MATCH
            "PARSE_TREE_FUNCTION"
    )
    set_tests_properties(syntaxhighlight_top_level_statements PROPERTIES LABELS "compiler;syntax_highlighting;stage1")

    add_crexx_syntax_highlight_test(
            syntaxhighlight_block_comment
            ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/syntaxhighlight_block_comment.rexx
            MUST_MATCH
            "Node: type=38 \\(LEXER_COMMENT\\), pos=0, len=[0-9]+, sev=48"
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
    target_link_libraries(test_highlight_cache PRIVATE rxclib dslsyntax_editor)
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
            ${CMAKE_SOURCE_DIR}/binutils/include
            ${DSL_SH_SOURCE_DIR}/codebuffer)

    add_executable(test_source_semantics src/test_source_semantics.c)
    target_link_libraries(test_source_semantics PRIVATE rxclib dslsyntax_editor)
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
            ${CMAKE_SOURCE_DIR}/binutils/include
            ${DSL_SH_SOURCE_DIR}/codebuffer)

    add_executable(test_highlight_editor_diagnostics src/test_highlight_editor_diagnostics.c)
    target_link_libraries(test_highlight_editor_diagnostics PRIVATE rxclib dslsyntax_editor)
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
            ${CMAKE_SOURCE_DIR}/binutils/include
            ${DSL_SH_SOURCE_DIR}/codebuffer)
    add_dependencies(test_highlight_editor_diagnostics rxc)
endif()

# Commented out until fix found for multiple calls to the VM
add_test(NAME BridgeConnectivity COMMAND test_bridge)
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
