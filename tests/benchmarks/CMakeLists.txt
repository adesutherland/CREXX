set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/awfy_json_rap_minified.json
        ${CMAKE_CURRENT_BINARY_DIR}/awfy_json_rap_minified.json
        COPYONLY
)

add_custom_target(language_benchmarks ALL)

function(crexx_add_language_benchmark NAME SOURCE SMOKE_ARG PASS_REGEX)
    set(_runtime_args ${LIBRPATH} -a ${SMOKE_ARG})
    set(_test_labels "benchmark;language")
    set(_required_output_regex "${PASS_REGEX}")
    if(NAME STREQUAL "benchmark_rexxcps_levelb")
        set(_runtime_args ${LIBRPATH} -a --smoke-count ${SMOKE_ARG})
        list(APPEND _test_labels "rexxcps-smoke" "noncanonical")
        list(APPEND _required_output_regex
                "REXXCPS-PROVENANCE contract=noncanonical-smoke"
                "NONCANONICAL: RexxCPS smoke count is below the canonical minimum of 100")
    endif()
    crexx_add_rexx_opt_matrix(
            NAME ${NAME}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}
            TARGET_GROUP language_benchmarks
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library compiler_exit_bin
            RUNTIME_ARGS ${_runtime_args}
            TEST_LABELS ${_test_labels}
            REQUIRED_OUTPUT_REGEX ${_required_output_regex}
            FORBIDDEN_OUTPUT_REGEX "FAIL:|Failed|PANIC:"
    )
endfunction()

crexx_add_language_benchmark(benchmark_awfy_sieve awfy_sieve.crexx 1
        "PASS: AWFY Sieve")
crexx_add_language_benchmark(benchmark_awfy_permute awfy_permute.crexx 1
        "PASS: AWFY Permute")
crexx_add_language_benchmark(benchmark_awfy_mandelbrot awfy_mandelbrot.crexx 1
        "PASS: AWFY Mandelbrot")
crexx_add_language_benchmark(benchmark_awfy_mandelbrot_hoisted_control awfy_mandelbrot_hoisted_control.crexx 1
        "PASS: AWFY Mandelbrot hoisted control")
crexx_add_language_benchmark(benchmark_awfy_towers awfy_towers.crexx 1
        "PASS: AWFY Towers")
crexx_add_language_benchmark(benchmark_awfy_bounce awfy_bounce.crexx 1
        "PASS: AWFY Bounce")
crexx_add_language_benchmark(benchmark_awfy_storage awfy_storage.crexx 1
        "PASS: AWFY Storage")
crexx_add_language_benchmark(benchmark_awfy_list awfy_list.crexx 1
        "PASS: AWFY List")
crexx_add_language_benchmark(benchmark_awfy_richards awfy_richards.crexx 1
        "PASS: AWFY Richards")
crexx_add_language_benchmark(benchmark_awfy_queens awfy_queens.crexx 1
        "PASS: AWFY Queens")
crexx_add_rexx_opt_matrix(
        NAME benchmark_awfy_deltablue
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/awfy_deltablue.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a 10
        TEST_LABELS "benchmark;language;stable-indexed-graph-adaptation"
        REQUIRED_OUTPUT_REGEX "PASS: AWFY DeltaBlue"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|Failed|PANIC:"
)
crexx_add_rexx_opt_matrix(
        NAME benchmark_awfy_cd
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/awfy_cd.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib/plugins/rxmath
        DEPENDS library compiler_exit_bin _rxmath
        RUNTIME_ARGS rx_rxmath ${LIBRPATH} -a 2
        TEST_LABELS "benchmark;language;indexed-red-black-tree-adaptation;native-math-control"
        REQUIRED_OUTPUT_REGEX "PASS: AWFY CD"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|Failed|PANIC:"
)
crexx_add_rexx_opt_matrix(
        NAME benchmark_awfy_json
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/awfy_json.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a 1
                ${CMAKE_CURRENT_BINARY_DIR}/awfy_json_rap_minified.json
        TEST_LABELS "benchmark;language;standard-library-adaptation"
        REQUIRED_OUTPUT_REGEX "PASS: Full AWFY Json"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|Failed|PANIC:"
)
crexx_add_language_benchmark(benchmark_json_parser json_parser.crexx 1
        "PASS: JSON Parser")
crexx_add_language_benchmark(benchmark_json_parse json_parse.crexx 1
        "PASS: JSON Parse")
crexx_add_language_benchmark(benchmark_json_query json_query.crexx 1
        "PASS: JSON Query")
crexx_add_language_benchmark(benchmark_base64_roundtrip base64_roundtrip.crexx 1
        "PASS: Base64 Roundtrip")
crexx_add_language_benchmark(benchmark_base64_roundtrip_v2 base64_roundtrip_v2.crexx 1
        "PASS: Base64 Roundtrip v2")
crexx_add_language_benchmark(benchmark_parse_frozen parse_frozen.crexx 1
        "PASS: Frozen PARSE")
crexx_add_language_benchmark(benchmark_parse_frozen_generic parse_frozen_generic.crexx 1
        "PASS: Generic Frozen PARSE")
crexx_add_language_benchmark(benchmark_rexxcps_levelb rexxcps_levelb.crexx 10
        "PASS: RexxCPS 2.2d cREXX port")

crexx_add_rexx_opt_matrix(
        NAME benchmark_awfy_nbody
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/awfy_nbody.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib/plugins/rxmath
        DEPENDS library compiler_exit_bin _rxmath
        RUNTIME_ARGS rx_rxmath ${LIBRPATH} -a 1
        TEST_LABELS "benchmark;language;native-math-control"
        REQUIRED_OUTPUT_REGEX "PASS: AWFY NBody"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|Failed|PANIC:"
)

