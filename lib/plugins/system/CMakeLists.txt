cmake_minimum_required(VERSION 3.24)
project(strings_plugin C)

set(CMAKE_C_STANDARD 99)
# -------------------------------------------------------------------------------------------
# Set plugin_name and test_rexx to match the specific plugin and its corresponding test case.
# -------------------------------------------------------------------------------------------
set(plugin_name "system")       # add plugin name
set(test_rexx "system_test")    # add rexx to test plugin
set(use_precomp TRUE)           # TRUE pre compile needed, FALSE if not

# ===========================================================================================
# The following sequence is generic and does not require modification.
#   plugin_template(${plugin_name} ${test_rexx} PRECOMP)
# ===========================================================================================
  include(${CMAKE_SOURCE_DIR}/rxpa/CMakePluginTemplate.txt)

  plugin_template(${plugin_name} ${test_rexx} ${PRECOMP_ARG} ${use_precomp})

# Create the static link module
add_static_plugin_target(_system system.c)
  
# Enable testing and define the test case
enable_testing()
# execute test
add_test(NAME ${test_rexx}
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm ${test_rexx} rx_${plugin_name} ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
