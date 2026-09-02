cmake_minimum_required(VERSION 3.24)

project(crexx_ui_gtk_driver C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)

include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(_ui_gtk_native ui_gtk.c)
target_link_libraries(_ui_gtk_native PRIVATE PkgConfig::GTK3)
