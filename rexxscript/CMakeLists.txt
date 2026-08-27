cmake_minimum_required(VERSION 3.24)

set(REXXSCRIPT_MODULES
  RexxScriptEvaluator
  RexxScriptRuntime
  RexxScriptEvaluateCompat
)

set(REXXSCRIPT_BINS)
set(REXXSCRIPT_MEMBER_ROOT "${CMAKE_CURRENT_BINARY_DIR}/members")
foreach(MOD IN LISTS REXXSCRIPT_MODULES)
  list(APPEND REXXSCRIPT_BINS
    "${REXXSCRIPT_MEMBER_ROOT}/${MOD}/${MOD}.rxbin")
endforeach()

set(REXXSCRIPT_MEMBER_BUILD_DEPS
  rxc
  rxas
  library
  rxfnsc
)

set(REXXSCRIPT_IMPORT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/imports")
set(REXXSCRIPT_IMPORT_LIBRARY "${REXXSCRIPT_IMPORT_ROOT}/library.rxbin")
set(REXXSCRIPT_IMPORT_RXFNSC "${REXXSCRIPT_IMPORT_ROOT}/rxfnsc.rxbin")
add_custom_command(
  OUTPUT
    "${REXXSCRIPT_IMPORT_LIBRARY}"
    "${REXXSCRIPT_IMPORT_RXFNSC}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${REXXSCRIPT_IMPORT_ROOT}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${REXXSCRIPT_IMPORT_LIBRARY}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
    "${REXXSCRIPT_IMPORT_RXFNSC}"
  DEPENDS
    library
    rxfnsc
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
  COMMENT "Staging immutable RexxScript compiler imports ..."
  VERBATIM
)
add_custom_target(rexxscript_imports
  DEPENDS "${REXXSCRIPT_IMPORT_LIBRARY}" "${REXXSCRIPT_IMPORT_RXFNSC}"
)

set(REXXSCRIPT_SELECTORS ${REXXSCRIPT_MODULES})
set(REXXSCRIPT_DEPS_RexxScriptRuntime RexxScriptEvaluator)
set(REXXSCRIPT_DEPS_RexxScriptEvaluateCompat RexxScriptRuntime)

foreach(_binfile IN LISTS REXXSCRIPT_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)
  get_filename_component(_member_dir "${_binfile}" DIRECTORY)
  set(_rexxscript_deps)
  set(_rexxscript_import_roots "${REXXSCRIPT_IMPORT_ROOT}")
  foreach(_dep IN LISTS REXXSCRIPT_DEPS_${_basename})
    list(APPEND _rexxscript_deps
      "${REXXSCRIPT_MEMBER_ROOT}/${_dep}/${_dep}.rxbin")
    list(APPEND _rexxscript_import_roots
      "${REXXSCRIPT_MEMBER_ROOT}/${_dep}")
  endforeach()
  add_custom_command(
    OUTPUT "${_binfile}"
    BYPRODUCTS "${_member_dir}/${_basename}.rxas"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_member_dir}"
    COMMAND $<TARGET_FILE:rxc>
    -x
    --no-exe-import
    -i "${_rexxscript_import_roots}"
    -o "${_basename}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      rexxscript_imports
      "${REXXSCRIPT_IMPORT_LIBRARY}"
      "${REXXSCRIPT_IMPORT_RXFNSC}"
      ${_rexxscript_deps}
      ${REXXSCRIPT_MEMBER_BUILD_DEPS}
    WORKING_DIRECTORY "${_member_dir}"
    COMMENT "cREXX compile and assemble RexxScript runtime ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_rexxscript_control_file ${CMAKE_CURRENT_BINARY_DIR}/rexxscript_link_$<CONFIG>.ctl)
set(_rexxscript_link_root "${CMAKE_CURRENT_BINARY_DIR}/linked")
set(_rexxscript_link_image "${_rexxscript_link_root}/rexxscript.rxbin")
set(_rexxscript_publish_image "${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin")
set(_rexxscript_publish_temp "${CMAKE_CURRENT_BINARY_DIR}/publish/rexxscript.rxbin.tmp")
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
  "OUTPUT ${_rexxscript_link_root}/rexxscript\n")

