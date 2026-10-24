set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(CLASSLIBRPATH ${CMAKE_BINARY_DIR}/bin/classlib)

# One pure fixture-oracle owner, shared by the workload guards and tiny
# ordinary wrong-result/nonfinite controls. It contains no timed kernel.
set(_perf_smoke_oracle_base "${CMAKE_CURRENT_BINARY_DIR}/perfsmokeoracles")
crexx_add_fixture_command(OUTPUT "${_perf_smoke_oracle_base}.rxbin"
    BYPRODUCTS "${_perf_smoke_oracle_base}.rxas"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -o "${_perf_smoke_oracle_base}"
        "${CMAKE_CURRENT_SOURCE_DIR}/perf_smoke_oracles.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_perf_smoke_oracle_base}"
    DEPENDS rxc rxas compiler_exit_bin "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
        "${CMAKE_CURRENT_SOURCE_DIR}/perf_smoke_oracles.crexx" VERBATIM)
add_custom_target(performance_smoke_oracles DEPENDS "${_perf_smoke_oracle_base}.rxbin")
crexx_add_import_root(performance_smoke_oracle_imports
    DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports/smoke-oracles"
    OUTPUT_VARIABLE _perf_smoke_oracle_import_files
    FILES "${_perf_smoke_oracle_base}.rxbin" DEPENDS performance_smoke_oracles)
set(_perf_smoke_oracle_deps performance_smoke_oracle_imports ${_perf_smoke_oracle_import_files})
set(_perf_smoke_oracle_import_root "${CMAKE_CURRENT_BINARY_DIR}/imports/smoke-oracles")
set(_perf_smoke_oracle_check "${CMAKE_CURRENT_BINARY_DIR}/check_smoke_oracles")
crexx_add_fixture_command(OUTPUT "${_perf_smoke_oracle_check}.rxbin"
    BYPRODUCTS "${_perf_smoke_oracle_check}.rxas"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
        -i "${_perf_smoke_oracle_import_root};${CMAKE_BINARY_DIR}/bin"
        -o "${_perf_smoke_oracle_check}" "${CMAKE_CURRENT_SOURCE_DIR}/check_smoke_oracles.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_perf_smoke_oracle_check}"
    DEPENDS rxc rxas compiler_exit_bin "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
        library "${CMAKE_BINARY_DIR}/bin/library.rxbin"
        ${_perf_smoke_oracle_deps} "${CMAKE_CURRENT_SOURCE_DIR}/check_smoke_oracles.crexx" VERBATIM)
add_custom_target(performance_smoke_oracle_contract_bin DEPENDS "${_perf_smoke_oracle_check}.rxbin")
add_test(NAME performance_smoke_oracle_contract
    COMMAND $<TARGET_FILE:${CREXX_DEFAULT_VM_TARGET}> "${_perf_smoke_oracle_check}.rxbin"
        "${_perf_smoke_oracle_base}" "${LIBRPATH}")
set_tests_properties(performance_smoke_oracle_contract PROPERTIES
    LABELS "performance;qualification;qa-fixture" TIMEOUT 60
    PASS_REGULAR_EXPRESSION "PERFORMANCE_SMOKE_ORACLES[|]failures=0"
    FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:")
crexx_register_test_prep_targets(TESTS performance_smoke_oracle_contract
    TARGETS performance_smoke_oracle_contract_bin ${CREXX_DEFAULT_VM_TARGET})

# CREXXRAG-SHA256 retains the test-only A-versus-D capability decision gate.
# It also provides the matched RXPA control used to confirm that the selected
# production rx_hash provider has not materially changed that accepted path.
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)
add_dynamic_plugin_target(_crexxrag_sha256_gate
        ${CMAKE_CURRENT_SOURCE_DIR}/crexxrag_sha256_gate_plugin.c)
