cmake_minimum_required(VERSION 3.24)

project(test_bifs)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

set(REXXBINS
        tlinesz.rxbin
        ts_abbrev.rxbin
	ts_abs.rxbin
	        ts_typed_numeric.rxbin
	        ts_address_protocol.rxbin
	ts_address.rxbin
        ts_address_crexx.rxbin
        ts_address_capture.rxbin
        ts_address_argv.rxbin
        ts_address_array_lifecycle.rxbin
        ts_binary.rxbin
        ts_c2x.rxbin
        ts_center.rxbin
        ts_changestr.rxbin
        ts_compare.rxbin
        ts_countstr.rxbin
        ts_d2c.rxbin
        ts_d2x.rxbin
        ts_date.rxbin
        ts_date2.rxbin
        ts_date_core.rxbin
        ts_delstr.rxbin
        ts_delword.rxbin
        ts_fileio_chars.rxbin
        ts_format.rxbin
        ts_fnv.rxbin
        ts_fsayfmt.rxbin
        ts_insert.rxbin
        ts_left.rxbin
        ts_length.rxbin
        ts_lower.rxbin
        ts_min_max.rxbin
        ts_overlay.rxbin
        ts_pos.rxbin
        ts_qpos.rxbin
        ts_qlibrary.rxbin
        ts_random.rxbin
        ts_reverse.rxbin
        ts_regex.rxbin
        ts_right.rxbin
        ts_sign.rxbin
        ts_space.rxbin
        ts_strip.rxbin
	    ts_symbol.rxbin
        ts_test.rxbin
        ts_time.rxbin
        ts_trace.rxbin
        ts_translate.rxbin
        ts_trunc.rxbin
        ts_upper.rxbin
        ts_upper_lower.rxbin
        ts_version.rxbin
        ts_sourceinfo.rxbin
        ts_value.rxbin
        ts_verify.rxbin
        ts_wordlength.rxbin
        ts_wordpos.rxbin
        ts_wrdix.rxbin
        ts_x2b.rxbin
        ts_x2c.rxbin
        ts_x2d.rxbin
        tscopies.rxbin
        tslastpos.rxbin
        tsubstr.rxbin
        tsword.rxbin
	    ts_words.rxbin
        ts_reradix.rxbin
        ts_sequence.rxbin
        ts_find.rxbin
        ts_index.rxbin
        ts_parse.rxbin
        ts_parseexec.rxbin
#        ts_parse2.rxbin
        ts_datatype.rxbin
        ts_numeric.rxbin
        ts_splice.rxbin
        ts_arraydelete.rxbin
        ts_arraydelete_focused.rxbin
        ts_arrayappend.rxbin
        ts_arraycontains.rxbin
        ts_arraycopy.rxbin
        ts_arraydrop.rxbin
        ts_arrayfind.rxbin
        ts_arrayget.rxbin
        ts_arrayhi.rxbin
        ts_arrayindexof.rxbin
        ts_arrayinsert.rxbin
        ts_arrayjoin.rxbin
        ts_arraymove.rxbin
        ts_arraypop.rxbin
        ts_arrayprepend.rxbin
        ts_arrayreverse.rxbin
        ts_arrayset.rxbin
        ts_arrayshift.rxbin
        ts_arraysort.rxbin
        ts_b2d.rxbin
        ts_b2x.rxbin
        ts_c2d.rxbin
        ts_centre.rxbin
        ts_d2b.rxbin
        ts_ftrunc.rxbin
        ts_getenv.rxbin
        ts_itrunc.rxbin
        ts_loadmodule.rxbin
        ts_max.rxbin
        ts_min.rxbin
        ts_objectarrayappend.rxbin
        ts_objectarraydelete.rxbin
        ts_objectarraydrop.rxbin
        ts_objectarrayinsert.rxbin
        ts_objectarraymove.rxbin
        ts_objectarrayprepend.rxbin
        ts_qwordlength.rxbin
        ts_raise.rxbin
        ts_rxsystem.rxbin
        ts_rxsystem_exit.rxbin
        ts_rxqueue.rxbin
        ts_signal.rxbin
        ts_substro.rxbin
        ts_subword.rxbin
        ts_xrange.rxbin
        ts_objectarray.rxbin
        ts_stem.rxbin
        ts_rxjson.rxbin
        ts_rxjson_document.rxbin
        ts_rxjson_numeric_projection.rxbin
        ts_rxjson_noisy_contract.rxbin
        ts_rxsocket.rxbin
        ts_socket_tls_live.rxbin
        ts_rxhttp.rxbin
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_executable(address_argv_fixture address_argv_fixture.c)
if(MINGW)
    target_link_options(address_argv_fixture PRIVATE -municode)
endif()

add_custom_target(testbifs ALL)

set(_loadmodule_provider "${CMAKE_CURRENT_BINARY_DIR}/ts_loadmodule_provider.rxbin")
add_custom_command(
        OUTPUT ${_loadmodule_provider}
        COMMAND $<TARGET_FILE:rxc> -x -o ts_loadmodule_provider
                ${CMAKE_CURRENT_SOURCE_DIR}/ts_loadmodule_provider.crexx
        COMMAND $<TARGET_FILE:rxas> -o ts_loadmodule_provider ts_loadmodule_provider
        DEPENDS rxc rxas compiler_exit_bin
                ${CMAKE_CURRENT_SOURCE_DIR}/ts_loadmodule_provider.crexx
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building the LOADMODULE focused provider"
        VERBATIM
)
add_custom_target(ts_loadmodule_provider_artifact DEPENDS ${_loadmodule_provider})
add_dependencies(testbifs ts_loadmodule_provider_artifact)

unset(_prev_target)
foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
    set(_rexx_dep "${_basename}.crexx")

    # if("${_basename}" STREQUAL "ts_parse2")
    #     # Technical Debt: ts_parse2 is pre-compiled from .rxpp
    #     set(_rexx_file "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.crexx")
    #     set(_rexx_dep "${_rexx_file}")
    #     add_custom_command(
    #             OUTPUT ${_rexx_file}
    #             COMMAND rxpp -I ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxpp -O ${_rexx_file} -M ${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx -VERBOSE0
    #             DEPENDS rxpp ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxpp ${CMAKE_SOURCE_DIR}/preprocessor/maclib.rexx
    #             COMMENT "Pre-compiling ${_basename}.rxpp"
    #     )
    # endif()

    add_custom_target(${_basename})
    add_dependencies(testbifs ${_basename})
    set(_rxc_flags -x)
    if("${_basename}" STREQUAL "ts_address" OR
            "${_basename}" STREQUAL "ts_address_crexx" OR
            "${_basename}" STREQUAL "ts_address_capture" OR
            "${_basename}" STREQUAL "ts_address_argv" OR
            "${_basename}" STREQUAL "ts_address_array_lifecycle")
        set(_rxc_flags)
    endif()
    set(_pass_test_name_argument)
    if("${_basename}" STREQUAL "ts_fileio_chars" OR
            "${_basename}" STREQUAL "ts_rxsystem" OR
            "${_basename}" STREQUAL "ts_signal")
        set(_pass_test_name_argument PASS_TEST_NAME_ARGUMENT)
    endif()
    set(_expected_exit_code)
    set(_expected_failure_description)
    if("${_basename}" STREQUAL "ts_rxsystem_exit")
        set(_expected_exit_code 7)
        set(_expected_failure_description "Level B _exit must preserve the requested process status")
    endif()
    set(_extra_depends)
    set(_runtime_args ${LIBRPATH})
    if("${_basename}" STREQUAL "ts_loadmodule")
        list(APPEND _extra_depends ts_loadmodule_provider_artifact ${_loadmodule_provider})
    endif()
    if("${_basename}" STREQUAL "ts_address_argv")
        list(APPEND _extra_depends address_argv_fixture)
        list(APPEND _runtime_args -a $<TARGET_FILE:address_argv_fixture>)
    endif()
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS ${_rxc_flags}
            DEPENDS library ${_rexx_dep} ${_prev_target} ${_extra_depends}
            RUNTIME_ARGS ${_runtime_args}
            EXPECTED_EXIT_CODE "${_expected_exit_code}"
            EXPECTED_FAILURE_DESCRIPTION "${_expected_failure_description}"
            ${_pass_test_name_argument}
    )
    set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()

