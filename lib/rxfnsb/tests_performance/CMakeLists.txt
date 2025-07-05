cmake_minimum_required(VERSION 3.24)

project(test_bifs)

enable_testing()

set(REXXBINS
        we.rxbin
        test1.rxbin
        crexxbrexx.rxbin
)

set(LIBRPATH ../library)

add_custom_target(testperfbifs ALL DEPENDS ${REXXBINS})

unset(_prev_target)
foreach(_binfile ${REXXBINS})
    get_filename_component(_basename ${_binfile} NAME_WE)
    add_custom_target(${_basename} DEPENDS ${_binfile}) # Create a "virtual target"

    add_custom_command(
            COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i ${CMAKE_BINARY_DIR}/lib/rxfnsb -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
            COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas ${_basename}
            DEPENDS rxas rxc library ${_basename}.rexx ${_prev_target}
            OUTPUT ${_binfile}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set(_prev_target ${_binfile})

    add_test(NAME ${_basename}-rxvm COMMAND rxvm ${_basename} ${LIBRPATH}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    add_test(NAME ${_basename}-rxbvm COMMAND rxbvm ${_basename} ${LIBRPATH}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

endforeach()
