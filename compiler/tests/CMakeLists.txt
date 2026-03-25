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

enable_testing()

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

# Robustness tests
macro(add_crexx_robustness_test NAME MAIN_FILE)
    # The list of dependencies are passed as additional arguments
    set(DEPS ${ARGN})
    set(DEP_BINS "")
    set(BUILD_DEPS_CMD "")
    foreach(DEP ${DEPS})
        # Extract lib name from filename
        get_filename_component(DEP_NAME ${DEP} NAME_WE)
        set(BUILD_DEPS_CMD "${BUILD_DEPS_CMD} && $<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness -o ${DEP_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/${DEP}")
        set(BUILD_DEPS_CMD "${BUILD_DEPS_CMD} && $<TARGET_FILE:rxas> -o ${DEP_NAME}.rxbin ${DEP_NAME}")
        set(DEP_BINS "${DEP_BINS} ${DEP_NAME}.rxbin")
    endforeach()

    add_test(NAME robustness_${NAME}
             COMMAND bash -c "$<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness -i ${CMAKE_BINARY_DIR}/bin -o ${NAME} ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/${MAIN_FILE} && $<TARGET_FILE:rxas> -o ${NAME}.rxbin ${NAME} ${BUILD_DEPS_CMD} && $<TARGET_FILE:rxvm> ${NAME}.rxbin ${DEP_BINS} ${CMAKE_BINARY_DIR}/bin/library.rxbin | diff -u ${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out -"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

add_crexx_robustness_test(set1_main set1_main.rexx set1_lib.rexx)
add_crexx_robustness_test(set2_main set2_main.rexx set2_lib.rexx)
add_crexx_robustness_test(set3_main set3_main.rexx set3_lib.rexx)
add_crexx_robustness_test(set4_main set4_main.rexx set4_mid.rexx set4_base.rexx)

# Set 5 - Encapsulation (Expected to fail compilation)
add_test(NAME robustness_set5_encapsulation
         COMMAND bash -c "! $<TARGET_FILE:rxc> -i ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness -o set5_main ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/robustness/set5_main.rexx"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Example tests
add_crexx_compiler_test(01_assign rexx_src/01_assign.rexx)
add_crexx_compiler_test(02_flow rexx_src/02_flow.rexx)
add_crexx_compiler_test(03_calc rexx_src/03_calc.rexx)
add_crexx_compiler_test(04_literals rexx_src/04_literals.rexx)
add_crexx_compiler_test(05_ops_common rexx_src/05_ops_common.rexx)
add_crexx_compiler_test(05_ops_classic rexx_src/05_ops_classic.rexx)
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

# Warning tests
add_crexx_compiler_warning_test(loop rexx_src/loop.rexx loop.txt)

# Regression tests for rxpp issue
add_crexx_compiler_test(repro_dotted_index rexx_src/repro_dotted_index.rexx)
add_crexx_compiler_test(repro_expand_until rexx_src/repro_expand_until.rexx)
add_crexx_compiler_test(repro_array_arg rexx_src/repro_array_arg.rexx)
add_crexx_compiler_test(repro_opt_args rexx_src/repro_opt_args.rexx)
add_crexx_compiler_test(basic_loops rexx_src/basic_loops.rexx)
add_crexx_compiler_test(basic_strings rexx_src/basic_strings.rexx)
add_crexx_compiler_test(basic_arrays rexx_src/basic_arrays.rexx)
add_crexx_compiler_test(block_expr_basic rexx_src/block_expr_basic.rexx)
add_crexx_compiler_test(block_expr_short_circuit rexx_src/block_expr_short_circuit.rexx)
add_crexx_compiler_test(inline_test1 rexx_src/inline_test1.rexx)

# Macro to add a run test
macro(add_crexx_run_test NAME SOURCE_FILE)
    add_test(NAME ${NAME}_run
             COMMAND bash -c "($<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o ${NAME}_run ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE} 2>&1 && $<TARGET_FILE:rxas> -o ${NAME}_run.rxbin ${NAME}_run 2>&1 && $<TARGET_FILE:rxvm> ${CMAKE_BINARY_DIR}/bin/library.rxbin ${NAME}_run.rxbin 2>&1) | diff -u ${CMAKE_CURRENT_SOURCE_DIR}/golden/run/${NAME}.out -"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endmacro()

add_crexx_run_test(repro_dotted_index rexx_src/repro_dotted_index.rexx)
add_crexx_run_test(repro_expand_until rexx_src/repro_expand_until.rexx)
add_crexx_run_test(repro_array_arg rexx_src/repro_array_arg.rexx)
add_crexx_run_test(repro_array_trailing_concat rexx_src/repro_array_trailing_concat.rexx)
add_crexx_run_test(repro_opt_args rexx_src/repro_opt_args.rexx)
add_crexx_run_test(basic_loops rexx_src/basic_loops.rexx)
add_crexx_run_test(basic_strings rexx_src/basic_strings.rexx)
add_crexx_run_test(basic_arrays rexx_src/basic_arrays.rexx)
add_crexx_run_test(block_expr_basic rexx_src/block_expr_basic.rexx)
add_crexx_run_test(block_expr_short_circuit rexx_src/block_expr_short_circuit.rexx)
add_crexx_run_test(repro_args_parse rexx_src/repro_args_parse.rexx)
add_crexx_run_test(repro_args_alias rexx_src/repro_args_alias.rexx)
add_crexx_run_test(repro_sconcat rexx_src/repro_sconcat.rexx)
add_crexx_run_test(test_implicit_address rexx_src/test_implicit_address.rexx)
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
add_test(NAME test_disjoint_detailed_warn
         COMMAND bash -c "$<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_disjoint_detailed_warn ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/test_disjoint_detailed.rexx 2>&1 | grep -q 'original definition @ 4:5'"
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
# DO constructs parsing coverage
add_crexx_parsing_test(do_simple_parse parsing_src/do_simple_parse.rexx)
add_crexx_parsing_test(do_counted_parse parsing_src/do_counted_parse.rexx)
add_crexx_parsing_test(do_while_parse parsing_src/do_while_parse.rexx)
add_crexx_parsing_test(do_until_parse parsing_src/do_until_parse.rexx)
add_crexx_parsing_test(block_expr_parse parsing_src/block_expr_parse.rexx)

# SELECT constructs parsing coverage
add_crexx_parsing_test(select_classic_parse parsing_src/select_classic_parse.rexx)
add_crexx_parsing_test(select_c_parse parsing_src/select_c_parse.rexx)

# Bridge Connectivity Test
add_custom_command(
        OUTPUT stub_plugin.rxbin
        COMMAND $<TARGET_FILE:rxc> -n -o stub_plugin ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/stub_plugin.rexx
        COMMAND $<TARGET_FILE:rxas> -o stub_plugin stub_plugin
        DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/stub_plugin.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling stub_plugin.rexx ..."
)

add_custom_target(stub_plugin_bin DEPENDS stub_plugin.rxbin)

add_executable(test_bridge src/test_bridge.c)
target_link_libraries(test_bridge PRIVATE rxvml rxpashim)
target_include_directories(test_bridge PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
add_dependencies(test_bridge stub_plugin_bin)

# Commented out until fix found for multiple calls to the VM
add_test(NAME BridgeConnectivity COMMAND test_bridge)
