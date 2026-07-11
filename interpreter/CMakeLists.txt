cmake_minimum_required(VERSION 3.24)

project(Interpreter C)

set(CMAKE_C_STANDARD 99) # enables inline keyword
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG") # Can do more I am sure but this is about as good as it get in practice

set(_rxvmintp_sanitizer_flags
        "${CMAKE_C_FLAGS} ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _rxvmintp_build_type)
    string(APPEND _rxvmintp_sanitizer_flags
            " ${CMAKE_C_FLAGS_${_rxvmintp_build_type}}"
            " ${CMAKE_EXE_LINKER_FLAGS_${_rxvmintp_build_type}}"
            " ${CMAKE_SHARED_LINKER_FLAGS_${_rxvmintp_build_type}}")
endif()
string(FIND "${_rxvmintp_sanitizer_flags}" "-fsanitize=address" _rxvmintp_asan_flag)
if(NOT _rxvmintp_asan_flag EQUAL -1
        AND (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
    # GCC 11 spends tens of minutes compiling this translation unit at -O2 with ASan.
    set_source_files_properties(rxvmintp.c PROPERTIES COMPILE_OPTIONS "-O1")
endif()

set(RXVM_CORE_OBJECT_SOURCES
        rxvmintp.h
        rxvmintp.c
        rxvmload.c
        interrupt.c
        ../avl_tree/rxastree.h
        rxsignal.h
        ../utf8/utf.h
        ../binutils/include/rxbin.h
        ../binutils/rxsignature.c
        ../binutils/include/rxsignature.h
        rxspawn.c
        rxcrexxcmd.c
        rxcrexxcmd.h
        exitfunc.c
        rxpafuncs.c
        rxvml/rxvm_run.c)

function(crexx_add_rxvm_core_objects target_name)
    add_library(${target_name} OBJECT ${RXVM_CORE_OBJECT_SOURCES})
    target_include_directories(${target_name} PRIVATE
            .
            ./rxvmplugin
            ../assembler
            ../binutils/include
            ${CMAKE_SOURCE_DIR}/rxpa
            ../platform
            ../avl_tree
            ../utf8
            ../inc
            rxvmplugin/rxvmplugins/mc_decimal/decnumber)
    # These objects are also archived into rxvml, which is linked into
    # libcrexxsaa.so on ELF platforms.
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()

set(RXVM_PLATFORM_OBJECT_SOURCES
        rxvmsock.c
        rxvmsock.h)

function(crexx_add_rxvm_platform_objects target_name)
    add_library(${target_name} OBJECT ${RXVM_PLATFORM_OBJECT_SOURCES})
    target_include_directories(${target_name} PRIVATE
            .
            ./rxvmplugin
            ../assembler
            ../binutils/include
            ${CMAKE_SOURCE_DIR}/rxpa
            ../platform
            ../avl_tree
            ../utf8
            ../inc
            rxvmplugin/rxvmplugins/mc_decimal/decnumber)
    # These objects are also archived into rxvml, which is linked into
    # libcrexxsaa.so on ELF platforms.
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()

crexx_add_rxvm_core_objects(rxvm_core_objects)
crexx_add_rxvm_core_objects(rxbvm_core_objects)
target_compile_definitions(rxbvm_core_objects PRIVATE NTHREADED=1)
crexx_add_rxvm_platform_objects(rxvm_platform_notls_objects)

add_library(rxvmref STATIC rxvmref.c rxvmref.h)
target_include_directories(rxvmref PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(rxvmplugin)

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
set(CREXX_NATIVE_RUNTIME_LINK_FLAGS_ARGS "${CREXX_NATIVE_RUNTIME_LINK_FLAGS}"
        CACHE INTERNAL "Native executable link flags as an argv-safe CMake list" FORCE)
message(STATUS "cREXX VM core socket TLS backend: ${CREXX_ENABLE_TLS_NORMALIZED}")

crexx_add_rxvm_platform_objects(rxvm_platform_objects)
if(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "OPENSSL")
    target_compile_definitions(rxvm_platform_objects PRIVATE CREXX_TLS_OPENSSL=1)
elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "NETWORK")
    target_compile_definitions(rxvm_platform_objects PRIVATE CREXX_TLS_NETWORK=1)
    target_compile_options(rxvm_platform_objects PRIVATE -fblocks)
elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "SCHANNEL")
    target_compile_definitions(rxvm_platform_objects PRIVATE CREXX_TLS_SCHANNEL=1)
endif()

##########################################
# Bare Interpreter
# Threaded Version
add_executable(rxvm rxvmmain.c $<TARGET_OBJECTS:rxvm_core_objects>)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvm rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxvm PRIVATE . ./rxvmplugin
        ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)

# Bytecode Version
add_executable(rxbvm rxvmmain.c $<TARGET_OBJECTS:rxbvm_core_objects>)
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
add_executable(rxvme rxvmmain.c ${CMAKE_CURRENT_BINARY_DIR}/library.c $<TARGET_OBJECTS:rxvm_core_objects>)
target_compile_definitions(rxvme PRIVATE LINK_CREXX_LIB=1)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvme rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxvme PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
add_dependencies(rxvme packed_library)

# Bytecode Version
add_executable(rxbvme rxvmmain.c ${CMAKE_CURRENT_BINARY_DIR}/library.c $<TARGET_OBJECTS:rxbvm_core_objects>)
target_compile_definitions(rxbvme PRIVATE NTHREADED=1 LINK_CREXX_LIB=1)
target_link_libraries(rxbvme rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxbvme PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
add_dependencies(rxbvme packed_library)

##########################################
# C Library Interpreter -  to be linked to make a standalone CREXX exe

# Threaded Version
add_library(rxvml STATIC rxvmlib.c rxvml.c $<TARGET_OBJECTS:rxvm_core_objects>)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvml rxvmplugin mc_decimal_manual avl_tree rxpa)
target_include_directories(rxvml PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)

# Bytecode Version
add_library(rxbvml STATIC rxvmlib.c rxvml.c $<TARGET_OBJECTS:rxbvm_core_objects>)
target_link_libraries(rxbvml rxvmplugin mc_decimal_manual avl_tree rxpa)
target_include_directories(rxbvml PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
target_compile_definitions(rxbvml PRIVATE NTHREADED=1)

function(crexx_enable_vm_platform target_name)
    target_sources(${target_name} PRIVATE $<TARGET_OBJECTS:rxvm_platform_objects>)
    if(CREXX_SOCKET_TARGET_LIBS)
        target_link_libraries(${target_name} ${CREXX_SOCKET_TARGET_LIBS})
    endif()
endfunction()

foreach(_rxvm_core_target rxvm rxbvm rxvme rxbvme rxvml rxbvml)
    target_link_libraries(${_rxvm_core_target} rxvmref)
    crexx_enable_vm_platform(${_rxvm_core_target})
endforeach()

# libcrexxsaa is intentionally thin and shared. On ELF platforms, every static
# archive pulled into it must be compiled as PIC.
foreach(_crexxsaa_static_dep rxvml rxvmref rxvmplugin mc_decimal_manual avl_tree rxpa platform decnumber)
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
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

# Value test executable
add_executable(ts_regvalue_tester tests/ts_regvalue_tester.c rxvalue.h rxvmvars.h)
target_include_directories(ts_regvalue_tester PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} . ../utf8 ../platform ../binutils/include)
target_link_libraries(ts_regvalue_tester rxvmref m)

add_executable(test_jtable_policy tests/test_jtable_policy.c)
target_include_directories(test_jtable_policy PRIVATE ../binutils/include)

add_executable(mutate_jtable_rxbin tests/mutate_jtable_rxbin.c)
target_include_directories(mutate_jtable_rxbin PRIVATE
        ../binutils/include
        ../platform)

add_executable(test_rxpa_utf_validation tests/test_rxpa_utf_validation.c rxpafuncs.c rxvalue.h rxvmvars.h)
target_include_directories(test_rxpa_utf_validation PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        .
        ../assembler
        ../binutils/include
        ${CMAKE_SOURCE_DIR}/rxpa
        ../platform
        ../avl_tree
        ../utf8
        ../inc)
target_link_libraries(test_rxpa_utf_validation rxvmref m)

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

add_test(
        NAME rxpa_utf_validation
        COMMAND test_rxpa_utf_validation
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxpa_utf_validation PROPERTIES LABELS "interpreter;rxpa;utf")

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
    add_dependencies(run_${SOURCE_NAME} rxvm rxbvm)
endfunction()

function(crexx_add_generated_jtable_matrix ALGORITHM DISPLAY_NAME)
    set(SOURCE_NAME tests_jump_table_${ALGORITHM}_sources)
    set(GENERATED_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_NAME}.rxas)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_jump_table_linear_sources.rxas MATRIX_SOURCE)
    string(REPLACE " linear" " ${ALGORITHM}" MATRIX_SOURCE "${MATRIX_SOURCE}")
    string(REPLACE "Linear" "${DISPLAY_NAME}" MATRIX_SOURCE "${MATRIX_SOURCE}")
    file(GENERATE OUTPUT ${GENERATED_SOURCE} CONTENT "${MATRIX_SOURCE}")

    add_custom_command(
            OUTPUT ${SOURCE_NAME}.rxbin
            COMMAND $<TARGET_FILE:rxas> -o ${SOURCE_NAME} ${GENERATED_SOURCE}
            DEPENDS tests/tests_jump_table_linear_sources.rxas rxas rxvm mc_decimal_dynamic
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "cREXX RXAS Instruction Tests (${ALGORITHM} jump table source matrix) ..."
    )
    add_custom_target(run_${SOURCE_NAME} ALL DEPENDS ${SOURCE_NAME}.rxbin)
    add_dependencies(run_${SOURCE_NAME} rxvm rxbvm)
    crexx_add_vm_dual_test(
            NAME rxasjumptable${ALGORITHM}sources
            RXVM_ARGS ${SOURCE_NAME}
            PASS_REGULAR_EXPRESSION "PASS: Jump Table ${DISPLAY_NAME} Source Matrix")
