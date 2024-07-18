cmake_minimum_required(VERSION 3.5)
project(rxpatest C)

set(CMAKE_C_STANDARD 90)

# Including RXPlugin Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic link module
add_dynamic_plugin_target(_rxpa_dynlink rxpa_dynlink.c)
add_dependencies(_rxpa_dynlink rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

# Create static link module
# The static declaration library
add_decl_plugin_target(_rxpalink rxpa_staticlink.c)
# The static link test - a version of rxc that statically links the _rxpa_decl library
add_executable(rxc_staticlink ${CMAKE_SOURCE_DIR}/compiler/rxc_main.c)
# Ensure the linker includes the plugin
get_plugin_init_name("_rxpalink" init_function_name)
configure_linker_for_static_lib(rxc_staticlink _rxpalink "${CMAKE_CURRENT_BINARY_DIR}/rx_rxpalink_decl.a")
# Add the dependencies and link libraries
add_dependencies(rxc_staticlink rxclib _rxpalink_decl rxpa library rxvm)
target_link_libraries(rxc_staticlink rxclib machine avl_tree platform rxaslib rxpa _rxpalink_decl m)

# Enable testing functionality
enable_testing()

# Add desbase_test as a test
add_test(NAME rxpa_dynlink_test COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -d
        -i "${CMAKE_BINARY_DIR}/lib/rxfns;${CMAKE_CURRENT_BINARY_DIR}" -o rxpa_dynlink ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_dynlink
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME rxpa_staticlink_test COMMAND rxc_staticlink -d
        -i "${CMAKE_BINARY_DIR}/lib/rxfns" -o rxpa_staticlink ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_staticlink
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

