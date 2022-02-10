cmake_minimum_required(VERSION 3.00)

enable_testing()
find_program(CTEST ctest)

set(REXXBINS
  tlinesz.rexx
  ts_abbrev.rexx
  ts_c2x.rexx
  ts_center.rexx
  ts_changestr.rexx
  ts_compare.rexx
  ts_countstr.rexx
  ts_d2c.rexx
  ts_d2x.rexx
  ts_date.rexx
  ts_date2.rexx
  ts_delstr.rexx
  ts_delword.rexx
  ts_format.rexx
  ts_insert.rexx
  ts_left.rexx
  ts_length.rexx
  ts_lower.rexx
  ts_min_max.rexx
  ts_overlay.rexx
  ts_pos.rexx
  ts_reverse.rexx
  ts_right.rexx
  ts_sign.rexx
  ts_space.rexx
  ts_strip.rexx
  ts_test.rexx
  ts_time.rexx
  ts_translate.rexx
  ts_trunc.rexx
  ts_upper.rexx
  ts_upper_lower.rexx
  ts_verify.rexx
  ts_wordlength.rexx
  ts_wordpos.rexx
  ts_wrdix.rexx
  ts_x2b.rexx
  ts_x2c.rexx
  ts_x2d.rexx
  tscopies.rexx
  tslastpos.rexx
  tsubstr.rexx
  tsword.rexx
        )

set(LIBRPATH
    ../rexx/substr
    ../rexx/length
    ../rexx/raise
    ../rexx/linesize
    ../rexx/abbrev
    ../rexx/x2d
    ../rexx/x2c
    ../rexx/x2b
    ../rexx/d2b
    ../rexx/d2c    
    ../rexx/d2x
    ../rexx/c2x    
    ../rexx/center
    ../rexx/changestr
    ../rexx/delstr
    ../rxas/right
    ../rxas/left    
    ../rxas/copies
    ../rxas/_elapsed    
    ../rxas/pos
    ../rxas/lastpos
    ../rxas/reverse
    ../rexx/centre
    ../rexx/compare
    ../rexx/countstr
    ../rexx/insert
    ../rexx/lower
    ../rexx/min
    ../rexx/max    
    ../rexx/date
    ../rexx/time
    ../rexx/_datei
    ../rexx/_dateo
    ../rexx/_jdn
    ../rexx/_itrunc
    ../rexx/_ftrunc    
    ../rexx/delword
    ../rexx/wordlength
    ../rexx/wordpos    
    ../rexx/format
    ../rexx/overlay
    ../rexx/sign
    ../rexx/space
    ../rexx/strip    
    ../rexx/translate    
    ../rexx/trunc
    ../rexx/upper    
    ../rexx/verify
    ../rxas/word
    ../rxas/words    
    ../rxas/wordindex

    )

add_custom_target(tests_rexx ALL
        DEPENDS ${REXXBINS})

foreach(_binfile ${REXXBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_command(
          #COMMAND ${CMAKE_COMMAND} -E chdir  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/compiler/rxc ${_basename}
          COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -l ${CMAKE_CURRENT_SOURCE_DIR} ${_basename}
          COMMAND ${CMAKE_COMMAND} -E copy   ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxas ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.rxas
          COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxas

          COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas ${_basename}
          COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.rxas

          DEPENDS ${_basename}.rexx rxas rxc rxvm rxbvm
          OUTPUT ${_binfile}
  )
  add_test(NAME ${_basename} COMMAND rxvm ${_basename} ${LIBRPATH}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

endforeach()
