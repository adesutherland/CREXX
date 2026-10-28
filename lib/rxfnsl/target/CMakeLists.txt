cmake_minimum_required(VERSION 3.24)

# Explicit optional target tools; core/bootstrap packages have no reverse edge.
set(_modules lltarget llinline)
set_property(GLOBAL PROPERTY RXFNSL_TARGET_SELECTORS "${_modules}")
set(_imports "${CMAKE_CURRENT_BINARY_DIR}/imports")
crexx_add_import_root(rxfnsl_target_imports DIRECTORY "${_imports}"
  OUTPUT_VARIABLE _import_files FILES
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin"
  DEPENDS library rxfnsl rxfnsl_analysis rxfnsl_rxas)
set(_flags)
if(CREXX_LEVEL_L_NOOPT)
  list(APPEND _flags -n)
endif()
set(_bins)
foreach(_module IN LISTS _modules)
  set(_deps)
  if(_module STREQUAL "llinline")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltarget.rxbin")
  endif()
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxas"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
      -i "${_imports};${CMAKE_CURRENT_BINARY_DIR}"
      -o "${_module}" "${CMAKE_CURRENT_SOURCE_DIR}/${_module}.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_flags} "${_module}"
    DEPENDS rxc rxas rxfnsl_target_imports ${_import_files} ${_deps}
      "${CMAKE_CURRENT_SOURCE_DIR}/${_module}.crexx"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  list(APPEND _bins "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
endforeach()
set(_control "${CMAKE_CURRENT_BINARY_DIR}/target.ctl")
file(GENERATE OUTPUT "${_control}" CONTENT
  "INPUT ${CMAKE_CURRENT_BINARY_DIR}/lltarget.rxbin\nINPUT ${CMAKE_CURRENT_BINARY_DIR}/llinline.rxbin\nINCLUDE lltarget\nINCLUDE llinline\nPRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsl-target\n")
crexx_add_fixture_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsl-target.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_control}"
  DEPENDS rxlink ${_bins} "${_control}" VERBATIM)
add_custom_target(rxfnsl_target DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsl-target.rxbin")

if(BUILD_TESTING)
  set(_test_imports "${CMAKE_CURRENT_BINARY_DIR}/test-imports")
  set(_images "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-target.rxbin")
  crexx_add_import_root(levell_target_test_imports DIRECTORY "${_test_imports}"
    OUTPUT_VARIABLE _test_files FILES ${_images}
    DEPENDS library rxfnsl rxfnsl_analysis rxfnsl_rxas rxfnsl_target)
  set(_fixture "${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/constructed-inline.crexx")
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/constructed-inline-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_test_imports}"
      -o constructed-inline "${_fixture}"
    COMMAND $<TARGET_FILE:rxas> constructed-inline
    COMMAND $<TARGET_FILE:rxlink> -s -o constructed-inline-linked constructed-inline ${_images}
    DEPENDS rxc rxas rxlink levell_target_test_imports ${_test_files} "${_fixture}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(levell_constructed_inline
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/constructed-inline-linked.rxbin")
  if(TARGET rxbvm)
    add_test(NAME levell_constructed_inline_rxbvm
      COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/constructed-inline-linked.rxbin")
    set_tests_properties(levell_constructed_inline_rxbvm PROPERTIES
      PASS_REGULAR_EXPRESSION "CONSTRUCTED_INLINE[|]failures=0"
      TIMEOUT 120 LABELS "level-l;research;inlining")
    crexx_register_test_prep_targets(TESTS levell_constructed_inline_rxbvm
      TARGETS levell_constructed_inline rxbvm)
  endif()
endif()
