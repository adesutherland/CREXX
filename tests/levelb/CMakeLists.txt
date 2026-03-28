cmake_minimum_required(VERSION 3.24)
project(levelb_tests C)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

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
  add_custom_target(${_basename}_artifact ALL)
  crexx_add_rexx_opt_matrix(
          NAME ${_basename}
          SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rexx
          TARGET_GROUP ${_basename}_artifact
          RXC_FLAGS -x
          DEPENDS ${_prev_target}
  )
  set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()