endfunction()

function(crexx_add_jtable_corruption_fixture NAME ALGORITHM MUTATION)
    set(BASE_NAME tests_jump_table_corruption_${ALGORITHM})
    set(BASE_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.rxas")
    set(BASE_RXBIN "${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.rxbin")
    set(CORRUPT_RXBIN "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.rxbin")

    set(ALGORITHM "${ALGORITHM}")
    configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_jump_table_corruption.rxas.in
            ${BASE_SOURCE}
            @ONLY)

    add_custom_command(
            OUTPUT ${BASE_RXBIN}
            COMMAND $<TARGET_FILE:rxas> -o ${BASE_NAME} ${BASE_SOURCE}
            DEPENDS ${BASE_SOURCE} rxas
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "cREXX RXAS jump-table corruption base (${ALGORITHM}) ...")

    add_custom_command(
            OUTPUT ${CORRUPT_RXBIN}
            COMMAND $<TARGET_FILE:mutate_jtable_rxbin>
                    ${BASE_RXBIN} ${CORRUPT_RXBIN} ${MUTATION}
            DEPENDS ${BASE_RXBIN} mutate_jtable_rxbin
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Mutating jump table ${NAME} (${MUTATION}) ...")

    add_custom_target(run_${NAME} ALL DEPENDS ${CORRUPT_RXBIN})
    foreach(RUNNER IN ITEMS rxvm rxbvm)
        add_test(
                NAME ${NAME}_${RUNNER}
                COMMAND ${CMAKE_COMMAND}
                        -DRUNNER=$<TARGET_FILE:${RUNNER}>
                        -DRXBIN=${CORRUPT_RXBIN}
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_jtable_corruption.cmake
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        set_tests_properties(${NAME}_${RUNNER} PROPERTIES
                LABELS "interpreter;rxas;jtable;corruption")
    endforeach()
endfunction()

function(crexx_add_rxas_vm_failure_regex NAME SOURCE_NAME EXPECTED_REGEX)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")
    set(DOLLAR "$")

    string(APPEND SCRIPT_CONTENT
            "execute_process(\n"
            "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"${SOURCE_NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/tests/${SOURCE_NAME}.rxas\"\n"
            "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
            "  OUTPUT_VARIABLE rxas_out\n"
            "  ERROR_VARIABLE rxas_err\n"
            "  RESULT_VARIABLE rxas_res\n"
            ")\n"
            "if(NOT rxas_res EQUAL 0)\n"
            "  message(FATAL_ERROR \"rxas failed for ${SOURCE_NAME}.\\nSTDOUT:\\n${DOLLAR}{rxas_out}\\nSTDERR:\\n${DOLLAR}{rxas_err}\")\n"
            "endif()\n"
            "execute_process(\n"
            "  COMMAND \"$<TARGET_FILE:rxvm>\" \"${SOURCE_NAME}\"\n"
            "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
            "  OUTPUT_VARIABLE rxvm_out\n"
            "  ERROR_VARIABLE rxvm_err\n"
            "  RESULT_VARIABLE rxvm_res\n"
            ")\n"
            "set(full_out \"${DOLLAR}{rxvm_out}${DOLLAR}{rxvm_err}\")\n"
            "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${DOLLAR}{full_out}\")\n"
            "if(NOT full_out MATCHES \"${EXPECTED_REGEX}\")\n"
            "  message(FATAL_ERROR \"Expected ${SOURCE_NAME} to report ${EXPECTED_REGEX}.\\nSTDOUT/STDERR:\\n${DOLLAR}{full_out}\\nExit: ${DOLLAR}{rxvm_res}\")\n"
            "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(
            NAME ${NAME}
            COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "interpreter;rxas;utf")
endfunction()

function(crexx_add_rxas_assembly_failure_regex NAME SOURCE_NAME EXPECTED_REGEX)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")
    set(DOLLAR "$")

    string(APPEND SCRIPT_CONTENT
            "execute_process(\n"
            "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"${SOURCE_NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/tests/${SOURCE_NAME}.rxas\"\n"
            "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
            "  OUTPUT_VARIABLE rxas_out\n"
            "  ERROR_VARIABLE rxas_err\n"
            "  RESULT_VARIABLE rxas_res\n"
            ")\n"
            "set(full_out \"${DOLLAR}{rxas_out}${DOLLAR}{rxas_err}\")\n"
            "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${DOLLAR}{full_out}\")\n"
            "if(rxas_res EQUAL 0)\n"
            "  message(FATAL_ERROR \"Expected rxas to reject ${SOURCE_NAME}.\\nSTDOUT/STDERR:\\n${DOLLAR}{full_out}\")\n"
            "endif()\n"
            "if(NOT full_out MATCHES \"${EXPECTED_REGEX}\")\n"
            "  message(FATAL_ERROR \"Expected ${SOURCE_NAME} to report ${EXPECTED_REGEX}.\\nSTDOUT/STDERR:\\n${DOLLAR}{full_out}\\nExit: ${DOLLAR}{rxas_res}\")\n"
            "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(
            NAME ${NAME}
            COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "interpreter;rxas;jtable")
endfunction()

# Value Tests
add_test(
		NAME ts_regvalue_tester
		COMMAND ts_regvalue_tester
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_test(
        NAME test_jtable_policy
        COMMAND test_jtable_policy
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(test_jtable_policy PROPERTIES LABELS "interpreter;rxas;jtable")

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
crexx_add_vm_dual_test(NAME rxasbreakpointtraceeventtests RXVM_ARGS tests_breakpoint_address PASS_REGULAR_EXPRESSION "bp-symbol: target")

# Instruction Sanity Tests - register flag partitioning
crexx_add_rxas_vm_binary(tests_register_flags "register flag partitioning")
crexx_add_vm_dual_test(NAME rxasregisterflagtests RXVM_ARGS tests_register_flags PASS_REGULAR_EXPRESSION "PASS: Register Flag Partition")

# Instruction Sanity Tests - UTF status cache flags
crexx_add_rxas_vm_binary(tests_utf_flags "UTF flag cache")
crexx_add_vm_dual_test(NAME rxasutfflagtests RXVM_ARGS tests_utf_flags PASS_REGULAR_EXPRESSION "PASS: UTF Flag Cache")
crexx_add_rxas_vm_failure_regex(rxasutfboundaryfreadlinetests tests_utf_freadline_boundary "UNICODE_ERROR")
crexx_add_rxas_vm_failure_regex(rxasutfboundaryfreadcdpttests tests_utf_freadcdpt_boundary "UNICODE_ERROR")

# Instruction Sanity Tests - bulk attribute insert/delete
crexx_add_rxas_vm_binary(tests_attributes "attribute bulk insert/delete")
crexx_add_vm_dual_test(NAME rxasattributebulktests RXVM_ARGS tests_attributes PASS_REGULAR_EXPRESSION "PASS: Attribute Bulk Ops")

# Instruction Sanity Tests - reference values
crexx_add_rxas_vm_binary(tests_references "reference value instructions")
crexx_add_vm_dual_test(NAME rxasreferencetests RXVM_ARGS tests_references PASS_REGULAR_EXPRESSION "PASS: Reference RXAS Ops")
crexx_add_vm_dual_test(NAME rxasreferencecatchtests RXVM_ARGS tests_references PASS_REGULAR_EXPRESSION "caught-reference-invalid")

# Instruction Sanity Tests - object initialization lifecycle
crexx_add_rxas_vm_binary(tests_object_initialization "object initialization lifecycle instructions")
crexx_add_vm_dual_test(NAME rxasobjectinitializationtests RXVM_ARGS tests_object_initialization PASS_REGULAR_EXPRESSION "PASS: Object Initialization RXAS Ops")
crexx_add_vm_dual_test(NAME rxasobjectinitializationcatchtests RXVM_ARGS tests_object_initialization PASS_REGULAR_EXPRESSION "caught-object-not-initialized")

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

crexx_add_rxas_vm_binary(tests_int_contract "signed 64-bit integer contract")
crexx_add_vm_dual_test(
        NAME rxasintcontracttests
        RXVM_ARGS tests_int_contract
        PASS_REGULAR_EXPRESSION "PASS: Signed 64-bit Integer Contract")
crexx_add_rxas_assembly_failure_regex(
        rxasintoverflowpositive tests_int_overflow_positive
        "integer literal is outside the signed 64-bit range")
crexx_add_rxas_assembly_failure_regex(
        rxasintoverflownegative tests_int_overflow_negative
        "integer literal is outside the signed 64-bit range")
set_tests_properties(rxasintoverflowpositive rxasintoverflownegative PROPERTIES
        LABELS "interpreter;rxas;integer;negative")

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

# Binary Buffer, I/O, and legacy cursor instruction tests
crexx_add_rxas_vm_binary(tests_binary "binary buffer and I/O")
crexx_add_vm_dual_test(NAME rxasbinarylegacytests RXVM_ARGS tests_binary PASS_REGULAR_EXPRESSION "PASS: Binary Buffer IO and Legacy Cursor Ops")
crexx_add_rxas_vm_binary(tests_binary_memory "typed binary memory operations")
crexx_add_vm_dual_test(NAME rxasbinarymemorytests RXVM_ARGS tests_binary_memory PASS_REGULAR_EXPRESSION "PASS: Binary Memory Typed Ops")
crexx_add_rxas_vm_binary(tests_jump_table "linear jump table dispatch")
crexx_add_vm_dual_test(NAME rxasjumptabletests RXVM_ARGS tests_jump_table PASS_REGULAR_EXPRESSION "PASS: Jump Table Linear Ops")
crexx_add_rxas_vm_binary(tests_jump_table_linear_sources "linear jump table source matrix")
crexx_add_vm_dual_test(NAME rxasjumptablelinearsources RXVM_ARGS tests_jump_table_linear_sources PASS_REGULAR_EXPRESSION "PASS: Jump Table Linear Source Matrix")
crexx_add_generated_jtable_matrix(openhash OpenHash)
crexx_add_generated_jtable_matrix(acph ACPH)
crexx_add_generated_jtable_matrix(auto Auto)
crexx_add_rxas_vm_binary(tests_jump_table_hash_edges "open-hash and ACPH jump table edge cases")
crexx_add_vm_dual_test(NAME rxasjumptablehashedges RXVM_ARGS tests_jump_table_hash_edges PASS_REGULAR_EXPRESSION "PASS: Jump Table Hash Edge Cases")
crexx_add_rxas_vm_binary(tests_jump_table_auto_edges "auto jump table policy paths")
crexx_add_vm_dual_test(NAME rxasjumptableautoedges RXVM_ARGS tests_jump_table_auto_edges PASS_REGULAR_EXPRESSION "PASS: Jump Table Auto Policy Paths")
crexx_add_rxas_vm_binary(tests_jump_table_rexx_strings "Rexx padded and numeric string jump tables")
crexx_add_vm_dual_test(NAME rxasjumptablerexxstrings RXVM_ARGS tests_jump_table_rexx_strings PASS_REGULAR_EXPRESSION "PASS: Jump Table Rexx String Ops")
crexx_add_rxas_vm_binary(tests_jump_table_boundaries "jump table syntax and value boundaries")
crexx_add_vm_dual_test(NAME rxasjumptableboundaries RXVM_ARGS tests_jump_table_boundaries PASS_REGULAR_EXPRESSION "PASS: Jump Table Boundaries")
crexx_add_jtable_corruption_fixture(rxasjtablecorruptalgorithm linear algorithm)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptheader linear header)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptlinearkey linear linear-key-offset)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptlineartarget linear linear-target)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptopenslot openhash open-slot-offset)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptacphslot acph acph-slot-kind)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptacphtarget acph acph-target)
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadnolabel tests_jump_table_bad_no_label "\\.jcase must decorate a same-line label")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadunknown tests_jump_table_bad_unknown "unknown jump table")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadjumpbsvariable tests_jump_table_bad_jumpbs_variable "jumpbs requires fixed-length")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadduplicate tests_jump_table_bad_duplicate "duplicate jump table case key")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadtypemismatch tests_jump_table_bad_type_mismatch "jump table key type mismatch")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadempty tests_jump_table_bad_empty "jump table has no cases")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadduplicatebinary tests_jump_table_bad_duplicate_binary "duplicate jump table case key")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadduplicatetable tests_jump_table_bad_duplicate_table "duplicate jump table declaration")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadalgorithmname tests_jump_table_bad_algorithm_name "jump table algorithm must be auto, linear, openhash, or acph")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadoperand tests_jump_table_bad_operand "jump table instruction requires a \\.jtable name operand")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadnumerickey tests_jump_table_bad_numeric_key "jumpn case key must be a non-NaN numeric string")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadnumericduplicate tests_jump_table_bad_numeric_duplicate "duplicate jump table case key after canonicalization")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadpaddedduplicate tests_jump_table_bad_padded_duplicate "duplicate jump table case key after canonicalization")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadmatchmode tests_jump_table_bad_match_mode "cannot mix exact, padded, and numeric")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadoutsideprocedure tests_jump_table_bad_outside_procedure "Jump tables can only be declared inside a procedure")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadcaseonly tests_jump_table_bad_case_only "unknown jump table")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadprocedurescope tests_jump_table_bad_procedure_scope "unknown jump table")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadjumpbsempty tests_jump_table_bad_jumpbs_empty "jumpbs requires fixed-length non-empty binary keys")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadutf8 tests_jump_table_bad_utf8 "jump table string key is not valid UTF-8")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadnumericzeroduplicate tests_jump_table_bad_numeric_zero_duplicate "duplicate jump table case key after canonicalization")

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
crexx_add_rxas_vm_binary(tests_selector_descriptors "descriptor selector contracts")
crexx_add_vm_dual_test(NAME rxasselectordescriptortests RXVM_ARGS tests_selector_descriptors PASS_REGULAR_EXPRESSION "PASS: Descriptor Selector Contracts")
crexx_add_rxas_vm_failure_regex(rxasselectordescriptorbadmethod tests_selector_descriptor_bad_method "rxsig1\\|describe\\|\\.string\\|")
crexx_add_rxas_vm_failure_regex(rxasselectordescriptorbadfactory tests_selector_descriptor_bad_factory "selector_bad_factory\\.shape")

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

