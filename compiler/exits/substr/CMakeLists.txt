cmake_minimum_required(VERSION 3.24)

# Build substr.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_substr_bin Substr)

crexx_add_compiler_exit_runtime_test(
        test_substr
        test_substr.crexx
        RUN_SUFFIX ${LIBRPATH}
)
