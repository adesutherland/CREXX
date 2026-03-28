cmake_minimum_required(VERSION 3.24)

# Keep the canonical exit bundle in /bin on the normal optimised path.
# Opt/no-opt comparisons for exit consumers are emitted as suffixed test-only
# artifacts in each exit subdirectory.
function(crexx_add_compiler_exit_runtime_test TARGET_NAME SOURCE_FILE)
    set(options)
    set(oneValueArgs PASS_REGULAR_EXPRESSION)
    set(multiValueArgs RUN_PREFIX RUN_SUFFIX)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_outputs)
    foreach(_mode IN ITEMS noopt opt)
        set(_base ${TARGET_NAME}_${_mode})
        set(_rxc_flags)
        set(_rxas_flags)
        if(_mode STREQUAL "noopt")
            list(APPEND _rxc_flags -n)
            list(APPEND _rxas_flags -n)
        endif()

        set(_output ${CMAKE_CURRENT_BINARY_DIR}/${_base}.rxbin)
        add_custom_command(
                OUTPUT ${_output}
                COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=rxcexits $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin ${_rxc_flags} -o ${_base} ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
                COMMAND $<TARGET_FILE:rxas> ${_rxas_flags} -o ${_base} ${_base}
                DEPENDS rxc rxas ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Compiling ${SOURCE_FILE} with exit (${_mode})"
        )
        list(APPEND _outputs ${_output})

        add_test(
                NAME ${TARGET_NAME}_${_mode}
                COMMAND $<TARGET_FILE:rxvm> ${CREXX_RUN_PREFIX} ${_output} ${CREXX_RUN_SUFFIX}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
    endforeach()

    add_custom_target(${TARGET_NAME}_bin ALL DEPENDS ${_outputs})
    add_dependencies(${TARGET_NAME}_bin compiler_exit_bin rxvm)

    if(CREXX_PASS_REGULAR_EXPRESSION)
        set_tests_properties(${TARGET_NAME}_noopt ${TARGET_NAME}_opt PROPERTIES
                PASS_REGULAR_EXPRESSION "${CREXX_PASS_REGULAR_EXPRESSION}")
    endif()
endfunction()

# Build token.rxbin
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -o token ${CMAKE_CURRENT_SOURCE_DIR}/token.rexx
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
add_subdirectory(add)
add_subdirectory(fsay)
add_subdirectory(parse)
add_subdirectory(dummy)

# Build the bundle by concatenating all exit components
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
    COMMAND ${CMAKE_COMMAND} -E cat token.rxbin dump/Dump.rxbin query/Query.rxbin sort/Sort.rxbin sortx/Sortx.rxbin pprint/Pprint.rxbin parse/Parse.rxbin execio/Execio.rxbin add/Add.rxbin fsay/Fsay.rxbin dummy/Dummy.rxbin > ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
    DEPENDS rxc rxas
            ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin 
            ${CMAKE_CURRENT_BINARY_DIR}/dump/Dump.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/query/Query.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/sort/Sort.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/sortx/Sortx.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/execio/Execio.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/add/Add.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/fsay/Fsay.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/pprint/Pprint.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/parse/Parse.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/dummy/Dummy.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building rxcexits.rxbin (bundled via concatenation)"
)
add_custom_target(compiler_exit_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin)
add_dependencies(compiler_exit_bin token_bin exit_dump_bin exit_query_bin exit_sort_bin exit_sortx_bin exit_pprint_bin exit_parse_bin exit_execio_bin exit_add_bin exit_fsay_bin exit_dummy_bin)
