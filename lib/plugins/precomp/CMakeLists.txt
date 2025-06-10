cmake_minimum_required(VERSION 3.24)
project(precomp_plugin C)

set(CMAKE_C_STANDARD 99)
# -------------------------------------------------------------------------------------------
# Set plugin_name and test_rexx to match the specific plugin and its corresponding test case.
# -------------------------------------------------------------------------------------------
set(plugin_name "precomp")       # add plugin name
set(test_rexx "rxppt")           # add rexx to test plugin
set(use_precomp FALSE)           # TRUE pre compile needed, FALSE if not
set(rx_include "\"${CMAKE_BINARY_DIR}/lib/rxfnsb;${CMAKE_CURRENT_BINARY_DIR}\"")
# ===========================================================================================
# The following sequence is generic and does not require modification.
#   plugin_template(${plugin_name} ${test_rexx} PRECOMP)
# ===========================================================================================
include(${CMAKE_SOURCE_DIR}/rxpa/CMakePluginTemplate.txt)

plugin_template(${plugin_name} ${test_rexx} ${PRECOMP_ARG} ${use_precomp})

# Enable testing and define the test case
enable_testing()
# execute test
add_test(NAME ${test_rexx}
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm ${test_rexx} rx_${plugin_name} ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i "${rx_include}" -o rxpp ${CMAKE_CURRENT_SOURCE_DIR}/rxpp
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas rxpp
        COMMAND ${CMAKE_BINARY_DIR}/cpacker/rxcpack rxpp ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        DEPENDS rxas rxc rxpp.rexx library rxcpack rxpa
        OUTPUT rxpp.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "rxpp compile, assemble and package rxpp ..."
)

add_executable(rxpp rxpp.c)

target_link_libraries(rxpp rxvml rxpa machine avl_tree platform m)

add_dependencies(rxpp rxvml machine rxpa avl_tree platform)

