cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_rxqueue_bin Queue)

crexx_add_compiler_exit_runtime_test(
        test_rxqueue
        test_rxqueue.crexx
        PASS_REGULAR_EXPRESSION "RXQUEUE EXIT PASSED"
        RUN_SUFFIX ${LIBRPATH}
)
