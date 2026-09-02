cmake_minimum_required(VERSION 3.24)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

add_custom_target(ui_functional_tests ALL)

crexx_add_rexx_opt_matrix(
        NAME ui_runtime_contract
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
