cmake_minimum_required(VERSION 3.24)

# Build Sort.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Sort.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Sort ${CMAKE_CURRENT_SOURCE_DIR}/Sort.rexx
        COMMAND $<TARGET_FILE:rxas> -o Sort Sort
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Sort.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Sort.rxbin"
)
add_custom_target(exit_sort_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Sort.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_sort
        test_sort.rexx
        RUN_SUFFIX ${LIBRPATH}
)
