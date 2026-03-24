cmake_minimum_required(VERSION 3.24)

# Build execio.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Execio.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Execio ${CMAKE_CURRENT_SOURCE_DIR}/Execio.rexx
        COMMAND $<TARGET_FILE:rxas> -o Execio Execio
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Execio.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Execio.rxbin"
)
add_custom_target(exit_execio_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Execio.rxbin)

# Test the sort exit
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_execio.rxas
        COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_execio ${CMAKE_CURRENT_SOURCE_DIR}/test_execio.rexx
        DEPENDS rxc ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/test_execio.rexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_execio.rexx with exit"
)
execute_process(
        COMMAND cmd /c "echo Debug breakpoint reached & pause"
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_execio.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_execio test_execio
        DEPENDS rxas ${CMAKE_CURRENT_BINARY_DIR}/test_execio.rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling test_execio.rxas"
)
add_custom_target(test_execio_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_execio.rxbin)
add_dependencies(test_execio_bin compiler_exit_bin)

add_test(NAME test_execio COMMAND rxvm test_execio.rxbin ${LIBRPATH} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
