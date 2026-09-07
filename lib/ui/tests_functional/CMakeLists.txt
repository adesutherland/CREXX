cmake_minimum_required(VERSION 3.24)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

add_custom_target(ui_functional_tests ALL)

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
        NAME ui_runtime_contract
        RUNNER rxvm
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_ui_runtime.crexx
        TARGET_GROUP ui_functional_tests
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS ui
        RUNTIME_ARGS
                ${CMAKE_BINARY_DIR}/bin/ui
                ${CMAKE_BINARY_DIR}/bin/library
                -a
                ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/sample.txt
                ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/error-looking-text.txt
                ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/does-not-exist.txt
        TEST_LABELS "library;ui;events;effects;classes"
        REQUIRED_OUTPUT_REGEX "PASS: UI runtime event and effect contract"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

crexx_add_rexx_opt_matrix(
        NAME ui_tui_contract
        RUNNER rxvm
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_ui_tui.crexx
        TARGET_GROUP ui_functional_tests
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
        DEPENDS ui ui_tui
        RUNTIME_ARGS
                ${CMAKE_BINARY_DIR}/bin/ui_tui
                ${CMAKE_BINARY_DIR}/bin/ui
                ${CMAKE_BINARY_DIR}/bin/library
        TEST_LABELS "library;ui;tui;event-loop;classes"
        REQUIRED_OUTPUT_REGEX "PASS: TUI drives semantic events"
        FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
