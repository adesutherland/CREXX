cmake_minimum_required(VERSION 3.24)
project(rxpatest C)
add_subdirectory(multplugins)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

# Including RXPlugin Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_custom_target(cri07_rxpa_sdk_consumer_prereqs
        DEPENDS rxc rxas rxvm rxbvm library)

add_test(
        NAME rxpa_external_sdk_consumer
        COMMAND ${CMAKE_COMMAND}
                -DCREXX_BUILD_DIR=${CMAKE_BINARY_DIR}
                -DCREXX_VERSION=${CREXX_VERSION}
                -DCREXX_VERSION_CORE=${CREXX_VERSION_CORE}
                -DCREXX_VERSION_DISPLAY=${CREXX_VERSION_DISPLAY}
                -DCREXX_SOURCE_DIR=${CMAKE_SOURCE_DIR}
                -DCMAKE_GENERATOR_NAME=${CMAKE_GENERATOR}
                -DEXE_SUFFIX=${CMAKE_EXECUTABLE_SUFFIX}
                -DPACKAGE_RELATIVE_DIR=${CREXX_CMAKE_PACKAGE_INSTALL_DIR}
                -DCONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/external_sdk
                -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/rxpa_external_sdk_consumer_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_external_sdk_consumer.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxpa_external_sdk_consumer PROPERTIES
        LABELS "install;package;rxpa;external-consumer"
        RUN_SERIAL TRUE
        TIMEOUT 240)

