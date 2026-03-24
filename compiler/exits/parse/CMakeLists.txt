cmake_minimum_required(VERSION 3.24)

# Build parse.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Parse.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Parse ${CMAKE_CURRENT_SOURCE_DIR}/Parse.rexx
        COMMAND $<TARGET_FILE:rxas> -o Parse Parse
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Parse.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Parse.rxbin"
)
add_custom_target(exit_parse_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Parse.rxbin)

# Test the sort exit
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_parse.rxas
        COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_parse ${CMAKE_CURRENT_SOURCE_DIR}/test_parse.rexx
        DEPENDS rxc ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/test_parse.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_parse.rexx with exit"
)
execute_process(
        COMMAND cmd /c "echo Debug breakpoint reached & pause"
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_parse.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_parse test_parse
        DEPENDS rxas ${CMAKE_CURRENT_BINARY_DIR}/test_parse.rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling test_parse.rxas"
)
add_custom_target(test_parse_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_parse.rxbin)
add_dependencies(test_parse_bin compiler_exit_bin)

add_test(NAME test_parse COMMAND rxvm test_parse.rxbin ${LIBRPATH} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
