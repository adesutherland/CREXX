cmake_minimum_required(VERSION 3.24)

# Explicit optional target tools; core/bootstrap packages have no reverse edge.
set(_modules lltarget lltargettransfer lltargetcfg lltargetremap
  lltargetflow lltargetbodyview lltargetsummary lltargetnormalize)
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
crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/_rxlinkinline.rxbin"
  COMMAND $<TARGET_FILE:rxas> -o _rxlinkinline "${CMAKE_CURRENT_SOURCE_DIR}/_rxlinkinline.rxas"
  DEPENDS rxas "${CMAKE_CURRENT_SOURCE_DIR}/_rxlinkinline.rxas"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
# Native declarations are compiler import metadata only. Packaging their empty
# concrete PROC records would create an executable empty-body target rather
# than the host's native procedure (the ADDRESS bridge uses the same boundary).
set(_bins)
set(_previous_module "")
set(_native_imports)
foreach(_module IN LISTS _modules)
  set(_deps)
  # Finish each predecessor image before staging a consumer's exact closure.
  if(_previous_module)
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/${_previous_module}.rxbin")
  endif()
  if(NOT _module STREQUAL "lltarget")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltarget.rxbin")
  endif()
  if(_module STREQUAL "lltargettransfer")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/_rxlinkinline.rxbin")
    set(_native_imports "${CMAKE_CURRENT_BINARY_DIR}/_rxlinkinline.rxbin")
  endif()
  if(_module STREQUAL "lltargetcfg" OR _module STREQUAL "lltargetremap"
      OR _module STREQUAL "lltargetflow")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargettransfer.rxbin")
  endif()
  if(_module STREQUAL "lltargetflow")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargetcfg.rxbin")
  endif()
  if(_module STREQUAL "lltargetbodyview" OR _module STREQUAL "lltargetsummary" OR _module STREQUAL "lltargetnormalize")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargetflow.rxbin")
  endif()
  if(_module STREQUAL "lltargetsummary")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargetbodyview.rxbin")
  endif()
  if(_module STREQUAL "lltargetnormalize")
    list(APPEND _deps "${CMAKE_CURRENT_BINARY_DIR}/lltargetremap.rxbin")
  endif()
  # The output directory also holds linked test consumers with embedded old
  # contracts. Import only current owning images, never that directory.
  set(_module_import_root "${CMAKE_CURRENT_BINARY_DIR}/module-imports/${_module}")
  crexx_add_import_root(rxfnsl_target_${_module}_imports
    DIRECTORY "${_module_import_root}"
    OUTPUT_VARIABLE _module_import_files
    FILES ${_import_files} ${_bins} ${_native_imports}
    DEPENDS rxfnsl_target_imports ${_deps})
  crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxas"
    COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import
      -i "${_module_import_root}"
      -o "${_module}" "${CMAKE_CURRENT_SOURCE_DIR}/${_module}.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_flags} "${_module}"
    DEPENDS rxc rxas rxfnsl_target_${_module}_imports ${_module_import_files} ${_deps}
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

option(CREXX_BUILD_TARGET_IO "Build the optional Level L target RXAS adapters" OFF)

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

  # Independent shared target clients, without an inliner or admission receipts.
  foreach(_foundation_case IN ITEMS
      target-cfg target-transfer-cfg target-binding-recipe target-selection
      target-bodyview target-program-defaults target-activation-abi
      target-context-restoration target-site-facets target-normalization-preservation
      target-correspondence target-normalization-site
      target-draft-budget target-execution-context target-signal-cfg
      target-signal-normalize target-canonical-flow target-scope-flow
      target-recipe target-module-remap target-flow target-summary
      target-cold-metadata target-generic-flow foundation-normalize)
    if((_foundation_case STREQUAL "foundation-normalize" OR _foundation_case STREQUAL "target-generic-flow") AND NOT CREXX_BUILD_TARGET_IO)
      continue()
    endif()
    string(REPLACE "-" "_" _foundation_name "${_foundation_case}")
    string(TOUPPER "${_foundation_name}" _foundation_marker)
    set(_foundation_images ${_images})
    set(_foundation_imports "${_test_imports}")
    set(_foundation_deps levell_target_test_imports ${_test_files})
    if(_foundation_case STREQUAL "foundation-normalize" OR _foundation_case STREQUAL "target-generic-flow")
      list(APPEND _foundation_images "${CMAKE_BINARY_DIR}/bin/rxfnsl-target-io.rxbin")
      list(APPEND _foundation_images "${CMAKE_BINARY_DIR}/langtools/common/llrxastext.rxbin")
      set(_foundation_imports "${CMAKE_CURRENT_BINARY_DIR}/${_foundation_case}-imports")
      crexx_add_import_root(levell_${_foundation_name}_imports
        DIRECTORY "${_foundation_imports}" OUTPUT_VARIABLE _foundation_import_files
        FILES ${_foundation_images}
        DEPENDS library rxfnsl rxfnsl_analysis rxfnsl_rxas rxfnsl_target rxfnsl_target_io llrxastext)
      set(_foundation_deps levell_${_foundation_name}_imports ${_foundation_import_files})
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
      DEPENDS rxc rxas rxlink ${_foundation_deps} "${_foundation_source}"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(levell_${_foundation_name} DEPENDS "${_foundation_output}")
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
            -DVM=$<TARGET_FILE:rxbvm> -DASSEMBLER=$<TARGET_FILE:rxas>
            -DLINKER=$<TARGET_FILE:rxlink> -DCLIENT=${_foundation_output}
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

if(CREXX_BUILD_TARGET_IO)
  add_subdirectory(io)
  if(BUILD_TESTING)
    add_subdirectory("${CMAKE_SOURCE_DIR}/research/level-l/toolchain/tests"
      "${CMAKE_BINARY_DIR}/research/level-l/toolchain/tests")
  endif()
endif()
