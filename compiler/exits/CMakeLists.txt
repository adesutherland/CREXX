cmake_minimum_required(VERSION 3.24)

# Build token.rxbin
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin
    COMMAND $<TARGET_FILE:rxc> -n -o token ${CMAKE_CURRENT_SOURCE_DIR}/token.rexx
    COMMAND $<TARGET_FILE:rxas> -o token.rxbin token.rxas
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/token.rexx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building token.rxbin"
)
add_custom_target(token_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin)

add_subdirectory(dump)

# Concatenate them
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/compiler_exit.rxbin
    COMMAND ${CMAKE_COMMAND} -E cat token.rxbin dump/Dump.rxbin > compiler_exit.rxbin
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin 
            ${CMAKE_CURRENT_BINARY_DIR}/dump/Dump.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building compiler_exit.rxbin"
)
add_custom_target(compiler_exit_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/compiler_exit.rxbin)

add_custom_command(TARGET compiler_exit_bin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/compiler_exit.rxbin ${CMAKE_BINARY_DIR}/bin/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/compiler_exit.rxbin ${CMAKE_BINARY_DIR}/compiler/
)