set_target_properties(_crexxrag_sha256_gate PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(_crexxrag_sha256_gate PRIVATE
        ${CMAKE_SOURCE_DIR}/binutils/include)
target_link_libraries(_crexxrag_sha256_gate PRIVATE rxbin)

add_executable(crexxrag_sha256_c_ceiling EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/crexxrag_sha256_c_ceiling.c)
target_link_libraries(crexxrag_sha256_c_ceiling PRIVATE rxbin)

add_custom_target(performance_crexxrag_sha256)
add_dependencies(performance_crexxrag_sha256
        _crexxrag_sha256_gate crexxrag_sha256_c_ceiling _hash)
crexx_add_import_root(crexxrag_sha256_imports
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/imports/crexxrag_sha256
        FILES ${CMAKE_CURRENT_BINARY_DIR}/rx_crexxrag_sha256_gate.rxplugin
        DEPENDS _crexxrag_sha256_gate
        OUTPUT_VARIABLE _crexxrag_sha256_import_files)
crexx_add_rexx_opt_matrix(
        NAME crexxrag_sha256_a_vs_d
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/crexxrag_sha256_a_vs_d.crexx
        TARGET_GROUP performance_crexxrag_sha256
        RUNNER rxvm
        IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR}/imports/crexxrag_sha256 ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin crexxrag_sha256_imports
                ${_crexxrag_sha256_import_files} _hash
        RUNTIME_ARGS rx_crexxrag_sha256_gate ${LIBRPATH} -a selftest
        TEST_LABELS "performance;sha256;crexx-rag;qualification"
        REQUIRED_OUTPUT_REGEX "PASS: CREXXRAG-SHA256 vectors"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:"
)

# These benchmarks are smoke-run under CTest so opt/noopt timing stays visible,
# but they do not assert performance thresholds because those vary by machine.
add_custom_target(performance_inline)

crexx_add_rexx_opt_matrix(
        NAME inlining_simple
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/inlining_simple.crexx
        TARGET_GROUP performance_inline
        RUNNER rxvm
        IMPORT_PATHS ${_perf_smoke_oracle_import_root}
        DEPENDS ${_perf_smoke_oracle_deps} library compiler_exit_bin
        RUNTIME_ARGS ${_perf_smoke_oracle_base}
        TEST_LABELS "performance;inlining"
        REQUIRED_OUTPUT_REGEX "PASS: simple inlining smoke"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:"
)

add_custom_target(performance_reference_iterator)
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
        ${CMAKE_SOURCE_DIR}/third_party/utf8)
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
            RUNNER rxvm
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
        RUNNER rxvm
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
            )
        endforeach()
    endforeach()
endforeach()

# These NR-15 target groups are deliberately not part of ALL and contain a
# mixed artifact set beyond their ordinary opt-matrix images.  Keep the same
# serialized fixture owning the complete groups so an isolated CTest run does
# not depend on a warm build tree.
add_dependencies(linked_opt_runtime_artifacts
        performance_nr15_stem_semantics
        performance_nr15_stem_access
        performance_nr16_17_stable_state)

add_custom_target(performance_classlib_iterator)
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

