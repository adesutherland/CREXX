cmake_minimum_required(VERSION 3.24)

project(test_rxfnsg)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

set(REXXBINS
  ts_llm_ollama
  ts_llm_providers
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(RXFNSG_RPATH ${CMAKE_BINARY_DIR}/bin/rxfnsg)

add_custom_target(testrxfnsg ALL)

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
    DEPENDS library classlib rxfnsg "${_rexx_file}" ${_prev_target}
    RUNTIME_ARGS ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib ${RXFNSG_RPATH}
  )
  set(_prev_target "${_basename}_noopt_artifact" "${_basename}_opt_artifact")
endforeach()

set_tests_properties(ts_llm_ollama_noopt ts_llm_ollama_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxfnsg llm ollama helpers")
set_tests_properties(ts_llm_providers_noopt ts_llm_providers_opt
  PROPERTIES PASS_REGULAR_EXPRESSION "PASS: rxfnsg llm provider helpers")

crexx_label_smoke_tests(
        ts_llm_providers_noopt
)

add_custom_target(ts_http_pooled)
add_dependencies(testrxfnsg ts_http_pooled)

set(_http_pooled_runners rxbvm)
if(TARGET rxtvm)
  list(APPEND _http_pooled_runners rxtvm)
endif()

foreach(_http_runner IN LISTS _http_pooled_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "ts_http_pooled_${_http_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_pooled.crexx"
    TARGET_GROUP ts_http_pooled
    RUNNER "${_http_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib rxfnsg "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_pooled.crexx"
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
    DEPENDS library classlib rxfnsg "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_streaming.crexx"
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
    DEPENDS library classlib rxfnsg "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_crexx_rag.crexx"
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
    DEPENDS library classlib rxfnsg "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_tls_live.crexx"
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
    DEPENDS library classlib rxfnsg "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_policy.crexx"
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
endforeach()

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
    DEPENDS library classlib rxfnsg "${CMAKE_CURRENT_SOURCE_DIR}/ts_http_codec.crexx"
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
endforeach()
