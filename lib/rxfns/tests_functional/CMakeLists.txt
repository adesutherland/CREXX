cmake_minimum_required(VERSION 3.00)

project(test_bifs)

enable_testing()
find_program(CTEST ctest)

set(REXXBINS
        tlinesz.rxbin
        ts_abbrev.rxbin
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
        ts_reverse.rxbin
        ts_right.rxbin
        ts_sign.rxbin
        ts_space.rxbin
        ts_strip.rxbin
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
        ts_reradix.rxbin
        ts_sequence.rxbin
        ts_find.rxbin
        ts_index.rxbin
)

set(LIBRPATH ../library)

add_custom_target(testbifs ALL DEPENDS ${REXXBINS})

foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    add_custom_command(
            COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
            COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas ${_basename}
            DEPENDS rxas rxc ${_basename}.rexx
            OUTPUT ${_binfile}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_test(NAME ${_basename}-rxvm COMMAND rxvm ${_basename} ${LIBRPATH}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    add_test(NAME ${_basename}-rxbvm COMMAND rxbvm ${_basename} ${LIBRPATH}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

endforeach()
