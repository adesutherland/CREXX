cmake_minimum_required(VERSION 3.24)

# Build sortx.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_sortx_bin Sortx)

crexx_add_compiler_exit_runtime_test(
        test_sortx
        test_sortx.crexx
        RUN_SUFFIX ${LIBRPATH}
)
