cmake_minimum_required(VERSION 3.24)

project(test_classlib)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

set(REXXBINS
  testrexx0
  testrexx
  testStringArrayList
  testStringHashMap
  testStringHashSet
  testStringLinkedList
  testStringTreeMap
  testStringTreeSet
  testStringStack
  testObjectCollections
  testStringObjectMaps
  testObjectObjectTreeMap
  testTreeMap
  testRexxComparator
  testQfind
  testTokenise
  testJSON
)

add_test(
  NAME classlib_rexx_method_test_coverage
  COMMAND ${CMAKE_COMMAND}
    -DCLASS_SOURCE=${CMAKE_SOURCE_DIR}/lib/classlib/Rexx.crexx
    -DTEST_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/testrexx.crexx
    -P ${CMAKE_SOURCE_DIR}/cmake/CheckRexxMethodTestCoverage.cmake
)
set_tests_properties(classlib_rexx_method_test_coverage PROPERTIES LABELS "classlib;coverage")

set(NATIVE_REXXBINS
  testid
  testKeyDB
  testOS
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(LIBRPATH_RXBIN ${LIBRPATH}.rxbin)
set(CLASSLIB_RXBIN ${CMAKE_BINARY_DIR}/bin/classlib.rxbin)
set(CLASSLIB_NATIVE_RXBIN ${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin)
set(CLASSLIB_RUNTIMES
  ${LIBRPATH}
  ${CMAKE_BINARY_DIR}/bin/classlib
)
set(CLASSLIB_NATIVE_RUNTIMES
  rx_id
  rx_keyaccess
  rx_system
  ${LIBRPATH}
  ${CMAKE_BINARY_DIR}/bin/classlib_native
)

add_custom_target(testclasses ALL)

unset(_prev_target)
foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
    set(_rexx_dep "${_basename}.crexx")


    add_custom_target(${_basename})
    add_dependencies(testclasses ${_basename} classlib)
    set(_rxc_flags -x)
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS ${_rxc_flags}
            DEPENDS library classlib ${LIBRPATH_RXBIN} ${CLASSLIB_RXBIN} ${_rexx_dep} ${_prev_target}
            RUNTIME_ARGS ${CLASSLIB_RUNTIMES}
    )
    set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()

unset(_prev_native_target)
foreach(_binfile ${NATIVE_REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
    set(_rexx_dep "${_basename}.crexx")

    add_custom_target(${_basename})
    add_dependencies(testclasses ${_basename} classlib_native)
    set(_rxc_flags -x)
    set(_pass_test_name_argument)
    if("${_basename}" STREQUAL "testKeyDB")
        set(_pass_test_name_argument PASS_TEST_NAME_ARGUMENT)
    endif()
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS ${_rxc_flags}
            DEPENDS library classlib_native ${LIBRPATH_RXBIN} ${CLASSLIB_NATIVE_RXBIN} _id _keyaccess _system ${_rexx_dep} ${_prev_native_target}
            RUNTIME_ARGS ${CLASSLIB_NATIVE_RUNTIMES}
            TEST_LABELS "classlib;native-adapter"
            ${_pass_test_name_argument}
    )
    set(_prev_native_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()

crexx_label_smoke_tests(
        testStringArrayList_noopt
        testStringHashMap_noopt
        testObjectCollections_noopt
        testJSON_noopt
        testOS_noopt
)
