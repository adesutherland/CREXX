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
)
crexx_add_import_root(levell_rxas_io_test_imports
  DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports" OUTPUT_VARIABLE _imports
  FILES ${_runtime}
  DEPENDS library rxfnsl rxfnsl_analysis rxfnsl_rxas rxfnsl_target
    rxfnsl_target_io llrxastext)
crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/io-service.rxbin"
  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
    -i "${CMAKE_CURRENT_BINARY_DIR}/imports"
    -o "${CMAKE_CURRENT_BINARY_DIR}/io-service"
    "${CMAKE_CURRENT_SOURCE_DIR}/io-service.crexx"
  COMMAND $<TARGET_FILE:rxas> "${CMAKE_CURRENT_BINARY_DIR}/io-service"
  DEPENDS rxc rxas levell_rxas_io_test_imports ${_imports}
    "${CMAKE_CURRENT_SOURCE_DIR}/io-service.crexx" VERBATIM)
add_custom_target(levell_rxas_io_test_prep
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/io-service.rxbin")
if(TARGET rxbvm)
  add_test(NAME levell_rxas_io_service
    COMMAND $<TARGET_FILE:rxbvm> "${CMAKE_CURRENT_BINARY_DIR}/io-service.rxbin" ${_runtime})
  set_tests_properties(levell_rxas_io_service PROPERTIES
    PASS_REGULAR_EXPRESSION "RXAS_IO[|].*failures=0"
    FAIL_REGULAR_EXPRESSION "FAIL:|PANIC:"
    TIMEOUT 120 LABELS "level-l;research;target-foundation;rxas-io")
  crexx_register_test_prep_targets(TESTS levell_rxas_io_service
    TARGETS levell_rxas_io_test_prep rxbvm)
endif()

crexx_finalize_directory_qa_tiers()
