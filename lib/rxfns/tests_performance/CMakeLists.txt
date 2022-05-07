cmake_minimum_required(VERSION 3.00)

project(test_bifs)

enable_testing()
find_program(CTEST ctest)

#FILE(GLOB REXXBINS "${CMAKE_CURRENT_SOURCE_DIR}/*.rexx")

set(REXXBINS
	we.rxbin
	test1.rxbin
	crexxbrexx.rxbin
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
    ../rexx/datatype
    )

add_custom_target(testperfbifs ALL
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
  add_test(NAME ${_basename}-rxvm COMMAND rxvm ${_basename} ${LIBRPATH}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_test(NAME ${_basename}-rxbvm COMMAND rxbvm ${_basename} ${LIBRPATH}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
