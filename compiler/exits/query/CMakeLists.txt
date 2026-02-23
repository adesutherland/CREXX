cmake_minimum_required(VERSION 3.24)

# Build Query.rxbin
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Query.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Query ${CMAKE_CURRENT_SOURCE_DIR}/Query.rexx
    COMMAND $<TARGET_FILE:rxas> -o Query.rxbin Query.rxas
    DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Query.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Query.rxbin"
)
add_custom_target(exit_query_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Query.rxbin)

# Test the query exit
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_query.rxas
    COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_query ${CMAKE_CURRENT_SOURCE_DIR}/test_query.rexx
    DEPENDS rxc ${CMAKE_CURRENT_SOURCE_DIR}/test_query.rexx compiler_exit_bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Compiling test_query.rexx with exit"
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_query.rxbin
    COMMAND $<TARGET_FILE:rxas> -o test_query test_query
    DEPENDS rxas ${CMAKE_CURRENT_BINARY_DIR}/test_query.rxas
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Assembling test_query.rxas"
)

add_custom_target(test_query_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_query.rxbin)

add_test(NAME test_query COMMAND $<TARGET_FILE:rxvm> ${CMAKE_BINARY_DIR}/bin/library.rxbin test_query.rxbin WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
