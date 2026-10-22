cmake_minimum_required(VERSION 3.24)

# The language tool is a downstream prerequisite, never a runtime dependency.
if(NOT TARGET levell_unicode_lexer)
  include("${CMAKE_SOURCE_DIR}/tools/unicode/LevelL.cmake")
endif()

set(RXUNICODE_CASEFOLD_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/CaseFolding.txt")
set(RXUNICODE_GRAPHEME_PROPERTY_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/auxiliary/GraphemeBreakProperty.txt")
set(RXUNICODE_GRAPHEME_TEST_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/auxiliary/GraphemeBreakTest.txt")
set(RXUNICODE_EMOJI_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/emoji/emoji-data.txt")
set(RXUNICODE_DERIVED_CORE_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/DerivedCoreProperties.txt")
set(RXUNICODE_UNICODE_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/UnicodeData.txt")
set(RXUNICODE_NORMALIZATION_PROPERTIES_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/DerivedNormalizationProps.txt")
set(RXUNICODE_NORMALIZATION_TEST_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/NormalizationTest.txt")
set(RXUNICODE_SPECIAL_CASING_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/SpecialCasing.txt")
set(RXUNICODE_WINDOWS1252_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/legacy/CP1252-2.0.0.txt")
set(RXUNICODE_IBM437_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/legacy/CP437-2.0.0.txt")
set(RXUNICODE_IBM850_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/legacy/CP850-2.0.0.txt")
set(RXUNICODE_IBM1047_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/legacy/IBM1047.txt")
set(RXUNICODE_PROPERTY_LIST_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/PropList.txt")

set(_rxunicode_runtime_inputs
  "${RXUNICODE_CASEFOLD_DATA}" "${RXUNICODE_GRAPHEME_PROPERTY_DATA}"
  "${RXUNICODE_EMOJI_DATA}" "${RXUNICODE_DERIVED_CORE_DATA}"
  "${RXUNICODE_UNICODE_DATA}" "${RXUNICODE_NORMALIZATION_PROPERTIES_DATA}"
  "${RXUNICODE_SPECIAL_CASING_DATA}" "${RXUNICODE_WINDOWS1252_DATA}"
  "${RXUNICODE_IBM437_DATA}" "${RXUNICODE_IBM850_DATA}" "${RXUNICODE_IBM1047_DATA}")
# Changed raw data must re-enter the pinned checksum gate before generation.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  ${_rxunicode_runtime_inputs}
  "${RXUNICODE_GRAPHEME_TEST_DATA}" "${RXUNICODE_NORMALIZATION_TEST_DATA}"
  "${RXUNICODE_PROPERTY_LIST_DATA}"
  "${CMAKE_CURRENT_SOURCE_DIR}/data/unicode-17.0.0/SHA256SUMS"
  "${CMAKE_CURRENT_SOURCE_DIR}/data/legacy/SHA256SUMS")

set(_rxunicode_expected_casefold_sha256
  "ff8d8fefbf123574205085d6714c36149eb946d717a0c585c27f0f4ef58c4183")
file(SHA256 "${RXUNICODE_CASEFOLD_DATA}" _rxunicode_actual_casefold_sha256)
if(NOT "${_rxunicode_actual_casefold_sha256}" STREQUAL
       "${_rxunicode_expected_casefold_sha256}")
  message(FATAL_ERROR
    "Pinned Unicode CaseFolding.txt checksum mismatch: "
    "expected ${_rxunicode_expected_casefold_sha256}, "
    "got ${_rxunicode_actual_casefold_sha256}")
endif()

set(_rxunicode_expected_grapheme_property_sha256
  "d6b51d1d2ae5c33b451b7ed994b48f1f4dc62b2272a5831e7fd418514a6bae89")
set(_rxunicode_expected_grapheme_test_sha256
  "e2d134d2c52919bace503ebb6a551c1855fe1a1faec18478c78fff254a1793ec")
set(_rxunicode_expected_emoji_sha256
  "2cb2bb9455cda83e8481541ecf5b6dfda66a3bb89efa3fa7c5297eccf607b72b")
set(_rxunicode_expected_derived_core_sha256
  "24c7fed1195c482faaefd5c1e7eb821c5ee1fb6de07ecdbaa64b56a99da22c08")
set(_rxunicode_expected_unicode_data_sha256
  "2e1efc1dcb59c575eedf5ccae60f95229f706ee6d031835247d843c11d96470c")
set(_rxunicode_expected_normalization_properties_sha256
  "71fd6a206a2c0cdd41feb6b7f656aa31091db45e9cedc926985d718397f9e488")
set(_rxunicode_expected_normalization_test_sha256
  "5019ffd530751a741900c849c0e010332f142a3612234639bd200b82138a87db")
set(_rxunicode_expected_special_casing_sha256
  "efc25faf19de21b92c1194c111c932e03d2a5eaf18194e33f1156e96de4c9588")
set(_rxunicode_expected_windows1252_sha256
  "f607ae328b4dff5e9bfef725f5fff0ae23f38797f8a5b95998a0d2735c0e8fad")
set(_rxunicode_expected_ibm437_sha256
  "fa637cccff54421668827e36340192f5579ec33443feafbcfb039dcac747b3b2")
set(_rxunicode_expected_ibm850_sha256
  "bb9e8092a706e3fd11113f4c0ae0b184d26dc66bcd15c8652bb7f5885c68e572")
set(_rxunicode_expected_ibm1047_sha256
  "e690ca160b98ab94ab7109bb606a3be1aab2fb7a9b456c0b3ddbcae9dca217f3")
set(_rxunicode_expected_property_list_sha256
  "130dcddcaadaf071008bdfce1e7743e04fdfbc910886f017d9f9ac931d8c64dd")
foreach(_rxunicode_checksum IN ITEMS
    "${RXUNICODE_PROPERTY_LIST_DATA}|${_rxunicode_expected_property_list_sha256}|PropList.txt"
    "${RXUNICODE_GRAPHEME_PROPERTY_DATA}|${_rxunicode_expected_grapheme_property_sha256}|GraphemeBreakProperty.txt"
    "${RXUNICODE_GRAPHEME_TEST_DATA}|${_rxunicode_expected_grapheme_test_sha256}|GraphemeBreakTest.txt"
    "${RXUNICODE_EMOJI_DATA}|${_rxunicode_expected_emoji_sha256}|emoji-data.txt"
    "${RXUNICODE_DERIVED_CORE_DATA}|${_rxunicode_expected_derived_core_sha256}|DerivedCoreProperties.txt"
    "${RXUNICODE_UNICODE_DATA}|${_rxunicode_expected_unicode_data_sha256}|UnicodeData.txt"
    "${RXUNICODE_NORMALIZATION_PROPERTIES_DATA}|${_rxunicode_expected_normalization_properties_sha256}|DerivedNormalizationProps.txt"
    "${RXUNICODE_NORMALIZATION_TEST_DATA}|${_rxunicode_expected_normalization_test_sha256}|NormalizationTest.txt"
    "${RXUNICODE_SPECIAL_CASING_DATA}|${_rxunicode_expected_special_casing_sha256}|SpecialCasing.txt"
    "${RXUNICODE_WINDOWS1252_DATA}|${_rxunicode_expected_windows1252_sha256}|CP1252-2.0.0.txt"
    "${RXUNICODE_IBM437_DATA}|${_rxunicode_expected_ibm437_sha256}|CP437-2.0.0.txt"
    "${RXUNICODE_IBM850_DATA}|${_rxunicode_expected_ibm850_sha256}|CP850-2.0.0.txt"
    "${RXUNICODE_IBM1047_DATA}|${_rxunicode_expected_ibm1047_sha256}|IBM1047.txt")
  string(REPLACE "|" ";" _rxunicode_checksum_fields "${_rxunicode_checksum}")
  list(GET _rxunicode_checksum_fields 0 _rxunicode_checksum_path)
  list(GET _rxunicode_checksum_fields 1 _rxunicode_checksum_expected)
  list(GET _rxunicode_checksum_fields 2 _rxunicode_checksum_name)
  file(SHA256 "${_rxunicode_checksum_path}" _rxunicode_checksum_actual)
  if(NOT "${_rxunicode_checksum_actual}" STREQUAL
         "${_rxunicode_checksum_expected}")
    message(FATAL_ERROR
      "Pinned Unicode ${_rxunicode_checksum_name} checksum mismatch: "
      "expected ${_rxunicode_checksum_expected}, "
      "got ${_rxunicode_checksum_actual}")
  endif()
endforeach()

include("${CMAKE_CURRENT_SOURCE_DIR}/UnicodeBuilder.cmake")

add_test(
  NAME rxunicode_casefold_rxas_shape
  COMMAND ${CMAKE_COMMAND}
    -DRXAS=${RXUNICODE_CORE_MEMBERS_ROOT}/unicode_casefold/unicode_casefold.rxas
    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx/unicode_casefold.crexx.in
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_casefold_rxas.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxunicode_casefold_rxas_shape PROPERTIES
  LABELS "library;unicode;casefold;levelb;shape")

add_test(
  NAME rxunicode_grapheme_rxas_shape
  COMMAND ${CMAKE_COMMAND}
    -DRXAS=${RXUNICODE_CORE_MEMBERS_ROOT}/unicode_grapheme/unicode_grapheme.rxas
    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/rexx/unicode_grapheme.crexx.in
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_grapheme_rxas.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxunicode_grapheme_rxas_shape PROPERTIES
  LABELS "library;unicode;grapheme;levelb;shape")

foreach(_rxunicode_shape_kind IN ITEMS normalization case_mapping codec)
  add_test(
    NAME "rxunicode_${_rxunicode_shape_kind}_rxas_shape"
    COMMAND ${CMAKE_COMMAND}
      -DKIND=${_rxunicode_shape_kind}
      -DRXAS=${RXUNICODE_CORE_MEMBERS_ROOT}/unicode_${_rxunicode_shape_kind}/unicode_${_rxunicode_shape_kind}.rxas
      -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_product_rxas.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties("rxunicode_${_rxunicode_shape_kind}_rxas_shape"
    PROPERTIES
      LABELS "library;unicode;${_rxunicode_shape_kind};levelb;shape")
endforeach()

foreach(_rxunicode_shape IN ITEMS casefold grapheme normalization case_mapping codec)
  crexx_register_test_prep_targets(TESTS "rxunicode_${_rxunicode_shape}_rxas_shape"
    TARGETS rxunicode_core)
endforeach()

foreach(_rxunicode_export IN ITEMS
    RXUNICODE_CORE_RXBIN RXUNICODE_CORE_MEMBERS_ROOT RXUNICODE_CORE_GENERATED_ROOT
    RXUNICODE_CASEFOLD_TOOL_DIR RXUNICODE_CASEFOLD_TABLE_RXBIN
    RXUNICODE_GRAPHEME_TABLE_RXBIN RXUNICODE_CASE_MAPPING_TABLE_RXBIN
    RXUNICODE_NORMALIZATION_TABLE_RXBIN RXUNICODE_ENCODING_TABLE_RXBIN
    RXUNICODE_CASEFOLD_DATA RXUNICODE_GRAPHEME_PROPERTY_DATA
    RXUNICODE_GRAPHEME_TEST_DATA RXUNICODE_EMOJI_DATA RXUNICODE_DERIVED_CORE_DATA
    RXUNICODE_UNICODE_DATA RXUNICODE_NORMALIZATION_PROPERTIES_DATA
    RXUNICODE_NORMALIZATION_TEST_DATA RXUNICODE_SPECIAL_CASING_DATA
    RXUNICODE_WINDOWS1252_DATA RXUNICODE_IBM437_DATA RXUNICODE_IBM850_DATA
    RXUNICODE_IBM1047_DATA RXUNICODE_PROPERTY_LIST_DATA)
  set(${_rxunicode_export} "${${_rxunicode_export}}" PARENT_SCOPE)
endforeach()

if(BUILD_TESTING)
  include("${CMAKE_CURRENT_SOURCE_DIR}/tests/CoreTests.cmake")
endif()

crexx_install_bytecode(rxunicode_core "${RXUNICODE_CORE_RXBIN}")
crexx_finalize_directory_qa_tiers()
