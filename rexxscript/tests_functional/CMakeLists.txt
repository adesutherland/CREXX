cmake_minimum_required(VERSION 3.24)

project(test_rexxscript_runtime)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(REXXBINS
  testRexxScriptRuntime
  testRexxScriptCompat
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(LIBRPATH_RXBIN ${LIBRPATH}.rxbin)
set(CLASSLIB_RXBIN ${CMAKE_BINARY_DIR}/bin/classlib.rxbin)
set(RXFNSC_RXBIN ${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin)
set(REXXSCRIPT_RXBIN ${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin)
set(REXXSCRIPT_RUNTIMES
  rx_treemap
  ${LIBRPATH}
  ${CMAKE_BINARY_DIR}/bin/classlib
  ${CMAKE_BINARY_DIR}/bin/rxfnsc
  ${CMAKE_BINARY_DIR}/bin/rexxscript
)

add_custom_target(testrexxscript ALL)

unset(_prev_target)
foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
    set(_rexx_dep "${_basename}.crexx")

    add_custom_target(${_basename})
    add_dependencies(testrexxscript ${_basename} rexxscript rxfnsc)
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS -x
            DEPENDS library classlib rxfnsc rexxscript ${LIBRPATH_RXBIN} ${CLASSLIB_RXBIN} ${RXFNSC_RXBIN} ${REXXSCRIPT_RXBIN} _treemap ${_rexx_dep} ${_prev_target}
            RUNTIME_ARGS ${REXXSCRIPT_RUNTIMES}
    )
    set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()

set_tests_properties(testRexxScriptRuntime_noopt testRexxScriptRuntime_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: RexxScript runtime")
set_tests_properties(testRexxScriptCompat_noopt testRexxScriptCompat_opt
        PROPERTIES PASS_REGULAR_EXPRESSION "PASS: evaluate helpers")
