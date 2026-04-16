cmake_minimum_required(VERSION 3.24)

add_subdirectory(rxvmplugin)

project(Interpreter C)

set(CMAKE_C_STANDARD 99) # enables inline keyword
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG") # Can do more I am sure but this is about as good as it get in practice

##########################################
# Bare Interpreter
# Threaded Version
add_executable(rxvm rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
        ../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c rxvml/rxvm_run.c)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvm rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxvm PRIVATE . ./rxvmplugin
        ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)

# Bytecode Version
add_executable(rxbvm rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
        ../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c rxvml/rxvm_run.c)
target_link_libraries(rxbvm rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxbvm PRIVATE . ./rxvmplugin
        ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
target_compile_definitions(rxbvm PRIVATE NTHREADED=1)

##########################################
# Interpreter linked to CREXX Stdlib

# Packed Library
add_custom_command(
        OUTPUT library.c
        COMMAND $<TARGET_FILE:rxcpack> -o library ${CMAKE_BINARY_DIR}/bin/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library rxcpack ${CMAKE_BINARY_DIR}/bin/library.rxbin
        COMMENT "C Packing cREXX RXFNSB Library ..."
)
add_custom_target(packed_library ALL DEPENDS library.c)

# Threaded Version
add_executable(rxvme rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
		../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c
		${CMAKE_CURRENT_BINARY_DIR}/library.c rxvml/rxvm_run.c)
target_compile_definitions(rxvme PRIVATE LINK_CREXX_LIB=1)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvme rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxvme PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
add_dependencies(rxvme packed_library)

# Bytecode Version
add_executable(rxbvme rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
		../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c
		${CMAKE_CURRENT_BINARY_DIR}/library.c rxvml/rxvm_run.c)
target_compile_definitions(rxbvme PRIVATE NTHREADED=1 LINK_CREXX_LIB=1)
target_link_libraries(rxbvme rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxbvme PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
add_dependencies(rxbvme packed_library)

##########################################
# C Library Interpreter -  to be linked to make a standalone CREXX exe

# Threaded Version
add_library(rxvml STATIC rxvmlib.c rxvml.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
        ../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c rxvml/rxvm_run.c)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvml rxvmplugin mc_decimal_manual avl_tree rxpa)
target_include_directories(rxvml PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)

# Bytecode Version
add_library(rxbvml STATIC rxvmlib.c rxvml.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
		../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c rxvml/rxvm_run.c)
target_link_libraries(rxbvml rxvmplugin mc_decimal_manual avl_tree rxpa)
target_include_directories(rxbvml PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
target_compile_definitions(rxbvml PRIVATE NTHREADED=1)

# RXPA Shim Library - Provides standard rxpa_ symbols for standalone targets
add_library(rxpashim STATIC rxpashim.c)
target_include_directories(rxpashim PUBLIC . ${CMAKE_SOURCE_DIR}/rxpa)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Value test executable
add_executable(ts_regvalue_tester tests/ts_regvalue_tester.c rxvalue.h rxvmvars.h)
target_include_directories(ts_regvalue_tester PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} . ../utf8 ../platform ../binutils/include)
target_link_libraries(ts_regvalue_tester m)

# Enable testing
enable_testing()

function(crexx_add_vm_dual_test)
    set(options)
    set(oneValueArgs NAME PASS_REGULAR_EXPRESSION)
    set(multiValueArgs RXVM_ARGS)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_test(
            NAME ${CREXX_NAME}
            COMMAND $<TARGET_FILE:rxvm> ${CREXX_RXVM_ARGS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_test(
            NAME ${CREXX_NAME}-rxbvm
            COMMAND $<TARGET_FILE:rxbvm> ${CREXX_RXVM_ARGS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    if(CREXX_PASS_REGULAR_EXPRESSION)
        set_tests_properties(${CREXX_NAME} ${CREXX_NAME}-rxbvm PROPERTIES
                PASS_REGULAR_EXPRESSION "${CREXX_PASS_REGULAR_EXPRESSION}")
    endif()
endfunction()

# Value Tests
add_test(
		NAME ts_regvalue_tester
		COMMAND ts_regvalue_tester
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Instruction Sanity Tests - Raise uncaptured signal
add_custom_command(
		OUTPUT tests_signals.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_signals ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_signals
		DEPENDS tests/tests_signals.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Instruction Tests (raise uncaptured signal) ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_signals ALL DEPENDS tests_signals.rxbin)

crexx_add_vm_dual_test(NAME rxassignalstests RXVM_ARGS tests_signals PASS_REGULAR_EXPRESSION "PANIC:")

# Basic Instruction Tests
add_custom_command(
		OUTPUT tests_basic.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_basic ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_basic
		DEPENDS tests/tests_basic.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Basic Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_basic ALL DEPENDS tests_basic.rxbin)

crexx_add_vm_dual_test(NAME rxasbasictests RXVM_ARGS tests_basic)

# Integer Math Instruction Tests
add_custom_command(
		OUTPUT tests_int_math.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_int_math ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_int_math
		DEPENDS tests/tests_int_math.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Integer Math Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_int_math ALL DEPENDS tests_int_math.rxbin)

crexx_add_vm_dual_test(NAME rxasintmathtests RXVM_ARGS tests_int_math)

# Float Math Instruction Tests
add_custom_command(
		OUTPUT tests_float_math.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_float_math ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_float_math
		DEPENDS tests/tests_float_math.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Float Math Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_float_math ALL DEPENDS tests_float_math.rxbin)

crexx_add_vm_dual_test(NAME rxasfloatmathtests RXVM_ARGS tests_float_math)

# String Instruction Tests
add_custom_command(
		OUTPUT tests_string.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_string ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_string
		DEPENDS tests/tests_string.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS String Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_string ALL DEPENDS tests_string.rxbin)

crexx_add_vm_dual_test(NAME rxasstringtests RXVM_ARGS tests_string)

# Logic and Flow Instruction Tests
add_custom_command(
		OUTPUT tests_logic_flow.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_logic_flow ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_logic_flow
		DEPENDS tests/tests_logic_flow.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Logic and Flow Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_logic_flow ALL DEPENDS tests_logic_flow.rxbin)

crexx_add_vm_dual_test(NAME rxaslogicflowtests RXVM_ARGS tests_logic_flow)

# System and I/O Instruction Tests
add_custom_command(
		OUTPUT tests_system_io.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_system_io ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_system_io
		DEPENDS tests/tests_system_io.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS System and I/O Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_system_io ALL DEPENDS tests_system_io.rxbin)

crexx_add_vm_dual_test(NAME rxassystemiotests RXVM_ARGS tests_system_io)

# Decimal Instruction Tests
add_custom_command(
		OUTPUT tests_decimal.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_decimal ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_decimal
		DEPENDS tests/tests_decimal.rxas rxas rxvm mc_decimal_dynamic db_decimal_manual
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Decimal Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_decimal ALL DEPENDS tests_decimal.rxbin)

crexx_add_vm_dual_test(NAME rxasdecimaltests RXVM_ARGS tests_decimal)
crexx_add_vm_dual_test(NAME rxasdecimaltests-mc RXVM_ARGS -p rxvm_mc_decimal tests_decimal)

# Conversion and Formatting Instruction Tests
add_custom_command(
		OUTPUT tests_conversion.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_conversion ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_conversion
		DEPENDS tests/tests_conversion.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Conversion Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_conversion ALL DEPENDS tests_conversion.rxbin)

crexx_add_vm_dual_test(NAME rxasconversiontests RXVM_ARGS tests_conversion)

# Object and Attribute Instruction Tests
add_custom_command(
		OUTPUT tests_objects.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_objects ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_objects
		DEPENDS tests/tests_objects.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Object Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_objects ALL DEPENDS tests_objects.rxbin)

crexx_add_vm_dual_test(NAME rxasobjecttests RXVM_ARGS tests_objects)

# Reflection and Introspection Instruction Tests
add_custom_command(
		OUTPUT tests_reflection.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_reflection ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_reflection
		DEPENDS tests/tests_reflection.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Reflection Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_reflection ALL DEPENDS tests_reflection.rxbin)

crexx_add_vm_dual_test(NAME rxasreflectiontests RXVM_ARGS tests_reflection)

# Stress and Edge Case Instruction Tests
add_custom_command(
		OUTPUT tests_stress.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_stress ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_stress
		DEPENDS tests/tests_stress.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Stress Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_stress ALL DEPENDS tests_stress.rxbin)

crexx_add_vm_dual_test(NAME rxasstresstests RXVM_ARGS tests_stress)

# Re-entrancy Test
add_custom_command(
        OUTPUT test_reentrancy.rxbin
        COMMAND $<TARGET_FILE:rxc> -o test_reentrancy ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_reentrancy.rexx
        COMMAND $<TARGET_FILE:rxas> test_reentrancy
        DEPENDS tests/rexx/test_reentrancy.rexx rxc rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_reentrancy.rexx ..."
)
add_custom_target(test_reentrancy_bin ALL DEPENDS test_reentrancy.rxbin)

add_executable(test_reentrancy tests/test_reentrancy.c)
target_include_directories(test_reentrancy PRIVATE ${CMAKE_SOURCE_DIR}/inc)
target_link_libraries(test_reentrancy rxvml)
add_dependencies(test_reentrancy test_reentrancy_bin)

add_test(NAME reentrancy_check COMMAND test_reentrancy WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Archive-link regression test for manual db_decimal packaging.
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link
        COMMAND ${CMAKE_C_COMPILER}
                -O0
                -I${CMAKE_CURRENT_SOURCE_DIR}
                -I${CMAKE_CURRENT_SOURCE_DIR}/rxvmplugin
                -I${CMAKE_SOURCE_DIR}/inc
                -o ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_db_decimal_archive_link.c
                $<TARGET_FILE:rxvml>
                $<TARGET_FILE:rxpashim>
                $<TARGET_FILE:rxvmplugin>
                $<TARGET_FILE:platform>
                $<TARGET_FILE:decnumber>
                $<TARGET_FILE:avl_tree>
                $<TARGET_FILE:rxpa>
                -lm
                $<TARGET_FILE:db_decimal_manual>
        DEPENDS
                tests/test_db_decimal_archive_link.c
                rxvml
                rxpashim
                rxvmplugin
                platform
                decnumber
                avl_tree
                rxpa
                db_decimal_manual
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Linking archive-level db_decimal regression harness ..."
        VERBATIM
)
add_custom_target(test_db_decimal_archive_link_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link)
add_test(NAME db_decimal_archive_link COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
