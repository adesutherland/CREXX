cmake_minimum_required(VERSION 3.24)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

set(_process "${CMAKE_CURRENT_BINARY_DIR}/process_designer.rxbin")
add_custom_command(OUTPUT "${_process}"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin" -o process_designer
                "${CMAKE_CURRENT_SOURCE_DIR}/process_designer.crexx"
        COMMAND $<TARGET_FILE:rxas> -o process_designer process_designer
        DEPENDS rxc rxas compiler_exit_bin ui ui_contract ui_scene
                "${CMAKE_CURRENT_SOURCE_DIR}/process_designer.crexx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/process_designer.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(example_ui_process_designer DEPENDS "${_process}")

add_custom_target(example_ui_process_artifacts)
foreach(_launcher IN ITEMS process_web process_gtk process_ansi)
    if(_launcher STREQUAL "process_gtk" AND NOT ENABLE_GTK)
        continue()
    endif()
    if(_launcher STREQUAL "process_ansi" AND NOT UNIX)
        continue()
    endif()
    if(_launcher STREQUAL "process_web")
        set(_driver ui_web)
    elseif(_launcher STREQUAL "process_gtk")
        set(_driver ui_gtk)
    else()
        set(_driver ui_ansi)
    endif()
    set(_output "${CMAKE_CURRENT_BINARY_DIR}/${_launcher}.rxbin")
    add_custom_command(OUTPUT "${_output}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}" -i "${CMAKE_BINARY_DIR}/bin"
                    -o ${_launcher} "${CMAKE_CURRENT_SOURCE_DIR}/${_launcher}.crexx"
            COMMAND $<TARGET_FILE:rxas> -o ${_launcher} ${_launcher}
            DEPENDS rxc rxas compiler_exit_bin example_ui_process_designer ${_driver}
                    "${CMAKE_CURRENT_SOURCE_DIR}/${_launcher}.crexx"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${_launcher}.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(${_launcher}_artifact DEPENDS "${_output}")
    add_dependencies(example_ui_process_artifacts ${_launcher}_artifact)
endforeach()

install(FILES "${_process}" "${CMAKE_CURRENT_BINARY_DIR}/process_web.rxbin"
        DESTINATION examples/ui/process-designer)
if(ENABLE_GTK)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/process_gtk.rxbin"
            DESTINATION examples/ui/process-designer)
endif()
if(UNIX)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/process_ansi.rxbin"
            DESTINATION examples/ui/process-designer)
endif()
set_property(GLOBAL APPEND PROPERTY CREXX_INSTALLED_PRODUCT_TARGETS
        example_ui_process_artifacts)

if(BUILD_TESTING)
    add_custom_target(example_ui_process_tests)
    crexx_add_rexx_opt_matrix(
            NAME ui_process_designer RUNNER rxvm
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_process_designer.crexx
            TARGET_GROUP example_ui_process_tests
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
            DEPENDS example_ui_process_designer ui_scene ui_contract
            RUNTIME_ARGS ${_process}
                    ${CMAKE_BINARY_DIR}/bin/ui_scene
                    ${CMAKE_BINARY_DIR}/bin/ui_contract
                    ${CMAKE_BINARY_DIR}/bin/ui_catalog
                    ${CMAKE_BINARY_DIR}/bin/ui
                    ${CMAKE_BINARY_DIR}/bin/library
            TEST_LABELS "examples;ui;scene;process"
            REQUIRED_OUTPUT_REGEX "PASS: process designer transitions"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    if(UNIX)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        add_test(NAME ui_process_ansi_pty
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/lib/plugins/console/tests/pty_checks.py
                        --build ${CMAKE_BINARY_DIR} --source ${CMAKE_SOURCE_DIR}
                        --mode process-designer)
        set_tests_properties(ui_process_ansi_pty PROPERTIES TIMEOUT 60
                LABELS "examples;ui;ansi;process"
                CREXX_PREP_TARGETS "example_ui_process_artifacts;rxvm")
    endif()
    if(ENABLE_GTK)
        crexx_add_rexx_opt_matrix(
                NAME ui_process_gtk RUNNER rxvm
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_process_gtk.crexx
                TARGET_GROUP example_ui_process_tests
                IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
                DEPENDS example_ui_process_designer ui_gtk _ui_gtk_native _ui_gtk_test
                RUNTIME_ARGS $<TARGET_FILE:_ui_gtk_native> $<TARGET_FILE:_ui_gtk_test>
                        ${_process}
                        ${CMAKE_BINARY_DIR}/bin/ui_gtk
                        ${CMAKE_BINARY_DIR}/bin/ui_scene
                        ${CMAKE_BINARY_DIR}/bin/ui_local_resources
                        ${CMAKE_BINARY_DIR}/bin/ui_contract
                        ${CMAKE_BINARY_DIR}/bin/ui_catalog
                        ${CMAKE_BINARY_DIR}/bin/ui
                        ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "examples;ui;gtk;scene;process"
                REQUIRED_OUTPUT_REGEX "PASS: GTK process graph and form"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
        set_tests_properties(ui_process_gtk_noopt ui_process_gtk_opt PROPERTIES
                TIMEOUT 60 RESOURCE_LOCK crexx_gtk_surface)
    endif()
endif()
