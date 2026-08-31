# Phase 3 boundary: CMake bootstraps one Level B program; that program owns the
# complete Level G/Unicode graph and its parallel dependency waves.

set(_stage_g_root "${CMAKE_CURRENT_BINARY_DIR}/newbuild")
set(_stage_g_bootstrap_root "${_stage_g_root}/bootstrap")
set(_stage_g_bootstrap_import_root "${_stage_g_bootstrap_root}/imports")
set(_stage_g_controller_import_root
  "${_stage_g_bootstrap_root}/controller-imports")
set(_stage_g_work_root "${_stage_g_root}/work")
set(_stage_g_members_root "${_stage_g_work_root}/members")
set(_stage_g_unicode_work_root "${_stage_g_work_root}/unicode")
set(_stage_g_unicode_tools_work_root "${_stage_g_unicode_work_root}/tools")
set(_stage_g_generated_source_root
  "${_stage_g_unicode_work_root}/generated")
set(_stage_g_linked_generator_root
  "${_stage_g_unicode_work_root}/linked-generators")

if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
  set(_stage_g_default_jobs 30)
else()
  set(_stage_g_default_jobs 5)
endif()
set(CREXX_LEVEL_G_BUILD_JOBS "${_stage_g_default_jobs}" CACHE STRING
  "Parallel jobs used inside the Level B Level G builder")
if(NOT CREXX_LEVEL_G_BUILD_JOBS MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "CREXX_LEVEL_G_BUILD_JOBS must be a positive integer")
endif()

set(_stage_g_controller_source
  "${CMAKE_SOURCE_DIR}/tools/newbuild/build_stage_g.crexx")
set(_stage_g_worker_source
  "${CMAKE_SOURCE_DIR}/tools/newbuild/build_stage_g_worker.crexx")
set(_stage_g_controller_copy
  "${_stage_g_bootstrap_root}/controller-source/build_stage_g.crexx")
set(_stage_g_worker_copy
  "${_stage_g_bootstrap_root}/worker-source/build_stage_g_worker.crexx")
set(_stage_g_controller_rxas
  "${_stage_g_bootstrap_root}/build_stage_g.rxas")
set(_stage_g_controller_rxbin
  "${_stage_g_bootstrap_root}/build_stage_g.rxbin")
set(_stage_g_worker_rxas
  "${_stage_g_bootstrap_root}/build_stage_g_worker.rxas")
set(_stage_g_worker_rxbin
  "${_stage_g_bootstrap_root}/build_stage_g_worker.rxbin")
set(_stage_g_linked_builder
  "${_stage_g_bootstrap_root}/build_stage_g_linked.rxbin")

crexx_add_import_root(stage_g_builder_imports
  DIRECTORY "${_stage_g_bootstrap_import_root}"
  OUTPUT_VARIABLE _stage_g_builder_import_files
  FILES
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rx_hash.rxplugin"
  DEPENDS library classlib compiler_exit_bin _hash
  COMMENT "Staging exact imports for the Level B Level G builder ...")

