cmake_minimum_required(VERSION 3.24)

# Build sortx.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Sortx.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Sortx ${CMAKE_CURRENT_SOURCE_DIR}/Sortx.rexx
        COMMAND $<TARGET_FILE:rxas> -o Sortx Sortx
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Sortx.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Sortx.rxbin"
)
add_custom_target(exit_sortx_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Sortx.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_sortx
        test_sortx.rexx
        RUN_SUFFIX ${LIBRPATH}
)
