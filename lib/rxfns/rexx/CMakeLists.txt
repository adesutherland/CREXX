cmake_minimum_required(VERSION 3.00)

set(REXXBINS
              _datei.rxbin
              _dateo.rxbin
              _ftrunc.rxbin
              _itrunc.rxbin
              _jdn.rxbin
              abbrev.rxbin
              c2x.rxbin
	      c2d.rxbin	
              center.rxbin
              centre.rxbin
              changestr.rxbin
              compare.rxbin
              countstr.rxbin
              d2b.rxbin
              d2c.rxbin
              d2x.rxbin
              date.rxbin
              delstr.rxbin
              delword.rxbin
              format.rxbin
              global.rxbin
              insert.rxbin
              length.rxbin
              linesize.rxbin
              lower.rxbin
              max.rxbin
              min.rxbin
              overlay.rxbin
              raise.rxbin
              sign.rxbin
              space.rxbin
              strip.rxbin
              substr.rxbin
              time.rxbin
              translate.rxbin
              trunc.rxbin
              upper.rxbin
              verify.rxbin
              wordlength.rxbin
              wordpos.rxbin
              x2b.rxbin
              x2c.rxbin
              x2d.rxbin
	      reradix.rxbin
	      sequence.rxbin
	      find.rxbin
	      index.rxbin
	      xrange.rxbin
	      random.rxbin
	      datatype.rxbin
        )

add_custom_target(lib_rexx ALL
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
          COMMENT "cREXX compile and assemble ${_basename} ..."
  )

endforeach()
