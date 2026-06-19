cmake_minimum_required(VERSION 3.24)

project(test_rxfnsc_runtime)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(REXXBINS
  testRexxValue
  testRexxRuntimePools
)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(LIBRPATH_RXBIN ${LIBRPATH}.rxbin)
set(CLASSLIB_RXBIN ${CMAKE_BINARY_DIR}/bin/classlib.rxbin)
set(RXFNSC_RXBIN ${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin)
set(RXFNSC_RUNTIMES
  rx_treemap
  ${LIBRPATH}
  ${CMAKE_BINARY_DIR}/bin/classlib
  ${CMAKE_BINARY_DIR}/bin/rxfnsc
)

add_custom_target(testrxfnsc ALL)

unset(_prev_target)
foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)

    set(_rexx_file "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx")
    set(_rexx_dep "${_basename}.crexx")

    add_custom_target(${_basename})
    add_dependencies(testrxfnsc ${_basename} rxfnsc)
    set(_rxc_flags -x)
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${_rexx_file}
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            RXC_FLAGS ${_rxc_flags}
            DEPENDS library classlib rxfnsc ${LIBRPATH_RXBIN} ${CLASSLIB_RXBIN} ${RXFNSC_RXBIN} _treemap ${_rexx_dep} ${_prev_target}
            RUNTIME_ARGS ${RXFNSC_RUNTIMES}
    )
    set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()
