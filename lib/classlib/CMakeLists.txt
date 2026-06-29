cmake_minimum_required(VERSION 3.24)

set(CLSS
  StringIterable
  StringIterator
  ObjectIterable
  ObjectIterator
  StringArrayList
  StringArrayListIterator
  ObjectArrayList
  ObjectArrayListIterator
  StringHashMap
  StringHashMapIterator
  StringObjectHashMap
  StringObjectHashMapIterator
  StringHashSet
  StringHashSetIterator
  StringLinkedList
  ObjectLinkedList
  Rexx
  StringStack
  ObjectStack
  StringTreeMap
  StringTreeMapIterator
  StringObjectTreeMap
  StringObjectTreeMapIterator
  StringTreeSet
  StringTreeSetIterator
  Qfind
  Scanlex
)

set(CLASSLIB_NATIVE_CLSS
  Id
  KeyDB
  Os
)

set(CLSBINS)
set(CLSSOURCES)
foreach(CLS IN LISTS CLSS)
  list(APPEND CLSBINS "${CMAKE_CURRENT_BINARY_DIR}/${CLS}.rxbin")
  list(APPEND CLSSOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${CLS}.crexx")
endforeach()

set(CLASSLIB_MEMBER_BUILD_DEPS
  rxc
  rxas
  library
  compiler_exit_bin
)

set(_classlib_member_clean_stamp "${CMAKE_CURRENT_BINARY_DIR}/classlib_member_clean.stamp")
set(_classlib_member_clean_paths "${CMAKE_BINARY_DIR}/bin/classlib.rxbin" ${CLSBINS})
if(WIN32)
  set(_classlib_member_clean_command
    COMMAND ${CMAKE_COMMAND}
      "-DREMOVE_PATHS=${_classlib_member_clean_paths}"
      -DREMOVE_RETRIES=120
      -DREMOVE_DELAY=0.25
      -P "${CMAKE_SOURCE_DIR}/cmake/CrexxRemoveWithRetry.cmake")
else()
  set(_classlib_member_clean_command
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/classlib.rxbin" ${CLSBINS})
endif()
add_custom_command(
  OUTPUT "${_classlib_member_clean_stamp}"
  ${_classlib_member_clean_command}
  COMMAND ${CMAKE_COMMAND} -E touch "${_classlib_member_clean_stamp}"
  DEPENDS ${CLSSOURCES} ${CLASSLIB_MEMBER_BUILD_DEPS}
  COMMENT "Clearing stale class library member images ..."
  VERBATIM
)

set(CLASSLIB_SELECTORS ${CLSS})
set(CLASS_DEPS_StringIterable StringIterator)
set(CLASS_DEPS_ObjectIterable ObjectIterator)
set(CLASS_DEPS_StringArrayList StringIterable)
set(CLASS_DEPS_StringArrayListIterator StringArrayList StringIterator)
set(CLASS_DEPS_ObjectArrayList ObjectIterable)
set(CLASS_DEPS_ObjectArrayListIterator ObjectArrayList ObjectIterator)
set(CLASS_DEPS_StringHashMap StringTreeSet StringHashMapIterator StringIterable)
set(CLASS_DEPS_StringHashMapIterator StringIterator)
set(CLASS_DEPS_StringObjectHashMap StringTreeSet StringObjectHashMapIterator StringIterator StringIterable)
set(CLASS_DEPS_StringObjectHashMapIterator StringIterator)
set(CLASS_DEPS_StringHashSet StringHashSetIterator)
set(CLASS_DEPS_StringHashSetIterator StringIterator)
set(CLASS_DEPS_StringTreeMap StringIterable StringTreeMapIterator StringTreeSet)
set(CLASS_DEPS_StringTreeMapIterator StringIterator)
set(CLASS_DEPS_StringObjectTreeMap StringTreeSet StringObjectTreeMapIterator StringIterator StringIterable)
set(CLASS_DEPS_StringObjectTreeMapIterator StringIterator)
set(CLASS_DEPS_StringTreeSet StringIterable StringIterator StringTreeSetIterator)
set(CLASS_DEPS_StringTreeSetIterator StringIterator)

if(WIN32)
  set(_classlib_remove_consolidated_command
    COMMAND ${CMAKE_COMMAND}
      "-DREMOVE_PATHS=${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
      -DREMOVE_RETRIES=120
      -DREMOVE_DELAY=0.25
      -P "${CMAKE_SOURCE_DIR}/cmake/CrexxRemoveWithRetry.cmake")
else()
  set(_classlib_remove_consolidated_command
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
endif()

foreach(_binfile IN LISTS CLSBINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)
  set(_class_deps)
  foreach(_dep IN LISTS CLASS_DEPS_${_basename})
    list(APPEND _class_deps "${CMAKE_CURRENT_BINARY_DIR}/${_dep}.rxbin")
  endforeach()
  add_custom_command(
    OUTPUT "${_binfile}"
    # Do not let classlib members import the previous consolidated classlib image
    # while regenerating the member artifacts that will replace it.
    ${_classlib_remove_consolidated_command}
    COMMAND $<TARGET_FILE:rxc>
    -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}"
    -o "${_basename}"
    "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      "${_classlib_member_clean_stamp}"
      ${_class_deps}
      ${CLASSLIB_MEMBER_BUILD_DEPS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "cREXX compile and assemble ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_classlib_control_file ${CMAKE_CURRENT_BINARY_DIR}/classlib_link_$<CONFIG>.ctl)
string(CONCAT _classlib_control_text
  "* Auto-generated control file for the consolidated cREXX class library\n")
foreach(_cls_bin IN LISTS CLSBINS)
  string(APPEND _classlib_control_text "INPUT ${_cls_bin}\n")
endforeach()
foreach(_selector IN LISTS CLASSLIB_SELECTORS)
  string(APPEND _classlib_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _classlib_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${CMAKE_BINARY_DIR}/bin/classlib\n")

file(GENERATE OUTPUT ${_classlib_control_file} CONTENT "${_classlib_control_text}")

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c ${_classlib_control_file}
  DEPENDS rxlink ${CLSBINS} ${_classlib_control_file}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked Class Library Image ..."
  VERBATIM
)

add_custom_target(classlib ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
)

add_dependencies(classlib library)

set(CLASSLIB_NATIVE_BINS)
set(CLASSLIB_NATIVE_SOURCES)
foreach(CLS IN LISTS CLASSLIB_NATIVE_CLSS)
  list(APPEND CLASSLIB_NATIVE_BINS "${CMAKE_CURRENT_BINARY_DIR}/${CLS}.rxbin")
  list(APPEND CLASSLIB_NATIVE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${CLS}.crexx")
endforeach()

set(CLASSLIB_NATIVE_MEMBER_BUILD_DEPS
  rxc
  rxas
  library
  compiler_exit_bin
  _id
  _keyaccess
  _system
)

set(_classlib_native_member_clean_stamp "${CMAKE_CURRENT_BINARY_DIR}/classlib_native_member_clean.stamp")
set(_classlib_native_member_clean_paths "${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin" ${CLASSLIB_NATIVE_BINS})
if(WIN32)
  set(_classlib_native_member_clean_command
    COMMAND ${CMAKE_COMMAND}
      "-DREMOVE_PATHS=${_classlib_native_member_clean_paths}"
      -DREMOVE_RETRIES=120
      -DREMOVE_DELAY=0.25
      -P "${CMAKE_SOURCE_DIR}/cmake/CrexxRemoveWithRetry.cmake")
else()
  set(_classlib_native_member_clean_command
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin" ${CLASSLIB_NATIVE_BINS})
endif()
add_custom_command(
  OUTPUT "${_classlib_native_member_clean_stamp}"
  ${_classlib_native_member_clean_command}
  COMMAND ${CMAKE_COMMAND} -E touch "${_classlib_native_member_clean_stamp}"
  DEPENDS ${CLASSLIB_NATIVE_SOURCES} ${CLASSLIB_NATIVE_MEMBER_BUILD_DEPS}
  COMMENT "Clearing stale native-backed class library member images ..."
  VERBATIM
)

if(WIN32)
  set(_classlib_native_remove_consolidated_command
    COMMAND ${CMAKE_COMMAND}
      "-DREMOVE_PATHS=${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin"
      -DREMOVE_RETRIES=120
      -DREMOVE_DELAY=0.25
      -P "${CMAKE_SOURCE_DIR}/cmake/CrexxRemoveWithRetry.cmake")
else()
  set(_classlib_native_remove_consolidated_command
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin")
endif()

foreach(_binfile IN LISTS CLASSLIB_NATIVE_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)
  add_custom_command(
    OUTPUT "${_binfile}"
    ${_classlib_native_remove_consolidated_command}
    COMMAND $<TARGET_FILE:rxc>
      -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}"
      -o "${_basename}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      "${_classlib_native_member_clean_stamp}"
      ${CLASSLIB_NATIVE_MEMBER_BUILD_DEPS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "cREXX compile and assemble native-backed classlib adapter ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_classlib_native_control_file ${CMAKE_CURRENT_BINARY_DIR}/classlib_native_link_$<CONFIG>.ctl)
string(CONCAT _classlib_native_control_text
  "* Auto-generated control file for native-backed cREXX class library adapters\n")
foreach(_cls_bin IN LISTS CLASSLIB_NATIVE_BINS)
  string(APPEND _classlib_native_control_text "INPUT ${_cls_bin}\n")
endforeach()
foreach(_selector IN LISTS CLASSLIB_NATIVE_CLSS)
  string(APPEND _classlib_native_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _classlib_native_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${CMAKE_BINARY_DIR}/bin/classlib_native\n")

file(GENERATE OUTPUT ${_classlib_native_control_file} CONTENT "${_classlib_native_control_text}")

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c ${_classlib_native_control_file}
  DEPENDS rxlink ${CLASSLIB_NATIVE_BINS} ${_classlib_native_control_file}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked native-backed Class Library Adapter Image ..."
  VERBATIM
)

add_custom_target(classlib_native ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin"
)

add_dependencies(classlib_native library _id _keyaccess _system)

add_test(
  NAME classlib_rxdas_inline_preserve_smoke
  COMMAND ${CMAKE_COMMAND}
    -DRXDAS=$<TARGET_FILE:rxdas>
    -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
    -DINPUT_RXBIN=${CMAKE_BINARY_DIR}/bin/classlib.rxbin
    -DOUTPUT_RXAS=${CMAKE_CURRENT_BINARY_DIR}/classlib_inline_check.rxas
    "-DEXPECT_TEXT_1=.meta \"data.stringarraylist.init\"=\".inline\""
    "-DEXPECT_TEXT_2=.meta \"data.stringhashmap.size\"=\".inline\""
    "-DEXPECT_ABSENT_1=.meta \"data_stringlist.stringarraylist.init\"=\".inline\""
    "-DEXPECT_ABSENT_2=.meta \"data_stringhashmap.stringhashmap.size\"=\".inline\""
    -P ${CMAKE_SOURCE_DIR}/linker/tests/check_rxdas_contains.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(classlib_rxdas_inline_preserve_smoke PROPERTIES LABELS "inline;library")

set(_classlib_rexxdoc_check_source "${CMAKE_SOURCE_DIR}/tools/rexxDoc/check_rexxdoc_coverage.crexx")
set(_classlib_rexxdoc_check_script "${CMAKE_CURRENT_BINARY_DIR}/run_classlib_rexxdoc_coverage.cmake")
set(_classlib_rexxdoc_check_content "")
string(APPEND _classlib_rexxdoc_check_content
  "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"check_rexxdoc_coverage\" \"${_classlib_rexxdoc_check_source}\" WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND _classlib_rexxdoc_check_content
  "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed for RexxDoc coverage checker: \${out}\${err}\")\nendif()\n")
string(APPEND _classlib_rexxdoc_check_content
  "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" \"check_rexxdoc_coverage\" WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND _classlib_rexxdoc_check_content
  "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed for RexxDoc coverage checker: \${out}\${err}\")\nendif()\n")
string(APPEND _classlib_rexxdoc_check_content
  "execute_process(COMMAND \"$<TARGET_FILE:rxvm>\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${CMAKE_CURRENT_BINARY_DIR}/check_rexxdoc_coverage.rxbin\" -a \"${CMAKE_SOURCE_DIR}/tools/rexxDoc/classlib_rexxdoc_baseline.txt\" \"${CMAKE_SOURCE_DIR}/lib\" WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND _classlib_rexxdoc_check_content
  "message(STATUS \"\${out}\${err}\")\nif(NOT res EQUAL 0)\n  message(FATAL_ERROR \"RexxDoc coverage check failed\")\nendif()\n")
file(GENERATE OUTPUT "${_classlib_rexxdoc_check_script}" CONTENT "${_classlib_rexxdoc_check_content}")

add_test(
  NAME classlib_rexxdoc_coverage
  COMMAND ${CMAKE_COMMAND} -P "${_classlib_rexxdoc_check_script}"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(classlib_rexxdoc_coverage PROPERTIES LABELS "docs;classlib")

add_subdirectory(tests_functional)
