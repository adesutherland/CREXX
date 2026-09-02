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

crexx_finalize_directory_qa_tiers()
