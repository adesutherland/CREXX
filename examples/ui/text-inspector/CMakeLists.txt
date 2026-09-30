cmake_minimum_required(VERSION 3.24)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(_text_inspector_rxpp_root "${CMAKE_CURRENT_BINARY_DIR}/rxpp")
set(_text_inspector_generated_root "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(_text_inspector_generated
        "${_text_inspector_generated_root}/text_inspector.crexx")
set(_text_inspector_tui_generated
        "${_text_inspector_generated_root}/text_inspector_tui.crexx")
set(_text_inspector_module "${CMAKE_CURRENT_BINARY_DIR}/text_inspector.rxbin")
set(_text_inspector_tui "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_tui.rxbin")
set(_text_inspector_contract_images
        "${CMAKE_BINARY_DIR}/bin/ui_local_resources.rxbin"
        "${CMAKE_BINARY_DIR}/bin/ui_contract.rxbin"
        "${CMAKE_BINARY_DIR}/bin/ui_catalog.rxbin")

set(_text_inspector_rxpp_support
        maclib.rexx macsys.rexx mathlib.rexx syslib.rexx)
set(_text_inspector_support_sources)
file(MAKE_DIRECTORY
        "${_text_inspector_rxpp_root}"
        "${_text_inspector_generated_root}")
foreach(_support IN LISTS _text_inspector_rxpp_support)
    list(APPEND _text_inspector_support_sources
            "${CMAKE_SOURCE_DIR}/preprocessor/${_support}")
    configure_file(
            "${CMAKE_SOURCE_DIR}/preprocessor/${_support}"
            "${_text_inspector_rxpp_root}/${_support}"
            COPYONLY)
endforeach()
set(_text_inspector_ui_macros)
# Retire only the formerly generated alias; never touch authored macro files.
file(REMOVE "${_text_inspector_rxpp_root}/ui/ui_session_launcher.rxpm")
foreach(macro IN ITEMS ui_node ui_command ui_launcher)
    list(APPEND _text_inspector_ui_macros
            "${CMAKE_SOURCE_DIR}/lib/ui/rxpp/ui/${macro}.rxpm")
    configure_file("${CMAKE_SOURCE_DIR}/lib/ui/rxpp/ui/${macro}.rxpm"
            "${_text_inspector_rxpp_root}/ui/${macro}.rxpm" COPYONLY)
endforeach()

add_custom_command(
        OUTPUT "${_text_inspector_generated}"
        COMMAND $<TARGET_FILE:rxpp> rxprecomp
                -I "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector.rxpp"
                -o "${_text_inspector_generated}"
                -m "${_text_inspector_rxpp_root}/maclib.rexx"
        DEPENDS rxpp rxpp_support_files
                "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector.rxpp"
                ${_text_inspector_ui_macros}
                ${_text_inspector_support_sources}
        COMMENT "Generating source-mapped Text Inspector application with RXPP"
        VERBATIM)

add_custom_command(
        OUTPUT "${_text_inspector_tui_generated}"
        COMMAND $<TARGET_FILE:rxpp> rxprecomp
                -I "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector_tui.rxpp"
                -o "${_text_inspector_tui_generated}"
                -m "${_text_inspector_rxpp_root}/maclib.rexx"
        DEPENDS rxpp rxpp_support_files
                "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector_tui.rxpp"
                ${_text_inspector_ui_macros}
                ${_text_inspector_support_sources}
        COMMENT "Generating Text Inspector TUI launcher with RXPP"
        VERBATIM)

