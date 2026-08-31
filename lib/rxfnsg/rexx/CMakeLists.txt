cmake_minimum_required(VERSION 3.24)

set(RXFNSG_REXX_MODULES
        integer
        decimal
        packednumeric
        statsvalue
        unicode_casefold
        unicode_case_mapping
        unicode_codec
        unicode_grapheme
        unicode_normalization
        unicode
        httpcodec
  httpcore
  http
  httpserver
  llm
)

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
foreach(_rxunicode_checksum IN ITEMS
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

set(_rxunicode_tools_source_dir
  "${CMAKE_CURRENT_SOURCE_DIR}/../unicode/tools")
set(_rxunicode_casefold_table_source
  "${_rxunicode_tools_source_dir}/casefold_table.crexx")
set(_rxunicode_casefold_generator_source
  "${_rxunicode_tools_source_dir}/generate_casefold.crexx")
set(_rxunicode_casefold_template
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode_casefold.crexx.in")
set(_rxunicode_casefold_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/generate_casefold.rxbin")
set(_rxunicode_casefold_linked_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/casefold_generator.rxbin")
set(_rxunicode_generated_source
  "${CMAKE_CURRENT_BINARY_DIR}/unicode_casefold.crexx")
set(_rxunicode_case_mapping_table_source
  "${_rxunicode_tools_source_dir}/case_mapping_table.crexx")
set(_rxunicode_case_mapping_generator_source
  "${_rxunicode_tools_source_dir}/generate_case_mapping.crexx")
set(_rxunicode_case_mapping_template
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode_case_mapping.crexx.in")
set(_rxunicode_case_mapping_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/generate_case_mapping.rxbin")
set(_rxunicode_case_mapping_linked_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/case_mapping_generator.rxbin")
set(_rxunicode_generated_case_mapping_source
  "${CMAKE_CURRENT_BINARY_DIR}/unicode_case_mapping.crexx")
set(_rxunicode_grapheme_table_source
  "${_rxunicode_tools_source_dir}/grapheme_table.crexx")
set(_rxunicode_grapheme_generator_source
  "${_rxunicode_tools_source_dir}/generate_grapheme.crexx")
set(_rxunicode_grapheme_template
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode_grapheme.crexx.in")
set(_rxunicode_grapheme_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/generate_grapheme.rxbin")
set(_rxunicode_grapheme_linked_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/grapheme_generator.rxbin")
set(_rxunicode_generated_grapheme_source
  "${CMAKE_CURRENT_BINARY_DIR}/unicode_grapheme.crexx")
set(_rxunicode_normalization_generator_source
  "${_rxunicode_tools_source_dir}/generate_normalization.crexx")
set(_rxunicode_normalization_template
  "${_rxunicode_tools_source_dir}/unicode_d.crexx")
set(_rxunicode_normalization_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/generate_normalization.rxbin")
set(_rxunicode_normalization_linked_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/normalization_generator.rxbin")
set(_rxunicode_generated_normalization_source
  "${CMAKE_CURRENT_BINARY_DIR}/unicode_normalization.crexx")
set(_rxunicode_encoding_table_source
  "${_rxunicode_tools_source_dir}/encoding_table.crexx")
set(_rxunicode_encoding_generator_source
  "${_rxunicode_tools_source_dir}/generate_encoding.crexx")
set(_rxunicode_encoding_template
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode_codec.crexx.in")
set(_rxunicode_encoding_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/generate_encoding.rxbin")
set(_rxunicode_encoding_linked_generator_rxbin
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/encoding_generator.rxbin")
set(_rxunicode_generated_encoding_source
  "${CMAKE_CURRENT_BINARY_DIR}/unicode_codec.crexx")
file(MAKE_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}")

# Compile each Unicode build tool from an isolated source directory and expose
# only the exact RXBIN metadata providers named by its imports. rxc searches
# both source and RXBIN metadata, so a shared tools directory can otherwise
# make a stale or half-built sibling eligible during a parallel build.
function(_rxunicode_compile_tool TOOL_NAME SOURCE_FILE)
  set(_options EXITS)
  set(_one_value_args COMMENT)
  set(_multi_value_args IMPORT_FILES)
  cmake_parse_arguments(
    RXUNICODE_TOOL
    "${_options}"
    "${_one_value_args}"
    "${_multi_value_args}"
    ${ARGN})

  set(_tool_import_root
    "${CMAKE_CURRENT_BINARY_DIR}/imports/tools/${TOOL_NAME}")
  set(_tool_import_inputs
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    ${RXUNICODE_TOOL_IMPORT_FILES})
  set(_tool_import_depends library)
  set(_tool_exit_args -x)
  if(RXUNICODE_TOOL_EXITS)
    list(APPEND _tool_import_inputs
      "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin")
    list(APPEND _tool_import_depends compiler_exit_bin)
    set(_tool_exit_args)
  endif()

  set(_tool_import_target "rxunicode_${TOOL_NAME}_imports")
  crexx_add_import_root(${_tool_import_target}
    DIRECTORY "${_tool_import_root}"
    OUTPUT_VARIABLE _tool_import_files
    FILES ${_tool_import_inputs}
    DEPENDS ${_tool_import_depends}
    COMMENT "Staging deterministic imports for Unicode tool ${TOOL_NAME} ...")

  set(_tool_source_root
    "${CMAKE_CURRENT_BINARY_DIR}/sources/tools/${TOOL_NAME}")
  set(_tool_source "${_tool_source_root}/${TOOL_NAME}.crexx")
  set(_tool_rxbin "${RXUNICODE_CASEFOLD_TOOL_DIR}/${TOOL_NAME}.rxbin")
  set(_tool_comment "Compiling Unicode build tool ${TOOL_NAME} ...")
  if(RXUNICODE_TOOL_COMMENT)
    set(_tool_comment "${RXUNICODE_TOOL_COMMENT}")
  endif()

  add_custom_command(
    OUTPUT "${_tool_rxbin}"
    BYPRODUCTS
      "${_tool_source}"
      "${RXUNICODE_CASEFOLD_TOOL_DIR}/${TOOL_NAME}.rxas"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_tool_source_root}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SOURCE_FILE}"
            "${_tool_source}"
    COMMAND $<TARGET_FILE:rxc>
            ${_tool_exit_args}
            --no-exe-import
            -i "${_tool_import_root}"
            -o "${TOOL_NAME}"
            "${_tool_source}"
    COMMAND $<TARGET_FILE:rxas> "${TOOL_NAME}"
    DEPENDS
      "${SOURCE_FILE}"
      ${_tool_import_target}
      ${_tool_import_files}
      rxc
      rxas
    WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
    COMMENT "${_tool_comment}"
    VERBATIM)

  set(${TOOL_NAME}_RXBIN "${_tool_rxbin}" PARENT_SCOPE)
endfunction()

_rxunicode_compile_tool(
  level_l_gennorm2
  "${_rxunicode_tools_source_dir}/level_l_gennorm2.crexx")
_rxunicode_compile_tool(
  unicode_gennorm2
  "${_rxunicode_tools_source_dir}/unicode_gennorm2.crexx"
  IMPORT_FILES "${level_l_gennorm2_RXBIN}")
_rxunicode_compile_tool(
  unicode_data
  "${_rxunicode_tools_source_dir}/unicode_data.crexx"
  IMPORT_FILES "${unicode_gennorm2_RXBIN}")
_rxunicode_compile_tool(
  unicode_normprops
  "${_rxunicode_tools_source_dir}/unicode_normprops.crexx")
_rxunicode_compile_tool(
  unicode_d
  "${_rxunicode_tools_source_dir}/unicode_d.crexx"
  EXITS
  IMPORT_FILES
    "${unicode_gennorm2_RXBIN}"
    "${unicode_data_RXBIN}"
    "${unicode_normprops_RXBIN}"
  COMMENT "Compiling the Unicode normalization prepared-table builder ...")
_rxunicode_compile_tool(
  generate_normalization
  "${_rxunicode_normalization_generator_source}"
  IMPORT_FILES
    "${unicode_data_RXBIN}"
    "${unicode_normprops_RXBIN}"
    "${unicode_d_RXBIN}"
  COMMENT "Compiling the Unicode normalization source generator ...")

set(RXUNICODE_NORMALIZATION_TABLE_RXBIN "${unicode_d_RXBIN}")
set(_rxunicode_normalization_generator_rxbin
  "${generate_normalization_RXBIN}")

add_custom_command(
  OUTPUT "${_rxunicode_normalization_linked_generator_rxbin}"
  COMMAND $<TARGET_FILE:rxlink>
          -s
          -o normalization_generator
          generate_normalization
          level_l_gennorm2
          unicode_gennorm2
          unicode_data
          unicode_normprops
          unicode_d
  DEPENDS
    "${_rxunicode_normalization_generator_rxbin}"
    "${RXUNICODE_CASEFOLD_TOOL_DIR}/level_l_gennorm2.rxbin"
    "${RXUNICODE_CASEFOLD_TOOL_DIR}/unicode_gennorm2.rxbin"
    "${RXUNICODE_CASEFOLD_TOOL_DIR}/unicode_data.rxbin"
    "${RXUNICODE_CASEFOLD_TOOL_DIR}/unicode_normprops.rxbin"
    "${RXUNICODE_NORMALIZATION_TABLE_RXBIN}"
    rxlink
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Linking the Unicode normalization source generator ..."
  VERBATIM)

add_custom_command(
  OUTPUT "${_rxunicode_generated_normalization_source}"
  COMMAND $<TARGET_FILE:rxbvm>
          normalization_generator
          "${CMAKE_BINARY_DIR}/bin/library"
          -a
          "${RXUNICODE_UNICODE_DATA}"
          "${RXUNICODE_NORMALIZATION_PROPERTIES_DATA}"
          "${_rxunicode_normalization_template}"
          "${_rxunicode_generated_normalization_source}"
  DEPENDS
    "${_rxunicode_normalization_linked_generator_rxbin}"
    "${RXUNICODE_UNICODE_DATA}"
    "${RXUNICODE_NORMALIZATION_PROPERTIES_DATA}"
    "${_rxunicode_normalization_template}"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    rxbvm
    library
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Generating the constant-backed private normalization module ..."
  VERBATIM)

add_custom_target(rxunicode_normalization_source
  DEPENDS "${_rxunicode_generated_normalization_source}")

_rxunicode_compile_tool(
  encoding_table
  "${_rxunicode_encoding_table_source}"
  COMMENT "Compiling the legacy encoding table builder ...")
_rxunicode_compile_tool(
  generate_encoding
  "${_rxunicode_encoding_generator_source}"
  IMPORT_FILES "${encoding_table_RXBIN}"
  COMMENT "Compiling the legacy encoding source generator ...")
set(RXUNICODE_ENCODING_TABLE_RXBIN "${encoding_table_RXBIN}")
set(_rxunicode_encoding_generator_rxbin "${generate_encoding_RXBIN}")

add_custom_command(
  OUTPUT "${_rxunicode_encoding_linked_generator_rxbin}"
  COMMAND $<TARGET_FILE:rxlink>
          -s
          -o encoding_generator
          generate_encoding
          encoding_table
  DEPENDS
    "${_rxunicode_encoding_generator_rxbin}"
    "${RXUNICODE_ENCODING_TABLE_RXBIN}"
    rxlink
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Linking the legacy encoding source generator ..."
  VERBATIM)

add_custom_command(
  OUTPUT "${_rxunicode_generated_encoding_source}"
  COMMAND $<TARGET_FILE:rxbvm>
          encoding_generator
          "${CMAKE_BINARY_DIR}/bin/library"
          -a
          "${RXUNICODE_WINDOWS1252_DATA}"
          "${RXUNICODE_IBM437_DATA}"
          "${RXUNICODE_IBM850_DATA}"
          "${RXUNICODE_IBM1047_DATA}"
          "${_rxunicode_encoding_template}"
          "${_rxunicode_generated_encoding_source}"
  DEPENDS
    "${_rxunicode_encoding_linked_generator_rxbin}"
    "${RXUNICODE_WINDOWS1252_DATA}"
    "${RXUNICODE_IBM437_DATA}"
    "${RXUNICODE_IBM850_DATA}"
    "${RXUNICODE_IBM1047_DATA}"
    "${_rxunicode_encoding_template}"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    rxbvm
    library
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Generating the constant-backed legacy encoding module ..."
  VERBATIM)

add_custom_target(rxunicode_encoding_source
  DEPENDS "${_rxunicode_generated_encoding_source}")

_rxunicode_compile_tool(
  casefold_table
  "${_rxunicode_casefold_table_source}"
  COMMENT "Compiling the Unicode case-fold table builder ...")
_rxunicode_compile_tool(
  generate_casefold
  "${_rxunicode_casefold_generator_source}"
  IMPORT_FILES "${casefold_table_RXBIN}"
  COMMENT "Compiling the Unicode case-fold source generator ...")
set(RXUNICODE_CASEFOLD_TABLE_RXBIN "${casefold_table_RXBIN}")
set(_rxunicode_casefold_generator_rxbin "${generate_casefold_RXBIN}")

add_custom_command(
  OUTPUT "${_rxunicode_casefold_linked_generator_rxbin}"
  COMMAND $<TARGET_FILE:rxlink>
          -s
          -o casefold_generator
          generate_casefold
          casefold_table
  DEPENDS
    "${_rxunicode_casefold_generator_rxbin}"
    "${RXUNICODE_CASEFOLD_TABLE_RXBIN}"
    rxlink
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Linking the Unicode case-fold source generator ..."
  VERBATIM
)

add_custom_command(
  OUTPUT "${_rxunicode_generated_source}"
  COMMAND $<TARGET_FILE:rxbvm>
          casefold_generator
          "${CMAKE_BINARY_DIR}/bin/library"
          -a
          "${RXUNICODE_CASEFOLD_DATA}"
          "${_rxunicode_casefold_template}"
          "${_rxunicode_generated_source}"
  DEPENDS
    "${_rxunicode_casefold_linked_generator_rxbin}"
    "${RXUNICODE_CASEFOLD_DATA}"
    "${_rxunicode_casefold_template}"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    rxbvm
    library
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Generating the constant-backed private case-fold module ..."
  VERBATIM
)

add_custom_target(rxunicode_casefold_source
  DEPENDS "${_rxunicode_generated_source}")

_rxunicode_compile_tool(
  case_mapping_table
  "${_rxunicode_case_mapping_table_source}"
  COMMENT "Compiling the Unicode default case-mapping table builder ...")
_rxunicode_compile_tool(
  generate_case_mapping
  "${_rxunicode_case_mapping_generator_source}"
  IMPORT_FILES "${case_mapping_table_RXBIN}"
  COMMENT "Compiling the Unicode default case-mapping source generator ...")
set(RXUNICODE_CASE_MAPPING_TABLE_RXBIN "${case_mapping_table_RXBIN}")
set(_rxunicode_case_mapping_generator_rxbin
  "${generate_case_mapping_RXBIN}")

add_custom_command(
  OUTPUT "${_rxunicode_case_mapping_linked_generator_rxbin}"
  COMMAND $<TARGET_FILE:rxlink>
          -s
          -o case_mapping_generator
          generate_case_mapping
          case_mapping_table
  DEPENDS
    "${_rxunicode_case_mapping_generator_rxbin}"
    "${RXUNICODE_CASE_MAPPING_TABLE_RXBIN}"
    rxlink
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Linking the Unicode default case-mapping source generator ..."
  VERBATIM)

add_custom_command(
  OUTPUT "${_rxunicode_generated_case_mapping_source}"
  COMMAND $<TARGET_FILE:rxbvm>
          case_mapping_generator
          "${CMAKE_BINARY_DIR}/bin/library"
          -a
          "${RXUNICODE_UNICODE_DATA}"
          "${RXUNICODE_SPECIAL_CASING_DATA}"
          "${RXUNICODE_DERIVED_CORE_DATA}"
          "${_rxunicode_case_mapping_template}"
          "${_rxunicode_generated_case_mapping_source}"
  DEPENDS
    "${_rxunicode_case_mapping_linked_generator_rxbin}"
    "${RXUNICODE_UNICODE_DATA}"
    "${RXUNICODE_SPECIAL_CASING_DATA}"
    "${RXUNICODE_DERIVED_CORE_DATA}"
    "${_rxunicode_case_mapping_template}"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    rxbvm
    library
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Generating the constant-backed private default case-mapping module ..."
  VERBATIM)

add_custom_target(rxunicode_case_mapping_source
  DEPENDS "${_rxunicode_generated_case_mapping_source}")

_rxunicode_compile_tool(
  grapheme_table
  "${_rxunicode_grapheme_table_source}"
  COMMENT "Compiling the Unicode grapheme property-table builder ...")
_rxunicode_compile_tool(
  generate_grapheme
  "${_rxunicode_grapheme_generator_source}"
  IMPORT_FILES "${grapheme_table_RXBIN}"
  COMMENT "Compiling the Unicode grapheme source generator ...")
set(RXUNICODE_GRAPHEME_TABLE_RXBIN "${grapheme_table_RXBIN}")
set(_rxunicode_grapheme_generator_rxbin "${generate_grapheme_RXBIN}")

add_custom_command(
  OUTPUT "${_rxunicode_grapheme_linked_generator_rxbin}"
  COMMAND $<TARGET_FILE:rxlink>
          -s
          -o grapheme_generator
          generate_grapheme
          grapheme_table
  DEPENDS
    "${_rxunicode_grapheme_generator_rxbin}"
    "${RXUNICODE_GRAPHEME_TABLE_RXBIN}"
    rxlink
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Linking the Unicode grapheme source generator ..."
  VERBATIM
)

add_custom_command(
  OUTPUT "${_rxunicode_generated_grapheme_source}"
  COMMAND $<TARGET_FILE:rxbvm>
          grapheme_generator
          "${CMAKE_BINARY_DIR}/bin/library"
          -a
          "${RXUNICODE_GRAPHEME_PROPERTY_DATA}"
          "${RXUNICODE_EMOJI_DATA}"
          "${RXUNICODE_DERIVED_CORE_DATA}"
          "${_rxunicode_grapheme_template}"
          "${_rxunicode_generated_grapheme_source}"
  DEPENDS
    "${_rxunicode_grapheme_linked_generator_rxbin}"
    "${RXUNICODE_GRAPHEME_PROPERTY_DATA}"
    "${RXUNICODE_EMOJI_DATA}"
    "${RXUNICODE_DERIVED_CORE_DATA}"
    "${_rxunicode_grapheme_template}"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    rxbvm
    library
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Generating the constant-backed private grapheme module ..."
  VERBATIM
)

add_custom_target(rxunicode_grapheme_source
  DEPENDS "${_rxunicode_generated_grapheme_source}")

set(RXFNSG_REXX_BINS)
foreach(_module IN LISTS RXFNSG_REXX_MODULES)
  list(APPEND RXFNSG_REXX_BINS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
endforeach()

set(_rxfnsg_import_root "${CMAKE_CURRENT_BINARY_DIR}/imports/base")
crexx_add_import_root(rxfnsg_imports
  DIRECTORY "${_rxfnsg_import_root}"
  OUTPUT_VARIABLE _rxfnsg_import_files
  FILES
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
  DEPENDS
    library
    classlib
  COMMENT "Staging deterministic Level G imports ...")

# Generated Unicode product modules use source-level SIGNAL, so they need the
# compiler-exit metadata. Keep that provider in its own exact root rather than
# re-enabling discovery across the shared executable directory.
set(_rxfnsg_exit_import_root
  "${CMAKE_CURRENT_BINARY_DIR}/imports/exits")
crexx_add_import_root(rxfnsg_exit_imports
  DIRECTORY "${_rxfnsg_exit_import_root}"
  OUTPUT_VARIABLE _rxfnsg_exit_import_files
  FILES "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
  DEPENDS compiler_exit_bin
  COMMENT "Staging deterministic Level G compiler-exit imports ...")

# The public HTTP module consumes the two private HTTP modules. Keep that
# dependency root exact so another partially built Level G member cannot become
# an eligible provider.
set(_rxfnsg_http_dependency_root
  "${CMAKE_CURRENT_BINARY_DIR}/imports/http")
crexx_add_import_root(rxfnsg_http_imports
  DIRECTORY "${_rxfnsg_http_dependency_root}"
  OUTPUT_VARIABLE _rxfnsg_http_import_files
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/httpcodec.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/httpcore.rxbin"
  COMMENT "Staging deterministic Level G HTTP imports ...")

# The public Unicode facade consumes five private providers which all publish
# the `_rxunicode` namespace. Stage exactly those providers together so the
# compiler cannot select a stale generated source or another Level G RXBIN
# while the consolidated public library is being rebuilt.
set(_rxfnsg_unicode_dependency_root
  "${CMAKE_CURRENT_BINARY_DIR}/imports/unicode")
crexx_add_import_root(rxfnsg_unicode_imports
  DIRECTORY "${_rxfnsg_unicode_dependency_root}"
  OUTPUT_VARIABLE _rxfnsg_unicode_import_files
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_casefold.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_case_mapping.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_codec.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_grapheme.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_normalization.rxbin"
  COMMENT "Staging deterministic Level G Unicode imports ...")

# `httpserver`, `llm` and `http` share the public `rxfnsg` namespace. During a
# clean or parallel build the final consolidated image does not exist yet, so
# publish the freshly compiled HTTP module under its namespace name in a
# private stage directory. This prevents later modules from resolving HTTP
# types against a stale installed/build-tree `rxfnsg.rxbin`.
set(_rxfnsg_http_stage_dir "${CMAKE_CURRENT_BINARY_DIR}/http-stage")
set(_rxfnsg_http_stage_image "${_rxfnsg_http_stage_dir}/rxfnsg.rxbin")
set(_rxfnsg_http_stage_core "${_rxfnsg_http_stage_dir}/httpcore.rxbin")
add_custom_command(
  OUTPUT
    "${_rxfnsg_http_stage_image}"
    "${_rxfnsg_http_stage_core}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_rxfnsg_http_stage_dir}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_CURRENT_BINARY_DIR}/http.rxbin"
          "${_rxfnsg_http_stage_image}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_CURRENT_BINARY_DIR}/httpcore.rxbin"
          "${_rxfnsg_http_stage_core}"
  DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/http.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/httpcore.rxbin"
  COMMENT "Staging current HTTP metadata for the Level G LLM module ..."
  VERBATIM
)

foreach(_binfile IN LISTS RXFNSG_REXX_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  set(_module_source_input "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
  if(_basename STREQUAL "unicode_casefold")
    set(_module_source_input "${_rxunicode_generated_source}")
  elseif(_basename STREQUAL "unicode_case_mapping")
    set(_module_source_input "${_rxunicode_generated_case_mapping_source}")
  elseif(_basename STREQUAL "unicode_codec")
    set(_module_source_input "${_rxunicode_generated_encoding_source}")
  elseif(_basename STREQUAL "unicode_grapheme")
    set(_module_source_input "${_rxunicode_generated_grapheme_source}")
  elseif(_basename STREQUAL "unicode_normalization")
    set(_module_source_input "${_rxunicode_generated_normalization_source}")
  endif()

  set(_module_dependencies)
  set(_module_import_args)
  set(_module_exit_args -x)
  if(_basename STREQUAL "unicode")
    list(APPEND _module_dependencies ${_rxfnsg_unicode_import_files})
    list(APPEND _module_import_args -i "${_rxfnsg_unicode_dependency_root}")
  elseif(_basename STREQUAL "unicode_casefold" OR
         _basename STREQUAL "unicode_case_mapping" OR
         _basename STREQUAL "unicode_codec" OR
         _basename STREQUAL "unicode_grapheme" OR
         _basename STREQUAL "unicode_normalization")
    # These generated runtimes use source-level SIGNAL. Their compiler-exit
    # provider remains explicit below, but executable-directory discovery is
    # still disabled.
    set(_module_exit_args)
    list(APPEND _module_dependencies
      rxfnsg_exit_imports
      ${_rxfnsg_exit_import_files})
    list(APPEND _module_import_args -i "${_rxfnsg_exit_import_root}")
  elseif(_basename STREQUAL "http")
    list(APPEND _module_dependencies ${_rxfnsg_http_import_files})
    list(APPEND _module_import_args -i "${_rxfnsg_http_dependency_root}")
  elseif(_basename STREQUAL "httpserver" OR _basename STREQUAL "llm")
    list(APPEND _module_dependencies
      "${_rxfnsg_http_stage_image}"
      "${_rxfnsg_http_stage_core}")
    list(APPEND _module_import_args -i "${_rxfnsg_http_stage_dir}")
  endif()

  set(_module_source_root
    "${CMAKE_CURRENT_BINARY_DIR}/sources/${_basename}")
  set(_module_source "${_module_source_root}/${_basename}.crexx")
  add_custom_command(
    OUTPUT "${_binfile}"
    BYPRODUCTS
      "${_module_source}"
      "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.rxas"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_module_source_root}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_module_source_input}"
            "${_module_source}"
    COMMAND $<TARGET_FILE:rxc>
            ${_module_exit_args}
            --no-exe-import
            ${_module_import_args}
            -i "${_rxfnsg_import_root}"
            -o "${_basename}"
            "${_module_source}"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${_module_source_input}"
      rxfnsg_imports
      ${_rxfnsg_import_files}
      rxc
      rxas
      ${_module_dependencies}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "cREXX compile and assemble ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_rxfnsg_control_file "${CMAKE_CURRENT_BINARY_DIR}/rxfnsg_link_$<CONFIG>.ctl")
string(CONCAT _rxfnsg_control_text
  "* Auto-generated control file for the consolidated cREXX Level G library\n")
foreach(_module_bin IN LISTS RXFNSG_REXX_BINS)
  string(APPEND _rxfnsg_control_text "INPUT ${_module_bin}\n")
endforeach()
foreach(_selector IN LISTS RXFNSG_REXX_MODULES)
  string(APPEND _rxfnsg_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _rxfnsg_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsg\n")

file(GENERATE OUTPUT "${_rxfnsg_control_file}" CONTENT "${_rxfnsg_control_text}")

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_rxfnsg_control_file}"
  DEPENDS rxlink ${RXFNSG_REXX_BINS} "${_rxfnsg_control_file}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked Level G Library Image ..."
  VERBATIM
)

add_custom_target(rxfnsg ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin"
)

add_dependencies(rxfnsg library classlib)

add_test(
  NAME rxunicode_casefold_rxas_shape
  COMMAND ${CMAKE_COMMAND}
    -DRXAS=${CMAKE_CURRENT_BINARY_DIR}/unicode_casefold.rxas
    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/unicode_casefold.crexx.in
    -P ${CMAKE_CURRENT_SOURCE_DIR}/../unicode/check_casefold_rxas.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(rxunicode_casefold_rxas_shape PROPERTIES
  LABELS "library;unicode;casefold;levelg;shape")

add_test(
  NAME rxunicode_grapheme_rxas_shape
  COMMAND ${CMAKE_COMMAND}
    -DRXAS=${CMAKE_CURRENT_BINARY_DIR}/unicode_grapheme.rxas
    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/unicode_grapheme.crexx.in
    -P ${CMAKE_CURRENT_SOURCE_DIR}/../unicode/check_grapheme_rxas.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(rxunicode_grapheme_rxas_shape PROPERTIES
  LABELS "library;unicode;grapheme;levelg;shape")

foreach(_rxunicode_shape_kind IN ITEMS normalization case_mapping codec)
  add_test(
    NAME "rxunicode_${_rxunicode_shape_kind}_rxas_shape"
    COMMAND ${CMAKE_COMMAND}
      -DKIND=${_rxunicode_shape_kind}
      -DRXAS=${CMAKE_CURRENT_BINARY_DIR}/unicode_${_rxunicode_shape_kind}.rxas
      -P ${CMAKE_CURRENT_SOURCE_DIR}/../unicode/check_product_rxas.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set_tests_properties("rxunicode_${_rxunicode_shape_kind}_rxas_shape"
    PROPERTIES
      LABELS "library;unicode;${_rxunicode_shape_kind};levelg;shape")
endforeach()

add_test(
  NAME rxunicode_contract_surface
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/unicode.crexx
    -DDOC=${CMAKE_CURRENT_SOURCE_DIR}/unicode.md
    -DUSER_DOC=${CMAKE_SOURCE_DIR}/docs/books/crexx_library_reference/unicode.md
    -DNORMALIZATION_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_normalization.crexx
    -DCASE_MAPPING_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_case_mapping.crexx
    -DCASEFOLD_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_casefold.crexx
    -DGRAPHEME_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_grapheme.crexx
    -DCODEC_TEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_codec.crexx
    -P ${CMAKE_CURRENT_SOURCE_DIR}/../unicode/check_casefold_contract.cmake
)
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
    # rxdas renders each shared Unicode binary constant as hexadecimal.  The
    # five product tables currently produce about 40 MB of bounded text even
    # though each table remains a single linked constant-pool item.
    -DMAX_OUTPUT_BYTES=50000000
    -P ${CMAKE_SOURCE_DIR}/linker/tests/check_rxdas_contains.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(rxfnsg_rxdas_inline_preserve_smoke PROPERTIES LABELS "inline;library")

crexx_finalize_directory_qa_tiers()
