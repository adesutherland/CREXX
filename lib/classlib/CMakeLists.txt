cmake_minimum_required(VERSION 3.24)

set(CLSS
  Stack
  TreeMap
  TreeMapIterator
  TreeSet
  TreeSetIterator
  os
)

set(CLSBINS)
foreach(CLS IN LISTS CLSS)
  list(APPEND CLSBINS "${CMAKE_CURRENT_BINARY_DIR}/${CLS}.rxbin")
endforeach()

foreach(_binfile IN LISTS CLSBINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)

  add_custom_command(
    OUTPUT "${_binfile}"
    COMMAND $<TARGET_FILE:rxc>
            -i "${CMAKE_BINARY_DIR}/bin"
            -o "${_basename}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rexx"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rexx"
      rxc
      rxas
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "cREXX compile and assemble ${_basename} ..."
    VERBATIM
  )
endforeach()

add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
  COMMAND ${CMAKE_COMMAND} -E cat ${CLSBINS} > "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
  DEPENDS ${CLSBINS}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX REXX Class Library ..."
  VERBATIM
)

add_custom_target(classlib ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
)

add_dependencies(classlib library)
