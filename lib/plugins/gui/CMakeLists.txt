cmake_minimum_required(VERSION 3.24)
project(gui_plugin C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(_gui gui.c)
target_link_libraries(_gui PRIVATE PkgConfig::GTK3)

if(BUILD_TESTING)
    add_custom_target(gui_test ALL)
    crexx_add_rexx_opt_matrix(
            PASS_TEST_NAME_ARGUMENT
            NAME gui_test
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/gui_test.crexx
            TARGET_GROUP gui_test
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS library _gui
            RUNTIME_ARGS rx_gui ${CMAKE_BINARY_DIR}/bin/library
            TEST_LABELS "library;plugins;rxpa;gui;gtk"
            REQUIRED_OUTPUT_REGEX "PASS: gui GTK public contract"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    set_tests_properties(gui_test_noopt gui_test_opt PROPERTIES TIMEOUT 30)
endif()