# Focused deterministic coverage for remaining RXAS instruction families
crexx_add_rxas_vm_binary(tests_instruction_gaps "remaining deterministic instruction gaps")
crexx_add_vm_dual_test(NAME rxasinstructiongaptests RXVM_ARGS tests_instruction_gaps PASS_REGULAR_EXPRESSION "PASS: RXAS Instruction Gap Coverage")

# Re-entrancy Test
add_custom_command(
        OUTPUT test_reentrancy.rxbin
        COMMAND $<TARGET_FILE:rxc> -o test_reentrancy ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_reentrancy.crexx
        COMMAND $<TARGET_FILE:rxas> test_reentrancy
        DEPENDS tests/rexx/test_reentrancy.crexx rxc rxas compiler_exit_bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_reentrancy.crexx ..."
)
add_custom_target(test_reentrancy_bin ALL DEPENDS test_reentrancy.rxbin)

add_executable(test_reentrancy tests/test_reentrancy.c)
target_include_directories(test_reentrancy PRIVATE ${CMAKE_SOURCE_DIR}/inc)
target_link_libraries(test_reentrancy rxvml)
add_dependencies(test_reentrancy test_reentrancy_bin)

add_test(NAME reentrancy_check COMMAND test_reentrancy WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Archive-link regression test for manual db_decimal packaging.
set(_crexx_archive_link_c_flags_text "${CMAKE_C_FLAGS}")
set(_crexx_archive_link_exe_flags_text "${CMAKE_EXE_LINKER_FLAGS}")
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _crexx_archive_link_build_type)
    string(APPEND _crexx_archive_link_c_flags_text " ${CMAKE_C_FLAGS_${_crexx_archive_link_build_type}}")
    string(APPEND _crexx_archive_link_exe_flags_text " ${CMAKE_EXE_LINKER_FLAGS_${_crexx_archive_link_build_type}}")
