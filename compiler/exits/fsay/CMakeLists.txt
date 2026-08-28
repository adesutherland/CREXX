cmake_minimum_required(VERSION 3.24)

# Build fsay.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_fsay_bin Fsay)

crexx_add_compiler_exit_runtime_test(
        test_fsay
        test_fsay.crexx
        RUN_SUFFIX ${LIBRPATH}
)