add_custom_command(
        OUTPUT "${_text_inspector_module}"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin"
                -o text_inspector "${_text_inspector_generated}"
        COMMAND $<TARGET_FILE:rxas> -o text_inspector text_inspector
        DEPENDS rxc rxas compiler_exit_bin ui ui_local_resources "${_text_inspector_generated}"
                ${_text_inspector_contract_images}
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
                "${_text_inspector_tui_generated}"
        COMMAND $<TARGET_FILE:rxas> -o text_inspector_tui text_inspector_tui
        DEPENDS rxc rxas compiler_exit_bin ui ui_tui
                text_inspector_module_artifact
                "${_text_inspector_tui_generated}"
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

set(_text_inspector_ansi_generated "${_text_inspector_generated_root}/text_inspector_ansi.crexx")
add_custom_command(OUTPUT "${_text_inspector_ansi_generated}"
        COMMAND $<TARGET_FILE:rxpp> rxprecomp
                -I "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector_ansi.rxpp"
                -o "${_text_inspector_ansi_generated}"
                -m "${_text_inspector_rxpp_root}/maclib.rexx"
        DEPENDS rxpp rxpp_support_files ${_text_inspector_ui_macros}
                ${_text_inspector_support_sources} text_inspector_ansi.rxpp
        VERBATIM)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_ansi.rxbin"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_CURRENT_BINARY_DIR}" -i "${CMAKE_BINARY_DIR}/bin"
                -o text_inspector_ansi "${_text_inspector_ansi_generated}"
        COMMAND $<TARGET_FILE:rxas> -o text_inspector_ansi text_inspector_ansi
        DEPENDS rxc rxas compiler_exit_bin ui_ansi text_inspector_module_artifact
                "${_text_inspector_ansi_generated}" "${_text_inspector_module}"
                "${CMAKE_BINARY_DIR}/bin/ui_ansi.rxbin"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_ansi.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(text_inspector_ansi_artifact
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_ansi.rxbin")
add_dependencies(example_text_inspector_artifacts text_inspector_ansi_artifact)

if(ENABLE_GTK)
    set(_text_inspector_gtk_generated
            "${_text_inspector_generated_root}/text_inspector_gtk.crexx")
    set(_text_inspector_gtk "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_gtk.rxbin")
    add_custom_command(
            OUTPUT "${_text_inspector_gtk_generated}"
            COMMAND $<TARGET_FILE:rxpp> rxprecomp
                    -I "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector_gtk.rxpp"
                    -o "${_text_inspector_gtk_generated}"
                    -m "${_text_inspector_rxpp_root}/maclib.rexx"
            DEPENDS rxpp rxpp_support_files
                    "${CMAKE_CURRENT_SOURCE_DIR}/text_inspector_gtk.rxpp"
                    ${_text_inspector_ui_macros}
                    ${_text_inspector_support_sources}
            COMMENT "Generating Text Inspector GTK launcher with RXPP"
            VERBATIM)
    add_custom_command(
            OUTPUT "${_text_inspector_gtk}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}"
                    -i "${CMAKE_BINARY_DIR}/bin"
                    -o text_inspector_gtk
                    "${_text_inspector_gtk_generated}"
            COMMAND $<TARGET_FILE:rxas> -o text_inspector_gtk text_inspector_gtk
            DEPENDS rxc rxas compiler_exit_bin ui ui_gtk _ui_gtk_native
                    text_inspector_module_artifact
                    "${_text_inspector_gtk_generated}"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_gtk.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Building Text Inspector GTK launcher"
            VERBATIM)
    add_custom_target(example_text_inspector_gtk DEPENDS "${_text_inspector_gtk}")
    add_dependencies(example_text_inspector_artifacts example_text_inspector_gtk)
endif()

if(BUILD_TESTING)
    set(_host_test_dir "${CMAKE_BINARY_DIR}/lib/ui/tests_functional")
    set(_host_test_image "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_test_host.rxbin")
    add_custom_command(OUTPUT "${_host_test_image}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}" -i "${_host_test_dir}"
                    -i "${CMAKE_BINARY_DIR}/bin" -o text_inspector_test_host
                    "${CMAKE_CURRENT_SOURCE_DIR}/test_host.crexx"
            COMMAND $<TARGET_FILE:rxas> -o text_inspector_test_host text_inspector_test_host
            DEPENDS rxc rxas ui_driver_probe text_inspector_module_artifact
                    "${_host_test_dir}/ui_driver_probe.rxbin"
                    "${_text_inspector_module}" test_host.crexx
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_test_host.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(text_inspector_test_host DEPENDS "${_host_test_image}")
    if(UNIX)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        add_test(NAME text_inspector_ansi_pty
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/lib/plugins/console/tests/pty_checks.py
                        --build ${CMAKE_BINARY_DIR} --source ${CMAKE_SOURCE_DIR} --mode app)
        set_tests_properties(text_inspector_ansi_pty PROPERTIES TIMEOUT 90
                LABELS "examples;ui;console;dialogs;pty")
        crexx_register_test_prep_targets(TESTS text_inspector_ansi_pty
                TARGETS example_text_inspector_artifacts rxvm)
    endif()
    set(_text_inspector_generated_checks
            -DGENERATED=${_text_inspector_generated}
            -DTUI_GENERATED=${_text_inspector_tui_generated}
            -DANSI_GENERATED=${_text_inspector_ansi_generated})
    if(ENABLE_GTK)
        list(APPEND _text_inspector_generated_checks
                -DGTK_GENERATED=${_text_inspector_gtk_generated})
    endif()
    add_test(NAME text_inspector_rxpp_generation
            COMMAND ${CMAKE_COMMAND}
                    ${_text_inspector_generated_checks}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/CheckGenerated.cmake)
    set_tests_properties(text_inspector_rxpp_generation PROPERTIES
            LABELS "examples;ui;rxpp;source-map"
            CREXX_PREP_TARGETS example_text_inspector_artifacts)

    crexx_add_rexx_opt_matrix(
            NAME text_inspector_core
            RUNNER rxvm
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_text_inspector_core.crexx
            TARGET_GROUP example_text_inspector_artifacts
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin ${_host_test_dir}
            DEPENDS ui text_inspector_module_artifact
                    "${_text_inspector_module}" ${_text_inspector_contract_images}
            RUNTIME_ARGS
                    ${_text_inspector_module}
                    ${CMAKE_BINARY_DIR}/bin/ui
                    ${CMAKE_BINARY_DIR}/bin/ui_local_resources
                    ${CMAKE_BINARY_DIR}/bin/ui_contract
                    ${CMAKE_BINARY_DIR}/bin/ui_catalog
                    ${CMAKE_BINARY_DIR}/bin/library
            TEST_LABELS "examples;ui;events;effects;rxpp;classes"
            REQUIRED_OUTPUT_REGEX "PASS: Text Inspector shared application contract"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

    crexx_add_rexx_opt_matrix(
            NAME text_inspector_tui
            RUNNER rxvm
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_text_inspector_tui.crexx
            TARGET_GROUP example_text_inspector_artifacts
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin ${_host_test_dir}
            DEPENDS ui ui_tui text_inspector_module_artifact text_inspector_test_host
                    "${_host_test_image}" "${_host_test_dir}/ui_driver_probe.rxbin"
                    "${_text_inspector_module}" ${_text_inspector_contract_images}
            RUNTIME_ARGS
                    ${_text_inspector_module}
                    ${CMAKE_BINARY_DIR}/bin/ui_tui
                    ${_host_test_image} ${_host_test_dir}/ui_driver_probe
                    ${CMAKE_BINARY_DIR}/bin/ui
                    ${CMAKE_BINARY_DIR}/bin/ui_local_resources
                    ${CMAKE_BINARY_DIR}/bin/ui_contract
                    ${CMAKE_BINARY_DIR}/bin/ui_catalog
                    ${CMAKE_BINARY_DIR}/bin/library
                    -a ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/sample.txt
            TEST_LABELS "examples;ui;tui;event-loop;effects;rxpp"
            REQUIRED_OUTPUT_REGEX "PASS: Text Inspector TUI end-to-end"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")

    if(ENABLE_GTK)
        crexx_add_rexx_opt_matrix(
                NAME text_inspector_gtk
                RUNNER rxvm
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_text_inspector_gtk.crexx
                TARGET_GROUP example_text_inspector_artifacts
                IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin ${_host_test_dir}
                DEPENDS ui ui_gtk _ui_gtk_native _ui_gtk_test text_inspector_test_host
                        "$<TARGET_FILE:_ui_gtk_test>" "${_host_test_image}" "${_host_test_dir}/ui_driver_probe.rxbin"
                        text_inspector_module_artifact
                        "${_text_inspector_module}" ${_text_inspector_contract_images}
                RUNTIME_ARGS
                        ${CMAKE_BINARY_DIR}/bin/rx_ui_gtk_native
                        $<TARGET_FILE:_ui_gtk_test>
                        ${_host_test_image} ${_host_test_dir}/ui_driver_probe
                        ${_text_inspector_module}
                        ${CMAKE_BINARY_DIR}/bin/ui_gtk
                        ${CMAKE_BINARY_DIR}/bin/ui
                        ${CMAKE_BINARY_DIR}/bin/ui_local_resources
                        ${CMAKE_BINARY_DIR}/bin/ui_contract
                        ${CMAKE_BINARY_DIR}/bin/ui_catalog
                        ${CMAKE_BINARY_DIR}/bin/library
                        -a ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/sample.txt
                TEST_LABELS "examples;ui;gtk;event-loop;rxpa;callback;rxpp"
                REQUIRED_OUTPUT_REGEX "PASS: Text Inspector GTK callback path"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
        set_tests_properties(text_inspector_gtk_noopt text_inspector_gtk_opt
                PROPERTIES TIMEOUT 60 RESOURCE_LOCK crexx_gtk_surface)
    endif()
endif()

install(FILES
        "${_text_inspector_module}"
        "${_text_inspector_tui}"
        "${CMAKE_CURRENT_BINARY_DIR}/text_inspector_ansi.rxbin"
        DESTINATION examples/ui/text-inspector)
if(ENABLE_GTK)
    install(FILES "${_text_inspector_gtk}"
            DESTINATION examples/ui/text-inspector)
endif()

crexx_finalize_directory_qa_tiers()
