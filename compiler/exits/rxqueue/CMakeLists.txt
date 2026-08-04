cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Queue.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Queue ${CMAKE_CURRENT_SOURCE_DIR}/Queue.crexx
        COMMAND $<TARGET_FILE:rxas> -o Queue Queue
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Queue.crexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Queue compiler exit"
)
add_custom_target(exit_rxqueue_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Queue.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_rxqueue
        test_rxqueue.crexx
        PASS_REGULAR_EXPRESSION "RXQUEUE EXIT PASSED"
        RUN_SUFFIX ${LIBRPATH}
)
