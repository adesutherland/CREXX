cmake_minimum_required(VERSION 3.24)
project(lotto_beta3_gui_provider C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
add_library(lotto_gui_plugin MODULE gui.c)
target_include_directories(lotto_gui_plugin PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/rxpa")
target_compile_definitions(lotto_gui_plugin PRIVATE BUILD_DLL PLUGIN_ID=rx_gui)
target_link_libraries(lotto_gui_plugin PRIVATE PkgConfig::GTK3)
set_target_properties(lotto_gui_plugin PROPERTIES
  PREFIX "" OUTPUT_NAME "rx_gui" SUFFIX ".rxplugin")
