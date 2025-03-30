cmake_minimum_required(VERSION 3.24)
add_subdirectory(levelb)
add_subdirectory(rxpa)

project(tests C)

set(CMAKE_C_STANDARD 90)

enable_testing()

set(ASMTESTBINS ascommon.rxbin asutf.rxbin)
set(REXXTESTBINS basic.rxbin loop.rxbin levelbfunc.rxbin)

unset(_prev_dependency) # Clear the previous dependency (just in case)
foreach(_binfile ${ASMTESTBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_target(${_basename} ALL DEPENDS ${_binfile}) # Create a "virtual target"
  add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
        DEPENDS ${_basename}.rxas rxas rxvm rxbvm library ${_prev_dependency}
        OUTPUT ${_binfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX Assemble Test ${_basename} ..."
  )
  add_test(NAME ${_basename}-rxvm COMMAND rxvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  add_test(NAME ${_basename}-rxbvm COMMAND rxbvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set(_prev_dependency ${_basename}.rxbin)
endforeach()

foreach(_binfile ${REXXTESTBINS})
   get_filename_component(_basename ${_binfile} NAME_WE)
   # Create a "virtual target" using add_custom_target
   add_custom_target(${_basename} ALL DEPENDS ${_binfile})
   add_custom_command(
           COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
           COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas ${_basename}
           DEPENDS ${_basename}.rexx rxas rxc rxvm rxbvm library ${_prev_dependency}
           OUTPUT ${_binfile}
           WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
           COMMENT "cREXX Assemble Test ${_basename} ..."
   )
   add_test(NAME ${_basename}-rxvm COMMAND rxvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
   add_test(NAME ${_basename}-rxbvm COMMAND rxbvm ${_basename} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
   set(_prev_dependency ${_basename}.rxbin)
endforeach()
