cmake_minimum_required(VERSION 3.24)

# Build sortx.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Sortx.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Sortx ${CMAKE_CURRENT_SOURCE_DIR}/Sortx.rexx
        COMMAND $<TARGET_FILE:rxas> -o Sortx Sortx
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Sortx.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Sortx.rxbin"
)
add_custom_target(exit_sortx_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Sortx.rxbin)

# Test the sort exit
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_sortx.rxas
        COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_sortx ${CMAKE_CURRENT_SOURCE_DIR}/test_sortx.rexx
        DEPENDS rxc ${CMAKE_CURRENT_SOURCE_DIR}/test_sortx.rexx compiler_exit_bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_sortx.rexx with exit"
)
execute_process(
        COMMAND cmd /c "echo Debug breakpoint reached & pause"
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_sortx.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_sortx test_sortx
        DEPENDS rxas ${CMAKE_CURRENT_BINARY_DIR}/test_sortx.rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling test_sortx.rxas"
)
add_custom_target(test_sortx_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_sortx.rxbin)

add_test(NAME test_sortx COMMAND rxvm test_sortx.rxbin ${LIBRPATH} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
