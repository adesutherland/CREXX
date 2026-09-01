cmake_minimum_required(VERSION 3.24)

# Build execio.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_execio_bin Execio)

crexx_add_compiler_exit_runtime_test(
        test_execio
        test_execio.crexx
        RUN_SUFFIX ${LIBRPATH}
)
