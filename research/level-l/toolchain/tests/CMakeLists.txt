cmake_minimum_required(VERSION 3.24)
if(NOT BUILD_TESTING)
  return()
endif()

set(_runtime "${CMAKE_BINARY_DIR}/bin/library.rxbin"
  "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin"
  "${CMAKE_BINARY_DIR}/bin/rxfnsl-rxas.rxbin"
  "${CMAKE_BINARY_DIR}/bin/rxfnsl-target.rxbin"
  "${CMAKE_BINARY_DIR}/bin/rxfnsl-target-io.rxbin"
  "${CMAKE_BINARY_DIR}/langtools/common/llrxastext.rxbin"
  "${CMAKE_BINARY_DIR}/langtools/common/llfileoutput.rxbin"
  "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
crexx_add_import_root(levell_rxas_io_test_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports" OUTPUT_VARIABLE _imports
  FILES ${_runtime}
  DEPENDS library rxfnsl rxfnsl_analysis rxfnsl_rxas rxfnsl_target
    rxfnsl_target_io llrxastext llfileoutput fs)
crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/io-service.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_CURRENT_BINARY_DIR}/imports"
    -o "${CMAKE_CURRENT_BINARY_DIR}/io-service"
    "${CMAKE_CURRENT_SOURCE_DIR}/io-service.crexx"
  COMMAND $<TARGET_FILE:rxas> "${CMAKE_CURRENT_BINARY_DIR}/io-service"
  DEPENDS rxc rxas levell_rxas_io_test_imports ${_imports}
    "${CMAKE_CURRENT_SOURCE_DIR}/io-service.crexx" VERBATIM)
crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/io-generated.rxas"
  COMMAND $<TARGET_FILE:rxc> -n -x --no-exe-import
    -i "${CMAKE_CURRENT_BINARY_DIR}/imports"
    -o "${CMAKE_CURRENT_BINARY_DIR}/io-generated"
    "${CMAKE_CURRENT_SOURCE_DIR}/io-generated.crexx"
  DEPENDS rxc levell_rxas_io_test_imports ${_imports}
    "${CMAKE_CURRENT_SOURCE_DIR}/io-generated.crexx" VERBATIM)
add_custom_target(levell_rxas_io_test_prep DEPENDS
  "${CMAKE_CURRENT_BINARY_DIR}/io-service.rxbin"
  "${CMAKE_CURRENT_BINARY_DIR}/io-generated.rxas")
add_dependencies(levell_rxas_io_test_prep llrxinline)
crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/signal-inline.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_CURRENT_BINARY_DIR}/imports"
    -o "${CMAKE_CURRENT_BINARY_DIR}/signal-inline"
    "${CMAKE_CURRENT_SOURCE_DIR}/signal-inline.crexx"
  COMMAND $<TARGET_FILE:rxas> "${CMAKE_CURRENT_BINARY_DIR}/signal-inline"
  DEPENDS rxc rxas levell_rxas_io_test_imports ${_imports}
    "${CMAKE_CURRENT_SOURCE_DIR}/signal-inline.crexx" VERBATIM)
add_custom_target(levell_rxas_signal_test_prep DEPENDS
  "${CMAKE_CURRENT_BINARY_DIR}/signal-inline.rxbin")
