cmake_minimum_required(VERSION 3.24)

# Build parse.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Parse.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Parse ${CMAKE_CURRENT_SOURCE_DIR}/Parse.rexx
        COMMAND $<TARGET_FILE:rxas> -o Parse Parse
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Parse.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Parse.rxbin"
)
add_custom_target(exit_parse_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Parse.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_parse
        test_parse.rexx
        RUN_SUFFIX ${LIBRPATH}
)
