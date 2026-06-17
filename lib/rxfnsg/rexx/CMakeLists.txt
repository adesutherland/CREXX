cmake_minimum_required(VERSION 3.24)

set(RXFNSG_REXX_MODULES
  llm
)

set(RXFNSG_REXX_BINS)
foreach(_module IN LISTS RXFNSG_REXX_MODULES)
  list(APPEND RXFNSG_REXX_BINS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
endforeach()

foreach(_binfile IN LISTS RXFNSG_REXX_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  add_custom_command(
    OUTPUT "${_binfile}"
    COMMAND $<TARGET_FILE:rxc>
            -x
            -i "${CMAKE_BINARY_DIR}/bin"
            -o "${_basename}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      "${CMAKE_BINARY_DIR}/bin/library.rxbin"
      rxc
      rxas
      library
      compiler_exit_bin
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

add_dependencies(rxfnsg library)

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
