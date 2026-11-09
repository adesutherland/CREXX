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

if(BUILD_TESTING)
    add_custom_target(example_ui_app_effects_tests)
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
