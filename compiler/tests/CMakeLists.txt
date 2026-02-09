cmake_minimum_required(VERSION 3.24)

project(CompilerTests C)

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
             $<TARGET_FILE:rxc> ${COMPILER_NOOPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_noopt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})

    # Test 2 (Opt)
    add_test(NAME ${NAME}_opt
             COMMAND crexx_test_driver
             ${CMAKE_CURRENT_SOURCE_DIR}/golden/opt/${NAME}.rxas
             ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt.rxas
             $<TARGET_FILE:rxc> ${COMPILER_OPT_FLAG} -o ${CMAKE_CURRENT_BINARY_DIR}/${NAME}_opt ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
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

# Example tests
add_crexx_compiler_test(01_assign rexx_src/01_assign.rexx)
add_crexx_compiler_test(02_flow rexx_src/02_flow.rexx)
add_crexx_compiler_test(03_calc rexx_src/03_calc.rexx)
add_crexx_compiler_test(04_literals rexx_src/04_literals.rexx)
add_crexx_compiler_test(05_ops_common rexx_src/05_ops_common.rexx)
add_crexx_compiler_test(05_ops_classic rexx_src/05_ops_classic.rexx)
add_crexx_compiler_test(06_logic rexx_src/06_logic.rexx)
add_crexx_compiler_test(07_loops rexx_src/07_loops.rexx)
add_crexx_compiler_test(08_select rexx_src/08_select.rexx)
add_crexx_compiler_test(09_nested rexx_src/09_nested.rexx)
add_crexx_compiler_test(10_procs rexx_src/10_procs.rexx)
add_crexx_compiler_test(11_scope rexx_src/11_scope.rexx)
add_crexx_compiler_test(12_arrays_lvlb rexx_src/12_arrays_lvlb.rexx)
add_crexx_compiler_test(13_stems rexx_src/13_stems.rexx)
add_crexx_compiler_test(14_strings rexx_src/14_strings.rexx)
add_crexx_compiler_test(val_magic_01 rexx_src/val_magic_01.rexx)
add_crexx_compiler_test(val_magic_02 rexx_src/val_magic_02.rexx)
add_crexx_compiler_test(opt_decimal_safety rexx_src/opt_decimal_safety.rexx)
add_crexx_compiler_test(opt_numeric_safety rexx_src/opt_numeric_safety.rexx)
add_crexx_compiler_dep_test(15_imports rexx_src/15_imports.rexx rexx_src/mymath.rexx)
add_crexx_parsing_test(16_classes rexx_src/16_classes.rexx)

# Negative tests
add_crexx_compiler_fail_test(err_01_type rexx_src/err_01_type.rexx err_01_type.txt)
add_crexx_compiler_fail_test(err_02_syntax rexx_src/err_02_syntax.rexx err_02_syntax.txt)

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
target_link_libraries(test_bridge rxvml avl_tree platform rxpa mc_decimal_manual m)
target_include_directories(test_bridge PRIVATE ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/binutils/include)
add_dependencies(test_bridge stub_plugin_bin)

# Commented out until fix found for multiple calls to the VM
# add_test(NAME BridgeConnectivity COMMAND test_bridge)
