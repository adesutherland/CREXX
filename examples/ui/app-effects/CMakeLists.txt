cmake_minimum_required(VERSION 3.24)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

add_custom_target(example_ui_app_effects_artifacts)
foreach(_app IN ITEMS graph_application map_application)
    set(_output "${CMAKE_CURRENT_BINARY_DIR}/${_app}.rxbin")
    add_custom_command(OUTPUT "${_output}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_BINARY_DIR}/examples/ui/scene"
                    -i "${CMAKE_BINARY_DIR}/bin"
                    -o ${_app} "${CMAKE_CURRENT_SOURCE_DIR}/${_app}.crexx"
            COMMAND $<TARGET_FILE:rxas> -o ${_app} ${_app}
            DEPENDS rxc rxas compiler_exit_bin example_ui_scene_editors
                    ui ui_contract "${CMAKE_CURRENT_SOURCE_DIR}/${_app}.crexx"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${_app}.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(${_app}_artifact DEPENDS "${_output}")
    add_dependencies(example_ui_app_effects_artifacts ${_app}_artifact)
endforeach()

add_custom_target(example_ui_application_packages)
foreach(_launcher IN ITEMS graph_application_web map_application_web graph_application_gtk map_application_gtk graph_application_ansi map_application_ansi)
    if(_launcher MATCHES "_gtk$" AND NOT ENABLE_GTK)
        continue()
    endif()
    if(_launcher MATCHES "_ansi$" AND NOT UNIX)
        continue()
    endif()
    if(_launcher MATCHES "_web$")
        set(_driver ui_web)
    elseif(_launcher MATCHES "_gtk$")
        set(_driver ui_gtk)
    else()
        set(_driver ui_ansi)
    endif()
    set(_output "${CMAKE_CURRENT_BINARY_DIR}/${_launcher}.rxbin")
    add_custom_command(OUTPUT "${_output}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}"
                    -i "${CMAKE_BINARY_DIR}/bin"
                    -o ${_launcher} "${CMAKE_CURRENT_SOURCE_DIR}/${_launcher}.crexx"
            COMMAND $<TARGET_FILE:rxas> -o ${_launcher} ${_launcher}
            DEPENDS rxc rxas compiler_exit_bin example_ui_app_effects_artifacts
                    ${_driver} "${CMAKE_CURRENT_SOURCE_DIR}/${_launcher}.crexx"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${_launcher}.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(${_launcher}_artifact DEPENDS "${_output}")
    add_dependencies(example_ui_application_packages ${_launcher}_artifact)
endforeach()

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/graph_application.rxbin"
        "${CMAKE_CURRENT_BINARY_DIR}/map_application.rxbin"
        "${CMAKE_CURRENT_BINARY_DIR}/graph_application_web.rxbin"
        "${CMAKE_CURRENT_BINARY_DIR}/map_application_web.rxbin"
        DESTINATION examples/ui/app-effects)
if(UNIX)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/graph_application_ansi.rxbin"
            "${CMAKE_CURRENT_BINARY_DIR}/map_application_ansi.rxbin"
            DESTINATION examples/ui/app-effects)
endif()
if(ENABLE_GTK)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/graph_application_gtk.rxbin"
            "${CMAKE_CURRENT_BINARY_DIR}/map_application_gtk.rxbin"
            DESTINATION examples/ui/app-effects)
endif()
install(FILES graph.package.json map.package.json graph-repository.mjs map-repository.mjs
        DESTINATION examples/ui/app-effects)
install(FILES ${CMAKE_SOURCE_DIR}/web/ui/run-package.mjs
        ${CMAKE_SOURCE_DIR}/web/ui/package-registry.mjs
        DESTINATION examples/ui/app-effects)
set_property(GLOBAL APPEND PROPERTY CREXX_INSTALLED_PRODUCT_TARGETS
        example_ui_app_effects_artifacts example_ui_application_packages)

if(BUILD_TESTING)
    if(UNIX)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        add_test(NAME ui_application_packages_ansi_pty
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/lib/plugins/console/tests/pty_checks.py
                        --build ${CMAKE_BINARY_DIR} --source ${CMAKE_SOURCE_DIR}
                        --mode application-packages)
        set_tests_properties(ui_application_packages_ansi_pty PROPERTIES TIMEOUT 60
                LABELS "examples;ui;application;ansi;package"
                CREXX_PREP_TARGETS "example_ui_application_packages;rxvm")
    endif()
    add_custom_target(example_ui_app_effects_tests)
    if(ENABLE_GTK)
        crexx_add_rexx_opt_matrix(
                NAME ui_application_packages_gtk RUNNER rxvm
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_application_gtk.crexx
                TARGET_GROUP example_ui_app_effects_tests
                IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR}
                             ${CMAKE_BINARY_DIR}/examples/ui/scene
                             ${CMAKE_BINARY_DIR}/bin
                DEPENDS example_ui_application_packages ui_gtk _ui_gtk_native _ui_gtk_test
                RUNTIME_ARGS $<TARGET_FILE:_ui_gtk_native> $<TARGET_FILE:_ui_gtk_test>
                        ${CMAKE_CURRENT_BINARY_DIR}/graph_application.rxbin
                        ${CMAKE_CURRENT_BINARY_DIR}/map_application.rxbin
                        ${CMAKE_BINARY_DIR}/examples/ui/scene/scene_editors.rxbin
                        ${CMAKE_BINARY_DIR}/bin/ui_gtk.rxbin
                        ${CMAKE_BINARY_DIR}/bin/ui_scene.rxbin
                        ${CMAKE_BINARY_DIR}/bin/ui_local_resources.rxbin
                        ${CMAKE_BINARY_DIR}/bin/ui_contract.rxbin
                        ${CMAKE_BINARY_DIR}/bin/ui_catalog.rxbin
                        ${CMAKE_BINARY_DIR}/bin/ui.rxbin
                        ${CMAKE_BINARY_DIR}/bin/library.rxbin
                TEST_LABELS "examples;ui;application;gtk;package"
                REQUIRED_OUTPUT_REGEX "PASS: GTK application package compositions"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
        set_tests_properties(ui_application_packages_gtk_noopt ui_application_packages_gtk_opt PROPERTIES
                TIMEOUT 60 RESOURCE_LOCK crexx_gtk_surface)
    endif()
    crexx_add_rexx_opt_matrix(
            NAME ui_application_effects RUNNER rxvm
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_application_effects.crexx
            TARGET_GROUP example_ui_app_effects_tests
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR}
                         ${CMAKE_BINARY_DIR}/examples/ui/scene
                         ${CMAKE_BINARY_DIR}/bin
            DEPENDS example_ui_app_effects_artifacts ui_scene ui_contract
            RUNTIME_ARGS ${CMAKE_CURRENT_BINARY_DIR}/graph_application.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/map_application.rxbin
                    ${CMAKE_BINARY_DIR}/examples/ui/scene/scene_editors.rxbin
                    ${CMAKE_BINARY_DIR}/bin/ui_scene.rxbin
                    ${CMAKE_BINARY_DIR}/bin/ui_contract.rxbin
                    ${CMAKE_BINARY_DIR}/bin/ui_catalog.rxbin
                    ${CMAKE_BINARY_DIR}/bin/ui.rxbin
                    ${CMAKE_BINARY_DIR}/bin/library.rxbin
            TEST_LABELS "examples;ui;application;effects"
            REQUIRED_OUTPUT_REGEX "PASS: independent graph and map application effects"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
endif()
