cmake_minimum_required(VERSION 3.24)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
add_custom_target(example_franz_lotto_artifacts)

foreach(module IN ITEMS lotto_logic lotto_surface)
    set(source "${CMAKE_CURRENT_SOURCE_DIR}/surface/${module}.crexx")
    set(module_dependencies ui ui_contract)
    if(module STREQUAL "lotto_logic")
        set(source "${CMAKE_CURRENT_SOURCE_DIR}/crexx/lotto_logic.crexx")
    else()
        list(APPEND module_dependencies franz_lotto_logic
                "${CMAKE_CURRENT_BINARY_DIR}/lotto_logic.rxbin")
    endif()
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${module}.rxbin"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}" -i "${CMAKE_BINARY_DIR}/bin"
                    -o ${module} "${source}"
            COMMAND $<TARGET_FILE:rxas> -o ${module} ${module}
            DEPENDS rxc rxas compiler_exit_bin ${module_dependencies} "${source}"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${module}.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(franz_${module} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${module}.rxbin")
endforeach()

set(shared_images "${CMAKE_CURRENT_BINARY_DIR}/lotto_surface.rxbin"
        "${CMAKE_CURRENT_BINARY_DIR}/lotto_logic.rxbin"
        "${CMAKE_BINARY_DIR}/bin/ui_local_resources.rxbin"
        "${CMAKE_BINARY_DIR}/bin/ui_contract.rxbin"
        "${CMAKE_BINARY_DIR}/bin/ui_catalog.rxbin"
        "${CMAKE_BINARY_DIR}/bin/ui.rxbin" "${CMAKE_BINARY_DIR}/bin/library.rxbin")
set(hosts console line)
if(ENABLE_GTK)
    list(APPEND hosts gtk)
endif()
foreach(host IN LISTS hosts)
    set(driver ui_${host})
    set(host_images "${CMAKE_BINARY_DIR}/bin/${driver}.rxbin")
    if(host STREQUAL "console")
        set(driver ui_ansi)
        set(host_images "${CMAKE_BINARY_DIR}/bin/ui_ansi.rxbin"
                "${CMAKE_BINARY_DIR}/bin/ui_dialogs.rxbin"
                "${CMAKE_BINARY_DIR}/bin/ui_terminal_view.rxbin")
    elseif(host STREQUAL "line")
        set(driver ui_tui)
        set(host_images "${CMAKE_BINARY_DIR}/bin/ui_tui.rxbin")
    endif()
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Lotto-${host}.rxbin"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}" -i "${CMAKE_BINARY_DIR}/bin"
                    -o lotto_${host} "${CMAKE_CURRENT_SOURCE_DIR}/surface/lotto_${host}.crexx"
            COMMAND $<TARGET_FILE:rxas> -o lotto_${host} lotto_${host}
            COMMAND $<TARGET_FILE:rxlink> -s -o Lotto-${host}
                    lotto_${host}.rxbin ${host_images} ${shared_images}
            DEPENDS rxc rxas rxlink compiler_exit_bin franz_lotto_surface ${driver}
                    ${host_images} ${shared_images} "surface/lotto_${host}.crexx"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/lotto_${host}.rxas"
                    "${CMAKE_CURRENT_BINARY_DIR}/lotto_${host}.rxbin"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(franz_lotto_${host}
            DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/Lotto-${host}.rxbin")
    add_dependencies(example_franz_lotto_artifacts franz_lotto_${host})
endforeach()

if(BUILD_TESTING)
    crexx_add_rexx_opt_matrix(
            NAME franz_lotto_surface RUNNER rxvm
            SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/surface/lotto_surface_tests.crexx"
            TARGET_GROUP example_franz_lotto_artifacts
            IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
            DEPENDS franz_lotto_surface ui_tui ${shared_images}
            RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/ui_tui ${shared_images}
            TEST_LABELS "examples;ui;tui;lotto;effects"
            REQUIRED_OUTPUT_REGEX "PASS: shared Lotto Save chain"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    if(ENABLE_GTK)
        crexx_add_rexx_opt_matrix(
                NAME franz_lotto_gtk RUNNER rxvm
                SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/surface/lotto_gtk_tests.crexx"
                TARGET_GROUP example_franz_lotto_artifacts
                IMPORT_PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR}/bin
                DEPENDS franz_lotto_surface ui_gtk _ui_gtk_native _ui_gtk_test ${shared_images}
                RUNTIME_ARGS $<TARGET_FILE:_ui_gtk_native> $<TARGET_FILE:_ui_gtk_test>
                        ${CMAKE_BINARY_DIR}/bin/ui_gtk ${shared_images}
                        -a "${CMAKE_CURRENT_BINARY_DIR}/GTK test tips.txt"
                TEST_LABELS "examples;ui;gtk;lotto;dialogs;effects"
                REQUIRED_OUTPUT_REGEX "PASS: shared Lotto GTK controls, paging, validation, Save and reuse"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
        set_tests_properties(franz_lotto_gtk_noopt franz_lotto_gtk_opt PROPERTIES
                TIMEOUT 60 RESOURCE_LOCK crexx_gtk_surface)
    endif()
endif()
