cmake_minimum_required(VERSION 3.24)

# Build Sort.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_sort_bin Sort)

crexx_add_compiler_exit_runtime_test(
        test_sort
        test_sort.crexx
        RUN_SUFFIX ${LIBRPATH}
)
