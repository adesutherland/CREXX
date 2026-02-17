cmake_minimum_required(VERSION 3.24)

# Build token.rxbin
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -n -o token ${CMAKE_CURRENT_SOURCE_DIR}/token.rexx
    COMMAND $<TARGET_FILE:rxas> -o token.rxbin token.rxas
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/token.rexx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building token.rxbin"
)
add_custom_target(token_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin)

add_subdirectory(dump)

# Build the bundle by concatenating all exit components
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rxcexits.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -n -i ${CMAKE_CURRENT_SOURCE_DIR}/dump -o rxcexits_base ${CMAKE_CURRENT_SOURCE_DIR}/rxcexits.rexx
    COMMAND $<TARGET_FILE:rxas> -o rxcexits_base.rxbin rxcexits_base.rxas
    COMMAND ${CMAKE_COMMAND} -E cat rxcexits_base.rxbin token.rxbin dump/Dump.rxbin > rxcexits.rxbin
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/rxcexits.rxbin ${CMAKE_BINARY_DIR}/bin/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/rxcexits.rxbin ${CMAKE_BINARY_DIR}/compiler/
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/rxcexits.rexx
            ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin 
            ${CMAKE_CURRENT_BINARY_DIR}/dump/Dump.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building rxcexits.rxbin (bundled via concatenation)"
)
add_custom_target(compiler_exit_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/rxcexits.rxbin)