file(GENERATE OUTPUT ${_rexxscript_control_file} CONTENT "${_rexxscript_control_text}")

add_custom_command(
  OUTPUT "${_rexxscript_link_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_rexxscript_link_root}"
  COMMAND $<TARGET_FILE:rxlink> -c ${_rexxscript_control_file}
  DEPENDS rxlink ${REXXSCRIPT_BINS} ${_rexxscript_control_file}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked RexxScript Runtime Library Image ..."
  VERBATIM
)

add_custom_command(
  OUTPUT "${_rexxscript_publish_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/publish"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${_rexxscript_link_image}"
    "${_rexxscript_publish_temp}"
  COMMAND ${CMAKE_COMMAND} -E rename
    "${_rexxscript_publish_temp}"
    "${_rexxscript_publish_image}"
  DEPENDS "${_rexxscript_link_image}"
  COMMENT "Publishing consolidated RexxScript runtime library image ..."
  VERBATIM
)

add_custom_target(rexxscript ALL
  DEPENDS "${_rexxscript_publish_image}"
)

add_dependencies(rexxscript library rxfnsc)

set(REXXSCRIPT_RUNNER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/RexxScriptRunner.crexx")
set(REXXSCRIPT_RUNNER_C "${CMAKE_CURRENT_BINARY_DIR}/RexxScriptRunner.c")
add_custom_command(
  OUTPUT "${REXXSCRIPT_RUNNER_C}"
  BYPRODUCTS
    "${CMAKE_CURRENT_BINARY_DIR}/RexxScriptRunner.rxbin"
    "${CMAKE_CURRENT_BINARY_DIR}/RexxScriptRunner_linked.rxbin"
  COMMAND $<TARGET_FILE:rxc>
    -x
    --no-exe-import
    -i "${CMAKE_BINARY_DIR}/bin"
    -o RexxScriptRunner
    "${REXXSCRIPT_RUNNER_SOURCE}"
  COMMAND $<TARGET_FILE:rxas> RexxScriptRunner
  COMMAND $<TARGET_FILE:rxlink> -s -o RexxScriptRunner_linked
    RexxScriptRunner
    "${CMAKE_BINARY_DIR}/bin/library"
    "${CMAKE_BINARY_DIR}/bin/classlib"
    "${CMAKE_BINARY_DIR}/bin/rxfnsc"
    "${CMAKE_BINARY_DIR}/bin/rexxscript"
  COMMAND $<TARGET_FILE:rxcpack> -o RexxScriptRunner RexxScriptRunner_linked
  DEPENDS
    rxas
    rxc
    rxlink
    rxcpack
    library
    classlib
    rxfnsc
    rexxscript
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin"
    "${REXXSCRIPT_RUNNER_SOURCE}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX compile, assemble, link and package standalone RexxScript runner ..."
  VERBATIM
)

add_executable(rexxscript_cli "${REXXSCRIPT_RUNNER_C}")
set_target_properties(rexxscript_cli PROPERTIES OUTPUT_NAME rexxscript)
target_link_libraries(rexxscript_cli rxvml rxpashim)
add_dependencies(rexxscript_cli rxvme rxvml rxpa avl_tree platform)

# The compiler-exit RexxScript fixtures are declared before this subdirectory
# and also name the consolidated image as a file prerequisite. Ninja can infer
# the cross-directory producer, but Unix Makefiles require the target-level
# ordering edge so they do not report bin/rexxscript.rxbin as rule-less during
# a clean parallel build.
foreach(_rexxscript_exit_consumer IN ITEMS
    test_rexxscript_bin
    test_rexxscript_direct_bin)
  if(TARGET ${_rexxscript_exit_consumer})
    add_dependencies(${_rexxscript_exit_consumer} rexxscript)
  endif()
endforeach()

add_subdirectory(tests_functional)
