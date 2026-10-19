cmake_minimum_required(VERSION 3.24)

project(test_rxfnsc_runtime)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

set(REXXBINS
  testRexxValue
  testRexxActivationArguments
  testRexxClassicContext
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
    if(_basename STREQUAL "testRexxClassicContext")
        # Explicit handler-skip recovery is part of the borrowed-reference
        # contract; this fixture therefore uses the certified SIGNAL exit.
        set(_rxc_flags)
        add_dependencies(${_basename} compiler_exit_bin)
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

# The borrowed-context lifetime contract must run on both concrete engines.
# Link during preparation so each test is only one VM invocation.
foreach(_mode opt noopt)
    set(_context_image "${CMAKE_CURRENT_BINARY_DIR}/testRexxClassicContext_${_mode}")
    set(_context_runtime ${LIBRPATH_RXBIN} ${CLASSLIB_RXBIN} ${RXFNSC_RXBIN}
        "${CMAKE_BINARY_DIR}/bin/rxunicode-core.rxbin")
    add_custom_command(OUTPUT "${_context_image}-linked.rxbin"
        COMMAND $<TARGET_FILE:rxlink> -o "${_context_image}-linked"
            "${_context_image}.rxbin" ${_context_runtime}
        DEPENDS rxlink rxunicode_core "${_context_image}.rxbin" ${_context_runtime}
        VERBATIM)
    add_custom_target(testRexxClassicContext_${_mode}_linked
        DEPENDS "${_context_image}-linked.rxbin")
    add_dependencies(testRexxClassicContext testRexxClassicContext_${_mode}_linked)
    foreach(_vm rxbvm rxtvm)
        if(TARGET ${_vm})
            set(_test "testRexxClassicContext_${_mode}_${_vm}")
            add_test(NAME ${_test} COMMAND $<TARGET_FILE:${_vm}>
                --no-autoload "${_context_image}-linked.rxbin")
            set_tests_properties(${_test} PROPERTIES TIMEOUT 300
                LABELS "library;levelc;optimizer-parity")
            crexx_register_test_prep_targets(TESTS ${_test}
                TARGETS testRexxClassicContext_${_mode}_linked fs cstream ${_vm})
        endif()
    endforeach()
endforeach()

crexx_label_smoke_tests(
        testRexxValue_noopt
        testRexxRuntimePools_noopt
        testRexxClassicBifs_noopt
)
