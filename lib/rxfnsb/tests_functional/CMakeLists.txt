cmake_minimum_required(VERSION 3.24)

project(test_bifs)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(REXXBINS
        tlinesz.rxbin
        ts_abbrev.rxbin
	    ts_address.rxbin
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
        ts_translate.rxbin
        ts_trunc.rxbin
        ts_upper.rxbin
        ts_upper_lower.rxbin
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
        ts_parse2.rxbin
        ts_datatype.rxbin
        ts_numeric.rxbin
        ts_splice.rxbin
        ts_arraydelete.rxbin
        ts_stem.rxbin
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_target(testbifs ALL)

unset(_prev_target)
foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rexx")
    set(_rexx_dep "${_basename}.rexx")

    if("${_basename}" STREQUAL "ts_parse2")
        # Technical Debt: ts_parse2 is pre-compiled from .rxpp
        set(_rexx_file "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.rexx")
        set(_rexx_dep "${_rexx_file}")
        add_custom_command(
                OUTPUT ${_rexx_file}
                COMMAND rxpp -I ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxpp -O ${_rexx_file} -M ${CMAKE_SOURCE_DIR}/lib/plugins/precomp/maclib.rexx -VERBOSE0
                DEPENDS rxpp ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxpp ${CMAKE_SOURCE_DIR}/lib/plugins/precomp/maclib.rexx
                COMMENT "Pre-compiling ${_basename}.rxpp"
        )
    endif()

    add_custom_target(${_basename})
    add_dependencies(testbifs ${_basename})
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS -x
            DEPENDS library ${_rexx_dep} ${_prev_target}
            RUNTIME_ARGS ${LIBRPATH}
    )
    set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()
