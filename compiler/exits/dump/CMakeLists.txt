cmake_minimum_required(VERSION 3.24)

# Build Dump.rxbin
# Needs token.rxbin for class metadata.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxbin
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Dump ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx
    COMMAND $<TARGET_FILE:rxas> -o Dump Dump
    DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxas
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Dump.rxbin"
)
add_custom_target(exit_dump_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxbin)
# Ensure the token class bundle is built before Dump
add_dependencies(exit_dump_bin token_bin)

crexx_add_compiler_exit_runtime_test(test_dump test_dump.rexx)
