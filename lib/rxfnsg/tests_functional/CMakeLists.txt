cmake_minimum_required(VERSION 3.24)

project(test_rxfnsg)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(REXXBINS
  ts_llm_ollama
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(RXFNSG_RPATH ${CMAKE_BINARY_DIR}/bin/rxfnsg)

add_custom_target(testrxfnsg ALL)

unset(_prev_target)
foreach(_binfile IN LISTS REXXBINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rexx")

  add_custom_target("${_basename}")
  add_dependencies(testrxfnsg "${_basename}")
  crexx_add_rexx_opt_matrix(
    NAME "${_basename}"
    SOURCE "${_rexx_file}"
    TARGET_GROUP "${_basename}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library rxfnsg "${_rexx_file}" ${_prev_target}
    RUNTIME_ARGS ${LIBRPATH} ${RXFNSG_RPATH}
  )
  set(_prev_target "${_basename}_noopt_artifact" "${_basename}_opt_artifact")
endforeach()

set_tests_properties(ts_llm_ollama_noopt ts_llm_ollama_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxfnsg llm ollama helpers")
