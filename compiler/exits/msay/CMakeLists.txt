cmake_minimum_required(VERSION 3.24)

# Build msay.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_msay_bin Msay)

crexx_add_compiler_exit_runtime_test(
        test_msay
        test_msay.crexx
        RUN_SUFFIX ${LIBRPATH}
)
