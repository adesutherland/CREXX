# CMake bootstraps the B controller using the existing compiler-sealed worker.
# The controller owns only the optional Unicode generation/runtime waves.

set(_unicode_root "${CMAKE_CURRENT_BINARY_DIR}/stage-unicode")
set(_unicode_bootstrap_root "${_unicode_root}/bootstrap")
set(_unicode_bootstrap_import_root "${_unicode_bootstrap_root}/imports")
set(_unicode_controller_import_root
  "${_unicode_bootstrap_root}/controller-imports")
set(_unicode_work_root "${_unicode_root}/work")
set(_unicode_members_root "${_unicode_work_root}/members")
set(_unicode_generation_work_root "${_unicode_work_root}/unicode")
set(_unicode_tools_work_root "${_unicode_generation_work_root}/tools")
set(_unicode_generated_source_root
  "${_unicode_generation_work_root}/generated")
set(_unicode_linked_generator_root
  "${_unicode_generation_work_root}/linked-generators")

if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
  set(_unicode_default_jobs 30)
else()
  set(_unicode_default_jobs 5)
endif()
set(CREXX_UNICODE_BUILD_JOBS "${_unicode_default_jobs}" CACHE STRING
  "Parallel jobs used inside the Level B Unicode core builder")
if(NOT CREXX_UNICODE_BUILD_JOBS MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "CREXX_UNICODE_BUILD_JOBS must be a positive integer")
endif()

set(_unicode_controller_source
  "${CMAKE_SOURCE_DIR}/tools/unicode/build_unicode.crexx")
set(_unicode_worker_source
  "${CMAKE_SOURCE_DIR}/tools/build-system/levelg/build_stage_g_worker.crexx")
set(_unicode_controller_copy
  "${_unicode_bootstrap_root}/controller-source/build_unicode.crexx")
set(_unicode_worker_copy
  "${_unicode_bootstrap_root}/worker-source/build_stage_g_worker.crexx")
set(_unicode_controller_rxas
  "${_unicode_bootstrap_root}/build_unicode.rxas")
set(_unicode_controller_rxbin
  "${_unicode_bootstrap_root}/build_unicode.rxbin")
set(_unicode_worker_rxas
  "${_unicode_bootstrap_root}/build_stage_g_worker.rxas")
set(_unicode_worker_rxbin
  "${_unicode_bootstrap_root}/build_stage_g_worker.rxbin")
set(_unicode_linked_builder
  "${_unicode_bootstrap_root}/build_unicode_linked.rxbin")

crexx_add_import_root(unicode_builder_imports
  DIRECTORY "${_unicode_bootstrap_import_root}"
  OUTPUT_VARIABLE _unicode_builder_import_files
  FILES
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rx_hash.rxplugin"
  DEPENDS library classlib compiler_exit_bin _hash
  COMMENT "Staging exact imports for the Level B Unicode core builder ...")

add_custom_command(
  OUTPUT "${_unicode_linked_builder}"
  BYPRODUCTS
    "${_unicode_controller_copy}"
    "${_unicode_worker_copy}"
    "${_unicode_controller_rxas}"
    "${_unicode_controller_rxbin}"
    "${_unicode_worker_rxas}"
    "${_unicode_worker_rxbin}"
    "${_unicode_controller_import_root}/build_stage_g_worker.rxbin"
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${_unicode_bootstrap_root}/controller-source"
          "${_unicode_bootstrap_root}/worker-source"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_unicode_worker_source}" "${_unicode_worker_copy}"
  COMMAND $<TARGET_FILE:rxc>
          --no-exe-import
          -i "${_unicode_bootstrap_import_root}"
          -o build_stage_g_worker
          "${_unicode_worker_copy}"
  COMMAND $<TARGET_FILE:rxas>
          -o build_stage_g_worker build_stage_g_worker
  COMMAND ${CMAKE_COMMAND} -E rm -rf
          "${_unicode_controller_import_root}"
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${_unicode_controller_import_root}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_unicode_worker_rxbin}"
          "${_unicode_controller_import_root}/build_stage_g_worker.rxbin"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_unicode_controller_source}" "${_unicode_controller_copy}"
  COMMAND $<TARGET_FILE:rxc>
          --no-exe-import
          -i "${_unicode_bootstrap_import_root}"
          -i "${_unicode_controller_import_root}"
          -o build_unicode
          "${_unicode_controller_copy}"
  COMMAND $<TARGET_FILE:rxas>
          -o build_unicode build_unicode
  COMMAND $<TARGET_FILE:rxlink>
          -s
          -o "${_unicode_bootstrap_root}/build_unicode_linked"
          "${_unicode_controller_rxbin}"
          "${_unicode_worker_rxbin}"
          "${CMAKE_BINARY_DIR}/bin/library.rxbin"
          "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
  DEPENDS
    "${_unicode_controller_source}"
    "${_unicode_worker_source}"
    unicode_builder_imports
    ${_unicode_builder_import_files}
    rxc rxas rxlink
    "$<TARGET_FILE:rxc>"
    "$<TARGET_FILE:rxas>"
    "$<TARGET_FILE:rxlink>"
  WORKING_DIRECTORY "${_unicode_bootstrap_root}"
  COMMENT "Bootstrapping the Level B Unicode core builder ..."
  VERBATIM)


set(_unicode_tools
  level_l_gennorm2 unicode_normprops encoding_table casefold_table
  case_mapping_table grapheme_table unicode_gennorm2 generate_encoding
  generate_casefold generate_case_mapping generate_grapheme unicode_data
  unicode_d generate_normalization)
