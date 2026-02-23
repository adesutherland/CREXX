cmake_minimum_required(VERSION 3.24)

# Build Dummy.rxbin
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Dummy.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Dummy ${CMAKE_CURRENT_SOURCE_DIR}/Dummy.rexx
    COMMAND $<TARGET_FILE:rxas> -o Dummy.rxbin Dummy.rxas
    DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Dummy.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Dummy.rxbin"
)
add_custom_target(exit_dummy_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Dummy.rxbin)
