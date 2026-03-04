cmake_minimum_required(VERSION 3.24)

# Build token.rxbin
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -n -o token ${CMAKE_CURRENT_SOURCE_DIR}/token.rexx
    COMMAND $<TARGET_FILE:rxas> -o token token
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/token.rexx
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/token.rxas
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building token.rxbin"
)
add_custom_target(token_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin)

add_subdirectory(dump)
add_subdirectory(query)
add_subdirectory(sort)
add_subdirectory(sortx)
add_subdirectory(pprint)
add_subdirectory(execio)
add_subdirectory(dummy)

# Build the bundle by concatenating all exit components
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
    COMMAND ${CMAKE_COMMAND} -E cat token.rxbin dump/Dump.rxbin query/Query.rxbin sort/Sort.rxbin sortx/Sortx.rxbin pprint/Pprint.rxbin execio/Execio.rxbin dummy/Dummy.rxbin > ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
    DEPENDS rxc rxas
            ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin 
            ${CMAKE_CURRENT_BINARY_DIR}/dump/Dump.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/query/Query.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/sort/Sort.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/sortx/Sortx.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/execio/Execio.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/pprint/Pprint.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/dummy/Dummy.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building rxcexits.rxbin (bundled via concatenation)"
)
add_custom_target(compiler_exit_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin)
add_dependencies(compiler_exit_bin token_bin exit_dump_bin exit_query_bin exit_dummy_bin)
