cmake_minimum_required(VERSION 3.24)

project(test_bifs)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(REXXBINS
        tlinesz.rxbin
        ts_abbrev.rxbin
	ts_abs.rxbin
	ts_address.rxbin
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
        ts_delstr.rxbin
        ts_delword.rxbin
        ts_fileio_chars.rxbin
        ts_format.rxbin
        ts_insert.rxbin
        ts_left.rxbin
        ts_length.rxbin
        ts_lower.rxbin
        ts_min_max.rxbin
        ts_overlay.rxbin
        ts_pos.rxbin
        ts_qpos.rxbin
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
#        ts_parse2.rxbin
        ts_datatype.rxbin
        ts_numeric.rxbin
        ts_splice.rxbin
        ts_arraydelete.rxbin
        ts_objectarray.rxbin
        ts_stem.rxbin
        ts_rxjson.rxbin
        ts_rxsocket.rxbin
        ts_socket_tls_live.rxbin
        ts_rxhttp.rxbin
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_target(testbifs ALL)

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
    #             COMMAND rxpp -I ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxpp -O ${_rexx_file} -M ${CMAKE_SOURCE_DIR}/lib/plugins/precomp/maclib.rexx -VERBOSE0
    #             DEPENDS rxpp ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxpp ${CMAKE_SOURCE_DIR}/lib/plugins/precomp/maclib.rexx
    #             COMMENT "Pre-compiling ${_basename}.rxpp"
    #     )
    # endif()

    add_custom_target(${_basename})
    add_dependencies(testbifs ${_basename})
    set(_rxc_flags -x)
    if("${_basename}" STREQUAL "ts_address")
        set(_rxc_flags)
    endif()
    set(_pass_test_name_argument)
    if("${_basename}" STREQUAL "ts_fileio_chars")
        set(_pass_test_name_argument PASS_TEST_NAME_ARGUMENT)
    endif()
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS ${_rxc_flags}
            DEPENDS library ${_rexx_dep} ${_prev_target}
            RUNTIME_ARGS ${LIBRPATH}
            ${_pass_test_name_argument}
    )
    set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()

set_tests_properties(ts_rxjson_noopt ts_rxjson_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxjson helpers")
set_tests_properties(ts_rxsocket_noopt ts_rxsocket_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxsocket loopback")
set_tests_properties(ts_socket_tls_live_noopt ts_socket_tls_live_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxsocket TLS live smoke")
set_tests_properties(ts_rxhttp_noopt ts_rxhttp_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxhttp helpers")
set_tests_properties(ts_trace_noopt ts_trace_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: trace helpers")
set_tests_properties(ts_version_noopt ts_version_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: version helper")
set_tests_properties(ts_binary_noopt ts_binary_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: binary helpers")
set_tests_properties(ts_fileio_chars_noopt ts_fileio_chars_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: file character helpers" RUN_SERIAL TRUE)
set_tests_properties(ts_arraydelete_noopt ts_arraydelete_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: array helpers")
