cmake_minimum_required(VERSION 3.24)

# Build add.rxbin
# Needs token.rxbin for class metadata.
set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Add.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Add ${CMAKE_CURRENT_SOURCE_DIR}/Add.rexx
        COMMAND $<TARGET_FILE:rxas> -o Add Add
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Add.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Add.rxbin"
)
add_custom_target(exit_add_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Add.rxbin)

# Test the sort exit
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_add.rxas
        COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o test_add ${CMAKE_CURRENT_SOURCE_DIR}/test_add.rexx
        DEPENDS rxc ${CMAKE_CURRENT_SOURCE_DIR}/test_add.rexx compiler_exit_bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_add.rexx with exit"
)
execute_process(
        COMMAND cmd /c "echo Debug breakpoint reached & pause"
)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_add.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_add test_add
        DEPENDS rxas ${CMAKE_CURRENT_BINARY_DIR}/test_add.rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling test_add.rxas"
)
add_custom_target(test_add_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_add.rxbin)

add_test(NAME test_add COMMAND rxvm test_add.rxbin ${LIBRPATH} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
