cmake_minimum_required(VERSION 3.24)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(_text_inspector_rxpp_root "${CMAKE_CURRENT_BINARY_DIR}/rxpp")
set(_text_inspector_generated_root "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(_text_inspector_generated
        "${_text_inspector_generated_root}/text_inspector.crexx")
set(_text_inspector_module "${CMAKE_CURRENT_BINARY_DIR}/text_inspector.rxbin")
set(_text_inspector_tui "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_tui.rxbin")

set(_text_inspector_rxpp_support
        maclib.rexx macsys.rexx mathlib.rexx syslib.rexx)
set(_text_inspector_support_sources)
set(_text_inspector_stage_commands)
foreach(_support IN LISTS _text_inspector_rxpp_support)
    list(APPEND _text_inspector_support_sources
            "${CMAKE_SOURCE_DIR}/preprocessor/${_support}")
    list(APPEND _text_inspector_stage_commands
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_SOURCE_DIR}/preprocessor/${_support}"
                    "${_text_inspector_rxpp_root}/${_support}")
endforeach()

add_custom_command(
        OUTPUT "${_text_inspector_generated}"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${_text_inspector_rxpp_root}"
                "${_text_inspector_generated_root}"
        ${_text_inspector_stage_commands}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/lib/ui/rxpp/ui_macros.rxpm"
                "${_text_inspector_rxpp_root}/ui_macros.rxpm"
        COMMAND $<TARGET_FILE:rxpp> rxprecomp
                -I "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector.rxpp"
                -o "${_text_inspector_generated}"
                -m "${_text_inspector_rxpp_root}/maclib.rexx"
        DEPENDS rxpp rxpp_support_files
                "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector.rxpp"
                "${CMAKE_SOURCE_DIR}/lib/ui/rxpp/ui_macros.rxpm"
                ${_text_inspector_support_sources}
        COMMENT "Generating source-mapped Text Inspector application with RXPP"
        VERBATIM)

add_custom_command(
        OUTPUT "${_text_inspector_module}"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin"
                -o text_inspector "${_text_inspector_generated}"
        COMMAND $<TARGET_FILE:rxas> -o text_inspector text_inspector
        DEPENDS rxc rxas compiler_exit_bin ui "${_text_inspector_generated}"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/text_inspector.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Compiling and assembling Text Inspector application"
        VERBATIM)
add_custom_target(text_inspector_module_artifact
        DEPENDS "${_text_inspector_module}")

add_custom_command(
        OUTPUT "${_text_inspector_tui}"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_CURRENT_BINARY_DIR}"
                -i "${CMAKE_BINARY_DIR}/bin"
                -o text_inspector_tui
                "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector_tui.crexx"
        COMMAND $<TARGET_FILE:rxas> -o text_inspector_tui text_inspector_tui
        DEPENDS rxc rxas compiler_exit_bin ui ui_tui
                text_inspector_module_artifact
                "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector_tui.crexx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_tui.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building Text Inspector TUI launcher"
        VERBATIM)
add_custom_target(text_inspector_tui_artifact
        DEPENDS "${_text_inspector_tui}")

add_custom_target(example_text_inspector_artifacts)
add_dependencies(example_text_inspector_artifacts
        text_inspector_module_artifact
        text_inspector_tui_artifact)

if(ENABLE_GTK)
    set(_text_inspector_gtk "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_gtk.rxbin")
    add_custom_command(
            OUTPUT "${_text_inspector_gtk}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}"
                    -i "${CMAKE_BINARY_DIR}/bin"
                    -o text_inspector_gtk
                    "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector_gtk.crexx"
            COMMAND $<TARGET_FILE:rxas> -o text_inspector_gtk text_inspector_gtk
            DEPENDS rxc rxas compiler_exit_bin ui ui_gtk _ui_gtk_native
                    text_inspector_module_artifact
                    "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector_gtk.crexx"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_gtk.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Building Text Inspector GTK launcher"
            VERBATIM)
    add_custom_target(example_text_inspector_gtk DEPENDS "${_text_inspector_gtk}")
    add_dependencies(example_text_inspector_artifacts example_text_inspector_gtk)
endif()

if(BUILD_TESTING)
    add_test(NAME text_inspector_rxpp_generation
            COMMAND ${CMAKE_COMMAND}
                    -DGENERATED=${_text_inspector_generated}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/CheckGenerated.cmake)
    set_tests_properties(text_inspector_rxpp_generation PROPERTIES
            LABELS "examples;ui;rxpp;source-map"
            CREXX_PREP_TARGETS example_text_inspector_artifacts)

    crexx_add_rexx_opt_matrix(
            NAME text_inspector_core
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_text_inspector_core.crexx
            TARGET_GROUP example_text_inspector_artifacts
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
            DEPENDS ui text_inspector_module_artifact
            RUNTIME_ARGS
                    ${_text_inspector_module}
                    ${CMAKE_BINARY_DIR}/bin/ui
                    ${CMAKE_BINARY_DIR}/bin/library
            TEST_LABELS "examples;ui;events;effects;rxpp;classes"
            REQUIRED_OUTPUT_REGEX "PASS: Text Inspector shared application contract"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

    crexx_add_rexx_opt_matrix(
            NAME text_inspector_tui
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_text_inspector_tui.crexx
            TARGET_GROUP example_text_inspector_artifacts
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
            DEPENDS ui ui_tui text_inspector_module_artifact
            RUNTIME_ARGS
                    ${_text_inspector_module}
                    ${CMAKE_BINARY_DIR}/bin/ui_tui
                    ${CMAKE_BINARY_DIR}/bin/ui
                    ${CMAKE_BINARY_DIR}/bin/library
                    -a ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/sample.txt
            TEST_LABELS "examples;ui;tui;event-loop;effects;rxpp"
            REQUIRED_OUTPUT_REGEX "PASS: Text Inspector TUI end-to-end"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

    if(ENABLE_GTK)
        crexx_add_rexx_opt_matrix(
                NAME text_inspector_gtk
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_text_inspector_gtk.crexx
                TARGET_GROUP example_text_inspector_artifacts
                IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
                DEPENDS ui ui_gtk _ui_gtk_native
                        text_inspector_module_artifact
                RUNTIME_ARGS
                        ${CMAKE_BINARY_DIR}/bin/rx_ui_gtk_native
                        ${_text_inspector_module}
                        ${CMAKE_BINARY_DIR}/bin/ui_gtk
                        ${CMAKE_BINARY_DIR}/bin/ui
                        ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "examples;ui;gtk;event-loop;rxpa;callback;rxpp"
                REQUIRED_OUTPUT_REGEX "PASS: Text Inspector GTK callback path"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
        set_tests_properties(text_inspector_gtk_noopt text_inspector_gtk_opt
                PROPERTIES TIMEOUT 30)
    endif()
endif()

install(FILES
        "${_text_inspector_module}"
        "${_text_inspector_tui}"
        DESTINATION examples/ui/text-inspector)
if(ENABLE_GTK)
    install(FILES "${_text_inspector_gtk}"
            DESTINATION examples/ui/text-inspector)
endif()

crexx_finalize_directory_qa_tiers()
