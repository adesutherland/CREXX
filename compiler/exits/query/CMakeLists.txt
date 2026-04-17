cmake_minimum_required(VERSION 3.24)

# Build Query.rxbin
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Query.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i "\"${CMAKE_CURRENT_BINARY_DIR}/..;${CMAKE_CURRENT_SOURCE_DIR}/..\"" -o Query ${CMAKE_CURRENT_SOURCE_DIR}/Query.rexx
    COMMAND $<TARGET_FILE:rxas> -o Query.rxbin Query.rxas
    DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Query.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/Query.rxas
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Query.rxbin"
)
add_custom_target(exit_query_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Query.rxbin)
# Ensure the token class bundle is built before Query
add_dependencies(exit_query_bin token_bin)

crexx_add_compiler_exit_runtime_test(
        test_query
        test_query.rexx
        RUN_PREFIX ${CMAKE_BINARY_DIR}/bin/library.rxbin
)
