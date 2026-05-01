cmake_minimum_required(VERSION 3.24)

add_subdirectory(rxvmplugin)

project(Interpreter C)

set(CMAKE_C_STANDARD 99) # enables inline keyword
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG") # Can do more I am sure but this is about as good as it get in practice

set(CREXX_SOCKET_TARGET_LIBS)
set(CREXX_SOCKET_MANUAL_LINK_LIBS)
set(CREXX_NATIVE_RUNTIME_LINK_FLAGS)
if(WIN32)
    set(CREXX_SOCKET_TARGET_LIBS ws2_32)
    set(CREXX_SOCKET_MANUAL_LINK_LIBS -lws2_32)
endif()

if(WIN32)
    set(CREXX_DEFAULT_TLS_BACKEND SCHANNEL)
elseif(APPLE)
    set(CREXX_DEFAULT_TLS_BACKEND NETWORK)
else()
    set(CREXX_DEFAULT_TLS_BACKEND OPENSSL)
endif()

set(CREXX_ENABLE_TLS "${CREXX_DEFAULT_TLS_BACKEND}" CACHE STRING "TLS backend for VM core sockets: OFF, OPENSSL, NETWORK, or SCHANNEL")
set_property(CACHE CREXX_ENABLE_TLS PROPERTY STRINGS OFF OPENSSL NETWORK SCHANNEL)
option(CREXX_TLS_STATIC_OPENSSL "Prefer static OpenSSL libraries for the VM core socket TLS backend" OFF)
string(TOUPPER "${CREXX_ENABLE_TLS}" CREXX_ENABLE_TLS_NORMALIZED)
if(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "ON")
    set(CREXX_ENABLE_TLS_NORMALIZED "OPENSSL")
endif()

if(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "OPENSSL")
    set(OPENSSL_USE_STATIC_LIBS ${CREXX_TLS_STATIC_OPENSSL})
    find_package(OpenSSL REQUIRED)
    list(APPEND CREXX_SOCKET_TARGET_LIBS OpenSSL::SSL OpenSSL::Crypto)
    list(APPEND CREXX_SOCKET_MANUAL_LINK_LIBS ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
    list(APPEND CREXX_NATIVE_RUNTIME_LINK_FLAGS -lssl -lcrypto)
elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "NETWORK")
    if(NOT APPLE)
        message(FATAL_ERROR "CREXX_ENABLE_TLS=NETWORK is only supported on Apple platforms.")
    endif()
    find_library(CREXX_NETWORK_FRAMEWORK Network REQUIRED)
    find_library(CREXX_SECURITY_FRAMEWORK Security REQUIRED)
    find_library(CREXX_COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
    list(APPEND CREXX_SOCKET_TARGET_LIBS
            ${CREXX_NETWORK_FRAMEWORK}
            ${CREXX_SECURITY_FRAMEWORK}
            ${CREXX_COREFOUNDATION_FRAMEWORK})
    list(APPEND CREXX_SOCKET_MANUAL_LINK_LIBS
            -framework Network
            -framework Security
            -framework CoreFoundation)
    list(APPEND CREXX_NATIVE_RUNTIME_LINK_FLAGS
            -framework Network
            -framework Security
            -framework CoreFoundation)
elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "SCHANNEL")
    if(NOT WIN32)
        message(FATAL_ERROR "CREXX_ENABLE_TLS=SCHANNEL is only supported on Windows.")
    endif()
    list(APPEND CREXX_SOCKET_TARGET_LIBS secur32 crypt32)
    list(APPEND CREXX_SOCKET_MANUAL_LINK_LIBS -lsecur32 -lcrypt32)
    list(APPEND CREXX_NATIVE_RUNTIME_LINK_FLAGS -lsecur32 -lcrypt32)
elseif(NOT CREXX_ENABLE_TLS_NORMALIZED STREQUAL "OFF")
    message(FATAL_ERROR "Unsupported CREXX_ENABLE_TLS value '${CREXX_ENABLE_TLS}'. Use OFF, OPENSSL, NETWORK, or SCHANNEL.")
endif()
string(JOIN " " CREXX_NATIVE_RUNTIME_LINK_FLAGS_TEXT ${CREXX_NATIVE_RUNTIME_LINK_FLAGS})
set(CREXX_NATIVE_RUNTIME_LINK_FLAGS_TEXT "${CREXX_NATIVE_RUNTIME_LINK_FLAGS_TEXT}"
        CACHE INTERNAL "Native executable link flags required by the configured VM runtime" FORCE)
