cmake_minimum_required(VERSION 3.00)

project(test_bifs)

enable_testing()
find_program(CTEST ctest)

#FILE(GLOB REXXBINS "${CMAKE_CURRENT_SOURCE_DIR}/*.rexx")

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
<<<<<<< HEAD
=======
  ts_find.rxbin
  ts_index.rxbin
>>>>>>> edcfea77434034edd26d8b5e4cfb966bf4096da5
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
    ../rexx/c2d			
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
    ../rexx/reradix
    ../rexx/sequence
<<<<<<< HEAD
=======
    ../rexx/find
    ../rexx/index
>>>>>>> edcfea77434034edd26d8b5e4cfb966bf4096da5
    )

add_custom_target(testbifs ALL
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
<<<<<<< HEAD
  add_test(NAME ${_basename} COMMAND rxvm ${_basename} ${LIBRPATH}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
=======
  add_test(NAME ${_basename}-rxvm COMMAND rxvm ${_basename} ${LIBRPATH}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
		add_test(NAME ${_basename}-rxbvm COMMAND rxbvm ${_basename} ${LIBRPATH}
	        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
>>>>>>> edcfea77434034edd26d8b5e4cfb966bf4096da5

endforeach()