add_custom_command(
  OUTPUT "${_stage_g_linked_builder}"
  BYPRODUCTS
    "${_stage_g_controller_copy}"
    "${_stage_g_worker_copy}"
    "${_stage_g_controller_rxas}"
    "${_stage_g_controller_rxbin}"
    "${_stage_g_worker_rxas}"
    "${_stage_g_worker_rxbin}"
    "${_stage_g_controller_import_root}/build_stage_g_worker.rxbin"
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${_stage_g_bootstrap_root}/controller-source"
          "${_stage_g_bootstrap_root}/worker-source"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_stage_g_worker_source}" "${_stage_g_worker_copy}"
  COMMAND $<TARGET_FILE:rxc>
          --no-exe-import
          -i "${_stage_g_bootstrap_import_root}"
          -o build_stage_g_worker
          "${_stage_g_worker_copy}"
  COMMAND $<TARGET_FILE:rxas>
          -o build_stage_g_worker build_stage_g_worker
  COMMAND ${CMAKE_COMMAND} -E rm -rf
          "${_stage_g_controller_import_root}"
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${_stage_g_controller_import_root}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_stage_g_worker_rxbin}"
          "${_stage_g_controller_import_root}/build_stage_g_worker.rxbin"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_stage_g_controller_source}" "${_stage_g_controller_copy}"
  COMMAND $<TARGET_FILE:rxc>
          --no-exe-import
          -i "${_stage_g_bootstrap_import_root}"
          -i "${_stage_g_controller_import_root}"
          -o build_stage_g
          "${_stage_g_controller_copy}"
  COMMAND $<TARGET_FILE:rxas>
          -o build_stage_g build_stage_g
  COMMAND $<TARGET_FILE:rxlink>
          -s
          -o "${_stage_g_bootstrap_root}/build_stage_g_linked"
          "${_stage_g_controller_rxbin}"
          "${_stage_g_worker_rxbin}"
          "${CMAKE_BINARY_DIR}/bin/library.rxbin"
          "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
  DEPENDS
    "${_stage_g_controller_source}"
    "${_stage_g_worker_source}"
    stage_g_builder_imports
    ${_stage_g_builder_import_files}
    rxc rxas rxlink
    "$<TARGET_FILE:rxc>"
    "$<TARGET_FILE:rxas>"
    "$<TARGET_FILE:rxlink>"
  WORKING_DIRECTORY "${_stage_g_bootstrap_root}"
  COMMENT "Bootstrapping the Level B Level G builder ..."
  VERBATIM)

set(_stage_g_unicode_tools
  level_l_gennorm2
  unicode_normprops
  encoding_table
  casefold_table
  case_mapping_table
  grapheme_table
  unicode_gennorm2
  generate_encoding
  generate_casefold
  generate_case_mapping
  generate_grapheme
  unicode_data
  unicode_d
  generate_normalization)
