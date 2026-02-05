cmake_minimum_required(VERSION 3.24)

add_subdirectory(rxvmplugin)

project(Interpreter C)

set(CMAKE_C_STANDARD 99) # enables inline keyword
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG") # Can do more I am sure but this is about as good as it get in practice

##########################################
# Bare Interpreter
# Threaded Version
add_executable(rxvm rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
        ../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvm avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxvm PRIVATE . ./rxvmplugin
        ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)

# Bytecode Version
add_executable(rxbvm rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
        ../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c)
target_link_libraries(rxbvm avl_tree rxvmplugin rxpa mc_decimal_manual platform m)
target_include_directories(rxbvm PRIVATE . ./rxvmplugin
        ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)
target_compile_definitions(rxbvm PRIVATE NTHREADED=1)

##########################################
# Interpreter linked to CREXX Stdlib

# Packed Library
add_custom_command(
        OUTPUT library.c
        COMMAND ${CMAKE_BINARY_DIR}/cpacker/rxcpack -o library ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library rxcpack ${CMAKE_BINARY_DIR}/lib/rxfnsb/library.rxbin
        COMMENT "C Packing cREXX RXFNSB Library ..."
)
add_custom_target(packed_library ALL DEPENDS library.c)

# Threaded Version
add_executable(rxvme rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
		../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c
		${CMAKE_CURRENT_BINARY_DIR}/library.c)
target_compile_definitions(rxvme PRIVATE LINK_CREXX_LIB=1)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvme avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxvme PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)
add_dependencies(rxvme packed_library)

# Bytecode Version
add_executable(rxbvme rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
		../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c
		${CMAKE_CURRENT_BINARY_DIR}/library.c)
target_compile_definitions(rxbvme PRIVATE NTHREADED=1 LINK_CREXX_LIB=1)
target_link_libraries(rxbvme avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxbvme PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)
add_dependencies(rxbvme packed_library)

##########################################
# C Library Interpreter -  to be linked to make a standalone CREXX exe

# Threaded Version
add_library(rxvml STATIC rxvmlib.c rxvml.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
        ../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvml avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxvml PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)

# Bytecode Version
add_library(rxbvml STATIC rxvmlib.c rxvml.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
		../utf8/utf.h ../binutils/include/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c)
target_link_libraries(rxbvml avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxbvml PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)
target_compile_definitions(rxbvml PRIVATE NTHREADED=1)

# Value test executable
add_executable(ts_regvalue_tester tests/ts_regvalue_tester.c rxvalue.h rxvmvars.h)
target_include_directories(ts_regvalue_tester PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} . ../utf8 ../platform ../binutils/include)
target_link_libraries(ts_regvalue_tester m)

# Enable testing
enable_testing()

# Value Tests
add_test(
		NAME ts_regvalue_tester
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ts_regvalue_tester
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Instruction Sanity Tests - Raise uncaptured signal
add_custom_command(
		OUTPUT tests_signals.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_signals ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_signals
		DEPENDS tests/tests_signals.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Instruction Tests (raise uncaptured signal) ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_signals ALL DEPENDS tests_signals.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxassignalstests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_signals
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_tests_properties(rxassignalstests PROPERTIES
		PASS_REGULAR_EXPRESSION "PANIC:"
)

# Basic Instruction Tests
add_custom_command(
		OUTPUT tests_basic.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_basic ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_basic
		DEPENDS tests/tests_basic.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Basic Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_basic ALL DEPENDS tests_basic.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasbasictests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_basic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Integer Math Instruction Tests
add_custom_command(
		OUTPUT tests_int_math.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_int_math ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_int_math
		DEPENDS tests/tests_int_math.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Integer Math Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_int_math ALL DEPENDS tests_int_math.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasintmathtests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_int_math
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Float Math Instruction Tests
add_custom_command(
		OUTPUT tests_float_math.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_float_math ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_float_math
		DEPENDS tests/tests_float_math.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Float Math Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_float_math ALL DEPENDS tests_float_math.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasfloatmathtests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_float_math
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# String Instruction Tests
add_custom_command(
		OUTPUT tests_string.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_string ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_string
		DEPENDS tests/tests_string.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS String Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_string ALL DEPENDS tests_string.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasstringtests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_string
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Logic and Flow Instruction Tests
add_custom_command(
		OUTPUT tests_logic_flow.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_logic_flow ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_logic_flow
		DEPENDS tests/tests_logic_flow.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Logic and Flow Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_logic_flow ALL DEPENDS tests_logic_flow.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxaslogicflowtests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_logic_flow
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# System and I/O Instruction Tests
add_custom_command(
		OUTPUT tests_system_io.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_system_io ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_system_io
		DEPENDS tests/tests_system_io.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS System and I/O Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_system_io ALL DEPENDS tests_system_io.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxassystemiotests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_system_io
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Decimal Instruction Tests
add_custom_command(
		OUTPUT tests_decimal.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_decimal ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_decimal
		DEPENDS tests/tests_decimal.rxas rxas rxvm mc_decimal_dynamic db_decimal_manual
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Decimal Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_decimal ALL DEPENDS tests_decimal.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasdecimaltests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_decimal
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(
		NAME rxasdecimaltests-mc
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm -p ./rxvmplugin/rxvmplugins/mc_decimal/rxvm_mc_decimal tests_decimal
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Conversion and Formatting Instruction Tests
add_custom_command(
		OUTPUT tests_conversion.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_conversion ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_conversion
		DEPENDS tests/tests_conversion.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Conversion Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_conversion ALL DEPENDS tests_conversion.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasconversiontests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_conversion
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Object and Attribute Instruction Tests
add_custom_command(
		OUTPUT tests_objects.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_objects ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_objects
		DEPENDS tests/tests_objects.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Object Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_objects ALL DEPENDS tests_objects.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasobjecttests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_objects
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Reflection and Introspection Instruction Tests
add_custom_command(
		OUTPUT tests_reflection.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_reflection ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_reflection
		DEPENDS tests/tests_reflection.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Reflection Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_reflection ALL DEPENDS tests_reflection.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasreflectiontests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_reflection
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Stress and Edge Case Instruction Tests
add_custom_command(
		OUTPUT tests_stress.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o tests_stress ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_stress
		DEPENDS tests/tests_stress.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Stress Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_stress ALL DEPENDS tests_stress.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasstresstests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tests_stress
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(rxvmcopy ALL DEPENDS rxvm)
add_custom_target(rxvmecopy ALL DEPENDS rxvme)

add_custom_command(TARGET rxvmcopy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/interpreter/rxvm${EXE_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
)
add_custom_command(TARGET rxvmecopy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/interpreter/rxvme${EXE_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
)

