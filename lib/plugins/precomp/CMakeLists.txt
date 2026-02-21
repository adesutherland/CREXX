cmake_minimum_required(VERSION 3.24)
project(precomp_plugin C)

set(CMAKE_C_STANDARD 99)

# Include RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create the static link module
add_static_plugin_target(precomp precomp.c)

# Create the dynamic link module - this is needed for rxc to compile rxpp.rexx
add_dynamic_plugin_target(precomp precomp.c)

add_custom_command(
        COMMAND $<TARGET_FILE:rxc> -i "\"${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}\"" -o rxpp ${CMAKE_CURRENT_SOURCE_DIR}/rxpp
        COMMAND $<TARGET_FILE:rxas> rxpp
        COMMAND $<TARGET_FILE:rxcpack> rxpp ${CMAKE_BINARY_DIR}/bin/library
        DEPENDS precomp rxas rxc rxpp.rexx library rxcpack rxpa
        OUTPUT rxpp.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "rxpp compile, assemble and package rxpp ..."
)

add_executable(rxpp rxpp.c)
# Ensure precomp static lib is force-loaded, then append providers (rxpashim, rxpa) after it
# This ordering is important for Unix linkers when resolving symbols from static archives.
# First, inject the whole-archive of precomp_static so its objects come before providers
configure_linker_for_static_lib(rxpp precomp)
# Now link other required libraries. Place rxpashim (which provides rxpa_* symbols) AFTER the precomp objects.
target_link_libraries(rxpp precomp_static rxvml rxpashim)
add_dependencies(rxpp rxvml rxpa avl_tree platform)
