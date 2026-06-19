cmake_minimum_required(VERSION 3.24)

set(RXFNSC_MODULES
  RexxValue
  RexxStem
  RexxVariablePool
)

set(RXFNSC_BINS)
set(RXFNSC_SOURCES)
foreach(MOD IN LISTS RXFNSC_MODULES)
  list(APPEND RXFNSC_BINS "${CMAKE_CURRENT_BINARY_DIR}/${MOD}.rxbin")
  list(APPEND RXFNSC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${MOD}.crexx")
endforeach()

set(RXFNSC_MEMBER_BUILD_DEPS
  rxc
  rxas
  library
  classlib
  compiler_exit_bin
  lib/plugins/_treemap
)

set(_rxfnsc_member_clean_stamp "${CMAKE_CURRENT_BINARY_DIR}/rxfnsc_member_clean.stamp")
add_custom_command(
  OUTPUT "${_rxfnsc_member_clean_stamp}"
  COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin" ${RXFNSC_BINS}
  COMMAND ${CMAKE_COMMAND} -E touch "${_rxfnsc_member_clean_stamp}"
  DEPENDS ${RXFNSC_SOURCES} ${RXFNSC_MEMBER_BUILD_DEPS}
  COMMENT "Clearing stale rxfnsc runtime member images ..."
  VERBATIM
)

set(RXFNSC_SELECTORS ${RXFNSC_MODULES})
set(RXFNSC_DEPS_RexxStem RexxValue)
set(RXFNSC_DEPS_RexxVariablePool RexxValue RexxStem)

foreach(_binfile IN LISTS RXFNSC_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)
  set(_rxfnsc_deps)
  foreach(_dep IN LISTS RXFNSC_DEPS_${_basename})
    list(APPEND _rxfnsc_deps "${CMAKE_CURRENT_BINARY_DIR}/${_dep}.rxbin")
  endforeach()
  add_custom_command(
    OUTPUT "${_binfile}"
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
    COMMAND $<TARGET_FILE:rxc>
    -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}"
    -o "${_basename}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      "${_rxfnsc_member_clean_stamp}"
      ${_rxfnsc_deps}
      ${RXFNSC_MEMBER_BUILD_DEPS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "cREXX compile and assemble rxfnsc runtime ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_rxfnsc_control_file ${CMAKE_CURRENT_BINARY_DIR}/rxfnsc_link_$<CONFIG>.ctl)
string(CONCAT _rxfnsc_control_text
  "* Auto-generated control file for the consolidated rxfnsc runtime library\n")
foreach(_rxfnsc_bin IN LISTS RXFNSC_BINS)
  string(APPEND _rxfnsc_control_text "INPUT ${_rxfnsc_bin}\n")
endforeach()
foreach(_selector IN LISTS RXFNSC_SELECTORS)
  string(APPEND _rxfnsc_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _rxfnsc_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsc\n")

file(GENERATE OUTPUT ${_rxfnsc_control_file} CONTENT "${_rxfnsc_control_text}")

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c ${_rxfnsc_control_file}
  DEPENDS rxlink ${RXFNSC_BINS} ${_rxfnsc_control_file}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked rxfnsc Runtime Library Image ..."
  VERBATIM
)

add_custom_target(rxfnsc ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
)

add_dependencies(rxfnsc library classlib)

add_subdirectory(tests_functional)
