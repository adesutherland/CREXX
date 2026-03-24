cmake_minimum_required(VERSION 3.24)

# Build Dump.rxbin
# Needs token.rxbin for class metadata.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Dump ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx
    COMMAND $<TARGET_FILE:rxas> -o Dump Dump
    DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxas
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Dump.rxbin"
)
add_custom_target(exit_dump_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxbin)
# Ensure the token class bundle is built before Dump
add_dependencies(exit_dump_bin token_bin)

# Test the dump exit
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_dump.rxas
    COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_dump ${CMAKE_CURRENT_SOURCE_DIR}/test_dump.rexx
    DEPENDS rxc ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/test_dump.rexx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Compiling test_dump.rexx with exit"
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_dump.rxbin
    COMMAND $<TARGET_FILE:rxas> -o test_dump test_dump
    DEPENDS rxas ${CMAKE_CURRENT_BINARY_DIR}/test_dump.rxas
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Assembling test_dump.rxas"
)

add_custom_target(test_dump_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_dump.rxbin)
add_dependencies(test_dump_bin rxvm)
add_dependencies(test_dump_bin compiler_exit_bin)

add_test(NAME test_dump COMMAND $<TARGET_FILE:rxvm> ${CMAKE_CURRENT_BINARY_DIR}/test_dump.rxbin WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