set(_unicode_private_modules
  unicode_casefold unicode_case_mapping unicode_codec unicode_grapheme
  unicode_normalization)
set(_unicode_modules ${_unicode_private_modules} unicode_api)
set(_unicode_api_source "${CMAKE_CURRENT_SOURCE_DIR}/rexx/unicode_api.crexx")
set(_unicode_tool_sources)
foreach(_unicode_tool IN LISTS _unicode_tools)
  list(APPEND _unicode_tool_sources
    "${CMAKE_SOURCE_DIR}/tools/unicode/${_unicode_tool}.crexx")
endforeach()
set(_unicode_templates)
foreach(_unicode_kind IN ITEMS casefold case_mapping codec grapheme)
  list(APPEND _unicode_templates
    "${CMAKE_CURRENT_SOURCE_DIR}/rexx/unicode_${_unicode_kind}.crexx.in")
endforeach()
set(_unicode_byproducts)
foreach(_unicode_module IN LISTS _unicode_modules)
  list(APPEND _unicode_byproducts
    "${_unicode_members_root}/${_unicode_module}/${_unicode_module}.rxas"
    "${_unicode_members_root}/${_unicode_module}/${_unicode_module}.rxbin")
endforeach()
foreach(_unicode_module IN LISTS _unicode_private_modules)
  list(APPEND _unicode_byproducts
    "${_unicode_generated_source_root}/${_unicode_module}.crexx")
endforeach()
foreach(_unicode_tool IN LISTS _unicode_tools)
  list(APPEND _unicode_byproducts
    "${_unicode_tools_work_root}/${_unicode_tool}/${_unicode_tool}.rxas"
    "${_unicode_tools_work_root}/${_unicode_tool}/${_unicode_tool}.rxbin")
endforeach()
foreach(_unicode_kind IN ITEMS encoding casefold case_mapping grapheme normalization)
  list(APPEND _unicode_byproducts
    "${_unicode_linked_generator_root}/${_unicode_kind}_generator.rxbin")
endforeach()

set(RXUNICODE_CORE_RXBIN "${CMAKE_BINARY_DIR}/bin/rxunicode-core.rxbin")
set(RXUNICODE_CORE_MEMBERS_ROOT "${_unicode_members_root}")
set(RXUNICODE_CORE_GENERATED_ROOT "${_unicode_generated_source_root}")
set(RXUNICODE_CASEFOLD_TOOL_DIR "${_unicode_tools_work_root}")
set(RXUNICODE_CASEFOLD_TABLE_RXBIN
  "${_unicode_tools_work_root}/casefold_table/casefold_table.rxbin")
set(RXUNICODE_GRAPHEME_TABLE_RXBIN
  "${_unicode_tools_work_root}/grapheme_table/grapheme_table.rxbin")
set(RXUNICODE_CASE_MAPPING_TABLE_RXBIN
  "${_unicode_tools_work_root}/case_mapping_table/case_mapping_table.rxbin")
set(RXUNICODE_NORMALIZATION_TABLE_RXBIN
  "${_unicode_tools_work_root}/unicode_d/unicode_d.rxbin")
set(RXUNICODE_ENCODING_TABLE_RXBIN
  "${_unicode_tools_work_root}/encoding_table/encoding_table.rxbin")

set(_unicode_completion_stamp "${_unicode_root}/complete.stamp")
set(_unicode_outputs "${RXUNICODE_CORE_RXBIN}" ${_unicode_byproducts})
add_custom_command(
  OUTPUT "${_unicode_completion_stamp}" ${_unicode_outputs}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_unicode_work_root}"
  COMMAND $<TARGET_FILE:rxbvm>
    "${_unicode_linked_builder}"
    -a
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/tools/unicode"
    "${_unicode_work_root}"
    "${CMAKE_BINARY_DIR}/bin"
    "$<TARGET_FILE:rxc>"
    "$<TARGET_FILE:rxas>"
    "$<TARGET_FILE:rxlink>"
    "$<TARGET_FILE:rxbvm>"
    "${CMAKE_COMMAND}"
    "$<CONFIG>"
    "${CREXX_UNICODE_BUILD_JOBS}"
  COMMAND ${CMAKE_COMMAND} -E touch "${_unicode_completion_stamp}"
  # Make secondary outputs must not remain older than the completion owner.
  # This runs only after a required, successful producer invocation.
  COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${_unicode_outputs}
  DEPENDS
    "${_unicode_linked_builder}"
    ${_unicode_tool_sources}
    ${_unicode_templates}
    "${_unicode_api_source}"
    ${_rxunicode_runtime_inputs}
    levell_unicode_lexer
    "${CMAKE_BINARY_DIR}/bin/gennorm2_language.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
    "$<TARGET_FILE:fs>"
    rxc rxas rxlink rxbvm library classlib compiler_exit_bin fs
    "$<TARGET_FILE:rxc>"
    "$<TARGET_FILE:rxas>"
    "$<TARGET_FILE:rxlink>"
    "$<TARGET_FILE:rxbvm>"
  WORKING_DIRECTORY "${_unicode_bootstrap_root}"
  COMMENT "Building shared B Unicode generators and runtime ..."
  VERBATIM)
add_custom_target(rxunicode_core ALL
  DEPENDS "${_unicode_completion_stamp}" ${_unicode_outputs})

# Preserve focused QA target names without adding a second source producer.
foreach(_unicode_source_target IN ITEMS normalization encoding casefold case_mapping grapheme)
  add_custom_target("rxunicode_${_unicode_source_target}_source")
  add_dependencies("rxunicode_${_unicode_source_target}_source" rxunicode_core)
endforeach()
