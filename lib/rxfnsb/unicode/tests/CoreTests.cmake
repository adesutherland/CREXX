# Compile and link during preparation; CTest only executes the linked images.
crexx_add_import_root(unicode_core_test_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests/imports"
  OUTPUT_VARIABLE _unicode_core_test_import_files
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${RXUNICODE_CORE_RXBIN}"
  DEPENDS library rxunicode_core)
set(_unicode_core_test_runners rxbvm)
if(TARGET rxtvm)
  list(APPEND _unicode_core_test_runners rxtvm)
endif()
foreach(_unicode_core_fixture IN ITEMS core public)
  add_custom_target(ts_unicode_${_unicode_core_fixture})
  set(_unicode_core_test_success "PASS: independent B Unicode core without G aggregation")
  if(_unicode_core_fixture STREQUAL "public")
    set(_unicode_core_test_success "PASS: public B Unicode API without G aggregation")
  endif()
  foreach(_unicode_core_test_mode IN ITEMS noopt opt)
    set(_unicode_core_test_flags)
    if(_unicode_core_test_mode STREQUAL "noopt")
      set(_unicode_core_test_flags -n)
    endif()
    set(_unicode_core_test_base
      "${CMAKE_CURRENT_BINARY_DIR}/tests/ts_unicode_${_unicode_core_fixture}_${_unicode_core_test_mode}")
    add_custom_command(OUTPUT "${_unicode_core_test_base}-linked.rxbin"
      BYPRODUCTS "${_unicode_core_test_base}.rxas" "${_unicode_core_test_base}.rxbin"
      COMMAND $<TARGET_FILE:rxc> ${_unicode_core_test_flags} -x
        --no-exe-import --no-autoload
        -i "${CMAKE_CURRENT_BINARY_DIR}/tests/imports"
        -o "${_unicode_core_test_base}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/ts_unicode_${_unicode_core_fixture}.crexx"
      COMMAND $<TARGET_FILE:rxas> ${_unicode_core_test_flags} "${_unicode_core_test_base}"
      COMMAND $<TARGET_FILE:rxlink> -s -o "${_unicode_core_test_base}-linked"
        "${_unicode_core_test_base}.rxbin"
        "${RXUNICODE_CORE_RXBIN}" "${CMAKE_BINARY_DIR}/bin/library.rxbin"
      DEPENDS rxc rxas rxlink unicode_core_test_imports
        ${_unicode_core_test_import_files}
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/ts_unicode_${_unicode_core_fixture}.crexx"
      VERBATIM)
    add_custom_target(ts_unicode_${_unicode_core_fixture}_${_unicode_core_test_mode}
      DEPENDS "${_unicode_core_test_base}-linked.rxbin")
    add_dependencies(ts_unicode_${_unicode_core_fixture} ts_unicode_${_unicode_core_fixture}_${_unicode_core_test_mode})
    foreach(_unicode_core_test_runner IN LISTS _unicode_core_test_runners)
      add_test(NAME ts_unicode_${_unicode_core_fixture}_${_unicode_core_test_runner}_${_unicode_core_test_mode}
        COMMAND $<TARGET_FILE:${_unicode_core_test_runner}>
          --no-autoload "${_unicode_core_test_base}-linked.rxbin")
      set_tests_properties(ts_unicode_${_unicode_core_fixture}_${_unicode_core_test_runner}_${_unicode_core_test_mode}
        PROPERTIES LABELS "library;unicode;levelb;optimizer-parity;${_unicode_core_test_runner}"
        TIMEOUT 120
        PASS_REGULAR_EXPRESSION "${_unicode_core_test_success}"
        FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:")
      crexx_register_test_prep_targets(
        TESTS ts_unicode_${_unicode_core_fixture}_${_unicode_core_test_runner}_${_unicode_core_test_mode}
        TARGETS ts_unicode_${_unicode_core_fixture}_${_unicode_core_test_mode} ${_unicode_core_test_runner})
      add_dependencies(ts_unicode_${_unicode_core_fixture} ${_unicode_core_test_runner})
    endforeach()
  endforeach()
endforeach()

# A separate compatibility control reads compiled metadata; public B runtime
# fixtures above still prepare/link only the minimal library and B core.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME unicode_public_metadata_contract
  COMMAND ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-public-metadata.py"
    "$<TARGET_FILE:rxdas>" "${RXUNICODE_CORE_RXBIN}"
    "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin")
