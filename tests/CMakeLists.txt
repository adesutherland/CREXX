cmake_minimum_required(VERSION 3.00)

project(tests C)

set(CMAKE_C_STANDARD 90)

enable_testing()
find_program(CTEST ctest)

set(ASMTESTBINS ascommon.rxbin asutf.rxbin)
set(REXXTESTBINS basic.rxbin)

add_custom_target(tests ALL
        DEPENDS ${ASMTESTBINS} ${REXXTESTBINS})

foreach(_binfile ${ASMTESTBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
        COMMAND ${CTEST} -R ${_basename} --output-on-failure
        DEPENDS ${_basename}.rxas rxas rxvm rxbvm library
        OUTPUT ${_binfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX Assemble Test ${_basename} ..."
  )
  add_test(NAME ${_basename} COMMAND rxvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_test(NAME ${_basename}-bytecode COMMAND rxbvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

foreach(_binfile ${REXXTESTBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_command(
          COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
          COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas ${_basename}
          COMMAND ${CTEST} -R ${_basename} --output-on-failure
          DEPENDS ${_basename}.rexx rxas rxc rxvm rxbvm library
          OUTPUT ${_binfile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "cREXX Assemble Test ${_basename} ..."
  )
  add_test(NAME ${_basename} COMMAND rxvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_test(NAME ${_basename}-bytecode COMMAND rxbvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
