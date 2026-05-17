cmake_minimum_required(VERSION 3.24)

set(CLSS
  StemList
  StemListIterator
  ArrayList
  ArrayListIterator
  HashMap
  HashMapIterator
  HashSet
  HashSetIterator
  Id
  KeyDB
  LinkedList
  Rexx
  Stack
  TreeMap
  TreeMapIterator
  TreeSet
  TreeSetIterator
  Os
  Qfind
)

set(CLSBINS)
foreach(CLS IN LISTS CLSS)
  list(APPEND CLSBINS "${CMAKE_CURRENT_BINARY_DIR}/${CLS}.rxbin")
endforeach()

set(CLASSLIB_SELECTORS ${CLSS})

foreach(_binfile IN LISTS CLSBINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  add_custom_command(
    OUTPUT "${_binfile}"
    COMMAND $<TARGET_FILE:rxc>
            -i "${CMAKE_BINARY_DIR}/bin"
            -o "${_basename}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      rxc
      rxas
      library
      compiler_exit_bin
      lib/plugins/_system
      lib/plugins/_treemap
      lib/plugins/_llist
      lib/plugins/_keyaccess
      lib/plugins/_id
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

add_subdirectory(tests_functional)
