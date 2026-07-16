set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

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
crexx_add_language_benchmark(benchmark_json_parser json_parser.crexx 1
        "PASS: JSON Parser")
crexx_add_language_benchmark(benchmark_base64_roundtrip base64_roundtrip.crexx 1
        "PASS: Base64 Roundtrip")
crexx_add_language_benchmark(benchmark_rexxcps_levelb rexxcps_levelb.crexx 10
        "PASS: RexxCPS 2.2c cREXX port")

crexx_add_rexx_opt_matrix(
        NAME benchmark_rexxcps_levelb_opaque
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rexxcps_levelb_opaque.crexx
        TARGET_GROUP language_benchmarks
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS library compiler_exit_bin
        RUNTIME_ARGS ${LIBRPATH} -a A Off 1 1
        TEST_LABELS "benchmark;language;optimizer-resistance"
        REQUIRED_OUTPUT_REGEX "PASS: RexxCPS 2.2c cREXX opaque/result-observation diagnostic"
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
                "benchmark,vm,mode,runs,min_ns,median_ns,mean_ns,max_ns,platform,crexx_version"
                "rexxcps,rxvm,opt,1"
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

crexx_label_smoke_tests(
        benchmark_awfy_sieve_opt
        benchmark_awfy_permute_opt
        benchmark_awfy_mandelbrot_opt
        benchmark_awfy_towers_opt
        benchmark_awfy_bounce_opt
        benchmark_awfy_storage_opt
        benchmark_awfy_list_opt
        benchmark_awfy_richards_opt
        benchmark_json_parser_opt
        benchmark_base64_roundtrip_opt
        benchmark_rexxcps_levelb_opt
        benchmark_rexxcps_levelb_opaque_opt
        benchmark_runner_opt
        nr03_evidence_tool_selftest_opt)
