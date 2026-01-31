cmake_minimum_required(VERSION 3.24)

project(RXVMPlugin C)

set(CMAKE_C_STANDARD 90)

# Add subdirectories first if needed
add_subdirectory(rxvmplugins)

# Add the rxvmplugin support library
add_library(rxvmplugin rxvmplugin_framework.c rxvmplugin.h rxvmplugin_framework.h)
target_include_directories(rxvmplugin PRIVATE .. ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/binutils/include)


