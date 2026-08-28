cmake_minimum_required(VERSION 3.24)

project(test_classlib)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxConcurrencyTests.cmake)

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
  testObjectObjectHashMap
  testObjectEquality
  testTreeMap
  testRexxComparator
  testQfind
  testTokenise
  testJSON
#  testBag
  testConcurrency
  testDateTime
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
  # keyaccess predates declarative provider packages and still needs its
  # explicit dynamic-plugin argument. RCC-5 providers autoload from metadata.
  rx_keyaccess
  ${LIBRPATH}
  ${CMAKE_BINARY_DIR}/bin/classlib_native
)

add_custom_target(testclasses ALL)

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
            DEPENDS library classlib ${LIBRPATH_RXBIN} ${CLASSLIB_RXBIN} ${_rexx_dep}
            RUNTIME_ARGS ${CLASSLIB_RUNTIMES}
    )
endforeach()

crexx_label_concurrency_tests(SP03
  testConcurrency_noopt
  testConcurrency_opt)
crexx_label_concurrency_tests(SP05
  testConcurrency_noopt
  testConcurrency_opt)
crexx_label_concurrency_tests(SP08
  testConcurrency_noopt
  testConcurrency_opt)

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
            DEPENDS library classlib_native ${LIBRPATH_RXBIN} ${CLASSLIB_NATIVE_RXBIN} id _keyaccess fs _platform ${_rexx_dep} ${_prev_native_target}
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

# The concurrency book examples are executable documentation. Keep every full
# source program on the normal compile, assemble, link and both-VM path in both
# optimization modes.
set(CONCURRENCY_DOC_EXAMPLES
  concurrency_basic
  concurrency_providers
  concurrency_transferable_objects
  concurrency_taskwork
)
set(CONCURRENCY_DOC_EXAMPLE_DIR
  ${CMAKE_SOURCE_DIR}/docs/books/crexx_programming_guide/examples)

add_custom_target(concurrency_doc_examples)
add_dependencies(testclasses concurrency_doc_examples)

set(_concurrency_doc_runners rxbvm)
if(TARGET rxtvm)
  list(APPEND _concurrency_doc_runners rxtvm)
endif()

foreach(_example IN LISTS CONCURRENCY_DOC_EXAMPLES)
  set(_example_source "${CONCURRENCY_DOC_EXAMPLE_DIR}/${_example}.crexx")
  foreach(_runner IN LISTS _concurrency_doc_runners)
    crexx_add_rexx_opt_matrix(
      LINKED_ALL_MODES
      NAME "docs_${_example}_${_runner}"
      SOURCE "${_example_source}"
      TARGET_GROUP concurrency_doc_examples
      RUNNER "${_runner}"
      IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
      RXC_FLAGS -x
      DEPENDS library classlib "${_example_source}"
      RUNTIME_ARGS ${CLASSLIB_RUNTIMES}
      TEST_LABELS docs concurrency levelg linked "${_runner}"
      REQUIRED_OUTPUT_REGEX "PASS:"
      FORBIDDEN_OUTPUT_REGEX "FAIL:"
    )
    crexx_label_concurrency_tests(SP06
      "docs_${_example}_${_runner}_noopt"
      "docs_${_example}_${_runner}_opt")
    crexx_label_concurrency_tests(SP09
      "docs_${_example}_${_runner}_noopt"
      "docs_${_example}_${_runner}_opt")
  endforeach()
endforeach()

# The task context endpoint adapter is reached only inside taskwork. Exercise
# the public method through the complete compiler, assembler, linker and both
# applicable VM paths rather than constructing its private implementation.
add_custom_target(testTaskContextEndpoint)
add_dependencies(testclasses testTaskContextEndpoint)

foreach(_runner IN LISTS _concurrency_doc_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "testTaskContextEndpoint_${_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/testTaskContextEndpoint.crexx"
    TARGET_GROUP testTaskContextEndpoint
    RUNNER "${_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib "${CMAKE_CURRENT_SOURCE_DIR}/testTaskContextEndpoint.crexx"
    RUNTIME_ARGS ${CLASSLIB_RUNTIMES}
    TEST_LABELS classlib concurrency levelg taskcontext endpoint linked "${_runner}"
    REQUIRED_OUTPUT_REGEX "PASS: taskcontext endpoint adapts a type-4 provider reference"
    FORBIDDEN_OUTPUT_REGEX "FAIL:"
  )
  crexx_label_concurrency_tests(SP04
    "testTaskContextEndpoint_${_runner}_noopt"
    "testTaskContextEndpoint_${_runner}_opt")
  crexx_label_concurrency_tests(SP05
    "testTaskContextEndpoint_${_runner}_noopt"
    "testTaskContextEndpoint_${_runner}_opt")
endforeach()

# A request-specific positive wait must retain its full caller-supplied bound.
# Exercise a terminal byte-endpoint deadline through the complete toolchain so
# unit conversion mistakes in the Level B wrapper cannot hide behind fast local
# completions.
add_custom_target(testChannelRequestWait)
add_dependencies(testclasses testChannelRequestWait)

foreach(_runner IN LISTS _concurrency_doc_runners)
  crexx_add_rexx_opt_matrix(
    LINKED_ALL_MODES
    NAME "testChannelRequestWait_${_runner}"
    SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/testChannelRequestWait.crexx"
    TARGET_GROUP testChannelRequestWait
    RUNNER "${_runner}"
    IMPORT_PATHS "${CMAKE_BINARY_DIR}/bin"
    RXC_FLAGS -x
    DEPENDS library classlib "${CMAKE_CURRENT_SOURCE_DIR}/testChannelRequestWait.crexx"
    RUNTIME_ARGS ${CLASSLIB_RUNTIMES}
    TEST_LABELS classlib concurrency levelb channel endpoint deadline linked "${_runner}"
    REQUIRED_OUTPUT_REGEX "PASS: channel request positive wait observes the operation deadline"
    FORBIDDEN_OUTPUT_REGEX "FAIL:"
  )
  crexx_label_concurrency_tests(SP04
    "testChannelRequestWait_${_runner}_noopt"
    "testChannelRequestWait_${_runner}_opt")
  crexx_label_concurrency_tests(SP05
    "testChannelRequestWait_${_runner}_noopt"
    "testChannelRequestWait_${_runner}_opt")
  crexx_label_concurrency_tests(SP08
    "testChannelRequestWait_${_runner}_noopt"
    "testChannelRequestWait_${_runner}_opt")
endforeach()

crexx_finalize_directory_qa_tiers()
