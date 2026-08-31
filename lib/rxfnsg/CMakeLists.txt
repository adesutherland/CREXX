cmake_minimum_required(VERSION 3.24)

set(RXUNICODE_CASEFOLD_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/CaseFolding.txt")
set(RXUNICODE_GRAPHEME_PROPERTY_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/auxiliary/GraphemeBreakProperty.txt")
set(RXUNICODE_GRAPHEME_TEST_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/auxiliary/GraphemeBreakTest.txt")
set(RXUNICODE_EMOJI_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/emoji/emoji-data.txt")
set(RXUNICODE_DERIVED_CORE_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/DerivedCoreProperties.txt")
set(RXUNICODE_UNICODE_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/UnicodeData.txt")
set(RXUNICODE_NORMALIZATION_PROPERTIES_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/DerivedNormalizationProps.txt")
set(RXUNICODE_NORMALIZATION_TEST_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/NormalizationTest.txt")
set(RXUNICODE_SPECIAL_CASING_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/SpecialCasing.txt")
set(RXUNICODE_WINDOWS1252_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/legacy/CP1252-2.0.0.txt")
set(RXUNICODE_IBM437_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/legacy/CP437-2.0.0.txt")
set(RXUNICODE_IBM850_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/legacy/CP850-2.0.0.txt")
set(RXUNICODE_IBM1047_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/legacy/IBM1047.txt")
add_subdirectory(rexx)
add_subdirectory(tests_functional)

if(BUILD_TESTING)
  # Non-owning preparation boundary for the designed Phase 3 sanitizer slice.
  # CTest remains a separate tools/asan-run.sh phase and never builds the
  # active tree from inside its worker pool.
  add_custom_target(phase3-level-g-sanitizer-artifacts)
  add_dependencies(phase3-level-g-sanitizer-artifacts
    rxfnsg
    rxdas
    test_signal_block_levelg_bin
    test_address_after_signal_block_bin
    ts_unicode_casefold
    ts_unicode_case_mapping
    ts_unicode_grapheme
    ts_unicode_codec
    ts_unicode_normalization)
endif()
