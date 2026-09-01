cmake_minimum_required(VERSION 3.24)

# Build add.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_add_bin Add)

crexx_add_compiler_exit_runtime_test(
        test_add
        test_add.crexx
        RUN_SUFFIX ${LIBRPATH}
)