set(_stage_g_product_sources
  "${CMAKE_CURRENT_SOURCE_DIR}/integer.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/decimal.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/packednumeric.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/statsvalue.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/httpcodec.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/httpcore.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/http.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/httpserver.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/llm.crexx"
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode_casefold.crexx.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode_case_mapping.crexx.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode_codec.crexx.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode_grapheme.crexx.in")
set(_stage_g_unicode_tool_sources)
foreach(_stage_g_tool IN LISTS _stage_g_unicode_tools)
  list(APPEND _stage_g_unicode_tool_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/../unicode/tools/${_stage_g_tool}.crexx")
endforeach()
set(_stage_g_unicode_inputs
  "${RXUNICODE_CASEFOLD_DATA}"
  "${RXUNICODE_GRAPHEME_PROPERTY_DATA}"
  "${RXUNICODE_EMOJI_DATA}"
  "${RXUNICODE_DERIVED_CORE_DATA}"
  "${RXUNICODE_UNICODE_DATA}"
  "${RXUNICODE_NORMALIZATION_PROPERTIES_DATA}"
  "${RXUNICODE_SPECIAL_CASING_DATA}"
  "${RXUNICODE_WINDOWS1252_DATA}"
  "${RXUNICODE_IBM437_DATA}"
  "${RXUNICODE_IBM850_DATA}"
  "${RXUNICODE_IBM1047_DATA}")

set(_rxunicode_generated_source
  "${_stage_g_generated_source_root}/unicode_casefold.crexx")
set(_rxunicode_generated_case_mapping_source
  "${_stage_g_generated_source_root}/unicode_case_mapping.crexx")
set(_rxunicode_generated_encoding_source
  "${_stage_g_generated_source_root}/unicode_codec.crexx")
set(_rxunicode_generated_grapheme_source
  "${_stage_g_generated_source_root}/unicode_grapheme.crexx")
set(_rxunicode_generated_normalization_source
  "${_stage_g_generated_source_root}/unicode_normalization.crexx")

set(RXUNICODE_CASEFOLD_TOOL_DIR "${_stage_g_unicode_tools_work_root}")
set(RXUNICODE_CASEFOLD_TABLE_RXBIN
  "${_stage_g_unicode_tools_work_root}/casefold_table/casefold_table.rxbin")
set(RXUNICODE_GRAPHEME_TABLE_RXBIN
  "${_stage_g_unicode_tools_work_root}/grapheme_table/grapheme_table.rxbin")
set(RXUNICODE_CASE_MAPPING_TABLE_RXBIN
  "${_stage_g_unicode_tools_work_root}/case_mapping_table/case_mapping_table.rxbin")
set(RXUNICODE_NORMALIZATION_TABLE_RXBIN
  "${_stage_g_unicode_tools_work_root}/unicode_d/unicode_d.rxbin")
set(RXUNICODE_ENCODING_TABLE_RXBIN
  "${_stage_g_unicode_tools_work_root}/encoding_table/encoding_table.rxbin")
foreach(_stage_g_parent_variable IN ITEMS
    RXUNICODE_CASEFOLD_TOOL_DIR
    RXUNICODE_CASEFOLD_TABLE_RXBIN
    RXUNICODE_GRAPHEME_TABLE_RXBIN
    RXUNICODE_CASE_MAPPING_TABLE_RXBIN
    RXUNICODE_NORMALIZATION_TABLE_RXBIN
    RXUNICODE_ENCODING_TABLE_RXBIN)
  set(${_stage_g_parent_variable} "${${_stage_g_parent_variable}}" PARENT_SCOPE)
endforeach()

set(_stage_g_byproducts
  "${_rxunicode_generated_source}"
  "${_rxunicode_generated_case_mapping_source}"
  "${_rxunicode_generated_encoding_source}"
  "${_rxunicode_generated_grapheme_source}"
  "${_rxunicode_generated_normalization_source}"
  "${_stage_g_linked_generator_root}/encoding_generator.rxbin"
  "${_stage_g_linked_generator_root}/casefold_generator.rxbin"
  "${_stage_g_linked_generator_root}/case_mapping_generator.rxbin"
  "${_stage_g_linked_generator_root}/grapheme_generator.rxbin"
  "${_stage_g_linked_generator_root}/normalization_generator.rxbin")
foreach(_stage_g_module IN LISTS RXFNSG_REXX_MODULES)
  list(APPEND _stage_g_byproducts
    "${_stage_g_members_root}/${_stage_g_module}/${_stage_g_module}.rxas"
    "${_stage_g_members_root}/${_stage_g_module}/${_stage_g_module}.rxbin")
endforeach()
foreach(_stage_g_tool IN LISTS _stage_g_unicode_tools)
  list(APPEND _stage_g_byproducts
    "${_stage_g_unicode_tools_work_root}/${_stage_g_tool}/${_stage_g_tool}.rxas"
    "${_stage_g_unicode_tools_work_root}/${_stage_g_tool}/${_stage_g_tool}.rxbin")
endforeach()

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin"
  BYPRODUCTS ${_stage_g_byproducts}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_stage_g_work_root}"
  COMMAND $<TARGET_FILE:rxbvm>
          "${_stage_g_linked_builder}"
          -a
          "${CMAKE_CURRENT_SOURCE_DIR}"
          "${_stage_g_work_root}"
          "${CMAKE_BINARY_DIR}/bin"
          "$<TARGET_FILE:rxc>"
          "$<TARGET_FILE:rxas>"
          "$<TARGET_FILE:rxlink>"
          "$<TARGET_FILE:rxbvm>"
          "${CMAKE_COMMAND}"
          "$<CONFIG>"
          "${CREXX_LEVEL_G_BUILD_JOBS}"
          all
  DEPENDS
    "${_stage_g_linked_builder}"
    ${_stage_g_product_sources}
    ${_stage_g_unicode_tool_sources}
    ${_stage_g_unicode_inputs}
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
    rxc rxas rxlink rxbvm library classlib compiler_exit_bin
    "$<TARGET_FILE:rxc>"
    "$<TARGET_FILE:rxas>"
    "$<TARGET_FILE:rxlink>"
    "$<TARGET_FILE:rxbvm>"
  WORKING_DIRECTORY "${_stage_g_bootstrap_root}"
  COMMENT "Building Level G and Unicode with the Level B builder ..."
  VERBATIM)

add_custom_target(rxfnsg ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin")

# Preserve focused QA target names without creating additional producers.
foreach(_stage_g_source_target IN ITEMS
    normalization encoding casefold case_mapping grapheme)
  add_custom_target("rxunicode_${_stage_g_source_target}_source")
  add_dependencies("rxunicode_${_stage_g_source_target}_source" rxfnsg)
endforeach()
