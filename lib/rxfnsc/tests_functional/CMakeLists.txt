cmake_minimum_required(VERSION 3.24)

project(test_rxfnsc_runtime)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

set(REXXBINS
  testRexxValue
  testRexxActivationArguments
  testRexxClassicConditionEvent
  testRexxRuntimePools
  testRexxDoState
  testRexxClassicBifs
  testRexxClassicBifAbbrev
  testRexxClassicBifAbs
  testRexxClassicBifAddress
  testRexxClassicBifArg
  testRexxClassicBifB2x
  testRexxClassicBifBitwise
  testRexxClassicBifC2d
  testRexxClassicBifC2x
  testRexxClassicBifCenter
  testRexxClassicBifCentre
  testRexxClassicBifChangestr
  testRexxClassicBifCompare
  testRexxClassicBifCondition
  testRexxClassicBifCopies
  testRexxClassicBifCountstr
  testRexxClassicBifD2x
  testRexxClassicBifD2c
  testRexxClassicBifDate
  testRexxClassicBifDatatype
  testRexxClassicBifDelstr
  testRexxClassicBifDelword
  testRexxClassicBifErrortext
  testRexxClassicBifFormat
  testRexxClassicBifInsert
  testRexxClassicBifLastpos
  testRexxClassicBifLeft
  testRexxClassicBifLength
  testRexxClassicBifMax
  testRexxClassicBifMin
  testRexxClassicBifNumeric
  testRexxClassicBifOverlay
  testRexxClassicBifPos
  testRexxClassicBifQueued
  testRexxClassicBifRandom
  testRexxClassicBifReverse
  testRexxClassicBifRight
  testRexxClassicBifSign
  testRexxClassicBifSourceline
  testRexxClassicBifSpace
  testRexxClassicBifStrip
  testRexxClassicBifSubstr
  testRexxClassicBifSubword
  testRexxClassicBifSymbol
  testRexxClassicBifTime
  testRexxClassicBifTrace
  testRexxClassicBifTranslate
  testRexxClassicBifTrunc
  testRexxClassicBifValue
  testRexxClassicBifVerify
  testRexxClassicBifWord
  testRexxClassicBifWordindex
  testRexxClassicBifWordlength
  testRexxClassicBifWordpos
  testRexxClassicBifWords
  testRexxClassicBifX2b
  testRexxClassicBifX2c
  testRexxClassicBifX2d
  testRexxClassicBifXrange
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(LIBRPATH_RXBIN ${LIBRPATH}.rxbin)
set(CLASSLIB_RXBIN ${CMAKE_BINARY_DIR}/bin/classlib.rxbin)
set(RXFNSC_RXBIN ${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin)
set(RXFNSC_RUNTIMES
  ${LIBRPATH}
  ${CMAKE_BINARY_DIR}/bin/classlib
  ${CMAKE_BINARY_DIR}/bin/rxfnsc
)

add_custom_target(testrxfnsc ALL)

foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
    set(_rexx_dep "${_basename}.crexx")

    add_custom_target(${_basename})
    add_dependencies(testrxfnsc ${_basename} rxfnsc)
    set(_rxc_flags -x)
    set(_pass_test_name_argument)
    if(_basename STREQUAL "testRexxClassicConditionEvent")
        set(_pass_test_name_argument PASS_TEST_NAME_ARGUMENT)
    endif()
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS ${_rxc_flags}
            DEPENDS library classlib rxfnsc ${LIBRPATH_RXBIN} ${CLASSLIB_RXBIN} ${RXFNSC_RXBIN} ${_rexx_dep}
            RUNTIME_ARGS ${RXFNSC_RUNTIMES}
            ${_pass_test_name_argument}
    )
endforeach()

crexx_label_smoke_tests(
        testRexxValue_noopt
        testRexxRuntimePools_noopt
        testRexxClassicBifs_noopt
)
