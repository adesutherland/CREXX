cmake_minimum_required(VERSION 3.24)

add_subdirectory(rxvmplugin)

project(Interpreter C)

set(CMAKE_C_STANDARD 99) # enables inline keyword
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG") # Can do more I am sure but this is about as good as it get in practice

##########################################
# Bare Interpreter
# Threaded Version
add_executable(rxvm rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
        ../utf8/utf.h ../machine/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvm machine avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxvm PRIVATE . ${CMAKE_BINARY_DIR}/machine ./rxvmplugin
        ../assembler ../machine ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)

# Bytecode Version
add_executable(rxbvm rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
        ../utf8/utf.h ../machine/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c)
target_link_libraries(rxbvm machine avl_tree rxvmplugin rxpa mc_decimal_manual platform m)
target_include_directories(rxbvm PRIVATE . ${CMAKE_BINARY_DIR}/machine ./rxvmplugin
        ../assembler ../machine ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)
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
		../utf8/utf.h ../machine/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c
		${CMAKE_CURRENT_BINARY_DIR}/library.c)
target_compile_definitions(rxvme PRIVATE LINK_CREXX_LIB=1)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvme machine avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxvme PRIVATE . ${CMAKE_BINARY_DIR}/machine ./rxvmplugin
		../assembler ../machine ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)
add_dependencies(rxvme packed_library)

# Bytecode Version
add_executable(rxbvme rxvmmain.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
		../utf8/utf.h ../machine/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c
		${CMAKE_CURRENT_BINARY_DIR}/library.c)
target_compile_definitions(rxbvme PRIVATE NTHREADED=1 LINK_CREXX_LIB=1)
target_link_libraries(rxbvme machine avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxbvme PRIVATE . ${CMAKE_BINARY_DIR}/machine ./rxvmplugin
		../assembler ../machine ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)
add_dependencies(rxbvme packed_library)

##########################################
# C Library Interpreter -  to be linked to make a standalone CREXX exe

# Threaded Version
add_library(rxvml STATIC rxvmlib.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
        ../utf8/utf.h ../machine/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvml machine avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxvml PRIVATE . ${CMAKE_BINARY_DIR}/machine ./rxvmplugin
		../assembler ../machine ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)

# Bytecode Version
add_library(rxbvml STATIC rxvmlib.c rxvmintp.h rxvmintp.c rxvmload.c interrupt.c ../avl_tree/rxastree.h rxsignal.h
		../utf8/utf.h ../machine/rxbin.h rxspawn.c exitfunc.c rxpafuncs.c)
target_link_libraries(rxbvml machine avl_tree rxpa rxvmplugin mc_decimal_manual platform m)
target_include_directories(rxbvml PRIVATE . ${CMAKE_BINARY_DIR}/machine ./rxvmplugin
		../assembler ../machine ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8)
target_compile_definitions(rxbvml PRIVATE NTHREADED=1)

# Value test executable
add_executable(ts_regvalue_tester ts_regvalue_tester.c rxvalue.h rxvmvars.h)
target_include_directories(ts_regvalue_tester PRIVATE . ../utf8 ../platform ../machine)
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
		OUTPUT raisesignaltest.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o raisesignaltest ${CMAKE_CURRENT_SOURCE_DIR}/raisesignaltest
		DEPENDS raisesignaltest.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Instruction Tests (raise uncaptured signal) ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_raisesignaltest ALL DEPENDS raisesignaltest.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasraisesignaltest
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm raisesignaltest
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_tests_properties(rxasraisesignaltest PROPERTIES
		PASS_REGULAR_EXPRESSION "PANIC:"
)

# Instruction Sanity Tests
add_custom_command(
		OUTPUT instructiontests.rxbin
		COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o instructiontests ${CMAKE_CURRENT_SOURCE_DIR}/instructiontests
		DEPENDS instructiontests.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_instructiontests ALL DEPENDS instructiontests.rxbin)

# Add the test to be executed after the custom command
add_test(
		NAME rxasinstructiontests
		COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm instructiontests
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(rxvmcopy ALL DEPENDS ${CMAKE_BINARY_DIR}/rxvm)
add_custom_target(rxvmecopy ALL DEPENDS ${CMAKE_BINARY_DIR}/rxvme)

add_custom_command(TARGET rxvmcopy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/interpreter/rxvm${EXE_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
)
add_custom_command(TARGET rxvmecopy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/interpreter/rxvme${EXE_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
)

