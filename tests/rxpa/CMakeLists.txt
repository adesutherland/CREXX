cmake_minimum_required(VERSION 3.24)
project(rxpatest C)
add_subdirectory(multplugins)
find_package(Threads REQUIRED)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxConcurrencyTests.cmake)

# Including RXPlugin Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_custom_target(cri07_rxpa_sdk_consumer_prereqs
        DEPENDS rxc rxas rxvm rxbvm crexx library rxfnsg
                _hash _hash_static
                stats stats_static
                vector vector_static
                id id_static
                fs fs_static
                _platform _platform_static)
if(TARGET qa-prep)
    # The installed-consumer test may build its private scratch consumer, but
    # must never rebuild the active CREXX tree from inside CTest.
    add_dependencies(qa-prep cri07_rxpa_sdk_consumer_prereqs)
endif()

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
                -DSTATIC_SUFFIX=${CMAKE_STATIC_LIBRARY_SUFFIX}
                -DPACKAGE_RELATIVE_DIR=${CREXX_CMAKE_PACKAGE_INSTALL_DIR}
                -DCONSUMER_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/external_sdk
                -DWORK_ROOT=${CMAKE_CURRENT_BINARY_DIR}/rxpa_external_sdk_consumer_ws
                -P ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_external_sdk_consumer.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxpa_external_sdk_consumer PROPERTIES
        LABELS "install;package;rxpa;external-consumer;hash;crexx-rag;rcc-5d;rcc-5e;rcc-5f;stats;vector;packed"
        RUN_SERIAL TRUE
        TIMEOUT 240)

add_test(
        NAME rxpa_native_return_class_import
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DIMPORT_PATH=${CMAKE_BINARY_DIR}/bin
                -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rxpa_native_return_class_import.crexx
                -DOUTPUT_BASE=${CMAKE_CURRENT_BINARY_DIR}/rxpa_native_return_class_import
                -P ${CMAKE_CURRENT_SOURCE_DIR}/check_native_return_class_import.cmake)
set_tests_properties(rxpa_native_return_class_import PROPERTIES
        LABELS "compiler;rxpa;provider;metadata;class;regression;rcc-5f"
        TIMEOUT 30)

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

