cmake_minimum_required(VERSION 3.24)

# Build Query.rxbin
crexx_add_compiler_exit_component(exit_query_bin Query)

crexx_add_compiler_exit_runtime_test(
        test_query
        test_query.crexx
        RUN_PREFIX ${CMAKE_BINARY_DIR}/bin/library.rxbin
)