endif()
separate_arguments(_crexx_archive_link_c_flags NATIVE_COMMAND "${_crexx_archive_link_c_flags_text}")
separate_arguments(_crexx_archive_link_exe_flags NATIVE_COMMAND "${_crexx_archive_link_exe_flags_text}")
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link
        COMMAND ${CMAKE_C_COMPILER}
                ${_crexx_archive_link_c_flags}
                -O0
                -I${CMAKE_CURRENT_SOURCE_DIR}
                -I${CMAKE_CURRENT_SOURCE_DIR}/rxvmplugin
                -I${CMAKE_SOURCE_DIR}/inc
                -o ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_db_decimal_archive_link.c
                $<TARGET_FILE:rxvml>
                $<TARGET_FILE:rxpashim>
                $<TARGET_FILE:rxvmplugin>
                $<TARGET_FILE:rxvmref>
                $<TARGET_FILE:platform>
                $<TARGET_FILE:decnumber>
                $<TARGET_FILE:avl_tree>
                $<TARGET_FILE:rxpa>
                ${_crexx_archive_link_exe_flags}
                -lm
                $<TARGET_FILE:db_decimal_manual>
                ${CREXX_SOCKET_MANUAL_LINK_LIBS}
        DEPENDS
                tests/test_db_decimal_archive_link.c
                rxvml
                rxpashim
                rxvmplugin
                rxvmref
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

crexx_label_smoke_tests(
        crexxsaa_cache_tool_location
        rxpa_utf_validation
        ts_regvalue_tester
        rxassignalstests
        rxasregisterflagtests
        rxasutfflagtests-rxbvm
        rxasreferencetests
        rxasbasictests
        rxasstringtests-rxbvm
        rxasdecimaltests-mc
        rxasinterfacetests-rxbvm
        compact_format_check
        reentrancy_check
        db_decimal_archive_link
)
