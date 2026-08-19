set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(CLASSLIBRPATH ${CMAKE_BINARY_DIR}/bin/classlib)

# These benchmarks are smoke-run under CTest so opt/noopt timing stays visible,
# but they do not assert performance thresholds because those vary by machine.
add_custom_target(performance_inline ALL)

crexx_add_rexx_opt_matrix(
        NAME inlining_simple
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/inlining_simple.crexx
        TARGET_GROUP performance_inline
        DEPENDS library compiler_exit_bin
        TEST_LABELS "performance;inlining"
)

add_custom_target(performance_reference_iterator ALL)
foreach(_crexx_runner IN ITEMS rxvm)
    crexx_add_rexx_opt_matrix(
            NAME reference_iterator_compare_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/reference_iterator_compare.crexx
            TARGET_GROUP performance_reference_iterator
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH}
            TEST_LABELS "performance;reference"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(reference_iterator_compare_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: Reference Iterator Source Performance Smoke"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

# DECIMAL-01 Gate 1 provider comparison. The workload performs no internal
# timing; CTest only qualifies deterministic execution across the two VMs,
# all three optimizer boundaries, and the explicit bundled providers.
add_custom_target(performance_decimal_gate1)
add_dependencies(performance_decimal_gate1 mc_decimal_dynamic db_decimal_dynamic)
add_dependencies(performance_decimal_gate1 rxdas)

add_executable(decimal_gate1_adapter
        ${CMAKE_CURRENT_SOURCE_DIR}/decimal/decimal_gate1_adapter.c)
target_include_directories(decimal_gate1_adapter PRIVATE
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/binutils/include
        ${CMAKE_SOURCE_DIR}/utf8)
target_link_libraries(decimal_gate1_adapter PRIVATE
        rxvmplugin ${CREXX_MATH_LIBRARY})
add_dependencies(decimal_gate1_adapter mc_decimal_dynamic db_decimal_dynamic)
if(TARGET mpd_decimal_dynamic)
    add_dependencies(decimal_gate1_adapter mpd_decimal_dynamic)
endif()
if(TARGET decquad_decimal_dynamic)
    add_dependencies(decimal_gate1_adapter decquad_decimal_dynamic)
endif()
add_dependencies(performance_decimal_gate1 decimal_gate1_adapter)

set(_decimal_adapter_providers mc db)
if(TARGET mpd_decimal_dynamic)
    list(APPEND _decimal_adapter_providers mpd)
endif()
if(TARGET decquad_decimal_dynamic)
    list(APPEND _decimal_adapter_providers decquad)
endif()
foreach(_decimal_adapter_provider IN LISTS _decimal_adapter_providers)
    foreach(_decimal_adapter_context IN ITEMS common18 classic9)
        if(_decimal_adapter_context STREQUAL "common18")
            set(_decimal_adapter_standard common)
            set(_decimal_adapter_digits 18)
        else()
            set(_decimal_adapter_standard classic)
            set(_decimal_adapter_digits 9)
        endif()
        foreach(_decimal_adapter_mode IN ITEMS
                arithmetic conversion compare copy-clear sync)
            set(_decimal_adapter_test
                    decimal_gate1_adapter_${_decimal_adapter_provider}_${_decimal_adapter_context}_${_decimal_adapter_mode})
            add_test(
                    NAME ${_decimal_adapter_test}
                    COMMAND $<TARGET_FILE:decimal_gate1_adapter>
                            rxvm_${_decimal_adapter_provider}_decimal
                            ${_decimal_adapter_standard}
                            ${_decimal_adapter_digits}
                            ${_decimal_adapter_mode}
                            2
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            )
            set_tests_properties(${_decimal_adapter_test} PROPERTIES
                    LABELS "performance;decimal;decimal01;gate1;adapter;qualification"
                    PASS_REGULAR_EXPRESSION "PASS: DECIMAL-01 Gate 1 adapter payload"
                    FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
                    FIXTURES_REQUIRED linked_opt_runtime_artifacts
            )
        endforeach()
    endforeach()
endforeach()

foreach(_decimal_standard IN ITEMS common18 classic9)
    set(_decimal_name decimal_gate1_${_decimal_standard})
    crexx_add_rexx_opt_matrix(
            NAME ${_decimal_name}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/decimal/${_decimal_name}.crexx
            TARGET_GROUP performance_decimal_gate1
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin mc_decimal_dynamic db_decimal_dynamic
            RUNTIME_ARGS ${LIBRPATH} -a all 2 1
            TEST_LABELS "performance;decimal;decimal01;gate1;qualification"
            REQUIRED_OUTPUT_REGEX "PASS: DECIMAL-01 Gate 1 decimal workload"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:"
    )

    # Assemble the optimized compiler output a second time with only the RXAS
    # optimizer disabled. This isolates RXAS effects from the full no-opt image,
    # which disables optimization in both rxc and rxas.
    set(_decimal_rxas_noopt_base ${_decimal_name}_opt_rxas_noopt)
    set(_decimal_rxas_noopt_artifact
            ${CMAKE_CURRENT_BINARY_DIR}/${_decimal_rxas_noopt_base}.rxbin)
    add_custom_command(
            OUTPUT ${_decimal_rxas_noopt_artifact}
            COMMAND $<TARGET_FILE:rxas>
                    -n -o ${_decimal_rxas_noopt_base} ${_decimal_name}_opt
            DEPENDS rxas ${_decimal_name}_opt_artifact
                    ${CMAKE_CURRENT_BINARY_DIR}/${_decimal_name}_opt.rxas
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set(_decimal_rxas_noopt_target ${_decimal_rxas_noopt_base}_artifact)
    add_custom_target(${_decimal_rxas_noopt_target}
            DEPENDS ${_decimal_rxas_noopt_artifact})
    add_dependencies(performance_decimal_gate1 ${_decimal_rxas_noopt_target})

    foreach(_decimal_mode IN ITEMS noopt opt opt_rxas_noopt)
        if(_decimal_mode STREQUAL "opt_rxas_noopt")
            set(_decimal_integrity_source
                    ${CMAKE_CURRENT_BINARY_DIR}/${_decimal_name}_opt.rxas)
        else()
            set(_decimal_integrity_source
                    ${CMAKE_CURRENT_BINARY_DIR}/${_decimal_name}_${_decimal_mode}.rxas)
        endif()
        set(_decimal_integrity_test
                ${_decimal_name}_${_decimal_mode}_rxas_integrity)
        add_test(
                NAME ${_decimal_integrity_test}
                COMMAND ${CMAKE_COMMAND}
                        -DRXAS_SOURCE=${_decimal_integrity_source}
                        -DRXBIN=${CMAKE_CURRENT_BINARY_DIR}/${_decimal_name}_${_decimal_mode}.rxbin
                        -DRXDAS=$<TARGET_FILE:rxdas>
                        -DDISASSEMBLY=${CMAKE_CURRENT_BINARY_DIR}/${_decimal_name}_${_decimal_mode}_final.rxas
                        -DCASE=${_decimal_name}_${_decimal_mode}
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/decimal/check_decimal_gate1_rxas.cmake
        )
        set_tests_properties(${_decimal_integrity_test} PROPERTIES
                LABELS "performance;decimal;decimal01;gate1;qualification;optimizer-integrity"
                FIXTURES_REQUIRED linked_opt_runtime_artifacts
        )
    endforeach()

    foreach(_decimal_runner IN ITEMS rxvm rxbvm)
        if(_decimal_runner STREQUAL "rxvm")
            set(_decimal_runner_executable ${CREXX_RXVM_EXECUTABLE})
        else()
            set(_decimal_runner_executable $<TARGET_FILE:${_decimal_runner}>)
        endif()
        foreach(_decimal_provider IN ITEMS default mc db)
            if(_decimal_provider STREQUAL "default")
                set(_decimal_plugin_args)
            else()
                set(_decimal_plugin_args -p rxvm_${_decimal_provider}_decimal)
            endif()
            foreach(_decimal_mode IN ITEMS noopt opt opt_rxas_noopt)
                set(_decimal_test
                        ${_decimal_name}_${_decimal_runner}_${_decimal_provider}_${_decimal_mode})
                add_test(
                        NAME ${_decimal_test}
                        COMMAND ${_decimal_runner_executable}
                                ${_decimal_plugin_args}
                                ${CMAKE_CURRENT_BINARY_DIR}/${_decimal_name}_${_decimal_mode}
                                ${LIBRPATH}
                                -a all 2 1
                        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
                )
                set_tests_properties(${_decimal_test} PROPERTIES
                        LABELS "performance;decimal;decimal01;gate1;qualification"
                        PASS_REGULAR_EXPRESSION
                                "PASS: DECIMAL-01 Gate 1 decimal workload"
                        FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
                        FIXTURES_REQUIRED linked_opt_runtime_artifacts
                )
            endforeach()
        endforeach()
    endforeach()
endforeach()

# The shared CTest setup target must also make every optimizer qualification
# image and dynamic provider available for an isolated DECIMAL-01 selection.
add_dependencies(linked_opt_runtime_artifacts performance_decimal_gate1)

crexx_add_rexx_opt_matrix(
        NAME decimal_gate1_rexxcps_family
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rexxcps_family_controls.crexx
        TARGET_GROUP performance_decimal_gate1
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin mc_decimal_dynamic db_decimal_dynamic
        RUNTIME_ARGS ${LIBRPATH} -a decimal-string 2
        TEST_LABELS "performance;decimal;decimal01;gate1;qualification;rexxcps-control"
        REQUIRED_OUTPUT_REGEX "PASS: RexxCPS family control"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:"
)

foreach(_decimal_runner IN ITEMS rxvm rxbvm)
    if(_decimal_runner STREQUAL "rxvm")
        set(_decimal_runner_executable ${CREXX_RXVM_EXECUTABLE})
    else()
        set(_decimal_runner_executable $<TARGET_FILE:${_decimal_runner}>)
    endif()
    foreach(_decimal_provider IN ITEMS default mc db)
        if(_decimal_provider STREQUAL "default")
            set(_decimal_plugin_args)
        else()
            set(_decimal_plugin_args -p rxvm_${_decimal_provider}_decimal)
        endif()
        foreach(_decimal_mode IN ITEMS noopt opt)
            set(_decimal_test
                    decimal_gate1_rexxcps_family_${_decimal_runner}_${_decimal_provider}_${_decimal_mode})
            add_test(
                    NAME ${_decimal_test}
                    COMMAND ${_decimal_runner_executable}
                            ${_decimal_plugin_args}
                            ${CMAKE_CURRENT_BINARY_DIR}/decimal_gate1_rexxcps_family_${_decimal_mode}
                            ${LIBRPATH}
                            -a decimal-string 2
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            )
            set_tests_properties(${_decimal_test} PROPERTIES
                    LABELS "performance;decimal;decimal01;gate1;qualification;rexxcps-control"
                    PASS_REGULAR_EXPRESSION "PASS: RexxCPS family control"
                    FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
                    FIXTURES_REQUIRED linked_opt_runtime_artifacts
            )
        endforeach()
    endforeach()
endforeach()

# crexx_add_rexx_opt_matrix registers optimized images with the shared runtime
# fixture, while these NR-15 target groups are deliberately not part of ALL.
# Make the same serialized fixture own both groups so an isolated CTest run can
# generate the four noopt images instead of depending on a warm build tree.
add_dependencies(linked_opt_runtime_artifacts
        performance_nr15_stem_semantics
        performance_nr15_stem_access
        performance_nr16_17_stable_state)

add_custom_target(performance_classlib_iterator ALL)
foreach(_crexx_runner IN ITEMS rxvm)
    crexx_add_rexx_opt_matrix(
            NAME stringarraylist_iterator_compare_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/stringarraylist_iterator_compare.crexx
            TARGET_GROUP performance_classlib_iterator
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library classlib compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH} ${CLASSLIBRPATH}
            TEST_LABELS "performance;reference;classlib"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(stringarraylist_iterator_compare_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: StringArrayList StringIterator Performance Smoke"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

add_custom_target(performance_classlib_treemap ALL)
crexx_add_rexx_opt_matrix(
        NAME stringtreemap_insert_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/stringtreemap_insert_compare.crexx
        TARGET_GROUP performance_classlib_treemap
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library classlib compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} ${CLASSLIBRPATH}
        TEST_LABELS "performance;classlib;treemap"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(stringtreemap_insert_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: StringTreeMap Insert Comparison"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_binary_fastpath ALL)
crexx_add_rexx_opt_matrix(
        NAME binary_fastpath_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/binary_fastpath_compare.crexx
        TARGET_GROUP performance_binary_fastpath
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH}
        TEST_LABELS "performance;binary"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(binary_fastpath_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: Binary Fastpath Compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_binary_byvalue ALL)
foreach(_crexx_runner IN ITEMS rxvm)
    crexx_add_rexx_opt_matrix(
            NAME binary_byvalue_compare_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/binary_byvalue_compare.crexx
            TARGET_GROUP performance_binary_byvalue
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH}
            TEST_LABELS "performance;binary;inlining;perf2_07"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(binary_byvalue_compare_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: PERF2-07 binary by-value compare"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

add_custom_target(performance_rxjson_parser ALL)
crexx_add_rexx_opt_matrix(
        NAME rxjson_parser_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxjson_parser_compare.crexx
        TARGET_GROUP performance_rxjson_parser
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH}
        TEST_LABELS "performance;json"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(rxjson_parser_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: rxjson parser compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_rxjson_numeric ALL)
crexx_add_rexx_opt_matrix(
        NAME rxjson_numeric_materialization_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxjson_numeric_materialization_compare.crexx
        TARGET_GROUP performance_rxjson_numeric
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a 256 2
        TEST_LABELS "performance;json;binary;cap01_j02"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(rxjson_numeric_materialization_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: CRI-13 JSON numeric materialization compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()
_crexx_register_runtime_test(
        NAME rxjson_numeric_materialization_compare_rxbvm_noopt
        RUNNER rxbvm
        PROGRAM rxjson_numeric_materialization_compare_noopt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} -a 256 2
        TEST_LABELS "performance;json;binary;cap01_j02"
)
_crexx_register_runtime_test(
        NAME rxjson_numeric_materialization_compare_rxbvm_opt
        RUNNER rxbvm
        PROGRAM rxjson_numeric_materialization_compare_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} -a 256 2
        TEST_LABELS "performance;json;binary;cap01_j02"
)
set_tests_properties(
        rxjson_numeric_materialization_compare_rxbvm_noopt
        rxjson_numeric_materialization_compare_rxbvm_opt
        PROPERTIES
        PASS_REGULAR_EXPRESSION "PASS: CRI-13 JSON numeric materialization compare"
        FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
)

add_custom_target(performance_postperf04_scalar_access ALL)
crexx_add_rexx_opt_matrix(
        NAME postperf04_scalar_access_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/postperf04_scalar_access_compare.crexx
        TARGET_GROUP performance_postperf04_scalar_access
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a 256 2
        TEST_LABELS "performance;scalar-access;postperf04"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(postperf04_scalar_access_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: POSTPERF-04 scalar access compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()
_crexx_register_runtime_test(
        NAME postperf04_scalar_access_compare_rxbvm_noopt
        RUNNER rxbvm
        PROGRAM postperf04_scalar_access_compare_noopt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} -a 256 2
        TEST_LABELS "performance;scalar-access;postperf04"
)
_crexx_register_runtime_test(
        NAME postperf04_scalar_access_compare_rxbvm_opt
        RUNNER rxbvm
        PROGRAM postperf04_scalar_access_compare_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} -a 256 2
        TEST_LABELS "performance;scalar-access;postperf04"
)
set_tests_properties(
        postperf04_scalar_access_compare_rxbvm_noopt
        postperf04_scalar_access_compare_rxbvm_opt
        PROPERTIES
        PASS_REGULAR_EXPRESSION "PASS: POSTPERF-04 scalar access compare"
        FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
)
add_test(
        NAME postperf04_scalar_access_codegen
        COMMAND ${CMAKE_COMMAND}
                -DOPT_IMAGE=${CMAKE_CURRENT_BINARY_DIR}/postperf04_scalar_access_compare_opt.rxas
                -DNOOPT_IMAGE=${CMAKE_CURRENT_BINARY_DIR}/postperf04_scalar_access_compare_noopt.rxas
                -DOPT_BINARY=${CMAKE_CURRENT_BINARY_DIR}/postperf04_scalar_access_compare_opt.rxbin
                -DRXDAS=$<TARGET_FILE:rxdas>
                -P ${CMAKE_CURRENT_SOURCE_DIR}/check_postperf04_scalar_access_codegen.cmake
)
set_tests_properties(postperf04_scalar_access_codegen PROPERTIES
        LABELS "performance;scalar-access;postperf04;codegen"
        FIXTURES_REQUIRED linked_opt_runtime_artifacts
)

add_custom_target(performance_tinyexpr_dispatch ALL)
crexx_add_rexx_opt_matrix(
        NAME tinyexpr_dispatch_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tinyexpr_dispatch_compare.crexx
        TARGET_GROUP performance_tinyexpr_dispatch
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        RXC_FLAGS -x
        DEPENDS library rxfnsl compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/rxfnsl
        TEST_LABELS "performance;dispatch;level-l"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(tinyexpr_dispatch_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: tinyexpr dispatch compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_jump_dispatch ALL)
crexx_add_rexx_opt_matrix(
        NAME jump_dispatch_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/jump_dispatch_compare.crexx
        TARGET_GROUP performance_jump_dispatch
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH}
        TEST_LABELS "performance;dispatch"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(jump_dispatch_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: jump dispatch compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_runtime_lookup ALL)
crexx_add_rexx_opt_matrix(
        NAME runtime_interface_lookup_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/runtime_interface_lookup_compare.crexx
        TARGET_GROUP performance_runtime_lookup
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH}
        TEST_LABELS "performance;vm;interfaces"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(runtime_interface_lookup_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: runtime interface lookup compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

crexx_add_rexx_opt_matrix(
        NAME inlining_complex
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/inlining_complex.crexx
        TARGET_GROUP performance_inline
        DEPENDS library compiler_exit_bin
        TEST_LABELS "performance;inlining"
)

add_custom_target(performance_nr15_stem_semantics)
add_custom_target(performance_nr15_stem_access)
foreach(_crexx_runner IN ITEMS rxvm)
    crexx_add_rexx_opt_matrix(
            NAME nr15_stem_semantics_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nr15_stem_semantics.crexx
            TARGET_GROUP performance_nr15_stem_semantics
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH}
            TEST_LABELS "performance;stem;nr15;semantics"
    )
    crexx_add_rexx_opt_matrix(
            NAME nr15_stem_access_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nr15_stem_access_compare.crexx
            TARGET_GROUP performance_nr15_stem_access
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH}
            TEST_LABELS "performance;stem;nr15"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(nr15_stem_semantics_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: NR-15 stem semantic matrix"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
        set_tests_properties(nr15_stem_access_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: NR-15 stem access compare"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

add_custom_target(performance_nr16_17_stable_state)
foreach(_crexx_runner IN ITEMS rxvm)
    crexx_add_rexx_opt_matrix(
            NAME nr16_17_stable_state_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nr16_17_stable_state_compare.crexx
            TARGET_GROUP performance_nr16_17_stable_state
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin
            RUNTIME_ARGS ${LIBRPATH} -a control 10
            TEST_LABELS "performance;trace;address;calls;nr16;nr17"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(nr16_17_stable_state_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: NR-16/17 stable-state compare"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()
