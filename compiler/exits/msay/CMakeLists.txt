cmake_minimum_required(VERSION 3.24)

# Build msay.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Msay.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Msay ${CMAKE_CURRENT_SOURCE_DIR}/Msay.crexx
        COMMAND $<TARGET_FILE:rxas> -o Msay Msay
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Msay.crexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Msay.rxbin"
)
add_custom_target(exit_msay_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Msay.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_msay
        test_msay.crexx
        RUN_SUFFIX ${LIBRPATH}
)