# CRI-09 is an integration-boundary contract, so retain the same optimized and
# non-optimized images under the independent bytecode VM as well as rxvm.
_crexx_register_runtime_test(
        NAME ts_rxjson_document_rxbvm_noopt
        RUNNER rxbvm
        PROGRAM ts_rxjson_document_noopt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)
_crexx_register_runtime_test(
        NAME ts_rxjson_document_rxbvm_opt
        RUNNER rxbvm
        PROGRAM ts_rxjson_document_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)
_crexx_register_runtime_test(
        NAME ts_rxjson_noisy_contract_rxbvm_noopt
        RUNNER rxbvm
        PROGRAM ts_rxjson_noisy_contract_noopt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)
_crexx_register_runtime_test(
        NAME ts_rxjson_noisy_contract_rxbvm_opt
        RUNNER rxbvm
        PROGRAM ts_rxjson_noisy_contract_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)
_crexx_register_runtime_test(
        NAME ts_rxjson_numeric_projection_rxbvm_noopt
        RUNNER rxbvm
        PROGRAM ts_rxjson_numeric_projection_noopt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)
_crexx_register_runtime_test(
        NAME ts_rxjson_numeric_projection_rxbvm_opt
        RUNNER rxbvm
        PROGRAM ts_rxjson_numeric_projection_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)

# CRI-10 locks the existing ADDRESS capture contract on both VMs and both
# compiler modes. It uses only deterministic CREXX-environment commands.
_crexx_register_runtime_test(
        NAME ts_address_capture_rxbvm_noopt
        RUNNER rxbvm
        PROGRAM ts_address_capture_noopt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)
