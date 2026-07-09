cmake_minimum_required(VERSION 3.24)

project(test_rxfnsl)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

set(REXXBINS
  ts_tinyexpr
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(RXFNSL_RPATH ${CMAKE_BINARY_DIR}/bin/rxfnsl)

add_custom_target(testrxfnsl ALL)

unset(_prev_target)
foreach(_binfile IN LISTS REXXBINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")

  add_custom_target("${_basename}")
  add_dependencies(testrxfnsl "${_basename}")
  crexx_add_rexx_opt_matrix(
    NAME "${_basename}"
    SOURCE "${_rexx_file}"
    TARGET_GROUP "${_basename}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library rxfnsl "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin" "${_rexx_file}" ${_prev_target}
    RUNTIME_ARGS ${LIBRPATH} ${RXFNSL_RPATH}
  )
  set(_prev_target "${_basename}_noopt_artifact" "${_basename}_opt_artifact")
endforeach()

set_tests_properties(ts_tinyexpr_noopt ts_tinyexpr_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxfnsl tinyexpr generated-shape demo")

crexx_label_smoke_tests(
        ts_tinyexpr_noopt
)
