cmake_minimum_required(VERSION 3.24)

project(test_bifs)

enable_testing()

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(REXXBINS
        we.rxbin
        test1.rxbin
        crexxbrexx.rxbin
)

set(LIBRPATH ../rxfnsc)

add_custom_target(testperfbifsc ALL)

unset(_prev_target)
foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)
    add_custom_target(${_basename})
    add_dependencies(testperfbifsc ${_basename})
    crexx_add_rexx_opt_matrix(
            NAME ${_basename}
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rexx
            TARGET_GROUP ${_basename}
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS rxfnsc ${_prev_target}
            RUNTIME_ARGS ${LIBRPATH}
    )
    set(_prev_target ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()
