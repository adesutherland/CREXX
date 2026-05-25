cmake_minimum_required(VERSION 3.24)

project(Disassembler C)

set(CMAKE_C_STANDARD 90)

include(CMakeParseArguments)

add_executable(rxdas rxdamain.c rxdadism.h rxdadism.c ../binutils/include/rxbin.h)

add_dependencies(rxdas avl_tree platform)

target_link_libraries(rxdas avl_tree platform)

target_include_directories(rxdas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../assembler ../platform ../binutils/include)

add_custom_target(rxdas_test_prereqs ALL
        DEPENDS rxdas rxas rxvm rxbvm mc_decimal_dynamic)

function(crexx_add_rxdas_roundtrip)
    set(options)
    set(oneValueArgs NAME SOURCE)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "" ${ARGN})

    if(NOT CREXX_NAME OR NOT CREXX_SOURCE)
        message(FATAL_ERROR "crexx_add_rxdas_roundtrip requires NAME and SOURCE")
    endif()

    get_filename_component(_crexx_stem "${CREXX_SOURCE}" NAME_WE)
    set(_crexx_artifact "${CMAKE_CURRENT_BINARY_DIR}/${_crexx_stem}.rxbin")

    add_custom_command(
            OUTPUT ${_crexx_artifact}
            COMMAND $<TARGET_FILE:rxas> -o ${_crexx_stem} ${CREXX_SOURCE}
            DEPENDS ${CREXX_SOURCE} rxas rxvm rxbvm mc_decimal_dynamic
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "cREXX RXDAS roundtrip fixture ${_crexx_stem} ..."
    )

    add_custom_target(${CREXX_NAME}_artifact ALL DEPENDS ${_crexx_artifact})
    add_dependencies(${CREXX_NAME}_artifact rxdas_test_prereqs)

    add_test(
            NAME ${CREXX_NAME}
            COMMAND ${CMAKE_COMMAND}
            -DRXDAS=$<TARGET_FILE:rxdas>
            -DRXAS=$<TARGET_FILE:rxas>
            -DRXVM=$<TARGET_FILE:rxvm>
            -DRXBVM=$<TARGET_FILE:rxbvm>
            -DINPUT_RXBIN=${_crexx_artifact}
            -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
            -DTEST_BASENAME=${CREXX_NAME}
            -DREQUIRE_NO_UNKNOWN=ON
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_roundtrip.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_basic
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_basic.rxas
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_float_pool
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_float_pool.rxas
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_compact_format
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_compact_format.rxas
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_interfaces
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_interfaces.rxas
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_metadata
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_disassembler_roundtrip.rxas
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_imported
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_disassembler_imported.rxas
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_binary
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_binary.rxas
)

add_executable(test_disassembler_dump tests/test_disassembler_dump.c rxdadism.c)
add_dependencies(test_disassembler_dump avl_tree platform rxdas_test_prereqs)
target_link_libraries(test_disassembler_dump avl_tree platform)
target_include_directories(test_disassembler_dump PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../assembler
        ../platform
        ../binutils/include)
set_target_properties(test_disassembler_dump PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_custom_command(TARGET test_disassembler_dump POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_BINARY_DIR}/bin/test_disassembler_dump)

add_test(
        NAME rxdas_dump_support
        COMMAND test_disassembler_dump
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