function(localize_test_plugin_output target)
    # Keep test-only plugins out of the shared release bin directory.
    set_target_properties(${target} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endfunction()

function(enable_rxpa_test_vm_platform target)
    # These static-plugin harness VMs need the core socket symbols, but they
    # intentionally keep the historical no-TLS test configuration.
    target_sources(${target} PRIVATE $<TARGET_OBJECTS:rxvm_platform_notls_objects>)
    if(WIN32)
        target_link_libraries(${target} ws2_32)
    endif()
endfunction()

# Create dynamic plugin module
add_dynamic_plugin_target(_rxpa_dynlink rxpa_dynlink.c)
localize_test_plugin_output(_rxpa_dynlink)
add_dependencies(_rxpa_dynlink rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

add_dynamic_plugin_target(_rxpa_invalid_manifest rxpa_invalid_manifest.c)
localize_test_plugin_output(_rxpa_invalid_manifest)
add_dependencies(_rxpa_invalid_manifest rxpa rxvm)

add_dynamic_plugin_target(_rxpa_bench_reentrant rxpa_call_kernel.c)
target_compile_definitions(_rxpa_bench_reentrant PRIVATE
        RXPA_BENCH_PROCESS_REENTRANT=1)
localize_test_plugin_output(_rxpa_bench_reentrant)

add_dynamic_plugin_target(_rxpa_bench_legacy rxpa_call_kernel.c)
localize_test_plugin_output(_rxpa_bench_legacy)

add_test(NAME rxpa_process_reentrant_manifest
        COMMAND $<TARGET_FILE:test_rxpa_concurrency>
                manifest
                $<TARGET_FILE_DIR:_rxpa_dynlink>
                $<TARGET_FILE_NAME:_rxpa_dynlink>)
set_tests_properties(rxpa_process_reentrant_manifest PROPERTIES
        LABELS "interpreter;worker;plugins;rxpa")
add_test(NAME rxpa_invalid_manifest_is_legacy
        COMMAND $<TARGET_FILE:test_rxpa_concurrency>
                invalid-manifest
                $<TARGET_FILE_DIR:_rxpa_invalid_manifest>
                $<TARGET_FILE_NAME:_rxpa_invalid_manifest>)
set_tests_properties(rxpa_invalid_manifest_is_legacy PROPERTIES
        LABELS "interpreter;worker;plugins;rxpa")
add_test(NAME rxpa_dynamic_context_ownership
        COMMAND $<TARGET_FILE:test_rxpa_concurrency>
                dynamic
                $<TARGET_FILE_DIR:_rxpa_dynlink>
                $<TARGET_FILE_NAME:_rxpa_dynlink>)
set_tests_properties(rxpa_dynamic_context_ownership PROPERTIES
        LABELS "interpreter;worker;plugins;rxpa")

# Native declaration plugin used to prove RXPA class/interface metadata is
# visible to the compiler without a Rexx-side contract shim.
add_dynamic_plugin_target(_rxpa_classdecl rxpa_classdecl.c)
localize_test_plugin_output(_rxpa_classdecl)
add_dependencies(_rxpa_classdecl rxpa library rxvm rxc)

# Compiler-facing signature validation fixture. The deliberately malformed
# declarations are never invoked; they exercise import diagnostics alongside
# three valid declarations that compile and run through both VM engines.
add_dynamic_plugin_target(_rxpa_bad_signatures rxpa_bad_signatures.c)
localize_test_plugin_output(_rxpa_bad_signatures)
add_dependencies(_rxpa_bad_signatures rxpa library rxvm rxbvm rxc rxas)

# Create a version of rxc that statically links the _rxpa_static library
# Create static link module for rxc
# The static declaration library
add_decl_plugin_target(_rxpalink rxpa_staticlink.c)
# The static link test - a version of rxc that statically links the _rxpa_decl library
add_executable(rxc_staticlink "${CMAKE_SOURCE_DIR}/compiler/rxc_main.c")
target_include_directories(rxc_staticlink PRIVATE ${CMAKE_SOURCE_DIR}/platform)
# Ensure the linker includes the plugin
configure_linker_for_decl_lib(rxc_staticlink _rxpalink)
# Add the dependencies and link libraries
add_dependencies(rxc_staticlink rxclib _rxpalink_decl rxpa library rxvm)
target_link_libraries(rxc_staticlink rxclib _rxpalink_decl)

add_decl_plugin_target(_rxpa_classdecl_static rxpa_classdecl.c)
add_executable(rxc_classdecl_static "${CMAKE_SOURCE_DIR}/compiler/rxc_main.c")
target_include_directories(rxc_classdecl_static PRIVATE ${CMAKE_SOURCE_DIR}/platform)
configure_linker_for_decl_lib(rxc_classdecl_static _rxpa_classdecl_static)
add_dependencies(rxc_classdecl_static rxclib _rxpa_classdecl_static_decl rxpa library rxvm)
target_link_libraries(rxc_classdecl_static rxclib _rxpa_classdecl_static_decl)

# Create static link module for rxvm
# The static definition library
add_static_plugin_target(_rxpalink rxpa_staticlink.c)

# Create a version of rxvm that statically links the _rxpa_static library
# Basic Build
add_executable(testvm
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmmain.c
        $<TARGET_OBJECTS:${CREXX_DEFAULT_VM_CORE_OBJECT_TARGET}>)
if(CREXX_DEFAULT_VM_USES_SWITCH)
    target_compile_definitions(testvm PRIVATE NTHREADED=1)
endif()
# Set the performance properties
# set_property(TARGET testvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET testvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET testvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
# Ensure the linker includes the plugin
configure_linker_for_static_lib(testvm _rxpalink)
enable_rxpa_test_vm_platform(testvm)
# Add the dependencies and link libraries
add_dependencies(testvm avl_tree rxpa platform _rxpalink_static)
target_link_libraries(testvm rxvmplugin rxvmref rxbin mc_decimal_manual avl_tree rxpa rxpashim _rxpalink_static)
target_include_directories(testvm PRIVATE
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/binutils/include
        ${CMAKE_SOURCE_DIR}/assembler
        ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/utf8
        ${CMAKE_SOURCE_DIR}/inc)

add_custom_target(rxpa_dynlink_run ALL)
crexx_add_rexx_opt_matrix(
        NAME rxpa_dynlink_run
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_dynlink.crexx
        TARGET_GROUP rxpa_dynlink_run
        IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
        DEPENDS library _rxpa_dynlink
        RUNTIME_ARGS rx_rxpa_dynlink ${CMAKE_BINARY_DIR}/bin/library
)

add_custom_target(rxpa_staticlink_run ALL)
crexx_add_rexx_opt_matrix(
        NAME rxpa_staticlink_run
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_staticlink.crexx
        TARGET_GROUP rxpa_staticlink_run
        COMPILER_TARGET rxc_staticlink
        IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNNER testvm
        RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
)

# Enable testing functionality
enable_testing()

add_test(
        NAME rxpa_signature_diagnostics
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXVM=${CREXX_RXVM_EXECUTABLE}
                -DRXBVM=$<TARGET_FILE:rxbvm>
                -DLIBRARY=${CMAKE_BINARY_DIR}/bin
                -DPLUGIN_DIR=$<TARGET_FILE_DIR:_rxpa_bad_signatures>
                -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
                -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/rxpa_signature_diagnostics_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_signature_diagnostics.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxpa_signature_diagnostics PROPERTIES
        LABELS "compiler;diagnostics;rxpa;runtime;optimizer"
        TIMEOUT 120)

foreach(_mode IN ITEMS noopt opt)
    set(_mode_flag)
    if(_mode STREQUAL "noopt")
        set(_mode_flag -n)
    endif()

    add_test(
            NAME rxpa_dynlink_compile_${_mode}
            COMMAND $<TARGET_FILE:rxc> -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}" ${_mode_flag} -o rxpa_dynlink_compile_${_mode} ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_dynlink
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_test(
            NAME rxpa_staticlink_compile_${_mode}
            COMMAND rxc_staticlink -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}" ${_mode_flag} -o rxpa_staticlink_compile_${_mode} ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_staticlink
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_test(
            NAME rxpa_classdecl_dynamic_compile_${_mode}
            COMMAND $<TARGET_FILE:rxc> -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}" ${_mode_flag} -o rxpa_classdecl_dynamic_compile_${_mode} ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_classdecl.crexx
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_test(
            NAME rxpa_classdecl_static_compile_${_mode}
            COMMAND $<TARGET_FILE:rxc_classdecl_static> -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}" ${_mode_flag} -o rxpa_classdecl_static_compile_${_mode} ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_classdecl.crexx
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()

crexx_label_smoke_tests(
        rxpa_dynlink_run_noopt
        rxpa_staticlink_run_noopt
        rxpa_classdecl_static_compile_opt
)

# Function Tests
# Create dynamic plugin module
add_dynamic_plugin_target(_testfuncs_dynamic rxpa_functests.c)
localize_test_plugin_output(_testfuncs_dynamic)
add_dependencies(_testfuncs_dynamic rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

# Create static plugin module for rxvm
add_static_plugin_target(_testfuncs rxpa_functests.c)

# Create a version of rxvm that statically links the _testfuncs_static library
# Basic Build
add_executable(funcvm
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmmain.c
        $<TARGET_OBJECTS:${CREXX_DEFAULT_VM_CORE_OBJECT_TARGET}>)
if(CREXX_DEFAULT_VM_USES_SWITCH)
    target_compile_definitions(funcvm PRIVATE NTHREADED=1)
endif()
# Set the performance properties
# set_property(TARGET funcvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET funcvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET funcvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
# Ensure the linker includes the plugin
configure_linker_for_static_lib(funcvm _testfuncs)
enable_rxpa_test_vm_platform(funcvm)
# Add the dependencies and link libraries
add_dependencies(funcvm avl_tree rxpa platform _testfuncs_static)
target_link_libraries(funcvm rxvmplugin rxvmref rxbin mc_decimal_manual avl_tree rxpa rxpashim _testfuncs_static)
target_include_directories(funcvm PRIVATE
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/binutils/include
        ${CMAKE_SOURCE_DIR}/assembler
        ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/utf8
        ${CMAKE_SOURCE_DIR}/inc)

# Additional functional tests (of callback functions)
set(FUNC_TESTS
        signal_funcs
        string_funcs
        int_funcs
        float_funcs
        array_push_funcs
        native_payload_funcs
        native_unwind_funcs
)
set(OUTPUTS
        Signal
        OK
        OK
        OK
        OK
        OK
        "PASS: Native Signal Call Mapping"
)

list(LENGTH FUNC_TESTS num_tests)
math(EXPR num_tests "${num_tests} - 1") # Correct upper bound for RANGE
foreach(idx RANGE 0 ${num_tests})  # Iterate from 0 to len-1
    list(GET FUNC_TESTS ${idx} _rexxtest)
    list(GET OUTPUTS ${idx} _expected_output)
    set(_rexxtest_source ${_rexxtest}.crexx)
    set(_runtime_expectations)
    if(_rexxtest STREQUAL "signal_funcs")
        list(APPEND _runtime_expectations
                EXPECTED_EXIT_CODE 3
                EXPECTED_FAILURE_DESCRIPTION "SIGNAL ERROR at bytecode address 9"
                REQUIRED_OUTPUT_REGEX
                    "PANIC: \\(SIGNAL ERROR\\)"
                    "address 9 \\(0x9\\)"
                FORBIDDEN_OUTPUT_REGEX
                    "Error - should not reach here"
        )
    endif()
    add_custom_target(rxpa_${_rexxtest}_dynamic ALL)
    crexx_add_rexx_opt_matrix(
            NAME rxpa_${_rexxtest}_dynamic
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_rexxtest_source}
            TARGET_GROUP rxpa_${_rexxtest}_dynamic
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
            DEPENDS library funcvm _testfuncs_dynamic
            RUNTIME_ARGS rx_testfuncs_dynamic ${CMAKE_BINARY_DIR}/bin/library
            ${_runtime_expectations}
    )
    if(NOT _rexxtest STREQUAL "signal_funcs")
        set_tests_properties(
                rxpa_${_rexxtest}_dynamic_noopt
                rxpa_${_rexxtest}_dynamic_opt
                PROPERTIES PASS_REGULAR_EXPRESSION ".*${_expected_output}.*"
        )
    endif()

    add_custom_target(rxpa_${_rexxtest}_static ALL)
    crexx_add_rexx_opt_matrix(
            NAME rxpa_${_rexxtest}_static
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_rexxtest_source}
            TARGET_GROUP rxpa_${_rexxtest}_static
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
            DEPENDS library funcvm _testfuncs_dynamic
            RUNNER funcvm
            RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
            ${_runtime_expectations}
    )
    if(NOT _rexxtest STREQUAL "signal_funcs")
        set_tests_properties(
                rxpa_${_rexxtest}_static_noopt
                rxpa_${_rexxtest}_static_opt
                PROPERTIES PASS_REGULAR_EXPRESSION ".*${_expected_output}.*"
        )
    endif()

endforeach()

crexx_label_smoke_tests(
        rxpa_signal_funcs_dynamic_noopt
        rxpa_native_payload_funcs_dynamic_noopt
        rxpa_native_unwind_funcs_dynamic_noopt
)

if(CREXX_VM_PROFILING)
    foreach(_vm_target rxvm)
        if(_vm_target STREQUAL "rxvm")
            set(_vm_executable "${CREXX_RXVM_EXECUTABLE}")
        else()
            set(_vm_executable "$<TARGET_FILE:${_vm_target}>")
        endif()
        add_test(
                NAME ${_vm_target}callcensusnativeunwind
                COMMAND ${CMAKE_COMMAND}
                        "-DVM=${_vm_executable}"
                        "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/rxpa_native_unwind_funcs_dynamic_noopt.rxbin"
                        "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/${_vm_target}-call-census-native-unwind.csv"
                        "-DCASE=native_unwind"
                        "-DRUNTIME_ARGS=rx_testfuncs_dynamic;${CMAKE_BINARY_DIR}/bin/library"
                        -P ${CMAKE_SOURCE_DIR}/interpreter/tests/check_vm_call_census.cmake)
        set_tests_properties(${_vm_target}callcensusnativeunwind PROPERTIES
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                LABELS "interpreter;profiling;call-census;rxpa")
    endforeach()
endif()
