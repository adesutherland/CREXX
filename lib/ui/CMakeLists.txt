cmake_minimum_required(VERSION 3.24)

project(crexx_ui NONE)

set(CREXX_UI_CORE_IMAGE "${CMAKE_BINARY_DIR}/bin/ui.rxbin")
set(CREXX_UI_TUI_IMAGE "${CMAKE_BINARY_DIR}/bin/ui_tui.rxbin")

add_custom_command(
        OUTPUT "${CREXX_UI_CORE_IMAGE}"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin"
                -o ui "${CMAKE_CURRENT_SOURCE_DIR}/ui.crexx"
        COMMAND $<TARGET_FILE:rxas> -o "${CMAKE_BINARY_DIR}/bin/ui" ui
        DEPENDS rxc rxas library compiler_exit_bin
                "${CMAKE_CURRENT_SOURCE_DIR}/ui.crexx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/ui.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building experimental backend-neutral Level G UI library"
        VERBATIM)

add_custom_command(
        OUTPUT "${CREXX_UI_TUI_IMAGE}"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin"
                -o ui_tui "${CMAKE_CURRENT_SOURCE_DIR}/ui_tui.crexx"
        COMMAND $<TARGET_FILE:rxas> -o "${CMAKE_BINARY_DIR}/bin/ui_tui" ui_tui
        DEPENDS rxc rxas library compiler_exit_bin
                "${CREXX_UI_CORE_IMAGE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/ui_tui.crexx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/ui_tui.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building portable Level G cREXX TUI driver"
        VERBATIM)

add_custom_target(ui ALL DEPENDS "${CREXX_UI_CORE_IMAGE}")
add_custom_target(ui_tui ALL DEPENDS "${CREXX_UI_TUI_IMAGE}")
add_dependencies(ui library)
add_dependencies(ui_tui ui)

# The vocabulary has one source. Generate language bindings and transport docs
# with CMake itself, so building the native product does not require Node/Python.
set(CREXX_UI_CONTRACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/contracts")
set(CREXX_UI_CATALOG_SOURCE "${CREXX_UI_CONTRACT_DIR}/ui_catalog.crexx")
add_custom_command(
        OUTPUT "${CREXX_UI_CATALOG_SOURCE}"
               "${CREXX_UI_CONTRACT_DIR}/ui-contract.ts"
               "${CREXX_UI_CONTRACT_DIR}/ui-contract.schema.json"
               "${CREXX_UI_CONTRACT_DIR}/CATALOGUE.md"
        COMMAND ${CMAKE_COMMAND}
                "-DCATALOGUE=${CMAKE_CURRENT_SOURCE_DIR}/contracts/catalogue.json"
                "-DOUT=${CREXX_UI_CONTRACT_DIR}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/contracts/Generate.cmake"
        DEPENDS contracts/catalogue.json contracts/Generate.cmake
        COMMENT "Generating UI contract v0 bindings and schema"
        VERBATIM)
foreach(module IN ITEMS ui_catalog ui_contract ui_compat)
    set(module_inputs "${CREXX_UI_CORE_IMAGE}")
    if(module STREQUAL "ui_catalog")
        set(module_source "${CREXX_UI_CATALOG_SOURCE}")
        set(module_dependencies ui)
    elseif(module STREQUAL "ui_contract")
        set(module_source "${CMAKE_CURRENT_SOURCE_DIR}/ui_contract.crexx")
        set(module_dependencies ui ui_catalog)
        list(APPEND module_inputs "${CMAKE_BINARY_DIR}/bin/ui_catalog.rxbin")
    else()
        set(module_source "${CMAKE_CURRENT_SOURCE_DIR}/ui_compat.crexx")
        set(module_dependencies ui ui_catalog ui_contract)
        list(APPEND module_inputs "${CMAKE_BINARY_DIR}/bin/ui_catalog.rxbin"
                "${CMAKE_BINARY_DIR}/bin/ui_contract.rxbin")
    endif()
    add_custom_command(
            OUTPUT "${CMAKE_BINARY_DIR}/bin/${module}.rxbin"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_BINARY_DIR}/bin" -o ${module} "${module_source}"
            COMMAND $<TARGET_FILE:rxas> -o "${CMAKE_BINARY_DIR}/bin/${module}" ${module}
            DEPENDS rxc rxas library compiler_exit_bin ${module_dependencies}
                    ${module_inputs} "${module_source}"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${module}.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            VERBATIM)
    add_custom_target(${module} ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/${module}.rxbin")
    add_dependencies(${module} ${module_dependencies})
    install(FILES "${CMAKE_BINARY_DIR}/bin/${module}.rxbin" DESTINATION bin)
endforeach()
install(FILES contracts/catalogue.json contracts/README.md contracts/conformance.jsonl
        "${CREXX_UI_CONTRACT_DIR}/ui-contract.ts"
        "${CREXX_UI_CONTRACT_DIR}/ui-contract.schema.json"
        "${CREXX_UI_CONTRACT_DIR}/CATALOGUE.md"
        DESTINATION share/crexx/ui/contracts)

if(ENABLE_GTK)
    add_subdirectory(drivers/gtk)
    set(CREXX_UI_GTK_IMAGE "${CMAKE_BINARY_DIR}/bin/ui_gtk.rxbin")
    add_custom_command(
            OUTPUT "${CREXX_UI_GTK_IMAGE}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_BINARY_DIR}/bin"
                    -o ui_gtk "${CMAKE_CURRENT_SOURCE_DIR}/ui_gtk.crexx"
            COMMAND $<TARGET_FILE:rxas> -o "${CMAKE_BINARY_DIR}/bin/ui_gtk" ui_gtk
            DEPENDS rxc rxas library compiler_exit_bin ui _ui_gtk_native
                    "${CMAKE_CURRENT_SOURCE_DIR}/ui_gtk.crexx"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/ui_gtk.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Building Level G cREXX GTK UI driver"
            VERBATIM)
    add_custom_target(ui_gtk ALL DEPENDS "${CREXX_UI_GTK_IMAGE}")
    add_dependencies(ui_gtk ui _ui_gtk_native)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests_functional)
endif()

install(FILES
        "${CREXX_UI_CORE_IMAGE}"
        "${CREXX_UI_TUI_IMAGE}"
        DESTINATION bin)
if(ENABLE_GTK)
    install(FILES "${CREXX_UI_GTK_IMAGE}" DESTINATION bin)
    install(TARGETS _ui_gtk_native
            LIBRARY DESTINATION bin
            RUNTIME DESTINATION bin)
endif()
install(FILES README.md DESTINATION share/crexx/ui)
install(FILES rxpp/ui_macros.rxpm DESTINATION share/crexx/ui/rxpp)
install(DIRECTORY rxpp/ui DESTINATION share/crexx/ui/rxpp)

crexx_finalize_directory_qa_tiers()
