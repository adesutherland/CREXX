cmake_minimum_required(VERSION 3.24)

project(test_rxfnsg)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxConcurrencyTests.cmake)

set(REXXBINS
  ts_http_service_contract
  ts_llm_ollama
  ts_llm_providers
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(RXFNSG_RPATH ${CMAKE_BINARY_DIR}/bin/rxfnsg)

add_custom_target(testrxfnsg ALL)

add_custom_target(ts_math_numeric)
add_dependencies(testrxfnsg ts_math_numeric)

set(_numeric_test_support_base
  "${CMAKE_CURRENT_BINARY_DIR}/numeric_test_support")
set(_numeric_test_support_rxbin
  "${_numeric_test_support_base}.rxbin")
set(_numeric_test_support_source
  "${CMAKE_SOURCE_DIR}/tests/support/numeric_test_support.crexx")
add_custom_command(
  OUTPUT "${_numeric_test_support_rxbin}"
  BYPRODUCTS "${_numeric_test_support_base}.rxas"
  COMMAND $<TARGET_FILE:rxc> -n
    -i "${CMAKE_BINARY_DIR}/bin"
    -o numeric_test_support
    "${_numeric_test_support_source}"
  COMMAND $<TARGET_FILE:rxas> -n
    -o numeric_test_support numeric_test_support
  DEPENDS rxc rxas compiler_exit_bin library
    "${_numeric_test_support_source}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
add_custom_target(rxfnsg_numeric_test_support
  DEPENDS "${_numeric_test_support_rxbin}")
add_dependencies(ts_math_numeric rxfnsg_numeric_test_support)

set(_math_numeric_runners rxbvm)
if(TARGET rxtvm)
  list(APPEND _math_numeric_runners rxtvm)
endif()

foreach(_math_numeric_runner IN LISTS _math_numeric_runners)
  crexx_add_rexx_opt_matrix(
    NAME "ts_math_numeric_${_math_numeric_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_math_numeric.crexx"
    TARGET_GROUP ts_math_numeric
    RUNNER "${_math_numeric_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x -s ${CMAKE_SOURCE_DIR}/tests/support
    DEPENDS library classlib rxfnsg rxfnsg_numeric_test_support
      "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin"
      "${CMAKE_CURRENT_SOURCE_DIR}/ts_math_numeric.crexx"
    RUNTIME_ARGS ${_numeric_test_support_base} ${LIBRPATH}
      ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS "library;math;integer;decimal;levelg;${_math_numeric_runner}"
    REQUIRED_OUTPUT_REGEX "PASS: RCC-5C integer and decimal algorithms"
    FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
endforeach()

add_test(
  NAME rcc5c_math_contract_surface_coverage
  COMMAND ${CMAKE_COMMAND}
    -DINTEGER_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/../rexx/integer.crexx
    -DINTEGER_CONTRACT=${CMAKE_CURRENT_SOURCE_DIR}/ts_math_integer_contract.crexx
    -DDECIMAL_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/../rexx/decimal.crexx
    -DDECIMAL_CONTRACT=${CMAKE_CURRENT_SOURCE_DIR}/ts_math_decimal_contract.crexx
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check_math_contract_surface.cmake)
set_tests_properties(rcc5c_math_contract_surface_coverage PROPERTIES
  LABELS "library;math;integer;decimal;levelg;contract")

foreach(_math_contract_suite IN ITEMS integer decimal)
  set(_math_contract_target "ts_math_${_math_contract_suite}_contract")
  set(_math_contract_source
    "${CMAKE_CURRENT_SOURCE_DIR}/${_math_contract_target}.crexx")
  if(_math_contract_suite STREQUAL "integer")
    set(_math_contract_output "PASS: RCC-5C exact integer contract")
  else()
    set(_math_contract_output "PASS: RCC-5C decimal mathematics contract")
  endif()

  add_custom_target("${_math_contract_target}")
  add_dependencies(testrxfnsg "${_math_contract_target}")
  add_dependencies("${_math_contract_target}" rxfnsg_numeric_test_support)

  foreach(_math_contract_runner IN LISTS _math_numeric_runners)
    crexx_add_rexx_opt_matrix(
      NAME "${_math_contract_target}_${_math_contract_runner}"
      SOURCE "${_math_contract_source}"
      TARGET_GROUP "${_math_contract_target}"
      RUNNER "${_math_contract_runner}"
      IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
      RXC_FLAGS -x -s ${CMAKE_SOURCE_DIR}/tests/support
      DEPENDS library classlib rxfnsg rxfnsg_numeric_test_support
        "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin"
        "${_math_contract_source}"
      RUNTIME_ARGS ${_numeric_test_support_base} ${LIBRPATH}
        ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
      TEST_LABELS
        "library;math;${_math_contract_suite};levelg;contract;${_math_contract_runner}"
      REQUIRED_OUTPUT_REGEX "${_math_contract_output}"
      FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
  endforeach()
endforeach()

add_custom_target(ts_packed_numeric)
add_dependencies(testrxfnsg ts_packed_numeric)

foreach(_packed_numeric_runner IN LISTS _math_numeric_runners)
  crexx_add_rexx_opt_matrix(
    NAME "ts_packed_numeric_${_packed_numeric_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_packed_numeric.crexx"
    TARGET_GROUP ts_packed_numeric
    RUNNER "${_packed_numeric_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg
      "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin"
      "${CMAKE_CURRENT_SOURCE_DIR}/ts_packed_numeric.crexx"
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS
      "library;binary;packed;binary01;levelg;${_packed_numeric_runner}"
    REQUIRED_OUTPUT_REGEX "PASS: Level G packedfloat and packedint contract"
    FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
endforeach()

unset(_prev_target)
foreach(_binfile IN LISTS REXXBINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")

  add_custom_target("${_basename}")
  add_dependencies(testrxfnsg "${_basename}")
  crexx_add_rexx_opt_matrix(
    NAME "${_basename}"
    SOURCE "${_rexx_file}"
    TARGET_GROUP "${_basename}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin" "${_rexx_file}" ${_prev_target}
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
  )
  set(_prev_target "${_basename}_noopt_artifact" "${_basename}_opt_artifact")
endforeach()

set_tests_properties(ts_llm_ollama_noopt ts_llm_ollama_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxfnsg llm ollama helpers")
set_tests_properties(ts_llm_providers_noopt ts_llm_providers_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxfnsg llm provider helpers")
set_tests_properties(ts_http_service_contract_noopt ts_http_service_contract_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: Level G HTTP service interface supplies the canonical dispatch adapter")
crexx_label_concurrency_tests(SP07
  ts_http_service_contract_noopt
  ts_http_service_contract_opt
  ts_llm_ollama_noopt
  ts_llm_ollama_opt
  ts_llm_providers_noopt
  ts_llm_providers_opt)

crexx_label_smoke_tests(
        ts_llm_providers_noopt
)

add_custom_target(ts_http_pooled)
add_dependencies(testrxfnsg ts_http_pooled)

set(_http_pooled_runners rxbvm)
if(TARGET rxtvm)
  list(APPEND _http_pooled_runners rxtvm)
endif()

add_custom_target(ts_http_server)
add_dependencies(testrxfnsg ts_http_server)

add_custom_target(ts_http_server_failures)
add_dependencies(testrxfnsg ts_http_server_failures)

foreach(_http_runner IN LISTS _http_pooled_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "ts_http_server_${_http_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_server.crexx"
    TARGET_GROUP ts_http_server
    RUNNER "${_http_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin" "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_server.crexx"
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS concurrency http server levelg library "${_http_runner}"
  )
  set_tests_properties(
    "ts_http_server_${_http_runner}_noopt"
    "ts_http_server_${_http_runner}_opt"
    PROPERTIES
      PASS_REGULAR_EXPRESSION "PASS: Level G HTTP server dispatched two bounded requests to task handlers"
      # Each case creates request, client-owner and server-handler worker pools.
      # Keep that topology isolated from compiler/runtime tests which launch
      # their own child processes, while preserving parallel clients inside
      # the tested VM.
      RUN_SERIAL TRUE
      TIMEOUT 90
  )
  crexx_label_concurrency_tests(SP07
    "ts_http_server_${_http_runner}_noopt"
    "ts_http_server_${_http_runner}_opt")
  crexx_label_concurrency_tests(SP08
    "ts_http_server_${_http_runner}_noopt"
    "ts_http_server_${_http_runner}_opt")
endforeach()

foreach(_http_runner IN LISTS _http_pooled_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "ts_http_server_failures_${_http_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_server_failures.crexx"
    TARGET_GROUP ts_http_server_failures
    RUNNER "${_http_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin" "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_server_failures.crexx"
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS concurrency http server failure levelg library "${_http_runner}"
  )
  set_tests_properties(
    "ts_http_server_failures_${_http_runner}_noopt"
    "ts_http_server_failures_${_http_runner}_opt"
    PROPERTIES
      PASS_REGULAR_EXPRESSION "PASS: Level G HTTP server bounds unusable and failed handlers"
      # Keep the negative server topology under the same CTest isolation as the
      # positive server integration cases above.
      RUN_SERIAL TRUE
      TIMEOUT 90
  )
  crexx_label_concurrency_tests(SP07
    "ts_http_server_failures_${_http_runner}_noopt"
    "ts_http_server_failures_${_http_runner}_opt")
  crexx_label_concurrency_tests(SP08
    "ts_http_server_failures_${_http_runner}_noopt"
    "ts_http_server_failures_${_http_runner}_opt")
endforeach()

foreach(_http_runner IN LISTS _http_pooled_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "ts_http_pooled_${_http_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_pooled.crexx"
    TARGET_GROUP ts_http_pooled
    RUNNER "${_http_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin" "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_pooled.crexx"
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS gate_f f1g http levelg library "${_http_runner}"
  )
  set_tests_properties(
    "ts_http_pooled_${_http_runner}_noopt"
    "ts_http_pooled_${_http_runner}_opt"
    PROPERTIES
      PASS_REGULAR_EXPRESSION "PASS: rxfnsg pooled HTTP reused one owned connection for four task requests"
      TIMEOUT 90
  )
  crexx_label_concurrency_tests(SP07
    "ts_http_pooled_${_http_runner}_noopt"
    "ts_http_pooled_${_http_runner}_opt")
endforeach()

add_custom_target(ts_http_streaming)
add_dependencies(testrxfnsg ts_http_streaming)

foreach(_http_runner IN LISTS _http_pooled_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "ts_http_streaming_${_http_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_streaming.crexx"
    TARGET_GROUP ts_http_streaming
    RUNNER "${_http_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin" "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_streaming.crexx"
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS gate_f f1g http streaming levelg library "${_http_runner}"
  )
  set_tests_properties(
    "ts_http_streaming_${_http_runner}_noopt"
    "ts_http_streaming_${_http_runner}_opt"
    PROPERTIES
      PASS_REGULAR_EXPRESSION "PASS: rxfnsg HTTP fixed and chunked request response streaming with bounded backpressure"
      TIMEOUT 120
  )
  crexx_label_concurrency_tests(SP07
    "ts_http_streaming_${_http_runner}_noopt"
    "ts_http_streaming_${_http_runner}_opt")
  crexx_label_concurrency_tests(SP08
    "ts_http_streaming_${_http_runner}_noopt"
    "ts_http_streaming_${_http_runner}_opt")
endforeach()

add_custom_target(ts_http_crexx_rag)
add_dependencies(testrxfnsg ts_http_crexx_rag)

foreach(_http_runner IN LISTS _http_pooled_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "ts_http_crexx_rag_${_http_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_crexx_rag.crexx"
    TARGET_GROUP ts_http_crexx_rag
    RUNNER "${_http_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin" "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_crexx_rag.crexx"
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS gate_f f1g http compression crexx_rag levelg library "${_http_runner}"
  )
  set_tests_properties(
    "ts_http_crexx_rag_${_http_runner}_noopt"
    "ts_http_crexx_rag_${_http_runner}_opt"
    PROPERTIES
      PASS_REGULAR_EXPRESSION "PASS: rxfnsg concurrent crexx-rag generation embedding gzip deflate integration"
      TIMEOUT 120
  )
  crexx_label_concurrency_tests(SP07
    "ts_http_crexx_rag_${_http_runner}_noopt"
    "ts_http_crexx_rag_${_http_runner}_opt")
endforeach()

add_custom_target(ts_http_tls_live)
add_dependencies(testrxfnsg ts_http_tls_live)

foreach(_http_runner IN LISTS _http_pooled_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "ts_http_tls_live_${_http_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_tls_live.crexx"
    TARGET_GROUP ts_http_tls_live
    RUNNER "${_http_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin" "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_tls_live.crexx"
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS gate_f f1g http tls levelg library "${_http_runner}"
  )
  set_tests_properties(
    "ts_http_tls_live_${_http_runner}_noopt"
    "ts_http_tls_live_${_http_runner}_opt"
    PROPERTIES
      PASS_REGULAR_EXPRESSION "PASS: rxfnsg HTTP TLS"
      TIMEOUT 180
  )
  crexx_label_concurrency_tests(SP07
    "ts_http_tls_live_${_http_runner}_noopt"
    "ts_http_tls_live_${_http_runner}_opt")
endforeach()

add_custom_target(ts_http_policy)
add_dependencies(testrxfnsg ts_http_policy)

foreach(_http_runner IN LISTS _http_pooled_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "ts_http_policy_${_http_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_policy.crexx"
    TARGET_GROUP ts_http_policy
    RUNNER "${_http_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin" "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_policy.crexx"
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS gate_f f1g http policy levelg library "${_http_runner}"
  )
  set_tests_properties(
    "ts_http_policy_${_http_runner}_noopt"
    "ts_http_policy_${_http_runner}_opt"
    PROPERTIES
      PASS_REGULAR_EXPRESSION "PASS: rxfnsg HTTP policy headers redirects retries ambiguity deadlines and limits"
      TIMEOUT 120
  )
  crexx_label_concurrency_tests(SP07
    "ts_http_policy_${_http_runner}_noopt"
    "ts_http_policy_${_http_runner}_opt")
  crexx_label_concurrency_tests(SP08
    "ts_http_policy_${_http_runner}_noopt"
    "ts_http_policy_${_http_runner}_opt")
endforeach()

add_test(
  NAME ts_http_policy_skipped_copy_reassignment_shape
  COMMAND ${CMAKE_COMMAND}
          -DRXAS_SOURCE=${CMAKE_CURRENT_BINARY_DIR}/ts_http_policy_rxtvm_opt.rxas
          -P ${CMAKE_SOURCE_DIR}/cmake/CheckFlowSkippedCopyReassignment.cmake
)
set_tests_properties(
  ts_http_policy_skipped_copy_reassignment_shape
  PROPERTIES
    LABELS "compiler;flow;regression;http;linked_opt"
)

add_custom_target(ts_http_codec)
add_dependencies(testrxfnsg ts_http_codec)

foreach(_http_runner IN LISTS _http_pooled_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "ts_http_codec_${_http_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_codec.crexx"
    TARGET_GROUP ts_http_codec
    RUNNER "${_http_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin" "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_codec.crexx"
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
    TEST_LABELS gate_f f1g http compression levelg library "${_http_runner}"
  )
  set_tests_properties(
    "ts_http_codec_${_http_runner}_noopt"
    "ts_http_codec_${_http_runner}_opt"
    PROPERTIES
      PASS_REGULAR_EXPRESSION "PASS: rxfnsg HTTP gzip deflate bounded decoder"
      TIMEOUT 90
  )
  crexx_label_concurrency_tests(SP07
    "ts_http_codec_${_http_runner}_noopt"
    "ts_http_codec_${_http_runner}_opt")
endforeach()

crexx_finalize_directory_qa_tiers()
