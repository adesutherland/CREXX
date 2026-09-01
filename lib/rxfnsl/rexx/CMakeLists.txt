cmake_minimum_required(VERSION 3.24)

set(RXFNSL_REXX_MODULES
  tinyexpr
)

set(RXFNSL_REXX_BINS)
foreach(_module IN LISTS RXFNSL_REXX_MODULES)
  list(APPEND RXFNSL_REXX_BINS "${CMAKE_CURRENT_BINARY_DIR}/${_module}.rxbin")
endforeach()

set(_rxfnsl_import_root "${CMAKE_CURRENT_BINARY_DIR}/imports")
crexx_add_import_root(rxfnsl_imports
  DIRECTORY "${_rxfnsl_import_root}"
  OUTPUT_VARIABLE _rxfnsl_import_files
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin"
  DEPENDS library
  COMMENT "Staging deterministic Level L imports ...")

foreach(_binfile IN LISTS RXFNSL_REXX_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  add_custom_command(
    OUTPUT "${_binfile}"
    COMMAND $<TARGET_FILE:rxc>
            -x
            --no-exe-import
            -i "${_rxfnsl_import_root}"
            -o "${_basename}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      rxfnsl_imports
      ${_rxfnsl_import_files}
      rxc
      rxas
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "cREXX compile and assemble ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_rxfnsl_control_file "${CMAKE_CURRENT_BINARY_DIR}/rxfnsl_link_$<CONFIG>.ctl")
string(CONCAT _rxfnsl_control_text
  "* Auto-generated control file for the consolidated cREXX Level L library\n")
foreach(_module_bin IN LISTS RXFNSL_REXX_BINS)
  string(APPEND _rxfnsl_control_text "INPUT ${_module_bin}\n")
endforeach()
foreach(_selector IN LISTS RXFNSL_REXX_MODULES)
  string(APPEND _rxfnsl_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _rxfnsl_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsl\n")

file(GENERATE OUTPUT "${_rxfnsl_control_file}" CONTENT "${_rxfnsl_control_text}")

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_rxfnsl_control_file}"
  DEPENDS rxlink ${RXFNSL_REXX_BINS} "${_rxfnsl_control_file}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked Level L Library Image ..."
  VERBATIM
)

add_custom_target(rxfnsl ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
)

add_dependencies(rxfnsl library)

add_test(
  NAME rxfnsl_tinyexpr_binary_surface_smoke
  COMMAND ${CMAKE_COMMAND}
    -DRXDAS=$<TARGET_FILE:rxdas>
    -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
    -DINPUT_RXBIN=${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin
    -DOUTPUT_RXAS=${CMAKE_CURRENT_BINARY_DIR}/rxfnsl_binary_surface_check.rxas
    "-DEXPECT_TEXT_1=bgetu8"
    "-DEXPECT_TEXT_2=bseti64"
    "-DEXPECT_TEXT_3=bcmpb"
    "-DEXPECT_ABSENT_1=bslice"
    -P ${CMAKE_SOURCE_DIR}/linker/tests/check_rxdas_contains.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(rxfnsl_tinyexpr_binary_surface_smoke PROPERTIES LABELS "library;level-l")

crexx_finalize_directory_qa_tiers()
