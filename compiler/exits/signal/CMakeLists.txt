cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Signal.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Signal ${CMAKE_CURRENT_SOURCE_DIR}/Signal.rexx
        COMMAND $<TARGET_FILE:rxas> -o Signal Signal
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Signal.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Signal.rxbin"
)
add_custom_target(exit_signal_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Signal.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_signal
        test_signal.rexx
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_signal_direct
        test_signal_direct.rexx
        EXTRA_DEPENDS ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${LIBRPATH}
)
