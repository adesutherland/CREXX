cmake_minimum_required(VERSION 3.24)

# Build Dump.rxbin
# Needs Token.rxbin for class metadata.
add_custom_command(
    OUTPUT Dump.rxbin
    COMMAND $<TARGET_FILE:rxc> -n -i ${CMAKE_CURRENT_SOURCE_DIR}/.. -o Dump ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx
    COMMAND $<TARGET_FILE:rxas> -o Dump.rxbin Dump.rxas
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/Dump.rexx token_bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Dump.rxbin"
)

add_custom_target(exit_dump_bin DEPENDS Dump.rxbin)
