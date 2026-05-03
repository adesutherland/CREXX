# Build Dummy.rxbin
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Dummy.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Dummy ${CMAKE_CURRENT_SOURCE_DIR}/Dummy.rexx
    COMMAND $<TARGET_FILE:rxas> -o Dummy Dummy
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/Dummy.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building Dummy.rxbin"
)
add_custom_target(exit_dummy_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Dummy.rxbin)
# Ensure the token class bundle is built before Dummy
add_dependencies(exit_dummy_bin token_bin)

crexx_add_compiler_exit_runtime_test(
        test_dummy
        test_dummy.rexx
        PASS_REGULAR_EXPRESSION "after-exit 99"
)
