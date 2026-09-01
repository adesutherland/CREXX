cmake_minimum_required(VERSION 3.24)

# Keep the canonical exit bundle in /bin on the normal optimised path.
# Opt/no-opt comparisons for exit consumers are emitted as suffixed test-only
# artifacts in each exit subdirectory.
set(CREXX_EXIT_IMPORT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/imports")
set(CREXX_EXIT_IMPORT_PATH "${CREXX_EXIT_IMPORT_ROOT}")

if(BUILD_TESTING)
    add_custom_target(qa-prep-compiler-exits)
endif()

function(crexx_add_compiler_exit_runtime_test TARGET_NAME SOURCE_FILE)
    crexx_schedule_directory_qa_tier_finalization()

    set(options)
    set(oneValueArgs PASS_REGULAR_EXPRESSION)
    set(multiValueArgs RUN_PREFIX RUN_SUFFIX EXTRA_DEPENDS)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_pass_regex "SUCCESS")
    if(CREXX_PASS_REGULAR_EXPRESSION)
        set(_pass_regex "${CREXX_PASS_REGULAR_EXPRESSION}")
    endif()

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
                DEPENDS rxc rxas ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE} ${CREXX_EXTRA_DEPENDS}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Compiling ${SOURCE_FILE} with exit (${_mode})"
        )
        list(APPEND _outputs ${_output})

        add_test(
                NAME ${TARGET_NAME}_${_mode}
                COMMAND ${CREXX_RXVM_EXECUTABLE} ${CREXX_RUN_PREFIX} ${_output} ${CREXX_RUN_SUFFIX}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        set_tests_properties(${TARGET_NAME}_${_mode} PROPERTIES
                PASS_REGULAR_EXPRESSION "${_pass_regex}"
                FAIL_REGULAR_EXPRESSION "^ERROR ;ERROR reading module file;#FUNCTION_NOT_FOUND;#UNKNOWN_TYPE;#UNKNOWN_VARIABLE;#BAD_CONVERSION")
    endforeach()

    add_custom_target(${TARGET_NAME}_bin ALL DEPENDS ${_outputs})
    add_dependencies(${TARGET_NAME}_bin compiler_exit_bin rxvm)

    if(BUILD_TESTING)
        add_dependencies(qa-prep-compiler-exits ${TARGET_NAME}_bin)
    endif()

endfunction()

# Build token.rxbin without admitting the sibling test-support source.
set(_token_source_root "${CMAKE_CURRENT_BINARY_DIR}/sources/token")
set(_token_source "${_token_source_root}/token.crexx")
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin
    BYPRODUCTS
        ${_token_source}
        ${CMAKE_CURRENT_BINARY_DIR}/token.rxas
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_token_source_root}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/token.crexx"
        "${_token_source}"
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1
        $<TARGET_FILE:rxc> -x --no-exe-import -o token "${_token_source}"
    COMMAND $<TARGET_FILE:rxas> -o token token
    DEPENDS rxc rxas ${CMAKE_CURRENT_SOURCE_DIR}/token.crexx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building token.rxbin"
    VERBATIM
)
add_custom_target(token_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin)

crexx_add_import_root(compiler_exit_imports
    DIRECTORY "${CREXX_EXIT_IMPORT_ROOT}"
    OUTPUT_VARIABLE CREXX_EXIT_IMPORT_FILES
    FILES
        "${CMAKE_BINARY_DIR}/bin/library.rxbin"
        "${CMAKE_CURRENT_BINARY_DIR}/token.rxbin"
    DEPENDS
        library
        token_bin
    COMMENT "Staging deterministic compiler-exit imports ...")

