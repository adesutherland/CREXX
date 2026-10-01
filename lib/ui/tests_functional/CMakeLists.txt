cmake_minimum_required(VERSION 3.24)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

add_custom_target(ui_functional_tests ALL)

# One test-only observer shared by direct line and GTK host scenarios.
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ui_driver_probe.rxbin"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import -i "${CMAKE_BINARY_DIR}/bin"
                -o ui_driver_probe "${CMAKE_CURRENT_SOURCE_DIR}/driver_probe.crexx"
        COMMAND $<TARGET_FILE:rxas> -o ui_driver_probe ui_driver_probe
        DEPENDS rxc rxas ui ui_contract
                "${CMAKE_BINARY_DIR}/bin/ui.rxbin"
                "${CMAKE_BINARY_DIR}/bin/ui_contract.rxbin" driver_probe.crexx
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/ui_driver_probe.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(ui_driver_probe DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ui_driver_probe.rxbin")

# Generated test data, not a 16KB source fixture or an unbounded read test.
string(REPEAT "x" 16385 _oversized_record)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/oversized-record.txt"
        CONTENT "${_oversized_record}\n")

if(UNIX)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    foreach(mode IN ITEMS noopt opt)
        set(flags)
        if(mode STREQUAL "noopt")
            set(flags -n)
        endif()
        set(edge_module "${CMAKE_CURRENT_BINARY_DIR}/ui_ansi_edges_${mode}_module")
        set(edge_image "${CMAKE_CURRENT_BINARY_DIR}/ui_ansi_edges_${mode}.rxbin")
        set(edge_inputs "${CMAKE_CURRENT_BINARY_DIR}/ui_driver_probe.rxbin")
        foreach(module IN ITEMS ui_ansi ui_dialogs ui_terminal_view ui_local_resources ui_contract ui_catalog ui library)
            list(APPEND edge_inputs "${CMAKE_BINARY_DIR}/bin/${module}.rxbin")
        endforeach()
        add_custom_command(OUTPUT "${edge_image}"
                COMMAND $<TARGET_FILE:rxc> --no-exe-import ${flags}
                        -i "${CMAKE_BINARY_DIR}/bin" -i "${CMAKE_CURRENT_BINARY_DIR}"
                        -o "${edge_module}" "${CMAKE_CURRENT_SOURCE_DIR}/test_ui_ansi_edges.crexx"
                COMMAND $<TARGET_FILE:rxas> ${flags} -o "${edge_module}" "${edge_module}"
                COMMAND $<TARGET_FILE:rxlink> -o "${edge_image}" "${edge_module}.rxbin" ${edge_inputs}
                DEPENDS rxc rxas rxlink ui_ansi ui_driver_probe ${edge_inputs} test_ui_ansi_edges.crexx
                BYPRODUCTS "${edge_module}.rxas" "${edge_module}.rxbin" VERBATIM)
        add_custom_target(ui_ansi_edges_${mode}_artifact DEPENDS "${edge_image}")
        add_dependencies(ui_functional_tests ui_ansi_edges_${mode}_artifact)
        add_test(NAME ui_ansi_host_edges_${mode}
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/lib/plugins/console/tests/pty_checks.py
                        --build ${CMAKE_BINARY_DIR} --source ${CMAKE_SOURCE_DIR}
                        --mode host-edges --variant ${mode})
        set_tests_properties(ui_ansi_host_edges_${mode} PROPERTIES TIMEOUT 60
                LABELS "library;ui;console;pty;conformance;optimizer-parity"
                CREXX_PREP_TARGETS "ui_ansi_edges_${mode}_artifact;rxvm")
    endforeach()
endif()

crexx_add_rexx_opt_matrix(
        NAME ui_standard_dialogs RUNNER rxvm
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_ui_dialogs.crexx
        TARGET_GROUP ui_functional_tests
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS ui ui_contract ui_catalog ui_dialogs ui_terminal_view ui_local_resources fs
                ${CMAKE_BINARY_DIR}/bin/ui_dialogs.rxbin
                ${CMAKE_BINARY_DIR}/bin/ui_terminal_view.rxbin
                ${CMAKE_BINARY_DIR}/bin/ui_local_resources.rxbin
        RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/ui_dialogs
                ${CMAKE_BINARY_DIR}/bin/ui_terminal_view
                ${CMAKE_BINARY_DIR}/bin/ui_local_resources
                ${CMAKE_BINARY_DIR}/bin/ui_contract
                ${CMAKE_BINARY_DIR}/bin/ui_catalog
                ${CMAKE_BINARY_DIR}/bin/ui
                ${CMAKE_BINARY_DIR}/bin/library
                -a ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
                ${CMAKE_CURRENT_BINARY_DIR}/oversized-record.txt
        TEST_LABELS "library;ui;console;dialogs;resource;classes"
        REQUIRED_OUTPUT_REGEX "PASS: standard dialogs, terminal geometry and resource grants"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

add_test(NAME ui_contract_generation
        COMMAND ${CMAKE_COMMAND}
                "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/../contracts"
                "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/generation-checks"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/../contracts/CheckGeneration.cmake")
set_tests_properties(ui_contract_generation PROPERTIES
        LABELS "library;ui;conformance")

crexx_add_rexx_opt_matrix(
        NAME ui_contract_v0
        RUNNER rxvm
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_ui_contract_v0.crexx
        TARGET_GROUP ui_functional_tests
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS ui ui_catalog ui_contract
                ${CMAKE_BINARY_DIR}/bin/ui_contract.rxbin
                ${CMAKE_BINARY_DIR}/bin/ui_catalog.rxbin
        RUNTIME_ARGS
                ${CMAKE_BINARY_DIR}/bin/ui_contract
                ${CMAKE_BINARY_DIR}/bin/ui_catalog
                ${CMAKE_BINARY_DIR}/bin/ui
                ${CMAKE_BINARY_DIR}/bin/library
                -a ${CMAKE_CURRENT_SOURCE_DIR}/../contracts/conformance.jsonl
        TEST_LABELS "library;ui;events;effects;classes;conformance"
        REQUIRED_OUTPUT_REGEX "PASS: UI v0 catalogue and session conformance"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

crexx_add_rexx_opt_matrix(
        NAME ui_tui_contract
        RUNNER rxvm
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_ui_tui.crexx
        TARGET_GROUP ui_functional_tests
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS ui ui_tui ui_contract ui_local_resources
                ${CMAKE_BINARY_DIR}/bin/ui_tui.rxbin
                ${CMAKE_BINARY_DIR}/bin/ui_contract.rxbin
                ${CMAKE_BINARY_DIR}/bin/ui_local_resources.rxbin
        RUNTIME_ARGS
                ${CMAKE_BINARY_DIR}/bin/ui_contract
                ${CMAKE_BINARY_DIR}/bin/ui_catalog
                ${CMAKE_BINARY_DIR}/bin/ui_local_resources
                ${CMAKE_BINARY_DIR}/bin/ui_tui
                ${CMAKE_BINARY_DIR}/bin/ui
                ${CMAKE_BINARY_DIR}/bin/library
        TEST_LABELS "library;ui;tui;event-loop;classes"
        REQUIRED_OUTPUT_REGEX "PASS: TUI drives semantic events"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
