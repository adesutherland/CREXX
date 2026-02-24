# Build Dummy.rxbin
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Dummy.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -i "\"${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..\"" -o Dummy ${CMAKE_CURRENT_SOURCE_DIR}/Dummy.rexx
    COMMAND $<TARGET_FILE:rxas> -o Dummy Dummy
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/Dummy.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Dummy.rxbin"
)
add_custom_target(exit_dummy_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Dummy.rxbin)

# Test: compile a small program using the dummy exit
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_dummy.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i $<TARGET_FILE_DIR:rxc> -o test_dummy ${CMAKE_CURRENT_SOURCE_DIR}/test_dummy.rexx
    COMMAND $<TARGET_FILE:rxas> -o test_dummy.rxbin test_dummy
    DEPENDS rxc rxas compiler_exit_bin ${CMAKE_CURRENT_SOURCE_DIR}/test_dummy.rexx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Compiling test_dummy.rexx with exit"
)
add_custom_target(test_dummy_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_dummy.rxbin)

add_test(NAME test_dummy
         COMMAND $<TARGET_FILE:rxvm> ${CMAKE_CURRENT_BINARY_DIR}/test_dummy.rxbin
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
