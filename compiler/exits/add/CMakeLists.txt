cmake_minimum_required(VERSION 3.24)

# Build add.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Add.rxbin
        COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Add ${CMAKE_CURRENT_SOURCE_DIR}/Add.rexx
        COMMAND $<TARGET_FILE:rxas> -o Add Add
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Add.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Add.rxbin"
)
add_custom_target(exit_add_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Add.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_add
        test_add.rexx
        RUN_SUFFIX ${LIBRPATH}
)
