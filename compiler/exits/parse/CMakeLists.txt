cmake_minimum_required(VERSION 3.24)

# Build parse.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_parse_bin Parse)

crexx_add_compiler_exit_runtime_test(
        test_parse
        test_parse.crexx
        PASS_REGULAR_EXPRESSION "PARSE TESTS PASSED"
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_parse_direct
        test_parse_direct.crexx
        EXTRA_DEPENDS exit_test_support_bin
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_parse_compiled_patterns
        test_parse_compiled_patterns.crexx
        PASS_REGULAR_EXPRESSION "PASS: compiled existing-instruction PARSE patterns"
        RUN_SUFFIX ${LIBRPATH}
)
