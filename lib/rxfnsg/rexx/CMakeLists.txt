cmake_minimum_required(VERSION 3.24)

set(RXFNSG_REXX_MODULES
        integer
        decimal
        packednumeric
        statsvalue
        httpcodec
  httpcore
  http
  httpserver
  llm
)

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

  set(_module_dependencies)
  set(_module_import_args)
  if(_basename STREQUAL "httpcore")
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
            "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
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
  NAME rxfnsg_rxdas_inline_preserve_smoke
  COMMAND ${CMAKE_COMMAND}
    -DRXDAS=$<TARGET_FILE:rxdas>
    -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
    -DINPUT_RXBIN=${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin
    -DOUTPUT_RXAS=${CMAKE_CURRENT_BINARY_DIR}/rxfnsg_inline_check.rxas
    "-DEXPECT_TEXT_1=.meta \"rxfnsg.ollama.status\"=\".inline\""
    "-DEXPECT_TEXT_2=.meta \"rxfnsg.openai.status\"=\".inline\""
    -P ${CMAKE_SOURCE_DIR}/linker/tests/check_rxdas_contains.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(rxfnsg_rxdas_inline_preserve_smoke PROPERTIES LABELS "inline;library")

crexx_finalize_directory_qa_tiers()
