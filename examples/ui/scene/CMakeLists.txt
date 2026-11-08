cmake_minimum_required(VERSION 3.24)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(_scene_example "${CMAKE_CURRENT_BINARY_DIR}/scene_examples.rxbin")
add_custom_command(OUTPUT "${_scene_example}"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin" -o scene_examples
                "${CMAKE_CURRENT_SOURCE_DIR}/scene_examples.crexx"
        COMMAND $<TARGET_FILE:rxas> -o scene_examples scene_examples
        DEPENDS rxc rxas compiler_exit_bin ui ui_contract ui_scene
                "${CMAKE_CURRENT_SOURCE_DIR}/scene_examples.crexx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/scene_examples.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(example_ui_scene DEPENDS "${_scene_example}")

set(_scene_editors "${CMAKE_CURRENT_BINARY_DIR}/scene_editors.rxbin")
add_custom_command(OUTPUT "${_scene_editors}"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin" -o scene_editors
                "${CMAKE_CURRENT_SOURCE_DIR}/scene_editors.crexx"
        COMMAND $<TARGET_FILE:rxas> -o scene_editors scene_editors
        DEPENDS rxc rxas compiler_exit_bin ui ui_contract ui_scene scene_editors.crexx
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/scene_editors.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(example_ui_scene_editors DEPENDS "${_scene_editors}")

add_custom_target(example_ui_scene_editor_artifacts)
foreach(_editor_launcher IN ITEMS graph_web map_web graph_editor_ansi map_editor_ansi graph_editor_gtk map_editor_gtk)
    if(_editor_launcher MATCHES "_gtk$" AND NOT ENABLE_GTK)
        continue()
    endif()
    if(_editor_launcher MATCHES "_ansi$" AND NOT UNIX)
        continue()
    endif()
    if(_editor_launcher MATCHES "_web$" )
        set(_editor_driver ui_web)
    elseif(_editor_launcher MATCHES "_gtk$")
        set(_editor_driver ui_gtk)
    else()
        set(_editor_driver ui_ansi)
    endif()
    set(_editor_output "${CMAKE_CURRENT_BINARY_DIR}/${_editor_launcher}.rxbin")
    add_custom_command(OUTPUT "${_editor_output}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}" -i "${CMAKE_BINARY_DIR}/bin"
                    -o ${_editor_launcher} "${CMAKE_CURRENT_SOURCE_DIR}/${_editor_launcher}.crexx"
            COMMAND $<TARGET_FILE:rxas> -o ${_editor_launcher} ${_editor_launcher}
            DEPENDS rxc rxas compiler_exit_bin example_ui_scene_editors ${_editor_driver}
                    "${CMAKE_CURRENT_SOURCE_DIR}/${_editor_launcher}.crexx"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${_editor_launcher}.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(${_editor_launcher}_artifact DEPENDS "${_editor_output}")
    add_dependencies(example_ui_scene_editor_artifacts ${_editor_launcher}_artifact)
endforeach()

if(UNIX)
    set(_scene_ansi "${CMAKE_CURRENT_BINARY_DIR}/graph_ansi.rxbin")
    add_custom_command(OUTPUT "${_scene_ansi}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}" -i "${CMAKE_BINARY_DIR}/bin"
                    -o graph_ansi "${CMAKE_CURRENT_SOURCE_DIR}/graph_ansi.crexx"
            COMMAND $<TARGET_FILE:rxas> -o graph_ansi graph_ansi
            DEPENDS rxc rxas compiler_exit_bin example_ui_scene ui_ansi graph_ansi.crexx
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/graph_ansi.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(example_ui_scene_ansi DEPENDS "${_scene_ansi}")
endif()

install(FILES "${_scene_example}" "${_scene_editors}"
        "${CMAKE_CURRENT_BINARY_DIR}/graph_web.rxbin"
        "${CMAKE_CURRENT_BINARY_DIR}/map_web.rxbin"
        DESTINATION examples/ui/scene)
set_property(GLOBAL APPEND PROPERTY CREXX_INSTALLED_PRODUCT_TARGETS
        example_ui_scene example_ui_scene_editor_artifacts)
if(UNIX)
    install(FILES "${_scene_ansi}"
            "${CMAKE_CURRENT_BINARY_DIR}/graph_editor_ansi.rxbin"
            "${CMAKE_CURRENT_BINARY_DIR}/map_editor_ansi.rxbin"
            DESTINATION examples/ui/scene)
    set_property(GLOBAL APPEND PROPERTY CREXX_INSTALLED_PRODUCT_TARGETS
            example_ui_scene_ansi)
endif()
if(ENABLE_GTK)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/graph_editor_gtk.rxbin"
            "${CMAKE_CURRENT_BINARY_DIR}/map_editor_gtk.rxbin"
            DESTINATION examples/ui/scene)
endif()

if(BUILD_TESTING)
    add_custom_target(example_ui_scene_tests)
    crexx_add_rexx_opt_matrix(
            NAME ui_scene_editors RUNNER rxvm
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_scene_editors.crexx
            TARGET_GROUP example_ui_scene_tests
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
            DEPENDS example_ui_scene_editors ui_scene ui_contract
            RUNTIME_ARGS ${_scene_editors}
                    ${CMAKE_BINARY_DIR}/bin/ui_scene
                    ${CMAKE_BINARY_DIR}/bin/ui_contract
                    ${CMAKE_BINARY_DIR}/bin/ui_catalog
                    ${CMAKE_BINARY_DIR}/bin/ui
                    ${CMAKE_BINARY_DIR}/bin/library
            TEST_LABELS "examples;ui;scene;graph;map"
            REQUIRED_OUTPUT_REGEX "PASS: graph and map editor transitions"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    crexx_add_rexx_opt_matrix(
            NAME ui_scene_examples RUNNER rxvm
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_scene_examples.crexx
            TARGET_GROUP example_ui_scene_tests
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
            DEPENDS example_ui_scene ui_terminal_view ui_scene ui_contract
            RUNTIME_ARGS ${_scene_example}
                    ${CMAKE_BINARY_DIR}/bin/ui_terminal_view
                    ${CMAKE_BINARY_DIR}/bin/ui_scene
                    ${CMAKE_BINARY_DIR}/bin/ui_contract
                    ${CMAKE_BINARY_DIR}/bin/ui_catalog
                    ${CMAKE_BINARY_DIR}/bin/ui
                    ${CMAKE_BINARY_DIR}/bin/library
            TEST_LABELS "examples;ui;scene;graph;map"
            REQUIRED_OUTPUT_REGEX "PASS: scene graph and map feature transitions"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    if(UNIX)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        add_test(NAME ui_scene_editors_ansi_pty
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/lib/plugins/console/tests/pty_checks.py
                        --build ${CMAKE_BINARY_DIR} --source ${CMAKE_SOURCE_DIR}
                        --mode scene-editors)
        set_tests_properties(ui_scene_editors_ansi_pty PROPERTIES TIMEOUT 60
                LABELS "examples;ui;ansi;scene;graph;map"
                CREXX_PREP_TARGETS "example_ui_scene_editor_artifacts;rxvm")
        add_test(NAME ui_scene_ansi_pty
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/lib/plugins/console/tests/pty_checks.py
                        --build ${CMAKE_BINARY_DIR} --source ${CMAKE_SOURCE_DIR}
                        --mode scene)
        set_tests_properties(ui_scene_ansi_pty PROPERTIES TIMEOUT 60
                LABELS "examples;ui;ansi;scene;graph"
                CREXX_PREP_TARGETS "example_ui_scene_ansi;rxvm")
    endif()
    if(ENABLE_GTK)
        crexx_add_rexx_opt_matrix(
                NAME ui_scene_editors_gtk RUNNER rxvm
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_scene_editors_gtk.crexx
                TARGET_GROUP example_ui_scene_tests
                IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
                DEPENDS example_ui_scene_editors ui_gtk _ui_gtk_native _ui_gtk_test
                RUNTIME_ARGS $<TARGET_FILE:_ui_gtk_native> $<TARGET_FILE:_ui_gtk_test>
                        ${_scene_editors}
                        ${CMAKE_BINARY_DIR}/bin/ui_gtk
                        ${CMAKE_BINARY_DIR}/bin/ui_scene
                        ${CMAKE_BINARY_DIR}/bin/ui_local_resources
                        ${CMAKE_BINARY_DIR}/bin/ui_contract
                        ${CMAKE_BINARY_DIR}/bin/ui_catalog
                        ${CMAKE_BINARY_DIR}/bin/ui
                        ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "examples;ui;gtk;scene;graph;map"
                REQUIRED_OUTPUT_REGEX "PASS: GTK graph and map editor routes"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
        set_tests_properties(ui_scene_editors_gtk_noopt ui_scene_editors_gtk_opt PROPERTIES
                TIMEOUT 60 RESOURCE_LOCK crexx_gtk_surface)
        crexx_add_rexx_opt_matrix(
                NAME ui_scene_gtk RUNNER rxvm
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_scene_gtk.crexx
                TARGET_GROUP example_ui_scene_tests
                IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
                DEPENDS example_ui_scene ui_gtk _ui_gtk_native _ui_gtk_test
                RUNTIME_ARGS $<TARGET_FILE:_ui_gtk_native> $<TARGET_FILE:_ui_gtk_test>
                        ${_scene_example}
                        ${CMAKE_BINARY_DIR}/bin/ui_gtk
                        ${CMAKE_BINARY_DIR}/bin/ui_scene
                        ${CMAKE_BINARY_DIR}/bin/ui_local_resources
                        ${CMAKE_BINARY_DIR}/bin/ui_contract
                        ${CMAKE_BINARY_DIR}/bin/ui_catalog
                        ${CMAKE_BINARY_DIR}/bin/ui
                        ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "examples;ui;gtk;scene;graph"
                REQUIRED_OUTPUT_REGEX "PASS: GTK graph scene projection"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
        set_tests_properties(ui_scene_gtk_noopt ui_scene_gtk_opt PROPERTIES
                TIMEOUT 60 RESOURCE_LOCK crexx_gtk_surface)
    endif()
endif()
