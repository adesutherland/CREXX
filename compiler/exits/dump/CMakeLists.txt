cmake_minimum_required(VERSION 3.24)

# Build Dump.rxbin
# Needs token.rxbin for class metadata.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -n -i ${CMAKE_CURRENT_SOURCE_DIR}/.. -o Dump ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx
    COMMAND $<TARGET_FILE:rxas> -o Dump Dump
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Dump.rxbin"
)
add_custom_target(exit_dump_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Dump.rxbin)

# Test the dump exit
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_dump.rxas
    COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=../rxcexits $<TARGET_FILE:rxc> -o test_dump ${CMAKE_CURRENT_SOURCE_DIR}/test_dump.rexx
    DEPENDS rxc ${CMAKE_CURRENT_SOURCE_DIR}/test_dump.rexx compiler_exit_bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Compiling test_dump.rexx with exit"
)

add_custom_command(
    OUTPUT test_dump.rxbin
    COMMAND $<TARGET_FILE:rxas> -o test_dump test_dump
    DEPENDS rxas test_dump.rxas
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Assembling test_dump.rxas"
)

add_custom_target(test_dump_bin ALL DEPENDS test_dump.rxbin)

add_test(NAME test_dump COMMAND rxvm test_dump.rxbin WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
