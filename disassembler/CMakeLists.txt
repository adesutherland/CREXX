cmake_minimum_required(VERSION 3.24)

project(Disassembler C)

set(CMAKE_C_STANDARD 90)

include(CMakeParseArguments)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

add_executable(rxdas rxdamain.c rxdadism.h rxdadism.c ../binutils/include/rxbin.h)

add_dependencies(rxdas avl_tree platform)

target_link_libraries(rxdas avl_tree rxbin platform)

target_include_directories(rxdas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../assembler ../platform ../binutils/include)

add_custom_target(rxdas_test_prereqs ALL
        DEPENDS rxdas rxas rxvm rxbvm mc_decimal_dynamic)

function(crexx_add_rxdas_roundtrip)
    set(options)
    set(oneValueArgs NAME SOURCE
            EXPECT_DISASSEMBLY_TEXT_1 EXPECT_DISASSEMBLY_TEXT_2
            EXPECT_DISASSEMBLY_TEXT_3 EXPECT_DISASSEMBLY_TEXT_4
            EXPECT_DISASSEMBLY_TEXT_5 EXPECT_DISASSEMBLY_TEXT_6
            EXPECT_DISASSEMBLY_TEXT_7 EXPECT_DISASSEMBLY_TEXT_8)
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
            -DEXPECT_DISASSEMBLY_TEXT_1=${CREXX_EXPECT_DISASSEMBLY_TEXT_1}
            -DEXPECT_DISASSEMBLY_TEXT_2=${CREXX_EXPECT_DISASSEMBLY_TEXT_2}
            -DEXPECT_DISASSEMBLY_TEXT_3=${CREXX_EXPECT_DISASSEMBLY_TEXT_3}
            -DEXPECT_DISASSEMBLY_TEXT_4=${CREXX_EXPECT_DISASSEMBLY_TEXT_4}
            -DEXPECT_DISASSEMBLY_TEXT_5=${CREXX_EXPECT_DISASSEMBLY_TEXT_5}
            -DEXPECT_DISASSEMBLY_TEXT_6=${CREXX_EXPECT_DISASSEMBLY_TEXT_6}
            -DEXPECT_DISASSEMBLY_TEXT_7=${CREXX_EXPECT_DISASSEMBLY_TEXT_7}
            -DEXPECT_DISASSEMBLY_TEXT_8=${CREXX_EXPECT_DISASSEMBLY_TEXT_8}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_roundtrip.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endfunction()

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_basic
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_basic.rxas
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_int64_contract
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_int_contract.rxas
        EXPECT_DISASSEMBLY_TEXT_1 "9223372036854775807"
        EXPECT_DISASSEMBLY_TEXT_2 "-9223372036854775808"
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_float_pool
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_float_pool.rxas
        EXPECT_DISASSEMBLY_TEXT_1 "3.1415926535897931"
        EXPECT_DISASSEMBLY_TEXT_2 "3.1415929999999999"
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_compact_format
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_compact_format.rxas
)

add_executable(mutate_graph_rxbin tests/mutate_graph_rxbin.c)
set_target_properties(mutate_graph_rxbin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(_crexx_corrupt_graph_rxbin
        "${CMAKE_CURRENT_BINARY_DIR}/tests_compact_format_corrupt_graph.rxbin")
add_custom_command(
        OUTPUT ${_crexx_corrupt_graph_rxbin}
        COMMAND $<TARGET_FILE:mutate_graph_rxbin>
                ${CMAKE_CURRENT_BINARY_DIR}/tests_compact_format.rxbin
                ${_crexx_corrupt_graph_rxbin}
        DEPENDS rxdas_roundtrip_compact_format_artifact mutate_graph_rxbin
        VERBATIM
        COMMENT "cREXX RXDAS invalid semantic-graph fixture ...")
add_custom_target(rxdas_graph_error_artifact ALL DEPENDS ${_crexx_corrupt_graph_rxbin})
add_dependencies(rxdas_graph_error_artifact rxdas)
add_test(
        NAME rxdas_graph_error
        COMMAND ${CMAKE_COMMAND}
                -DRXDAS=$<TARGET_FILE:rxdas>
                -DINPUT_RXBIN=${_crexx_corrupt_graph_rxbin}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_graph_error.cmake)

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

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_jtable_linear
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_jump_table.rxas
        EXPECT_DISASSEMBLY_TEXT_1 ".jtable jtable_"
        EXPECT_DISASSEMBLY_TEXT_2 " linear"
        EXPECT_DISASSEMBLY_TEXT_3 ".jcase jtable_"
        EXPECT_DISASSEMBLY_TEXT_4 " jumps "
        EXPECT_DISASSEMBLY_TEXT_5 " jumpb "
        EXPECT_DISASSEMBLY_TEXT_6 " jumpbs "
        EXPECT_DISASSEMBLY_TEXT_7 " jumpi "
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_jtable_hash
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_jump_table_hash_edges.rxas
        EXPECT_DISASSEMBLY_TEXT_1 " openhash"
        EXPECT_DISASSEMBLY_TEXT_2 " acph"
        EXPECT_DISASSEMBLY_TEXT_3 ".jcase jtable_"
)

crexx_add_rxdas_roundtrip(
        NAME rxdas_roundtrip_jtable_rexx_strings
        SOURCE ${CMAKE_SOURCE_DIR}/interpreter/tests/tests_jump_table_rexx_strings.rxas
        EXPECT_DISASSEMBLY_TEXT_1 " jumpr "
        EXPECT_DISASSEMBLY_TEXT_2 " jumpn "
        EXPECT_DISASSEMBLY_TEXT_3 ".jcase jtable_"
)

add_executable(test_disassembler_dump tests/test_disassembler_dump.c rxdadism.c)
add_dependencies(test_disassembler_dump avl_tree platform rxdas_test_prereqs)
target_link_libraries(test_disassembler_dump avl_tree rxbin platform)
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

crexx_label_smoke_tests(
        rxdas_roundtrip_basic
        rxdas_roundtrip_interfaces
        rxdas_graph_error
        rxdas_dump_support
)