set_tests_properties(unicode_public_metadata_contract PROPERTIES
  LABELS "library;unicode;levelb;contract" TIMEOUT 120
  PASS_REGULAR_EXPRESSION "UNICODE_PUBLIC_METADATA[|]PASS")
crexx_register_test_prep_targets(TESTS unicode_public_metadata_contract
  TARGETS rxdas rxunicode_core rxfnsg)

# Retained isolated timing entrypoint; it is not part of ALL.
add_custom_target(unicode_classic_provider_measure
  COMMAND ${CMAKE_COMMAND} -E time ${Python3_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-classic-provider.py"
    --bin-root "${CMAKE_BINARY_DIR}/bin" --vm rxbvm --mode opt
  DEPENDS rxc-c rxas rxlink rxdas library classlib rxfnsc rxunicode_core
    rxfnsg fs cstream rxbvm
  VERBATIM USES_TERMINAL)

# Fourteen nested commands: Debug 8s, maintained Apple ASan 17s. Serialize
# the workload and keep a generous hang guard rather than a speed requirement.
foreach(_unicode_provider_mode IN ITEMS opt noopt)
  foreach(_unicode_provider_vm IN LISTS _unicode_core_test_runners)
    set(_unicode_provider_test
      "unicode_classic_provider_${_unicode_provider_vm}_${_unicode_provider_mode}")
    add_test(NAME ${_unicode_provider_test}
      COMMAND ${Python3_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-classic-provider.py"
        --bin-root "${CMAKE_BINARY_DIR}/bin"
        --vm ${_unicode_provider_vm} --mode ${_unicode_provider_mode})
    set_tests_properties(${_unicode_provider_test} PROPERTIES
      LABELS "library;unicode;levelc;optimizer-parity;${_unicode_provider_vm}"
      RUN_SERIAL TRUE TIMEOUT 900
      PASS_REGULAR_EXPRESSION "PASS: Classic Unicode providers")
    crexx_register_test_prep_targets(TESTS ${_unicode_provider_test}
      TARGETS rxc-c rxas rxlink rxdas library classlib rxfnsc rxunicode_core
        rxfnsg fs cstream ${_unicode_provider_vm})
  endforeach()
endforeach()

# Nineteen nested configure/build commands, using the actual staging rule and
# stand-in files. Debug/maintained-ASan context: Make 4.3/3.9s, Ninja 1.7/1.8s.
# These are build-graph checks, not execution of instrumented cREXX code.
set(_unicode_import_generators)
if(UNIX)
  find_program(_unicode_import_make NAMES gmake make)
  if(_unicode_import_make)
    list(APPEND _unicode_import_generators "Unix Makefiles")
  endif()
endif()
find_program(_unicode_import_ninja NAMES ninja ninja-build)
if(_unicode_import_ninja)
  list(APPEND _unicode_import_generators Ninja)
endif()
add_custom_target(unicode_classic_import_build_measure)
foreach(_unicode_import_generator IN LISTS _unicode_import_generators)
  if(_unicode_import_generator STREQUAL "Unix Makefiles")
    set(_unicode_import_suffix make)
  else()
    set(_unicode_import_suffix ninja)
  endif()
  add_custom_command(TARGET unicode_classic_import_build_measure POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E time ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-classic-import-build.py"
      --cmake "${CMAKE_COMMAND}" --generator "${_unicode_import_generator}"
    VERBATIM USES_TERMINAL)
  set(_unicode_import_test "unicode_classic_import_build_${_unicode_import_suffix}")
  add_test(NAME ${_unicode_import_test}
    COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-classic-import-build.py"
      --cmake "${CMAKE_COMMAND}" --generator "${_unicode_import_generator}")
  set_tests_properties(${_unicode_import_test} PROPERTIES
    LABELS "library;unicode;levelc;build-graph"
    RUN_SERIAL TRUE TIMEOUT 600
    PASS_REGULAR_EXPRESSION "PASS: Classic import staging")
endforeach()
