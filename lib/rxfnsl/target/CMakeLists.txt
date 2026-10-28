cmake_minimum_required(VERSION 3.24)

# Explicit optional target tools; core/bootstrap packages have no reverse edge.
set(_modules lltarget lltargetfacts llinline llcleanup)
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
  if(NOT _module STREQUAL "lltarget")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltarget.rxbin")
  endif()
  if(_module STREQUAL "llinline" OR _module STREQUAL "llcleanup")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargetfacts.rxbin")
  endif()
  if(_module STREQUAL "llcleanup")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/llinline.rxbin")
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
set(_target_control "")
foreach(_module IN LISTS _modules)
  string(APPEND _target_control "INPUT ${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin\n")
endforeach()
foreach(_module IN LISTS _modules)
  string(APPEND _target_control "INCLUDE ${_module}\n")
endforeach()
string(APPEND _target_control "PRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsl-target\n")
file(GENERATE OUTPUT "${_control}" CONTENT "${_target_control}")
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

  set(_scalar_fixture "${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/scalar-transport.crexx")
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scalar-transport-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_test_imports}"
      -o scalar-transport "${_scalar_fixture}"
    COMMAND $<TARGET_FILE:rxas> scalar-transport
    COMMAND $<TARGET_FILE:rxlink> -s -o scalar-transport-linked scalar-transport ${_images}
    DEPENDS rxc rxas rxlink levell_target_test_imports ${_test_files} "${_scalar_fixture}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(levell_scalar_transport
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/scalar-transport-linked.rxbin")
  if(TARGET rxbvm)
    add_test(NAME levell_scalar_transport_rxbvm
      COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/scalar-transport-linked.rxbin")
    set_tests_properties(levell_scalar_transport_rxbvm PROPERTIES
      PASS_REGULAR_EXPRESSION "SCALAR_TRANSPORT[|]failures=0"
      FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
      TIMEOUT 120 LABELS "level-l;research;inlining")
    crexx_register_test_prep_targets(TESTS levell_scalar_transport_rxbvm
      TARGETS levell_scalar_transport rxbvm)
  endif()

  set(_rebinding_fixture "${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/rebinding-inline.crexx")
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/rebinding-inline-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_test_imports}"
      -o rebinding-inline "${_rebinding_fixture}"
    COMMAND $<TARGET_FILE:rxas> rebinding-inline
    COMMAND $<TARGET_FILE:rxlink> -s -o rebinding-inline-linked rebinding-inline ${_images}
    DEPENDS rxc rxas rxlink levell_target_test_imports ${_test_files} "${_rebinding_fixture}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(levell_rebinding_inline
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/rebinding-inline-linked.rxbin")
  if(TARGET rxbvm)
    add_test(NAME levell_rebinding_inline_rxbvm
      COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/rebinding-inline-linked.rxbin")
    set_tests_properties(levell_rebinding_inline_rxbvm PROPERTIES
      PASS_REGULAR_EXPRESSION "REBINDING_INLINE[|]failures=0"
      FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
      TIMEOUT 120 LABELS "level-l;research;inlining")
    crexx_register_test_prep_targets(TESTS levell_rebinding_inline_rxbvm
      TARGETS levell_rebinding_inline rxbvm)
  endif()

  set(_storage_fixture "${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/storage-inline.crexx")
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/storage-inline-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_test_imports}"
      -o storage-inline "${_storage_fixture}"
    COMMAND $<TARGET_FILE:rxas> storage-inline
    COMMAND $<TARGET_FILE:rxlink> -s -o storage-inline-linked storage-inline ${_images}
    DEPENDS rxc rxas rxlink levell_target_test_imports ${_test_files} "${_storage_fixture}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(levell_storage_inline
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/storage-inline-linked.rxbin")
  if(TARGET rxbvm)
    add_test(NAME levell_storage_inline_rxbvm
      COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/storage-inline-linked.rxbin")
    set_tests_properties(levell_storage_inline_rxbvm PROPERTIES
      PASS_REGULAR_EXPRESSION "STORAGE_INLINE[|]failures=0"
      FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
      TIMEOUT 120 LABELS "level-l;research;inlining")
    crexx_register_test_prep_targets(TESTS levell_storage_inline_rxbvm
      TARGETS levell_storage_inline rxbvm)
  endif()

  set(_nested_fixture "${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/nested-inline.crexx")
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/nested-inline-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_test_imports}"
      -o nested-inline "${_nested_fixture}"
    COMMAND $<TARGET_FILE:rxas> nested-inline
    COMMAND $<TARGET_FILE:rxlink> -s -o nested-inline-linked nested-inline ${_images}
    DEPENDS rxc rxas rxlink levell_target_test_imports ${_test_files} "${_nested_fixture}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(levell_nested_inline
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/nested-inline-linked.rxbin")
  if(TARGET rxbvm)
    add_test(NAME levell_nested_inline_rxbvm
      COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/nested-inline-linked.rxbin")
    set_tests_properties(levell_nested_inline_rxbvm PROPERTIES
      PASS_REGULAR_EXPRESSION "NESTED_INLINE[|]failures=0"
      FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
      TIMEOUT 120 LABELS "level-l;research;inlining")
    crexx_register_test_prep_targets(TESTS levell_nested_inline_rxbvm
      TARGETS levell_nested_inline rxbvm)
  endif()
endif()

option(CREXX_BUILD_TARGET_IO "Build the optional Level L target RXAS adapters" OFF)
if(CREXX_BUILD_TARGET_IO)
  add_subdirectory(io)
  add_subdirectory("${CMAKE_SOURCE_DIR}/research/level-l/toolchain/cli"
    "${CMAKE_BINARY_DIR}/research/level-l/toolchain/cli")
  if(BUILD_TESTING)
    add_subdirectory("${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests"
      "${CMAKE_BINARY_DIR}/research/level-l/toolchain/tests")
  endif()
endif()