message(STATUS "cREXX VM core socket TLS backend: ${CREXX_ENABLE_TLS_NORMALIZED}")

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

function(crexx_enable_core_sockets target_name)
    target_sources(${target_name} PRIVATE rxvmsock.c rxvmsock.h)
    if(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "OPENSSL")
        target_compile_definitions(${target_name} PRIVATE CREXX_TLS_OPENSSL=1)
    elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "NETWORK")
        target_compile_definitions(${target_name} PRIVATE CREXX_TLS_NETWORK=1)
        target_compile_options(${target_name} PRIVATE -fblocks)
    elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "SCHANNEL")
        target_compile_definitions(${target_name} PRIVATE CREXX_TLS_SCHANNEL=1)
    endif()
    if(CREXX_SOCKET_TARGET_LIBS)
        target_link_libraries(${target_name} ${CREXX_SOCKET_TARGET_LIBS})
    endif()
endfunction()

foreach(_rxvm_core_target rxvm rxbvm rxvme rxbvme rxvml rxbvml)
    crexx_enable_core_sockets(${_rxvm_core_target})
endforeach()

# libcrexxsaa is intentionally thin and shared. On ELF platforms, every static
# archive pulled into it must be compiled as PIC.
foreach(_crexxsaa_static_dep rxvml rxvmplugin mc_decimal_manual avl_tree rxpa platform decnumber)
    if(TARGET ${_crexxsaa_static_dep})
        set_target_properties(${_crexxsaa_static_dep} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()
endforeach()

add_library(crexxsaa SHARED crexxsaa.c crexxsaa.h)
target_link_libraries(crexxsaa PUBLIC rxvml rxvmplugin mc_decimal_manual avl_tree rxpa)
target_include_directories(crexxsaa PUBLIC
        .
        ./rxvmplugin
        ../assembler
        ../binutils/include
        ${CMAKE_SOURCE_DIR}/rxpa
        ../platform
        ../avl_tree
        ../utf8
        ../inc)
set_target_properties(crexxsaa PROPERTIES
        OUTPUT_NAME crexxsaa
        POSITION_INDEPENDENT_CODE ON)
install(TARGETS crexxsaa
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION lib)
install(FILES crexxsaa.h DESTINATION include)

add_executable(crexxsaa_tool crexxsaa_tool.c)
target_link_libraries(crexxsaa_tool PRIVATE crexxsaa)
target_include_directories(crexxsaa_tool PRIVATE .)
set_target_properties(crexxsaa_tool PROPERTIES
        OUTPUT_NAME crexxsaa)
if(APPLE)
    set_target_properties(crexxsaa_tool PROPERTIES
            BUILD_RPATH "${CMAKE_BINARY_DIR}/bin"
            INSTALL_RPATH "@loader_path")
elseif(UNIX)
    set_target_properties(crexxsaa_tool PROPERTIES
            BUILD_RPATH "${CMAKE_BINARY_DIR}/bin"
            INSTALL_RPATH "$ORIGIN")
endif()
install(TARGETS crexxsaa_tool RUNTIME DESTINATION bin)

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

add_test(
        NAME crexxsaa_cache_tool_location
        COMMAND crexxsaa_tool --cache-dir ${CMAKE_CURRENT_BINARY_DIR}/crexxsaa-cache-tool-test --location)
set_tests_properties(crexxsaa_cache_tool_location PROPERTIES
        PASS_REGULAR_EXPRESSION "crexxsaa-cache-tool-test")

add_test(
        NAME crexxsaa_cache_tool_clear
        COMMAND crexxsaa_tool --cache-dir ${CMAKE_CURRENT_BINARY_DIR}/crexxsaa-cache-tool-test --clear --list)
set_tests_properties(crexxsaa_cache_tool_clear PROPERTIES
        PASS_REGULAR_EXPRESSION "cleared")

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

function(crexx_add_rxas_vm_binary SOURCE_NAME DESCRIPTION)
    add_custom_command(
            OUTPUT ${SOURCE_NAME}.rxbin
            COMMAND $<TARGET_FILE:rxas> -o ${SOURCE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/tests/${SOURCE_NAME}
            DEPENDS tests/${SOURCE_NAME}.rxas rxas rxvm mc_decimal_dynamic
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "cREXX RXAS Instruction Tests (${DESCRIPTION}) ..."
    )

    add_custom_target(run_${SOURCE_NAME} ALL DEPENDS ${SOURCE_NAME}.rxbin)
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

# Instruction Sanity Tests - fallback panic location and source reporting
add_custom_command(
		OUTPUT tests_panic_source.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_panic_source ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_panic_source
		DEPENDS tests/tests_panic_source.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Instruction Tests (fallback panic source reporting) ..."
)

add_custom_target(run_tests_panic_source ALL DEPENDS tests_panic_source.rxbin)

crexx_add_vm_dual_test(NAME rxaspanicaddresstests RXVM_ARGS tests_panic_source PASS_REGULAR_EXPRESSION "address [0-9]+ \\(0x[0-9a-fA-F]+\\)")
crexx_add_vm_dual_test(NAME rxaspanicsourcetests RXVM_ARGS tests_panic_source PASS_REGULAR_EXPRESSION "source: panic_source\\.rexx:7:3:")

# Instruction Sanity Tests - breakpoint reports the next instruction address
add_custom_command(
		OUTPUT tests_breakpoint_address.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_breakpoint_address ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_breakpoint_address
		DEPENDS tests/tests_breakpoint_address.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Instruction Tests (breakpoint address reporting) ..."
)

add_custom_target(run_tests_breakpoint_address ALL DEPENDS tests_breakpoint_address.rxbin)

crexx_add_vm_dual_test(NAME rxasbreakpointaddresstests RXVM_ARGS tests_breakpoint_address PASS_REGULAR_EXPRESSION "bp-source: say 'target'")

# Instruction Sanity Tests - focused signal handler mode coverage
crexx_add_rxas_vm_binary(tests_signal_call_attrs "signal call argument attributes")
crexx_add_vm_dual_test(NAME rxassignalattrcodetests RXVM_ARGS tests_signal_call_attrs PASS_REGULAR_EXPRESSION "attr-code: 30")
crexx_add_vm_dual_test(NAME rxassignalattrnametests RXVM_ARGS tests_signal_call_attrs PASS_REGULAR_EXPRESSION "attr-name: OTHER")
crexx_add_vm_dual_test(NAME rxassignalattrpayloadtests RXVM_ARGS tests_signal_call_attrs PASS_REGULAR_EXPRESSION "attr-payload: attr payload")
crexx_add_vm_dual_test(NAME rxassignalattrsourcetests RXVM_ARGS tests_signal_call_attrs PASS_REGULAR_EXPRESSION "attr-source: signal other")

crexx_add_rxas_vm_binary(tests_signal_handler_modes "signal handler response modes")
crexx_add_vm_dual_test(NAME rxassignalmodeignoretests RXVM_ARGS tests_signal_handler_modes PASS_REGULAR_EXPRESSION "mode-ignore-ok")
crexx_add_vm_dual_test(NAME rxassignalmodebranchtests RXVM_ARGS tests_signal_handler_modes PASS_REGULAR_EXPRESSION "mode-branch-hit")
crexx_add_vm_dual_test(NAME rxassignalmodecallbranchtests RXVM_ARGS tests_signal_handler_modes PASS_REGULAR_EXPRESSION "mode-callbr-handler: callbr payload")
crexx_add_vm_dual_test(NAME rxassignalmodereturntests RXVM_ARGS tests_signal_handler_modes PASS_REGULAR_EXPRESSION "mode-ret-after")

crexx_add_rxas_vm_binary(tests_signal_silent_halt "signal silent halt response")
crexx_add_vm_dual_test(NAME rxassignalsilenthalttests RXVM_ARGS tests_signal_silent_halt PASS_REGULAR_EXPRESSION "silent-before")

crexx_add_rxas_vm_binary(tests_signal_frame_scope "signal frame handler table scope")
crexx_add_vm_dual_test(NAME rxassignalframescopechildtests RXVM_ARGS tests_signal_frame_scope PASS_REGULAR_EXPRESSION "frame-handler: from child")
crexx_add_vm_dual_test(NAME rxassignalframescopeparenttests RXVM_ARGS tests_signal_frame_scope PASS_REGULAR_EXPRESSION "frame-handler: after child ignore")

crexx_add_rxas_vm_binary(tests_signal_nested_pending "nested pending signal delivery")
crexx_add_vm_dual_test(NAME rxassignalnestedpendingtests RXVM_ARGS tests_signal_nested_pending PASS_REGULAR_EXPRESSION "nested:outer-before>outer-after>inner>main-after")

crexx_add_rxas_vm_binary(tests_signal_actions "action-aware signal handlers")
crexx_add_vm_dual_test(NAME rxassignalactionskiptests RXVM_ARGS tests_signal_actions PASS_REGULAR_EXPRESSION "action-skip-after")
crexx_add_vm_dual_test(NAME rxassignalactiondynamictests RXVM_ARGS tests_signal_actions PASS_REGULAR_EXPRESSION "action-handler: dynamic payload")

crexx_add_rxas_vm_binary(tests_signal_pushpop "signal handler push/pop scope")
crexx_add_vm_dual_test(NAME rxassignalpushpoprestoretests RXVM_ARGS tests_signal_pushpop PASS_REGULAR_EXPRESSION "pushpop-restored-ignore")
crexx_add_vm_dual_test(NAME rxassignalpushpopbranchtests RXVM_ARGS tests_signal_pushpop PASS_REGULAR_EXPRESSION "pushpop-branch-hit")
crexx_add_vm_dual_test(NAME rxassignalpushpophandlerrestoretests RXVM_ARGS tests_signal_pushpop PASS_REGULAR_EXPRESSION "pushpop-after-restored-ignore")

crexx_add_rxas_vm_binary(tests_signal_branch_value "signal branch with bound value")
crexx_add_vm_dual_test(NAME rxassignalbrvrawtests RXVM_ARGS tests_signal_branch_value PASS_REGULAR_EXPRESSION "brv-has-raw: 1")
crexx_add_vm_dual_test(NAME rxassignalbrvnametests RXVM_ARGS tests_signal_branch_value PASS_REGULAR_EXPRESSION "brv-name: OTHER")
crexx_add_vm_dual_test(NAME rxassignalbrvpayloadtests RXVM_ARGS tests_signal_branch_value PASS_REGULAR_EXPRESSION "brv-payload: value payload")

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

# Float Constant Pool Format and Runtime Tests
add_custom_command(
        OUTPUT tests_float_pool.rxbin
        COMMAND $<TARGET_FILE:rxas> -o tests_float_pool ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_float_pool
        DEPENDS tests/tests_float_pool.rxas rxas rxvm mc_decimal_dynamic
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXAS Float Constant Pool Tests ..."
)

add_custom_target(run_tests_float_pool ALL DEPENDS tests_float_pool.rxbin)

crexx_add_vm_dual_test(NAME rxasfloatpooltests RXVM_ARGS tests_float_pool)

add_executable(test_float_pool_format tests/test_float_pool_format.c)
target_include_directories(test_float_pool_format PRIVATE
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/binutils/include)
add_dependencies(test_float_pool_format run_tests_float_pool)
add_test(NAME float_pool_format_check COMMAND test_float_pool_format WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Packed RXBIN Format Tests
add_custom_command(
        OUTPUT tests_compact_format.rxbin
        COMMAND $<TARGET_FILE:rxas> -o tests_compact_format ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_compact_format
        DEPENDS tests/tests_compact_format.rxas rxas rxvm mc_decimal_dynamic
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXBIN packed-format tests ..."
)

add_custom_target(run_tests_compact_format ALL DEPENDS tests_compact_format.rxbin)

crexx_add_vm_dual_test(NAME rxascompactformattests RXVM_ARGS tests_compact_format)

add_executable(test_compact_format tests/test_compact_format.c)
target_include_directories(test_compact_format PRIVATE
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/binutils/include)
add_dependencies(test_compact_format run_tests_compact_format)
add_test(NAME compact_format_check COMMAND test_compact_format WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

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

# Socket Instruction Tests
add_custom_command(
		OUTPUT tests_socket.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_socket ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_socket
		DEPENDS tests/tests_socket.rxas rxas rxvm rxbvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Socket Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_socket ALL DEPENDS tests_socket.rxbin)

crexx_add_vm_dual_test(NAME rxassockettests RXVM_ARGS tests_socket PASS_REGULAR_EXPRESSION "PASS: Core Socket Instructions")

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

# Interface dispatch instruction tests
add_custom_command(
		OUTPUT tests_interfaces.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_interfaces ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_interfaces
		DEPENDS tests/tests_interfaces.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Interface Dispatch Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_interfaces ALL DEPENDS tests_interfaces.rxbin)

crexx_add_vm_dual_test(NAME rxasinterfacetests RXVM_ARGS tests_interfaces PASS_REGULAR_EXPRESSION "PASS: Interface Dispatch Suite")

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
        DEPENDS tests/rexx/test_reentrancy.rexx rxc rxas compiler_exit_bin
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
                ${CREXX_SOCKET_MANUAL_LINK_LIBS}
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
