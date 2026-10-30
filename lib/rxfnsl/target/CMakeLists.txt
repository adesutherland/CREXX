cmake_minimum_required(VERSION 3.24)

# Explicit optional target tools; core/bootstrap packages have no reverse edge.
set(_modules lltarget lltargettransfer lltargetcfg lltargetremap
  lltargetflow lltargetsummary lltargetnormalize lltargetfacts llinline llcleanup)
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
set(_previous_module "")
foreach(_module IN LISTS _modules)
  set(_deps)
  # rxc scans the shared local import directory. Finish each image before
  # starting the next reader so parallel builds never inspect a partial RXBIN.
  if(_previous_module)
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/${_previous_module}.rxbin")
  endif()
  if(NOT _module STREQUAL "lltarget")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltarget.rxbin")
  endif()
  if(_module STREQUAL "lltargetcfg" OR _module STREQUAL "lltargetremap"
      OR _module STREQUAL "lltargetflow")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargettransfer.rxbin")
  endif()
  if(_module STREQUAL "lltargetflow")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargetcfg.rxbin")
  endif()
  if(_module STREQUAL "lltargetsummary" OR _module STREQUAL "lltargetnormalize")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargetflow.rxbin")
  endif()
  if(_module STREQUAL "lltargetnormalize")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargetremap.rxbin")
  endif()
  if(_module STREQUAL "llinline")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargetremap.rxbin")
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
  set(_previous_module "${_module}")
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

  set(_import_fixture "${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/imported-inline.crexx")
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/imported-inline-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_test_imports}"
      -o imported-inline "${_import_fixture}"
    COMMAND $<TARGET_FILE:rxas> imported-inline
    COMMAND $<TARGET_FILE:rxlink> -s -o imported-inline-linked imported-inline ${_images}
    DEPENDS rxc rxas rxlink levell_target_test_imports ${_test_files} "${_import_fixture}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(levell_imported_inline
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/imported-inline-linked.rxbin")
  if(TARGET rxbvm)
    add_test(NAME levell_imported_inline_rxbvm
      COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/imported-inline-linked.rxbin")
    set_tests_properties(levell_imported_inline_rxbvm PROPERTIES
      PASS_REGULAR_EXPRESSION "IMPORTED_INLINE[|]failures=0"
      FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
      TIMEOUT 120 LABELS "level-l;research;inlining")
    crexx_register_test_prep_targets(TESTS levell_imported_inline_rxbvm
      TARGETS levell_imported_inline rxbvm)
  endif()

  set(_import_dag_fixture "${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/imported-dag.crexx")
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/imported-dag-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_test_imports}"
      -o imported-dag "${_import_dag_fixture}"
    COMMAND $<TARGET_FILE:rxas> imported-dag
    COMMAND $<TARGET_FILE:rxlink> -s -o imported-dag-linked imported-dag ${_images}
    DEPENDS rxc rxas rxlink levell_target_test_imports ${_test_files} "${_import_dag_fixture}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(levell_imported_dag
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/imported-dag-linked.rxbin")
  if(TARGET rxbvm)
    add_test(NAME levell_imported_dag_rxbvm
      COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/imported-dag-linked.rxbin")
    set_tests_properties(levell_imported_dag_rxbvm PROPERTIES
      PASS_REGULAR_EXPRESSION "IMPORTED_DAG[|]failures=0"
      FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
      TIMEOUT 120 LABELS "level-l;research;inlining")
    crexx_register_test_prep_targets(TESTS levell_imported_dag_rxbvm
      TARGETS levell_imported_dag rxbvm)
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

  set(_cfg_fixture "${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/target-cfg.crexx")
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/target-cfg-linked.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_test_imports}"
      -o target-cfg "${_cfg_fixture}"
    COMMAND $<TARGET_FILE:rxas> target-cfg
    COMMAND $<TARGET_FILE:rxlink> -s -o target-cfg-linked target-cfg ${_images}
    DEPENDS rxc rxas rxlink levell_target_test_imports ${_test_files} "${_cfg_fixture}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
  add_custom_target(levell_target_cfg
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/target-cfg-linked.rxbin")
  if(TARGET rxbvm)
    add_test(NAME levell_target_cfg_rxbvm
      COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/target-cfg-linked.rxbin")
    set_tests_properties(levell_target_cfg_rxbvm PROPERTIES
      PASS_REGULAR_EXPRESSION "TARGET_CFG[|]failures=0"
      FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
      TIMEOUT 120 LABELS "level-l;research;inlining")
    crexx_register_test_prep_targets(TESTS levell_target_cfg_rxbvm
      TARGETS levell_target_cfg rxbvm)
  endif()

  # One bounded owner panel for the shared Level L target foundation.
  foreach(_foundation_case IN ITEMS
      target-transfer-cfg target-module-remap target-flow target-summary foundation-normalize)
    if(_foundation_case STREQUAL "foundation-normalize" AND NOT CREXX_BUILD_TARGET_IO)
      continue()
    endif()
    string(REPLACE "-" "_" _foundation_name "${_foundation_case}")
    string(TOUPPER "${_foundation_name}" _foundation_marker)
    set(_foundation_images ${_images})
    set(_foundation_imports "${_test_imports}")
    set(_foundation_deps levell_target_test_imports ${_test_files})
    if(_foundation_case STREQUAL "foundation-normalize")
      list(APPEND _foundation_images "${CMAKE_BINARY_DIR}/bin/rxfnsl-target-io.rxbin")
      list(APPEND _foundation_images "${CMAKE_BINARY_DIR}/langtools/common/llrxastext.rxbin")
      set(_foundation_imports "${CMAKE_CURRENT_BINARY_DIR}/foundation-normalize-imports")
      crexx_add_import_root(levell_foundation_normalize_imports
        DIRECTORY "${_foundation_imports}" OUTPUT_VARIABLE _foundation_import_files
        FILES ${_foundation_images}
        DEPENDS library rxfnsl rxfnsl_analysis rxfnsl_rxas rxfnsl_target rxfnsl_target_io llrxastext)
      set(_foundation_deps levell_foundation_normalize_imports ${_foundation_import_files})
    endif()
    set(_foundation_source
      "${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/${_foundation_case}.crexx")
    set(_foundation_output
      "${CMAKE_CURRENT_BINARY_DIR}/${_foundation_case}-linked.rxbin")
    crexx_add_fixture_command(OUTPUT "${_foundation_output}"
      COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_foundation_imports}"
        -o "${_foundation_case}" "${_foundation_source}"
      COMMAND $<TARGET_FILE:rxas> "${_foundation_case}"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_foundation_case}-linked"
        "${_foundation_case}" ${_foundation_images}
      DEPENDS rxc rxas rxlink ${_foundation_deps}
        "${_foundation_source}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(levell_${_foundation_name}
      DEPENDS "${_foundation_output}")
    if(TARGET rxbvm)
      add_test(NAME levell_${_foundation_name}_rxbvm
        COMMAND $<TARGET_FILE:rxbvm> "${_foundation_output}")
      set_tests_properties(levell_${_foundation_name}_rxbvm PROPERTIES
        PASS_REGULAR_EXPRESSION "${_foundation_marker}[|]failures=0"
        FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
        TIMEOUT 120 LABELS "level-l;research;target-foundation")
      crexx_register_test_prep_targets(TESTS levell_${_foundation_name}_rxbvm
        TARGETS levell_${_foundation_name} rxbvm)
      if(_foundation_case STREQUAL "foundation-normalize")
        add_test(NAME levell_foundation_normalize_native
          COMMAND ${CMAKE_COMMAND}
            -DVM=$<TARGET_FILE:rxbvm>
            -DASSEMBLER=$<TARGET_FILE:rxas>
            -DLINKER=$<TARGET_FILE:rxlink>
            -DCLIENT=${_foundation_output}
            -DWORK=${CMAKE_CURRENT_BINARY_DIR}/foundation-normalize-native-work
            -P ${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests/run-foundation-normalize-native.cmake)
        set_tests_properties(levell_foundation_normalize_native PROPERTIES
          PASS_REGULAR_EXPRESSION "FOUNDATION_NATIVE[|]failures=0"
          TIMEOUT 120 LABELS "level-l;research;target-foundation")
        crexx_register_test_prep_targets(TESTS levell_foundation_normalize_native
          TARGETS levell_foundation_normalize rxbvm rxas rxlink)
      endif()
    endif()
  endforeach()
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
