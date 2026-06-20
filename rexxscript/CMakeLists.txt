cmake_minimum_required(VERSION 3.24)

set(REXXSCRIPT_MODULES
  RexxScriptRuntime
  RexxScriptEvaluateCompat
)

set(REXXSCRIPT_BINS)
set(REXXSCRIPT_SOURCES)
foreach(MOD IN LISTS REXXSCRIPT_MODULES)
  list(APPEND REXXSCRIPT_BINS "${CMAKE_CURRENT_BINARY_DIR}/${MOD}.rxbin")
  list(APPEND REXXSCRIPT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${MOD}.crexx")
endforeach()

set(REXXSCRIPT_MEMBER_BUILD_DEPS
  rxc
  rxas
  library
  rxfnsc
)

set(_rexxscript_member_clean_stamp "${CMAKE_CURRENT_BINARY_DIR}/rexxscript_member_clean.stamp")
add_custom_command(
  OUTPUT "${_rexxscript_member_clean_stamp}"
  COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin" ${REXXSCRIPT_BINS}
  COMMAND ${CMAKE_COMMAND} -E touch "${_rexxscript_member_clean_stamp}"
  DEPENDS ${REXXSCRIPT_SOURCES} ${REXXSCRIPT_MEMBER_BUILD_DEPS}
  COMMENT "Clearing stale RexxScript runtime member images ..."
  VERBATIM
)

set(REXXSCRIPT_SELECTORS ${REXXSCRIPT_MODULES})
set(REXXSCRIPT_DEPS_RexxScriptEvaluateCompat RexxScriptRuntime)

foreach(_binfile IN LISTS REXXSCRIPT_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)
  set(_rexxscript_deps)
  foreach(_dep IN LISTS REXXSCRIPT_DEPS_${_basename})
    list(APPEND _rexxscript_deps "${CMAKE_CURRENT_BINARY_DIR}/${_dep}.rxbin")
  endforeach()
  add_custom_command(
    OUTPUT "${_binfile}"
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin"
    COMMAND $<TARGET_FILE:rxc>
    -x
    -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}"
    -o "${_basename}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      "${_rexxscript_member_clean_stamp}"
      ${_rexxscript_deps}
      ${REXXSCRIPT_MEMBER_BUILD_DEPS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "cREXX compile and assemble RexxScript runtime ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_rexxscript_control_file ${CMAKE_CURRENT_BINARY_DIR}/rexxscript_link_$<CONFIG>.ctl)
string(CONCAT _rexxscript_control_text
  "* Auto-generated control file for the consolidated RexxScript runtime library\n")
foreach(_rexxscript_bin IN LISTS REXXSCRIPT_BINS)
  string(APPEND _rexxscript_control_text "INPUT ${_rexxscript_bin}\n")
endforeach()
foreach(_selector IN LISTS REXXSCRIPT_SELECTORS)
  string(APPEND _rexxscript_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _rexxscript_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${CMAKE_BINARY_DIR}/bin/rexxscript\n")

file(GENERATE OUTPUT ${_rexxscript_control_file} CONTENT "${_rexxscript_control_text}")

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c ${_rexxscript_control_file}
  DEPENDS rxlink ${REXXSCRIPT_BINS} ${_rexxscript_control_file}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked RexxScript Runtime Library Image ..."
  VERBATIM
)

add_custom_target(rexxscript ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin"
)

add_dependencies(rexxscript library)

add_subdirectory(tests_functional)
