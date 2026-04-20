cmake_minimum_required(VERSION 3.24)

project(test_classlib)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(REXXBINS
  testid
  testOS
  testArrayList
  testHashMap
  testHashSet
  testTreeMap
  testTreeSet
  testrexx
  testStack
  ts_TreeMap
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_target(testclasses ALL)

unset(_prev_target)
foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rexx")
    set(_rexx_dep "${_basename}.rexx")


    add_custom_target(${_basename})
    add_dependencies(testbifs ${_basename})
    set(_rxc_flags -x)
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS ${_rxc_flags}
            DEPENDS library ${_rexx_dep} ${_prev_target}
            RUNTIME_ARGS ${LIBRPATH}
    )
    set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()
