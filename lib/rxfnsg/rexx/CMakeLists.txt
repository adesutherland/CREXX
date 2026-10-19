cmake_minimum_required(VERSION 3.24)

set(RXFNSG_REXX_MODULES
  integer
  decimal
  packednumeric
  unicode
  httpcodec
  httpcore
  http
  httpserver
  llm)

include(${CMAKE_CURRENT_SOURCE_DIR}/LevelGBuilder.cmake)

add_test(
  NAME rxunicode_contract_surface
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/unicode.crexx
    -DAPI_SOURCE=${CMAKE_SOURCE_DIR}/lib/rxfnsb/unicode/rexx/unicode_api.crexx
    -DDOC=${CMAKE_CURRENT_SOURCE_DIR}/unicode.md
    -DUSER_DOC=${CMAKE_SOURCE_DIR}/docs/books/crexx_library_reference/unicode.md
    -DNORMALIZATION_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_normalization.crexx
    -DCASE_MAPPING_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_case_mapping.crexx
    -DCASEFOLD_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_casefold.crexx
    -DGRAPHEME_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_grapheme.crexx
    -DCODEC_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_codec.crexx
    -P ${CMAKE_CURRENT_SOURCE_DIR}/../unicode/check_casefold_contract.cmake)
set_tests_properties(rxunicode_contract_surface PROPERTIES
  LABELS "library;unicode;levelg;contract")

add_test(
  NAME rxfnsg_rxdas_inline_preserve_smoke
  COMMAND ${CMAKE_COMMAND}
    -DRXDAS=$<TARGET_FILE:rxdas>
    -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
    -DINPUT_RXBIN=${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin
    -DOUTPUT_RXAS=${CMAKE_CURRENT_BINARY_DIR}/rxfnsg_inline_check.rxas
    "-DEXPECT_TEXT_1=.meta \"rxfnsg.ollama.status\"=\".inline\""
    "-DEXPECT_TEXT_2=.meta \"rxfnsg.openai.status\"=\".inline\""
    "-DEXPECT_TEXT_3=.const §rxdas.const."
    # rxdas renders each shared Unicode binary constant as hexadecimal. The
    # five product tables produce about 40 MB of bounded text even though each
    # table remains a single linked constant-pool item.
    -DMAX_OUTPUT_BYTES=50000000
    -P ${CMAKE_SOURCE_DIR}/linker/tests/check_rxdas_contains.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxfnsg_rxdas_inline_preserve_smoke PROPERTIES
  LABELS "inline;library")

crexx_finalize_directory_qa_tiers()
