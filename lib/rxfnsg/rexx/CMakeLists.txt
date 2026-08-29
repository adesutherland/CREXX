cmake_minimum_required(VERSION 3.24)

set(RXFNSG_REXX_MODULES
        integer
        decimal
        packednumeric
        statsvalue
        unicode_casefold
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
file(MAKE_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}")

add_custom_command(
  OUTPUT "${RXUNICODE_CASEFOLD_TABLE_RXBIN}"
  BYPRODUCTS "${RXUNICODE_CASEFOLD_TOOL_DIR}/casefold_table.rxas"
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMAND $<TARGET_FILE:rxc>
          -x
          -i "${CMAKE_BINARY_DIR}/bin"
          -o casefold_table
          "${_rxunicode_casefold_table_source}"
  COMMAND $<TARGET_FILE:rxas> casefold_table
  DEPENDS
    "${_rxunicode_casefold_table_source}"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    rxc
    rxas
    library
    compiler_exit_bin
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Compiling the Unicode case-fold table builder ..."
  VERBATIM
)

add_custom_command(
  OUTPUT "${_rxunicode_casefold_generator_rxbin}"
  BYPRODUCTS "${RXUNICODE_CASEFOLD_TOOL_DIR}/generate_casefold.rxas"
  COMMAND $<TARGET_FILE:rxc>
          -x
          -i "${CMAKE_BINARY_DIR}/bin"
          -i "${RXUNICODE_CASEFOLD_TOOL_DIR}"
          -o generate_casefold
          "${_rxunicode_casefold_generator_source}"
  COMMAND $<TARGET_FILE:rxas> generate_casefold
  DEPENDS
    "${_rxunicode_casefold_generator_source}"
    "${RXUNICODE_CASEFOLD_TABLE_RXBIN}"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    rxc
    rxas
    library
    compiler_exit_bin
  WORKING_DIRECTORY "${RXUNICODE_CASEFOLD_TOOL_DIR}"
  COMMENT "Compiling the Unicode case-fold source generator ..."
  VERBATIM
)

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
  COMMENT "Generating the constant-backed Level G rxunicode module ..."
  VERBATIM
)

add_custom_target(rxunicode_casefold_source
  DEPENDS "${_rxunicode_generated_source}")

set(RXFNSG_REXX_BINS)
foreach(_module IN LISTS RXFNSG_REXX_MODULES)
  list(APPEND RXFNSG_REXX_BINS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
endforeach()

# `httpserver`, `llm` and `http` share the public `rxfnsg` namespace. During a
# clean or parallel build the final consolidated image does not exist yet, so
# publish the freshly compiled HTTP module under its namespace name in a
# private stage directory. This prevents later modules from resolving HTTP
# types against a stale installed/build-tree `rxfnsg.rxbin`.
set(_rxfnsg_http_stage_dir "${CMAKE_CURRENT_BINARY_DIR}/http-stage")
set(_rxfnsg_http_stage_image "${_rxfnsg_http_stage_dir}/rxfnsg.rxbin")
add_custom_command(
  OUTPUT "${_rxfnsg_http_stage_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_rxfnsg_http_stage_dir}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_CURRENT_BINARY_DIR}/http.rxbin"
          "${_rxfnsg_http_stage_image}"
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/http.rxbin"
  COMMENT "Staging current HTTP metadata for the Level G LLM module ..."
  VERBATIM
)

foreach(_binfile IN LISTS RXFNSG_REXX_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  set(_module_source "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
  if(_basename STREQUAL "unicode_casefold")
    set(_module_source "${_rxunicode_generated_source}")
  endif()

  set(_module_dependencies)
  set(_module_import_args)
  if(_basename STREQUAL "unicode")
    list(APPEND _module_dependencies
      "${CMAKE_CURRENT_BINARY_DIR}/unicode_casefold.rxbin")
    list(APPEND _module_import_args -i "${CMAKE_CURRENT_BINARY_DIR}")
  elseif(_basename STREQUAL "httpcore")
    list(APPEND _module_dependencies "${CMAKE_CURRENT_BINARY_DIR}/httpcodec.rxbin")
  elseif(_basename STREQUAL "http")
    list(APPEND _module_dependencies
      "${CMAKE_CURRENT_BINARY_DIR}/httpcodec.rxbin"
      "${CMAKE_CURRENT_BINARY_DIR}/httpcore.rxbin")
  elseif(_basename STREQUAL "httpserver" OR _basename STREQUAL "llm")
    list(APPEND _module_dependencies
      "${CMAKE_CURRENT_BINARY_DIR}/httpcore.rxbin"
      "${_rxfnsg_http_stage_image}")
    list(APPEND _module_import_args -i "${_rxfnsg_http_stage_dir}")
  endif()

  add_custom_command(
    OUTPUT "${_binfile}"
    COMMAND $<TARGET_FILE:rxc>
            -x
            ${_module_import_args}
            -i "${CMAKE_BINARY_DIR}/bin"
            -o "${_basename}"
            "${_module_source}"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${_module_source}"
      "${CMAKE_BINARY_DIR}/bin/library.rxbin"
      "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
      rxc
      rxas
      library
      classlib
      compiler_exit_bin
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
    -P ${CMAKE_CURRENT_SOURCE_DIR}/../unicode/check_casefold_rxas.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(rxunicode_casefold_rxas_shape PROPERTIES
  LABELS "library;unicode;casefold;levelg;shape")

add_test(
  NAME rxunicode_casefold_contract_surface
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/unicode.crexx
    -DDOC=${CMAKE_CURRENT_SOURCE_DIR}/unicode.md
    -DTEST=${CMAKE_CURRENT_SOURCE_DIR}/../tests_functional/ts_unicode_casefold.crexx
    -P ${CMAKE_CURRENT_SOURCE_DIR}/../unicode/check_casefold_contract.cmake
)
set_tests_properties(rxunicode_casefold_contract_surface PROPERTIES
  LABELS "library;unicode;casefold;levelg;contract")

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
    -DMAX_OUTPUT_BYTES=20000000
    -P ${CMAKE_SOURCE_DIR}/linker/tests/check_rxdas_contains.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(rxfnsg_rxdas_inline_preserve_smoke PROPERTIES LABELS "inline;library")