function(crexx_add_compiler_exit_component TARGET_NAME MODULE_NAME)
    set(_source_root "${CMAKE_CURRENT_BINARY_DIR}/source")
    set(_source "${_source_root}/${MODULE_NAME}.crexx")
    set(_output "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.rxbin")

    add_custom_command(
        OUTPUT "${_output}"
        BYPRODUCTS
            "${_source}"
            "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_source_root}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.crexx"
            "${_source}"
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1
            $<TARGET_FILE:rxc>
            -x
            --no-exe-import
            -i "${CREXX_EXIT_IMPORT_ROOT}"
            -o "${MODULE_NAME}"
            "${_source}"
        COMMAND $<TARGET_FILE:rxas> -o "${MODULE_NAME}" "${MODULE_NAME}"
        DEPENDS
            rxc
            rxas
            compiler_exit_imports
            ${CREXX_EXIT_IMPORT_FILES}
            "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building ${MODULE_NAME}.rxbin"
        VERBATIM)
    add_custom_target(${TARGET_NAME} DEPENDS "${_output}")
endfunction()

set(_exit_test_support_source_root
    "${CMAKE_CURRENT_BINARY_DIR}/sources/ExitTestSupport")
set(_exit_test_support_source
    "${_exit_test_support_source_root}/ExitTestSupport.crexx")
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin
    BYPRODUCTS
        ${_exit_test_support_source}
        ${CMAKE_CURRENT_BINARY_DIR}/exittestsupport.rxas
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${_exit_test_support_source_root}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/ExitTestSupport.crexx"
        "${_exit_test_support_source}"
    COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1
        $<TARGET_FILE:rxc>
        -x
        --no-exe-import
        -i "${CREXX_EXIT_IMPORT_PATH}"
        -o exittestsupport
        "${_exit_test_support_source}"
    COMMAND $<TARGET_FILE:rxas> -o ${CMAKE_BINARY_DIR}/bin/exittestsupport exittestsupport
    DEPENDS
        rxc
        rxas
        compiler_exit_imports
        ${CREXX_EXIT_IMPORT_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/ExitTestSupport.crexx
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building exittestsupport.rxbin"
    VERBATIM
)
add_custom_target(exit_test_support_bin DEPENDS ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin)

add_subdirectory(dump)
add_subdirectory(query)
add_subdirectory(sort)
add_subdirectory(sortx)
add_subdirectory(address)
add_subdirectory(execio)
add_subdirectory(add)
add_subdirectory(fsay)
add_subdirectory(msay)
add_subdirectory(parse)
add_subdirectory(signal)
add_subdirectory(trace)
add_subdirectory(rexxscript)
add_subdirectory(substr)
add_subdirectory(dummy)
add_subdirectory(rxqueue)

# Build the bundle by concatenating all exit components
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
    COMMAND ${CMAKE_COMMAND} -E cat token.rxbin dump/Dump.rxbin query/Query.rxbin sort/Sort.rxbin sortx/Sortx.rxbin address/Address.rxbin parse/Parse.rxbin signal/Signal.rxbin trace/Trace.rxbin rexxscript/RexxScript.rxbin execio/Execio.rxbin add/Add.rxbin fsay/Fsay.rxbin msay/Msay.rxbin substr/Substr.rxbin dummy/Dummy.rxbin rxqueue/Queue.rxbin > ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
    DEPENDS rxc rxas
            ${CMAKE_CURRENT_BINARY_DIR}/token.rxbin 
            ${CMAKE_CURRENT_BINARY_DIR}/dump/Dump.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/query/Query.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/sort/Sort.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/sortx/Sortx.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/address/Address.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/execio/Execio.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/add/Add.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/fsay/Fsay.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/msay/Msay.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/parse/Parse.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/signal/Signal.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/trace/Trace.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/rexxscript/RexxScript.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/substr/Substr.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/dummy/Dummy.rxbin
            ${CMAKE_CURRENT_BINARY_DIR}/rxqueue/Queue.rxbin
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building rxcexits.rxbin (bundled via concatenation)"
)
add_custom_target(compiler_exit_bin ALL DEPENDS ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin)
add_dependencies(compiler_exit_bin token_bin exit_dump_bin exit_query_bin exit_sort_bin exit_sortx_bin exit_address_bin exit_parse_bin exit_signal_bin exit_trace_bin exit_rexxscript_bin exit_execio_bin exit_add_bin exit_fsay_bin exit_msay_bin exit_substr_bin exit_dummy_bin exit_rxqueue_bin)

crexx_finalize_directory_qa_tiers()
