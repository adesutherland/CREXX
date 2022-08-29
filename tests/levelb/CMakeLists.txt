cmake_minimum_required(VERSION 3.00)

project(tests C)

set(CMAKE_C_STANDARD 90)

enable_testing()
find_program(CTEST ctest)

#set(ASMTESTBINS ascommon.rxbin asutf.rxbin)
set(REXXTESTBINS concatenation.rxbin)

add_custom_target(levelbtests ALL
        DEPENDS ${REXXTESTBINS})
#	DEPENDS ${ASMTESTBINS} ${REXXTESTBINS})							

# foreach(_binfile ${ASMTESTBINS})
#   get_filename_component(_basename ${_binfile} NAME_WE)
#   add_custom_command(
#         COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -l ${CMAKE_CURRENT_SOURCE_DIR} ${_basename}
#         #COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/${_binfile} ${CMAKE_CURRENT_BINARY_DIR}/${_binfile}
#         COMMAND ${CMAKE_COMMAND} -E copy   ${CMAKE_CURRENT_SOURCE_DIR}/${_binfile} ${CMAKE_CURRENT_BINARY_DIR}/${_binfile}
#         COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_SOURCE_DIR}/${_binfile}
#         #
#         COMMAND ${CTEST} -R ${_basename} --output-on-failure
#         DEPENDS ${_basename}.rxas rxas rxvm rxbvm lib_rexx lib_rxas
#         OUTPUT ${_binfile}
#         COMMENT "cREXX Assemble Test ${_basename} ..."
#   )
#   add_test(NAME ${_basename} COMMAND rxvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
#   add_test(NAME ${_basename}-bytecode COMMAND rxbvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
# endforeach()

foreach(_binfile ${REXXTESTBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_command(
          COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -l ${CMAKE_CURRENT_SOURCE_DIR} ${_basename}
          #COMMAND mv ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxas ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.rxas
          COMMAND ${CMAKE_COMMAND} -E copy   ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxas ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.rxas
          COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxas
          #
          COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -l ${CMAKE_CURRENT_BINARY_DIR} ${_basename}
#         COMMAND ${CTEST} -R ${_basename} --output-on-failure
          DEPENDS ${_basename}.rexx rxas rxc rxvm rxbvm library
          OUTPUT ${_binfile}
          COMMENT "cREXX Assemble Test ${_basename} ..."
  )
  add_test(NAME ${_basename}-rxvm COMMAND rxvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_test(NAME ${_basename}-rxbvm COMMAND rxbvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