_crexx_register_runtime_test(
        NAME ts_address_capture_rxbvm_opt
        RUNNER rxbvm
        PROGRAM ts_address_capture_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)

# CRI-11 proves that CREXX run expands a bound string array directly to argv;
# COMMAND/SYSTEM shell string parsing is deliberately outside this route.
_crexx_register_runtime_test(
        NAME ts_address_argv_rxbvm_noopt
        RUNNER rxbvm
        PROGRAM ts_address_argv_noopt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} -a $<TARGET_FILE:address_argv_fixture>
)
_crexx_register_runtime_test(
        NAME ts_address_argv_rxbvm_opt
        RUNNER rxbvm
        PROGRAM ts_address_argv_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH} -a $<TARGET_FILE:address_argv_fixture>
)

# CRI-12 locks array redirection's append lifecycle, including explicit
# arraydrop before replacement-style reuse, empty output and failure output.
_crexx_register_runtime_test(
        NAME ts_address_array_lifecycle_rxbvm_noopt
        RUNNER rxbvm
        PROGRAM ts_address_array_lifecycle_noopt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)
_crexx_register_runtime_test(
        NAME ts_address_array_lifecycle_rxbvm_opt
        RUNNER rxbvm
        PROGRAM ts_address_array_lifecycle_opt
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_ARGS ${LIBRPATH}
)

# Run each caught-signal sequence in one VM so the harnesses verify that call
# unwind preserves exposed arrays and object receivers across repeated errors.
add_custom_target(ts_arraysort_errors)
add_dependencies(testbifs ts_arraysort_errors)
crexx_add_rexx_opt_matrix(
        NAME ts_arraysort_errors
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ts_arraysort_errors.crexx
        TARGET_GROUP ts_arraysort_errors
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        RXC_FLAGS -x
        DEPENDS library ${CMAKE_CURRENT_SOURCE_DIR}/ts_arraysort_errors.crexx
        RUNTIME_ARGS ${LIBRPATH}
)

add_custom_target(ts_stem_errors)
add_dependencies(testbifs ts_stem_errors)
crexx_add_rexx_opt_matrix(
        NAME ts_stem_errors
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ts_stem_errors.crexx
        TARGET_GROUP ts_stem_errors
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        RXC_FLAGS -x
        DEPENDS library ${CMAKE_CURRENT_SOURCE_DIR}/ts_stem_errors.crexx
        RUNTIME_ARGS ${LIBRPATH}
)

set_tests_properties(ts_rxjson_noopt ts_rxjson_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxjson helpers")
set_tests_properties(ts_rxjson_noisy_contract_noopt ts_rxjson_noisy_contract_opt
        ts_rxjson_noisy_contract_rxbvm_noopt ts_rxjson_noisy_contract_rxbvm_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxjson noisy contract")
set_tests_properties(ts_rxjson_numeric_projection_noopt ts_rxjson_numeric_projection_opt
        ts_rxjson_numeric_projection_rxbvm_noopt ts_rxjson_numeric_projection_rxbvm_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxjson numeric projection contract")
set_tests_properties(ts_address_capture_noopt ts_address_capture_opt
        ts_address_capture_rxbvm_noopt ts_address_capture_rxbvm_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: ADDRESS capture contract")
set_tests_properties(ts_address_argv_noopt ts_address_argv_opt
        ts_address_argv_rxbvm_noopt ts_address_argv_rxbvm_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: ADDRESS argv contract")

crexx_label_smoke_tests(
        ts_address_noopt
        ts_parse_noopt
        ts_stem_noopt
        ts_rxjson_noopt
        ts_rxsocket_noopt
)
set_tests_properties(ts_rxsocket_noopt ts_rxsocket_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxsocket loopback")
set_tests_properties(ts_socket_tls_live_noopt ts_socket_tls_live_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxsocket TLS live smoke")
set_tests_properties(ts_rxhttp_noopt ts_rxhttp_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxhttp helpers")
set_tests_properties(ts_trace_noopt ts_trace_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: trace helpers")
set_tests_properties(ts_version_noopt ts_version_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: version")
set_tests_properties(ts_rxqueue_noopt ts_rxqueue_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxqueue shared state")
set_tests_properties(ts_sourceinfo_noopt ts_sourceinfo_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: sourceinfo")
set_tests_properties(ts_binary_noopt ts_binary_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: binary helpers")
set_tests_properties(ts_fileio_chars_noopt ts_fileio_chars_opt
        PROPERTIES RUN_SERIAL TRUE)
set_tests_properties(ts_arraydelete_noopt ts_arraydelete_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: array helpers")
set_tests_properties(ts_parse_noopt ts_parse_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: legacy parse helpers")
set_tests_properties(ts_parseexec_noopt ts_parseexec_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: parseExec stream executor")
set_tests_properties(ts_getenv_noopt ts_getenv_opt
        PROPERTIES ENVIRONMENT "CREXX_GETENV_SELECTOR_TEST_VALUE=alpha beta")