add_custom_target(performance_classlib_treemap)
crexx_add_rexx_opt_matrix(
        NAME stringtreemap_insert_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/stringtreemap_insert_compare.crexx
        TARGET_GROUP performance_classlib_treemap
        RUNNER rxvm
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

add_custom_target(performance_binary_fastpath)
crexx_add_rexx_opt_matrix(
        NAME binary_fastpath_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/binary_fastpath_compare.crexx
        TARGET_GROUP performance_binary_fastpath
        RUNNER rxvm
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

add_custom_target(performance_binary_byvalue)
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

add_custom_target(performance_rxjson_parser)
crexx_add_rexx_opt_matrix(
        NAME rxjson_parser_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxjson_parser_compare.crexx
        TARGET_GROUP performance_rxjson_parser
        RUNNER rxvm
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

add_custom_target(performance_rxjson_numeric)
crexx_add_rexx_opt_matrix(
        NAME rxjson_numeric_materialization_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxjson_numeric_materialization_compare.crexx
        TARGET_GROUP performance_rxjson_numeric
        RUNNER rxvm
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
        PREP_TARGETS rxjson_numeric_materialization_compare_noopt_artifact rxbvm
        TEST_LABELS "performance;json;binary;cap01_j02"
)
_crexx_register_runtime_test(
        NAME rxjson_numeric_materialization_compare_rxbvm_opt
        RUNNER rxbvm
        PROGRAM rxjson_numeric_materialization_compare_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} -a 256 2
        PREP_TARGETS rxjson_numeric_materialization_compare_opt_artifact rxbvm rxlink
        TEST_LABELS "performance;json;binary;cap01_j02"
)
set_tests_properties(
        rxjson_numeric_materialization_compare_rxbvm_noopt
        rxjson_numeric_materialization_compare_rxbvm_opt
        PROPERTIES
        PASS_REGULAR_EXPRESSION "PASS: CRI-13 JSON numeric materialization compare"
        FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
)

add_custom_target(performance_postperf04_scalar_access)
crexx_add_rexx_opt_matrix(
        NAME postperf04_scalar_access_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/postperf04_scalar_access_compare.crexx
        TARGET_GROUP performance_postperf04_scalar_access
        RUNNER rxvm
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
        PREP_TARGETS postperf04_scalar_access_compare_noopt_artifact rxbvm
        TEST_LABELS "performance;scalar-access;postperf04"
)
_crexx_register_runtime_test(
        NAME postperf04_scalar_access_compare_rxbvm_opt
        RUNNER rxbvm
        PROGRAM postperf04_scalar_access_compare_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} -a 256 2
        PREP_TARGETS postperf04_scalar_access_compare_opt_artifact rxbvm rxlink
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
        LABELS "performance;scalar-access;postperf04;codegen;qualification"
)
add_dependencies(performance_postperf04_scalar_access rxdas)
crexx_register_test_prep_targets(TESTS postperf04_scalar_access_codegen
        TARGETS postperf04_scalar_access_compare_noopt_artifact
                postperf04_scalar_access_compare_opt_artifact rxdas)

add_custom_target(performance_binary_packed_float)
crexx_add_rexx_opt_matrix(
        NAME binary_packed_float_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/binary_packed_float_compare.crexx
        TARGET_GROUP performance_binary_packed_float
        RUNNER rxvm
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a 1000 2 packed-read
        TEST_LABELS "performance;binary;packed;binary01"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(binary_packed_float_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: BINARY-01 packed float compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()
_crexx_register_runtime_test(
        NAME binary_packed_float_compare_rxbvm_opt
        RUNNER rxbvm
        PROGRAM binary_packed_float_compare_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} -a 1000 2 packed-read
        PREP_TARGETS binary_packed_float_compare_opt_artifact rxbvm rxlink
        TEST_LABELS "performance;binary;packed;binary01"
)
set_tests_properties(binary_packed_float_compare_rxbvm_opt PROPERTIES
        PASS_REGULAR_EXPRESSION "PASS: BINARY-01 packed float compare"
        FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
)

add_custom_target(performance_packed_numeric_class)
crexx_add_rexx_opt_matrix(
        NAME packed_numeric_class_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/packed_numeric_class_compare.crexx
        TARGET_GROUP performance_packed_numeric_class
        RUNNER rxvm
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        RXC_FLAGS -x
        DEPENDS library classlib rxfnsg compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} ${CLASSLIBRPATH} ${CMAKE_BINARY_DIR}/bin/rxfnsg
                     -a 1000 2 wrapper-int-read
        TEST_LABELS "performance;binary;packed;binary01;levelg"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(packed_numeric_class_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: BINARY-01 packed numeric class compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()
_crexx_register_runtime_test(
        NAME packed_numeric_class_compare_rxbvm_opt
        RUNNER rxbvm
        PROGRAM packed_numeric_class_compare_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} ${CLASSLIBRPATH} ${CMAKE_BINARY_DIR}/bin/rxfnsg
                     -a 1000 2 wrapper-int-read
        PREP_TARGETS packed_numeric_class_compare_opt_artifact rxbvm rxlink
        TEST_LABELS "performance;binary;packed;binary01;levelg"
)
set_tests_properties(packed_numeric_class_compare_rxbvm_opt PROPERTIES
        PASS_REGULAR_EXPRESSION "PASS: BINARY-01 packed numeric class compare"
        FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
)

# RCC-5F keeps the former boxed surface as a test-only numerical/performance
# oracle and compiles a one-call native control from the exact production
# accumulation kernel. Neither provider is installed or part of Level G.
add_library(rcc5f_stats_kernel STATIC EXCLUDE_FROM_ALL
        ${CMAKE_SOURCE_DIR}/lib/plugins/stats/rxstats.c)
set_target_properties(rcc5f_stats_kernel PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(rcc5f_stats_kernel PRIVATE RXSTATS_KERNEL_ONLY)
target_include_directories(rcc5f_stats_kernel PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/plugins/stats)
_crexx_configure_rxpa_plugin_target(rcc5f_stats_kernel)
target_link_libraries(rcc5f_stats_kernel PRIVATE ${CREXX_MATH_LIBRARY})

add_dynamic_plugin_target(_rcc5f_stats_boxed rcc5f_stats_boxed.c)
set_target_properties(_rcc5f_stats_boxed PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(_rcc5f_stats_boxed PRIVATE ${CREXX_MATH_LIBRARY})

add_dynamic_plugin_target(_rcc5f_stats_direct rcc5f_stats_direct.c)
set_target_properties(_rcc5f_stats_direct PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(_rcc5f_stats_direct PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/plugins/stats)
target_link_libraries(_rcc5f_stats_direct PRIVATE
        rcc5f_stats_kernel ${CREXX_MATH_LIBRARY})

add_custom_target(performance_rcc5f_rxstats)
add_dependencies(performance_rcc5f_rxstats
        stats rxfnsg _rcc5f_stats_boxed _rcc5f_stats_direct)
crexx_add_import_root(rcc5f_imports
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/imports/rcc5f
        FILES ${CMAKE_CURRENT_BINARY_DIR}/rx_rcc5f_stats_boxed.rxplugin
              ${CMAKE_CURRENT_BINARY_DIR}/rx_rcc5f_stats_direct.rxplugin
        DEPENDS _rcc5f_stats_boxed _rcc5f_stats_direct
        OUTPUT_VARIABLE _rcc5f_import_files)

crexx_add_rexx_opt_matrix(
        NAME rcc5f_rxstats_packed
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rcc5f_rxstats_packed.crexx
        TARGET_GROUP performance_rcc5f_rxstats
        RUNNER rxvm
        IMPORT_PATHS ${_perf_smoke_oracle_import_root} ${CMAKE_BINARY_DIR}/bin
        DEPENDS ${_perf_smoke_oracle_deps} library rxfnsg stats compiler_exit_bin
        RUNTIME_ARGS ${_perf_smoke_oracle_base} $<TARGET_FILE:stats> ${LIBRPATH}
                     ${CMAKE_BINARY_DIR}/bin/rxfnsg
        TEST_LABELS "performance;math;stats;packed;rcc5f"
        REQUIRED_OUTPUT_REGEX "PASS: RCC-5F packed statistics workload"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

crexx_add_rexx_opt_matrix(
        NAME rcc5f_rxstats_boxed
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rcc5f_rxstats_boxed.crexx
        TARGET_GROUP performance_rcc5f_rxstats
        RUNNER rxvm
        IMPORT_PATHS ${_perf_smoke_oracle_import_root} ${CMAKE_CURRENT_BINARY_DIR}/imports/rcc5f ${CMAKE_BINARY_DIR}/bin
        DEPENDS ${_perf_smoke_oracle_deps} library rcc5f_imports ${_rcc5f_import_files} compiler_exit_bin
        RUNTIME_ARGS ${_perf_smoke_oracle_base} $<TARGET_FILE:_rcc5f_stats_boxed> ${LIBRPATH}
        TEST_LABELS "performance;math;stats;boxed;oracle;rcc5f"
        REQUIRED_OUTPUT_REGEX "PASS: RCC-5F boxed statistics oracle workload"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

crexx_add_rexx_opt_matrix(
        NAME rcc5f_rxstats_direct
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rcc5f_rxstats_direct.crexx
        TARGET_GROUP performance_rcc5f_rxstats
        RUNNER rxvm
        IMPORT_PATHS ${_perf_smoke_oracle_import_root} ${CMAKE_CURRENT_BINARY_DIR}/imports/rcc5f ${CMAKE_BINARY_DIR}/bin
        DEPENDS ${_perf_smoke_oracle_deps} library rxfnsg rcc5f_imports ${_rcc5f_import_files} compiler_exit_bin
        RUNTIME_ARGS ${_perf_smoke_oracle_base} $<TARGET_FILE:_rcc5f_stats_direct> ${LIBRPATH}
                     ${CMAKE_BINARY_DIR}/bin/rxfnsg
        TEST_LABELS "performance;math;stats;packed;direct-control;rcc5f"
        REQUIRED_OUTPUT_REGEX "PASS: RCC-5F direct statistics control"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

foreach(_rcc5f_variant IN ITEMS packed boxed direct)
    if(_rcc5f_variant STREQUAL "packed")
        set(_rcc5f_runtime_args $<TARGET_FILE:stats> ${LIBRPATH}
                ${CMAKE_BINARY_DIR}/bin/rxfnsg ${_perf_smoke_oracle_base})
        set(_rcc5f_pass "PASS: RCC-5F packed statistics workload")
    elseif(_rcc5f_variant STREQUAL "boxed")
        set(_rcc5f_runtime_args $<TARGET_FILE:_rcc5f_stats_boxed>
                ${LIBRPATH} ${_perf_smoke_oracle_base})
        set(_rcc5f_pass "PASS: RCC-5F boxed statistics oracle workload")
    else()
        set(_rcc5f_runtime_args $<TARGET_FILE:_rcc5f_stats_direct>
                ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/rxfnsg ${_perf_smoke_oracle_base})
        set(_rcc5f_pass "PASS: RCC-5F direct statistics control")
    endif()
    _crexx_register_runtime_test(
            NAME rcc5f_rxstats_${_rcc5f_variant}_rxbvm_opt
            RUNNER rxbvm
            PROGRAM rcc5f_rxstats_${_rcc5f_variant}_opt
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RUNTIME_ARGS ${_rcc5f_runtime_args}
            PREP_TARGETS rcc5f_rxstats_${_rcc5f_variant}_opt_artifact rxbvm rxlink
        TEST_LABELS "performance;math;stats;rcc5f;rxbvm"
            REQUIRED_OUTPUT_REGEX ${_rcc5f_pass}
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
endforeach()

# RXVECTOR-01 compares the approved public packed provider with a pure Level B
# oracle and a one-call direct control compiled from the exact production
# kernel. The target is explicit so normal builds do not construct the large
# representative measurement workload.
add_library(rxvector01_kernel STATIC EXCLUDE_FROM_ALL
        ${CMAKE_SOURCE_DIR}/lib/plugins/vector/rxvector.c)
set_target_properties(rxvector01_kernel PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(rxvector01_kernel PRIVATE RXVECTOR_KERNEL_ONLY)
target_include_directories(rxvector01_kernel PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/plugins/vector)
_crexx_configure_rxpa_plugin_target(rxvector01_kernel)
target_link_libraries(rxvector01_kernel PRIVATE ${CREXX_MATH_LIBRARY})

add_dynamic_plugin_target(_rxvector01_direct rxvector01_direct.c)
set_target_properties(_rxvector01_direct PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(_rxvector01_direct PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/plugins/vector)
target_link_libraries(_rxvector01_direct PRIVATE
        rxvector01_kernel ${CREXX_MATH_LIBRARY})

add_custom_target(performance_rxvector01)
add_dependencies(performance_rxvector01
        vector rxfnsg _rxvector01_direct)
crexx_add_import_root(rxvector01_imports
        DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/imports/rxvector01
        FILES ${CMAKE_CURRENT_BINARY_DIR}/rx_rxvector01_direct.rxplugin
        DEPENDS _rxvector01_direct
        OUTPUT_VARIABLE _rxvector01_import_files)
crexx_add_rexx_opt_matrix(
        NAME rxvector01_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxvector01_compare.crexx
        TARGET_GROUP performance_rxvector01
        RUNNER rxvm
        IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR}/imports/rxvector01 ${CMAKE_BINARY_DIR}/bin
        DEPENDS library rxfnsg vector rxvector01_imports
                ${_rxvector01_import_files} compiler_exit_bin
        RUNTIME_ARGS $<TARGET_FILE:_rxvector01_direct> ${LIBRPATH}
                     ${CMAKE_BINARY_DIR}/bin/rxfnsg -a public 1
        TEST_LABELS "performance;vector;packed;rxvector01"
        REQUIRED_OUTPUT_REGEX "PASS: RXVECTOR-01 exact comparison"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

add_custom_target(performance_tinyexpr_dispatch)
crexx_add_rexx_opt_matrix(
        NAME tinyexpr_dispatch_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tinyexpr_dispatch_compare.crexx
        TARGET_GROUP performance_tinyexpr_dispatch
        RUNNER rxvm
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/langtools/examples
        RXC_FLAGS -x
        DEPENDS library rxlang_example_tinyexpr compiler_exit_bin
            "${CMAKE_BINARY_DIR}/langtools/examples/tinyexpr.rxbin"
        RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/langtools/examples/tinyexpr
        TEST_LABELS "performance;dispatch;level-l"
)
foreach(_crexx_mode IN ITEMS noopt opt)
    set_tests_properties(tinyexpr_dispatch_compare_${_crexx_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: tinyexpr dispatch compare"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    )
endforeach()

add_custom_target(performance_jump_dispatch)
crexx_add_rexx_opt_matrix(
        NAME jump_dispatch_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/jump_dispatch_compare.crexx
        TARGET_GROUP performance_jump_dispatch
        RUNNER rxvm
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

add_custom_target(performance_runtime_lookup)
crexx_add_rexx_opt_matrix(
        NAME runtime_interface_lookup_compare
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/runtime_interface_lookup_compare.crexx
        TARGET_GROUP performance_runtime_lookup
        RUNNER rxvm
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
        RUNNER rxvm
        IMPORT_PATHS ${_perf_smoke_oracle_import_root}
        DEPENDS ${_perf_smoke_oracle_deps} library compiler_exit_bin
        RUNTIME_ARGS ${_perf_smoke_oracle_base}
        TEST_LABELS "performance;inlining"
        REQUIRED_OUTPUT_REGEX "PASS: complex inlining smoke"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:"
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
            TEST_LABELS "performance;stem;nr15;semantics;qualification"
    )
    crexx_add_rexx_opt_matrix(
            NAME nr15_stem_access_${_crexx_runner}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/nr15_stem_access_compare.crexx
            TARGET_GROUP performance_nr15_stem_access
            RUNNER ${_crexx_runner}
            IMPORT_PATHS ${_perf_smoke_oracle_import_root} ${CMAKE_BINARY_DIR}/bin
            DEPENDS ${_perf_smoke_oracle_deps} library compiler_exit_bin
            RUNTIME_ARGS ${_perf_smoke_oracle_base} ${LIBRPATH}
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
            IMPORT_PATHS ${_perf_smoke_oracle_import_root} ${CMAKE_BINARY_DIR}/bin
            DEPENDS ${_perf_smoke_oracle_deps} library "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                rxfnsc "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin" compiler_exit_bin
            RUNTIME_ARGS ${_perf_smoke_oracle_base} ${LIBRPATH} -a control 10
            TEST_LABELS "performance;trace;address;calls;nr16;nr17"
    )
    foreach(_crexx_mode IN ITEMS noopt opt)
        set_tests_properties(nr16_17_stable_state_${_crexx_runner}_${_crexx_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "PASS: NR-16/17 stable-state compare"
                FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        )
    endforeach()
endforeach()

foreach(_inline_mode IN ITEMS noopt opt)
    set_tests_properties(inlining_simple_${_inline_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: simple inlining smoke"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:")
    set_tests_properties(inlining_complex_${_inline_mode} PROPERTIES
            PASS_REGULAR_EXPRESSION "PASS: complex inlining smoke"
            FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:")
endforeach()

# The old stable-state registration is the control lane. Exercise the two
# advertised steady-state mechanisms with its same prepared optimized image.
foreach(_stable_path IN ITEMS trace-off address-same-static)
    if(_stable_path STREQUAL "trace-off")
        set(_stable_name nr16_17_trace_off_rxvm_opt)
    else()
        set(_stable_name nr16_17_address_same_rxvm_opt)
    endif()
    add_test(NAME ${_stable_name} COMMAND ${CMAKE_COMMAND}
        "-DNR16_NAME=${_stable_name}" "-DNR16_MODE=${_stable_path}"
        "-DNR16_RUNNER=${CREXX_RXVM_EXECUTABLE}" "-DNR16_RXLINK=$<TARGET_FILE:rxlink>"
        "-DNR16_PROGRAM=${CMAKE_CURRENT_BINARY_DIR}/nr16_17_stable_state_rxvm_opt.rxbin"
        "-DNR16_LIBRARY=${LIBRPATH}.rxbin" "-DNR16_ORACLE=${_perf_smoke_oracle_base}.rxbin"
        "-DNR16_WORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/nr16_17_runtime.cmake")
    set_tests_properties(${_stable_name} PROPERTIES
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        LABELS "performance;trace;address;nr16;nr17;performance-smoke;linked_opt"
        CREXX_PREP_TARGETS "nr16_17_stable_state_rxvm_opt_artifact;rxvm;rxlink")
endforeach()
add_test(NAME nr16_17_output_guard_contract COMMAND ${CMAKE_COMMAND}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/nr16_17_output_contract.cmake")
set_tests_properties(nr16_17_output_guard_contract PROPERTIES
    LABELS "performance;qualification;qa-fixture" TIMEOUT 30 CREXX_PREP_NONE TRUE)

# Explicit nonrepresentative smoke membership. Do not automatically add a new
# timing workload, no-opt mode or alias merely because it has a similar label.
set(_performance_smoke_tests
    inlining_simple_opt reference_iterator_compare_rxvm_opt
    stringarraylist_iterator_compare_rxvm_opt stringtreemap_insert_compare_opt
    binary_fastpath_compare_opt binary_byvalue_compare_rxvm_opt
    rxjson_parser_compare_opt rxjson_numeric_materialization_compare_opt
    postperf04_scalar_access_compare_opt binary_packed_float_compare_opt
    packed_numeric_class_compare_opt rcc5f_rxstats_packed_opt
    rcc5f_rxstats_boxed_opt rcc5f_rxstats_direct_opt rxvector01_compare_opt
    tinyexpr_dispatch_compare_opt jump_dispatch_compare_opt
    runtime_interface_lookup_compare_opt inlining_complex_opt
    nr15_stem_access_rxvm_opt nr16_17_stable_state_rxvm_opt)
add_custom_target(performance_smoke_prep)
foreach(_performance_smoke_test IN LISTS _performance_smoke_tests)
    set_property(TEST ${_performance_smoke_test} APPEND PROPERTY LABELS performance-smoke)
    set_property(GLOBAL APPEND PROPERTY CREXX_PERFORMANCE_SMOKE_TARGETS
            ${_performance_smoke_test}_artifact)
endforeach()
set_property(GLOBAL APPEND PROPERTY CREXX_PERFORMANCE_SMOKE_TARGETS rxvm rxlink)
function(_crexx_finalize_performance_smoke_prep)
    get_property(_smoke_targets GLOBAL PROPERTY CREXX_PERFORMANCE_SMOKE_TARGETS)
    list(REMOVE_DUPLICATES _smoke_targets)
    add_dependencies(performance_smoke_prep ${_smoke_targets})
endfunction()
cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL _crexx_finalize_performance_smoke_prep)

crexx_finalize_directory_qa_tiers()
