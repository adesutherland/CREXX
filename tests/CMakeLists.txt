cmake_minimum_required(VERSION 3.24)
project(tests C)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(native_signals)
add_subdirectory(levelb)
add_subdirectory(performance)
add_subdirectory(rxpa)

set(CMAKE_C_STANDARD 90)

enable_testing()

set(ASMTESTBINS ascommon.rxbin asutf.rxbin)
set(REXXTESTBINS basic.rxbin levelbfunc.rxbin)

unset(_prev_dependency) # Clear the previous dependency (just in case)
foreach(_binfile ${ASMTESTBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_target(${_basename} ALL)
  crexx_add_rxas_opt_matrix(
          NAME ${_basename}
          SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
          TARGET_GROUP ${_basename}
          DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxas library compiler_exit_bin ${_prev_dependency}
  )
  set(_prev_dependency ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()

foreach(_binfile ${REXXTESTBINS})
   get_filename_component(_basename ${_binfile} NAME_WE)
   add_custom_target(${_basename} ALL)
   crexx_add_rexx_opt_matrix(
           NAME ${_basename}
           SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rexx
           TARGET_GROUP ${_basename}
           DEPENDS library compiler_exit_bin ${_prev_dependency}
   )
   set(_prev_dependency ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()