# RCC-1/RCC-2 fixture: one stable provider identity with dynamic and static
# delivery forms published through the production provider-package helper.
add_dynamic_plugin_target(_rcc_provider rcc_provider.c)
add_static_plugin_target(_rcc_provider rcc_provider.c)
add_rxpa_provider_package(_rcc_provider
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/providers")

# CRI-17: a session-aware provider used from attached local-task workers.
# The controller and every worker must resolve the same declarative provider
# identity while owning a distinct RXPA V2 session.
add_dynamic_plugin_target(_cri17_attached
        PROVIDER_ID rx_cri17provider
        cri17_attached_provider.c)
add_static_plugin_target(_cri17_attached
        PROVIDER_ID rx_cri17provider
        cri17_attached_provider.c)
localize_test_plugin_output(_cri17_attached)
add_rxpa_provider_package(_cri17_attached
        OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/providers")

add_custom_target(cri17_attached_provider_test ALL)
crexx_add_rexx_opt_matrix(
        NAME cri17_attached_provider
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/cri17_attached_provider.crexx
        TARGET_GROUP cri17_attached_provider_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        RXC_FLAGS -x
        DEPENDS library classlib _cri17_attached _cri17_attached_static
        RUNTIME_ARGS
                ${CMAKE_BINARY_DIR}/bin/library
                ${CMAKE_BINARY_DIR}/bin/classlib
        TEST_LABELS "interpreter;concurrency;rxpa;provider;cri-17;regression"
        REQUIRED_OUTPUT_REGEX
                "PASS: CRI-17 attached workers resolve distinct RXPA V2 sessions")
set_tests_properties(
        cri17_attached_provider_noopt
        cri17_attached_provider_opt
        PROPERTIES
        ENVIRONMENT "CREXX_PROVIDER_PATH=${CMAKE_CURRENT_BINARY_DIR}/providers"
        TIMEOUT 60)

function(crexx_configure_cri17_attached_control TARGET_NAME)
    target_include_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/interpreter
            ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
            ${CMAKE_SOURCE_DIR}/inc
            ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/binutils/include
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/avl_tree
            ${CMAKE_SOURCE_DIR}/utf8)
    target_link_libraries(${TARGET_NAME} PRIVATE Threads::Threads)
    add_dependencies(${TARGET_NAME}
            cri17_attached_provider_noopt_artifact
            _cri17_attached)
    add_test(
            NAME ${TARGET_NAME}
            COMMAND ${TARGET_NAME}
                    ${CMAKE_CURRENT_BINARY_DIR}/cri17_attached_provider_noopt.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/providers
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${TARGET_NAME} PROPERTIES
            LABELS "interpreter;concurrency;rxpa;provider;cri-17;regression;negative"
            TIMEOUT 60)
endfunction()

add_executable(cri17_attached_provider_control-rxvml
        cri17_attached_provider_control.c)
target_link_libraries(cri17_attached_provider_control-rxvml PRIVATE rxvml)
crexx_configure_cri17_attached_control(
        cri17_attached_provider_control-rxvml)

add_executable(cri17_attached_provider_control-rxbvml
        cri17_attached_provider_control.c)
target_link_libraries(cri17_attached_provider_control-rxbvml PRIVATE rxbvml)
target_compile_definitions(cri17_attached_provider_control-rxbvml PRIVATE
        NTHREADED=1)
crexx_configure_cri17_attached_control(
        cri17_attached_provider_control-rxbvml)

add_custom_target(rcc_provider_autoload_test ALL)
crexx_add_rexx_opt_matrix(
        NAME rcc_provider_autoload
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rcc_provider.crexx
        TARGET_GROUP rcc_provider_autoload_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _rcc_provider _rcc_provider_static
        TEST_LABELS "rxpa;provider;autoload")
# The optimized matrix deliberately stages the linked image under linked_opt/;
# exercise the explicit trusted-path route for that isolated container.
set_tests_properties(rcc_provider_autoload_opt PROPERTIES
        ENVIRONMENT "CREXX_PROVIDER_PATH=${CMAKE_CURRENT_BINARY_DIR}/providers")

add_test(
        NAME rcc_provider_wrong_identity_rejected
        COMMAND ${CMAKE_COMMAND}
                -DRUNNER=${CREXX_RXVM_EXECUTABLE}
                -DPROGRAM=${CMAKE_CURRENT_BINARY_DIR}/rcc_provider_autoload_noopt.rxbin
                -DWRONG_PLUGIN=$<TARGET_FILE:_rxpa_dynlink>
                -DSCRATCH=${CMAKE_CURRENT_BINARY_DIR}/rcc_provider_rejection
                -P ${CMAKE_CURRENT_SOURCE_DIR}/check_provider_rejection.cmake)
set_tests_properties(rcc_provider_wrong_identity_rejected PROPERTIES
        LABELS "rxpa;provider;autoload;negative")

add_test(
        NAME rcc_provider_package_names
        COMMAND ${CMAKE_COMMAND}
                -DPROVIDER_DIR=${CMAKE_CURRENT_BINARY_DIR}/providers
                -DPROVIDER_ID=rx_rcc_provider
                -DSTATIC_SUFFIX=${CMAKE_STATIC_LIBRARY_SUFFIX}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/check_provider_package_names.cmake)
set_tests_properties(rcc_provider_package_names PROPERTIES
        LABELS "rxpa;provider;package")

add_test(
        NAME rcc_provider_unused_import_elided
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXLINK=$<TARGET_FILE:rxlink>
                -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rcc_provider_unused.crexx
                "-DIMPORT_PATH=${CMAKE_BINARY_DIR}/bin"
                -DSCRATCH=${CMAKE_CURRENT_BINARY_DIR}/rcc_provider_unused
                -P ${CMAKE_CURRENT_SOURCE_DIR}/check_unused_provider.cmake)
set_tests_properties(rcc_provider_unused_import_elided PROPERTIES
        LABELS "compiler;rxpa;provider;negative")
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

add_dynamic_plugin_target(_rxpa_bench_session rxpa_call_kernel.c)
target_compile_definitions(_rxpa_bench_session PRIVATE
        RXPA_BENCH_SESSION_AWARE=1)
localize_test_plugin_output(_rxpa_bench_session)

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
crexx_label_concurrency_tests(SP01
        rxpa_process_reentrant_manifest
        rxpa_invalid_manifest_is_legacy
        rxpa_dynamic_context_ownership)

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
target_compile_features(testvm PRIVATE c_std_11)
if(MSVC)
    target_compile_options(testvm PRIVATE /experimental:c11atomics)
endif()
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
        # This composite contract launches 20 instrumented compiler,
        # assembler, and VM subprocesses. Under a full parallel sanitizer run
        # it needs the repository's established long-test allowance.
        TIMEOUT 300)

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
target_compile_features(funcvm PRIVATE c_std_11)
if(MSVC)
    target_compile_options(funcvm PRIVATE /experimental:c11atomics)
endif()
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
                EXPECTED_FAILURE_DESCRIPTION "native SIGNAL ERROR message"
                REQUIRED_OUTPUT_REGEX
                    "PANIC: .*\\(SIGNAL ERROR\\)"
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

crexx_finalize_directory_qa_tiers()
