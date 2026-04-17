cmake_minimum_required(VERSION 3.24)

# Build fsay.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Fsay.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i "\"${CMAKE_CURRENT_BINARY_DIR}/..;${CMAKE_CURRENT_SOURCE_DIR}/..\"" -o Fsay ${CMAKE_CURRENT_SOURCE_DIR}/Fsay.rexx
        COMMAND $<TARGET_FILE:rxas> -o Fsay Fsay
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Fsay.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Fsay.rxbin"
)
add_custom_target(exit_fsay_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Fsay.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_fsay
        test_fsay.rexx
        RUN_SUFFIX ${LIBRPATH}
)
