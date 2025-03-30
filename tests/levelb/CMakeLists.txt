cmake_minimum_required(VERSION 3.24)
project(levelb_tests C)

set(TESTS
        concatenation
)

foreach (BIF ${TESTS})
  set(REXXBINS ${REXXBINS} ${BIF}.rxbin)
  set(REXXSRC ${REXXSRC} ${BIF}.rexx)
endforeach ()

unset(_prev_target)
foreach(_binfile ${REXXBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_command(
          COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
          COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas ${_basename}
          DEPENDS rxas rxc ${REXXSRC} ${_prev_target}
          OUTPUT ${_binfile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "cREXX compile and assemble ${_basename} ..."
  )

  # "Virtual target" for the .rxbin
  add_custom_target(${_basename}_artifact ALL DEPENDS ${_binfile})

  # Add the tests (rxvm and rxbvm)
  add_test(NAME test-${_basename}-rxvm COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_test(NAME test-${_basename}-rxbvm COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxbvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

  set(_prev_target ${_binfile})
endforeach()
