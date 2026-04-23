cmake_minimum_required(VERSION 3.24)

project(Linker C)

set(CMAKE_C_STANDARD 99)

add_executable(rxlink rxlinkmain.c ../binutils/include/rxbin.h)
target_link_libraries(rxlink platform)
target_include_directories(rxlink PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../platform
        ../binutils/include)

add_custom_target(rxlink_test_prereqs ALL
        DEPENDS rxlink rxas rxvm rxbvm rxdas mc_decimal_dynamic)

add_executable(rxlink_concat_files tests/concat_files.c)
target_include_directories(rxlink_concat_files PRIVATE
        ${CMAKE_SOURCE_DIR}/platform)

function(crexx_add_rxas_fixture STEM SOURCE)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${STEM}.rxbin
            COMMAND $<TARGET_FILE:rxas> -o ${STEM} ${SOURCE}
            DEPENDS ${SOURCE} rxas rxvm rxbvm mc_decimal_dynamic
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "cREXX RXLINK fixture ${STEM} ..."
    )
    add_custom_target(${STEM}_fixture ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${STEM}.rxbin)
    add_dependencies(${STEM}_fixture rxlink_test_prereqs)
endfunction()

function(crexx_add_vm_dual_test)
    set(options)
    set(oneValueArgs NAME PASS_REGULAR_EXPRESSION)
    set(multiValueArgs RXVM_ARGS)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_test(
            NAME ${CREXX_NAME}
            COMMAND $<TARGET_FILE:rxvm> ${CREXX_RXVM_ARGS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_test(
            NAME ${CREXX_NAME}-rxbvm
            COMMAND $<TARGET_FILE:rxbvm> ${CREXX_RXVM_ARGS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    if(CREXX_PASS_REGULAR_EXPRESSION)
        set_tests_properties(${CREXX_NAME} ${CREXX_NAME}-rxbvm PROPERTIES
                PASS_REGULAR_EXPRESSION "${CREXX_PASS_REGULAR_EXPRESSION}")
    endif()
endfunction()

crexx_add_rxas_fixture(tests_link_root ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_link_root.rxas)
crexx_add_rxas_fixture(tests_link_provider_a ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_link_provider_a.rxas)
crexx_add_rxas_fixture(tests_link_provider_b ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_link_provider_b.rxas)
crexx_add_rxas_fixture(tests_link_interface_root ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_link_interface_root.rxas)
crexx_add_rxas_fixture(tests_link_interface_impl ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_link_interface_impl.rxas)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tests_linked_success.rxbin
        COMMAND $<TARGET_FILE:rxlink> -o tests_linked_success tests_link_root.rxbin tests_link_provider_a.rxbin
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests_link_root.rxbin ${CMAKE_CURRENT_BINARY_DIR}/tests_link_provider_a.rxbin rxlink
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXLINK success fixture ..."
)
add_custom_target(tests_linked_success_artifact ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests_linked_success.rxbin)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tests_linked_omit.rxbin
        COMMAND $<TARGET_FILE:rxlink> -c ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_link_omit.ctl -o tests_linked_omit
                tests_link_root.rxbin tests_link_provider_a.rxbin tests_link_provider_b.rxbin
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests_link_root.rxbin ${CMAKE_CURRENT_BINARY_DIR}/tests_link_provider_a.rxbin
                ${CMAKE_CURRENT_BINARY_DIR}/tests_link_provider_b.rxbin rxlink
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXLINK control-file fixture ..."
)
add_custom_target(tests_linked_omit_artifact ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests_linked_omit.rxbin)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tests_linked_interface.rxbin
        COMMAND $<TARGET_FILE:rxlink> -o tests_linked_interface tests_link_interface_root.rxbin tests_link_interface_impl.rxbin
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests_link_interface_root.rxbin ${CMAKE_CURRENT_BINARY_DIR}/tests_link_interface_impl.rxbin rxlink
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXLINK interface fixture ..."
)
add_custom_target(tests_linked_interface_artifact ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests_linked_interface.rxbin)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tests_record_stream_combo.rxbin
        COMMAND $<TARGET_FILE:rxlink_concat_files>
                tests_record_stream_combo.rxbin
                tests_link_root.rxbin
                tests_linked_success.rxbin
                tests_link_interface_root.rxbin
                tests_linked_interface.rxbin
        DEPENDS rxlink_concat_files
                ${CMAKE_CURRENT_BINARY_DIR}/tests_link_root.rxbin
                ${CMAKE_CURRENT_BINARY_DIR}/tests_linked_success.rxbin
                ${CMAKE_CURRENT_BINARY_DIR}/tests_link_interface_root.rxbin
                ${CMAKE_CURRENT_BINARY_DIR}/tests_linked_interface.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXLINK record-stream concat fixture ..."
)
add_custom_target(tests_record_stream_combo_artifact ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests_record_stream_combo.rxbin)

crexx_add_vm_dual_test(
        NAME rxlink_success_run
        RXVM_ARGS tests_linked_success
        PASS_REGULAR_EXPRESSION "PASS: Provider A[^\r\n]*\r?\nPASS: Root Returned[^\r\n]*\r?\n10"
)

crexx_add_vm_dual_test(
        NAME rxlink_control_run
        RXVM_ARGS tests_linked_omit
        PASS_REGULAR_EXPRESSION "PASS: Provider A[^\r\n]*\r?\nPASS: Root Returned[^\r\n]*\r?\n10"
)

crexx_add_vm_dual_test(
        NAME rxlink_interface_run
        RXVM_ARGS tests_linked_interface
        PASS_REGULAR_EXPRESSION "PASS: Interface Linked"
)

add_executable(test_linked_format tests/test_linked_format.c)
target_include_directories(test_linked_format PRIVATE
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/binutils/include)
add_dependencies(test_linked_format tests_linked_success_artifact tests_record_stream_combo_artifact)
add_test(NAME rxlink_format_check COMMAND test_linked_format WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(
        NAME rxlink_duplicate_provider_error
        COMMAND ${CMAKE_COMMAND}
        -DRXLINK=$<TARGET_FILE:rxlink>
        -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
        -DOUTPUT_BASENAME=tests_linked_duplicate
        "-DINPUTS=tests_link_root.rxbin\;tests_link_provider_a.rxbin\;tests_link_provider_b.rxbin"
        -DEXPECT_TEXT=Duplicate providers for import linktest.helper
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_expected_failure.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(
        NAME rxlink_rxdas_smoke
        COMMAND ${CMAKE_COMMAND}
        -DRXDAS=$<TARGET_FILE:rxdas>
        -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
        -DINPUT_RXBIN=${CMAKE_CURRENT_BINARY_DIR}/tests_linked_success.rxbin
        -DOUTPUT_RXAS=${CMAKE_CURRENT_BINARY_DIR}/tests_linked_success.rxas
        -DEXPECT_TEXT_1=tests_link_provider_a.rxas
        -DEXPECT_TEXT_2=main() .locals
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_rxdas_contains.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