add_dependencies(levell_rxas_signal_test_prep llrxinline)
if(TARGET rxbvm)
  get_target_property(_cli llrxinline LLRXINLINE_BYTECODE)
  add_test(NAME levell_rxas_named_signal
    COMMAND ${CMAKE_COMMAND}
      "-DVM=$<TARGET_FILE:rxbvm>" "-DASSEMBLER=$<TARGET_FILE:rxas>"
      "-DCLI=${_cli}" "-DCONSTRUCTED=${CMAKE_CURRENT_BINARY_DIR}/signal-inline.rxbin"
      "-DRUNTIME=${_runtime}" "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/named-signal-work"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/run-named-signal.cmake")
  set_tests_properties(levell_rxas_named_signal PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_NAMED_SIGNAL[|]failures=0"
    TIMEOUT 180 LABELS "level-l;research;inlining;rxas")
  crexx_register_test_prep_targets(TESTS levell_rxas_named_signal
    TARGETS levell_rxas_signal_test_prep rxbvm rxas)
  add_test(NAME levell_rxas_io_service
    COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/io-service.rxbin" ${_runtime})
  set_tests_properties(levell_rxas_io_service PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_IO[|]checks=[0-9]+[|]failures=0"
    FAIL_REGULAR_EXPRESSION "FAIL[|]|PANIC:"
    TIMEOUT 120 LABELS "level-l;research;inlining;rxas")
  crexx_register_test_prep_targets(TESTS levell_rxas_io_service
    TARGETS levell_rxas_io_test_prep rxbvm)
  add_test(NAME levell_rxas_io_cli
    COMMAND ${CMAKE_COMMAND}
      "-DVM=$<TARGET_FILE:rxbvm>" "-DASSEMBLER=$<TARGET_FILE:rxas>"
      "-DDISASSEMBLER=$<TARGET_FILE:rxdas>"
      "-DCLI=${_cli}"
      "-DRUNTIME=${_runtime}"
      "-DGENERATED=${CMAKE_CURRENT_BINARY_DIR}/io-generated.rxas"
      "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DOPTIMIZER_FIXTURES=${CMAKE_SOURCE_DIR}/tests/rxas_optimizer"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/io-cli-work"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/run-io-cli.cmake")
  set_tests_properties(levell_rxas_io_cli PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_IO_CLI[|]failures=0"
    TIMEOUT 240 LABELS "level-l;research;inlining;rxas")
  crexx_register_test_prep_targets(TESTS levell_rxas_io_cli
    TARGETS levell_rxas_io_test_prep rxbvm rxas rxdas)
  add_test(NAME levell_rxas_linked_window
    COMMAND ${CMAKE_COMMAND}
      "-DVM=$<TARGET_FILE:rxbvm>" "-DASSEMBLER=$<TARGET_FILE:rxas>"
      "-DCLI=${_cli}" "-DRUNTIME=${_runtime}"
      "-DFIXTURE=${CMAKE_CURRENT_SOURCE_DIR}/io-linked-window.rxas"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/linked-window-work"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/run-linked-window.cmake")
  set_tests_properties(levell_rxas_linked_window PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_LINKED_WINDOW[|]failures=0"
    TIMEOUT 180 LABELS "level-l;research;inlining;rxas")
  crexx_register_test_prep_targets(TESTS levell_rxas_linked_window
    TARGETS llrxinline rxbvm rxas)
  add_test(NAME levell_rxas_imported_leaf
    COMMAND ${CMAKE_COMMAND}
      "-DVM=$<TARGET_FILE:rxbvm>" "-DASSEMBLER=$<TARGET_FILE:rxas>"
      "-DLINKER=$<TARGET_FILE:rxlink>" "-DCLI=${_cli}" "-DRUNTIME=${_runtime}"
      "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/import-leaf-work"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/run-import-leaf.cmake")
  set_tests_properties(levell_rxas_imported_leaf PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_IMPORTED_LEAF[|]failures=0"
    TIMEOUT 180 LABELS "level-l;research;inlining;rxas")
  crexx_register_test_prep_targets(TESTS levell_rxas_imported_leaf
    TARGETS llrxinline rxbvm rxas rxlink)
  add_test(NAME levell_rxas_imported_private
    COMMAND ${CMAKE_COMMAND}
      "-DVM=$<TARGET_FILE:rxbvm>" "-DASSEMBLER=$<TARGET_FILE:rxas>"
      "-DLINKER=$<TARGET_FILE:rxlink>" "-DCLI=${_cli}" "-DRUNTIME=${_runtime}"
      "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/import-private-work"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/run-import-private.cmake")
  set_tests_properties(levell_rxas_imported_private PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_IMPORTED_PRIVATE[|]failures=0"
    TIMEOUT 180 LABELS "level-l;research;inlining;rxas")
  crexx_register_test_prep_targets(TESTS levell_rxas_imported_private
    TARGETS llrxinline rxbvm rxas rxlink)
  add_test(NAME levell_rxas_imported_dag
    COMMAND ${CMAKE_COMMAND}
      "-DVM=$<TARGET_FILE:rxbvm>" "-DASSEMBLER=$<TARGET_FILE:rxas>"
      "-DLINKER=$<TARGET_FILE:rxlink>" "-DCLI=${_cli}" "-DRUNTIME=${_runtime}"
      "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/import-dag-work"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/run-import-dag.cmake")
  set_tests_properties(levell_rxas_imported_dag PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_IMPORTED_DAG[|]failures=0"
    TIMEOUT 180 LABELS "level-l;research;inlining;rxas")
  crexx_register_test_prep_targets(TESTS levell_rxas_imported_dag
    TARGETS llrxinline rxbvm rxas rxlink)
  add_test(NAME levell_rxas_imported_nested
    COMMAND ${CMAKE_COMMAND}
      "-DVM=$<TARGET_FILE:rxbvm>" "-DASSEMBLER=$<TARGET_FILE:rxas>"
      "-DLINKER=$<TARGET_FILE:rxlink>" "-DCLI=${_cli}" "-DRUNTIME=${_runtime}"
      "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/import-nested-work"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/run-import-nested.cmake")
  set_tests_properties(levell_rxas_imported_nested PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_IMPORTED_NESTED[|]failures=0"
    TIMEOUT 240 LABELS "level-l;research;inlining;rxas")
  crexx_register_test_prep_targets(TESTS levell_rxas_imported_nested
    TARGETS llrxinline rxbvm rxas rxlink)
  add_test(NAME levell_rxas_imported_global
    COMMAND ${CMAKE_COMMAND}
      "-DVM=$<TARGET_FILE:rxbvm>" "-DASSEMBLER=$<TARGET_FILE:rxas>"
      "-DLINKER=$<TARGET_FILE:rxlink>" "-DCLI=${_cli}" "-DRUNTIME=${_runtime}"
      "-DFIXTURES=${CMAKE_CURRENT_SOURCE_DIR}"
      "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/import-global-work"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/run-import-global.cmake")
  set_tests_properties(levell_rxas_imported_global PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_IMPORTED_GLOBAL[|]failures=0"
    TIMEOUT 240 LABELS "level-l;research;inlining;rxas")
  crexx_register_test_prep_targets(TESTS levell_rxas_imported_global
    TARGETS llrxinline rxbvm rxas rxlink)
endif()
