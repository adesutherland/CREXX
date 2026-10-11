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
    DEPENDS library rxfnsl "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin" "${_rexx_file}"
    RUNTIME_ARGS ${LIBRPATH} ${RXFNSL_RPATH}
  )
endforeach()

set_tests_properties(ts_tinyexpr_noopt ts_tinyexpr_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxfnsl tinyexpr generated-shape demo")

crexx_label_smoke_tests(
        ts_tinyexpr_noopt
)

# A second consumer, packed-image validation, ownership and source diagnostics.
set(_generated_spec "${CMAKE_CURRENT_SOURCE_DIR}/../tools/example.levell")
set(_generated_grammar "${CMAKE_CURRENT_BINARY_DIR}/levell_example.rxas")
add_custom_command(OUTPUT "${_generated_grammar}"
  COMMAND ${CMAKE_COMMAND} -DLEVEL_L=$<TARGET_FILE:levell_cli>
    -DSPEC=${_generated_spec} -DOUTPUT=${_generated_grammar} -DFORMAT=rxas
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
  DEPENDS levell_cli "${_generated_spec}" "${CMAKE_CURRENT_SOURCE_DIR}/../tools/generate.cmake"
  VERBATIM)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/testGenerated-linked.rxbin"
  COMMAND $<TARGET_FILE:rxas> levell_example
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}"
    -o testGenerated "${CMAKE_CURRENT_SOURCE_DIR}/testGenerated.crexx"
  COMMAND $<TARGET_FILE:rxas> testGenerated
  COMMAND $<TARGET_FILE:rxlink> -s -o testGenerated-linked testGenerated levell_example
    "${CMAKE_BINARY_DIR}/bin/rxfnsl" "${CMAKE_BINARY_DIR}/bin/library"
  DEPENDS rxc rxas rxlink rxfnsl "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${_generated_grammar}" testGenerated.crexx
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(testlevell_generated ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/testGenerated-linked.rxbin")
add_dependencies(testrxfnsl testlevell_generated)
foreach(_vm rxtvm rxbvm)
  if(TARGET ${_vm})
    add_test(NAME levell_generated_${_vm}
      COMMAND $<TARGET_FILE:${_vm}> "${CMAKE_CURRENT_BINARY_DIR}/testGenerated-linked.rxbin" -a "${_generated_spec}")
    set_tests_properties(levell_generated_${_vm} PROPERTIES
      PASS_REGULAR_EXPRESSION "generated_failures=0" TIMEOUT 300 LABELS "library;level-l")
  endif()
endforeach()
