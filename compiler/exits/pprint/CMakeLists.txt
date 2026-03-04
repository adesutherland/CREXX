cmake_minimum_required(VERSION 3.24)

# Build pprint.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Pprint.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Pprint ${CMAKE_CURRENT_SOURCE_DIR}/Pprint.rexx
        COMMAND $<TARGET_FILE:rxas> -o Pprint Pprint
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Pprint.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Pprint.rxbin"
)
add_custom_target(exit_pprint_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Pprint.rxbin)

# Test the sort exit
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_pprint.rxas
        COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_pprint ${CMAKE_CURRENT_SOURCE_DIR}/test_pprint.rexx
        DEPENDS rxc ${CMAKE_CURRENT_SOURCE_DIR}/test_pprint.rexx compiler_exit_bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_pprint.rexx with exit"
)
execute_process(
        COMMAND cmd /c "echo Debug breakpoint reached & pause"
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_pprint.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_pprint test_pprint
        DEPENDS rxas ${CMAKE_CURRENT_BINARY_DIR}/test_pprint.rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling test_pprint.rxas"
)
add_custom_target(test_pprint_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_pprint.rxbin)

add_test(NAME test_pprint COMMAND rxvm test_pprint.rxbin ${LIBRPATH} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
