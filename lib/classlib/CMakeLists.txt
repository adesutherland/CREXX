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
add_custom_command(
  OUTPUT "${_classlib_member_clean_stamp}"
  COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/classlib.rxbin" ${CLSBINS}
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
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
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

add_test(
  NAME classlib_rxdas_inline_preserve_smoke
  COMMAND ${CMAKE_COMMAND}
    -DRXDAS=$<TARGET_FILE:rxdas>
    -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
    -DINPUT_RXBIN=${CMAKE_BINARY_DIR}/bin/classlib.rxbin
    -DOUTPUT_RXAS=${CMAKE_CURRENT_BINARY_DIR}/classlib_inline_check.rxas
    "-DEXPECT_TEXT_1=.meta \"data_stringlist.stringarraylist.init\"=\".inline\""
    "-DEXPECT_TEXT_2=.meta \"data_stringhashmap.stringhashmap.size\"=\".inline\""
    -P ${CMAKE_SOURCE_DIR}/linker/tests/check_rxdas_contains.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(classlib_rxdas_inline_preserve_smoke PROPERTIES LABELS "inline;library")

add_subdirectory(tests_functional)
