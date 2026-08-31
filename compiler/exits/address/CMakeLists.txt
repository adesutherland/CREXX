cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_address_bin Address)

crexx_add_compiler_exit_runtime_test(
        test_address
        test_address.crexx
        EXTRA_DEPENDS library
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_address_direct
        test_address_direct.crexx
        EXTRA_DEPENDS exit_test_support_bin
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_address_after_signal_block
        test_address_after_signal_block.crexx
        EXTRA_DEPENDS rxc
        RUN_SUFFIX ${LIBRPATH} -a $<TARGET_FILE:rxc> ${CMAKE_CURRENT_BINARY_DIR}/missing-after-signal.rxbin
)