add_test(
        NAME benchmark_rexxcps_decimal_register_integrity
        COMMAND ${CMAKE_COMMAND}
                -DRXAS=${CMAKE_CURRENT_BINARY_DIR}/benchmark_rexxcps_levelb_opt.rxas
                -P ${CMAKE_CURRENT_SOURCE_DIR}/check_rexxcps_decimal_registers.cmake
)
set_tests_properties(benchmark_rexxcps_decimal_register_integrity PROPERTIES
        LABELS "benchmark;performance;decimal;optimizer-integrity;rexxcps-control"
)

crexx_add_rexx_opt_matrix(
        NAME benchmark_rexxcps_levelb_opaque
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rexxcps_levelb_opaque.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a A Off 1 1
        TEST_LABELS "benchmark;language;optimizer-resistance"
        REQUIRED_OUTPUT_REGEX "PASS: RexxCPS 2.2d cREXX opaque/result-observation diagnostic"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|Failed|PANIC:"
)

crexx_add_rexx_opt_matrix(
        NAME benchmark_runner
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/run_benchmarks.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a
                --build-dir ${CMAKE_BINARY_DIR}
                --vm rxvm
                --mode opt
                --warmups 0
                --runs 1
                --benchmark rexxcps
        TEST_LABELS "benchmark;runner;canonical-contract"
        REQUIRED_OUTPUT_REGEX
                "benchmark,vm,mode,runs,min_ns,median_ns,mean_ns,max_ns,platform,crexx_version,vm_role,result_role"
                "rexxcps,rxvm,opt,1.*product,process-smoke"
        FORBIDDEN_OUTPUT_REGEX "ERROR:|FAIL:|PANIC:"
)

crexx_add_rexx_opt_matrix(
        NAME nr03_evidence_tool_selftest
        SOURCE ${CMAKE_SOURCE_DIR}/performance/tools/run_evidence_bundle.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a --self-test
        TEST_LABELS "benchmark;runner;performance"
        REQUIRED_OUTPUT_REGEX "PASS: NR-03 evidence tool self-test"
        FORBIDDEN_OUTPUT_REGEX "ERROR:|FAIL:|PANIC:"
)

crexx_add_rexx_opt_matrix(
        NAME nr10_cross_runtime_matrix_selftest
        SOURCE ${CMAKE_SOURCE_DIR}/performance/tools/run_cross_runtime_matrix.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a --self-test
        TEST_LABELS "benchmark;runner;performance"
        REQUIRED_OUTPUT_REGEX "PASS: compact cross-runtime matrix tool self-test"
        FORBIDDEN_OUTPUT_REGEX "ERROR:|FAIL:|PANIC:"
)

crexx_add_rexx_opt_matrix(
        NAME nr10_lifecycle_tool_selftest
        SOURCE ${CMAKE_SOURCE_DIR}/performance/tools/run_lifecycle.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a --self-test
        TEST_LABELS "benchmark;runner;performance"
        REQUIRED_OUTPUT_REGEX "PASS: lifecycle tool self-test"
        FORBIDDEN_OUTPUT_REGEX "ERROR:|FAIL:|PANIC:"
)

crexx_add_rexx_opt_matrix(
        NAME nr10_artifact_inventory_selftest
        SOURCE ${CMAKE_SOURCE_DIR}/performance/tools/inventory_performance_artifacts.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a --self-test
        TEST_LABELS "benchmark;runner;performance"
        REQUIRED_OUTPUT_REGEX "PASS: performance artifact inventory self-test"
        FORBIDDEN_OUTPUT_REGEX "ERROR:|FAIL:|PANIC:"
)

crexx_add_rexx_opt_matrix(
        NAME perf3_fusion_inventory_selftest
        SOURCE ${CMAKE_SOURCE_DIR}/performance/tools/inventory_fusions.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a --self-test
        TEST_LABELS "benchmark;runner;performance;fusion"
        REQUIRED_OUTPUT_REGEX "PASS: fusion inventory tool self-test"
        FORBIDDEN_OUTPUT_REGEX "ERROR:|FAIL:|PANIC:"
)

crexx_label_smoke_tests(
        benchmark_awfy_sieve_opt
        benchmark_awfy_permute_opt
        benchmark_awfy_mandelbrot_opt
        benchmark_awfy_mandelbrot_hoisted_control_opt
        benchmark_awfy_towers_opt
        benchmark_awfy_bounce_opt
        benchmark_awfy_storage_opt
        benchmark_awfy_list_opt
        benchmark_awfy_richards_opt
        benchmark_awfy_queens_opt
        benchmark_awfy_deltablue_opt
        benchmark_awfy_cd_opt
        benchmark_awfy_json_opt
        benchmark_awfy_nbody_opt
        benchmark_json_parser_opt
        benchmark_json_parse_opt
        benchmark_json_query_opt
        benchmark_base64_roundtrip_opt
        benchmark_base64_roundtrip_v2_opt
        benchmark_parse_frozen_opt
        benchmark_rexxcps_levelb_opt
        benchmark_rexxcps_levelb_opaque_opt
        benchmark_runner_opt
        nr03_evidence_tool_selftest_opt
        nr10_cross_runtime_matrix_selftest_opt
        nr10_lifecycle_tool_selftest_opt
        nr10_artifact_inventory_selftest_opt
        perf3_fusion_inventory_selftest_opt)
