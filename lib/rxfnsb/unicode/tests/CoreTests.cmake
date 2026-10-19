# Compile and link during preparation; CTest only executes the linked images.
crexx_add_import_root(unicode_core_test_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/imports"
  OUTPUT_VARIABLE _unicode_core_test_import_files
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${RXUNICODE_CORE_RXBIN}"
  DEPENDS library rxunicode_core)
add_custom_target(ts_unicode_core)
set(_unicode_core_test_runners rxbvm)
if(TARGET rxtvm)
  list(APPEND _unicode_core_test_runners rxtvm)
endif()
foreach(_unicode_core_test_mode IN ITEMS noopt opt)
  set(_unicode_core_test_flags)
  if(_unicode_core_test_mode STREQUAL "noopt")
    set(_unicode_core_test_flags -n)
  endif()
  set(_unicode_core_test_base
    "${CMAKE_CURRENT_BINARY_DIR}/tests/ts_unicode_core_${_unicode_core_test_mode}")
  add_custom_command(OUTPUT "${_unicode_core_test_base}-linked.rxbin"
    BYPRODUCTS "${_unicode_core_test_base}.rxas" "${_unicode_core_test_base}.rxbin"
    COMMAND $<TARGET_FILE:rxc> ${_unicode_core_test_flags} -x
      --no-exe-import --no-autoload
      -i "${CMAKE_CURRENT_BINARY_DIR}/tests/imports"
      -o "${_unicode_core_test_base}"
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/ts_unicode_core.crexx"
    COMMAND $<TARGET_FILE:rxas> ${_unicode_core_test_flags} "${_unicode_core_test_base}"
    COMMAND $<TARGET_FILE:rxlink> -s -o "${_unicode_core_test_base}-linked"
      "${_unicode_core_test_base}.rxbin"
      "${RXUNICODE_CORE_RXBIN}" "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    DEPENDS rxc rxas rxlink unicode_core_test_imports
      ${_unicode_core_test_import_files}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/ts_unicode_core.crexx"
    VERBATIM)
  add_custom_target(ts_unicode_core_${_unicode_core_test_mode}
    DEPENDS "${_unicode_core_test_base}-linked.rxbin")
  add_dependencies(ts_unicode_core ts_unicode_core_${_unicode_core_test_mode})
  foreach(_unicode_core_test_runner IN LISTS _unicode_core_test_runners)
    add_test(NAME ts_unicode_core_${_unicode_core_test_runner}_${_unicode_core_test_mode}
      COMMAND $<TARGET_FILE:${_unicode_core_test_runner}>
        --no-autoload "${_unicode_core_test_base}-linked.rxbin")
    set_tests_properties(ts_unicode_core_${_unicode_core_test_runner}_${_unicode_core_test_mode}
      PROPERTIES LABELS "library;unicode;levelb;optimizer-parity;${_unicode_core_test_runner}"
      TIMEOUT 120
      PASS_REGULAR_EXPRESSION "PASS: independent B Unicode core without G aggregation"
      FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:")
    crexx_register_test_prep_targets(
      TESTS ts_unicode_core_${_unicode_core_test_runner}_${_unicode_core_test_mode}
      TARGETS ts_unicode_core_${_unicode_core_test_mode} ${_unicode_core_test_runner})
    add_dependencies(ts_unicode_core ${_unicode_core_test_runner})
  endforeach()
endforeach()
