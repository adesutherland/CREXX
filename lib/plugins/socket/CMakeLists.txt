cmake_minimum_required(VERSION 3.24)
project(strings_plugin C)

set(CMAKE_C_STANDARD 99)
if(WIN32)
    link_libraries(ws2_32 wsock32)
endif()
# -------------------------------------------------------------------------------------------
# Set plugin_name and test_rexx to match the specific plugin and its corresponding test case.
# -------------------------------------------------------------------------------------------
set(plugin_name "socket")       # add plugin name
set(test_rexx "socket_test")    # add rexx to test plugin
set(use_precomp FALSE)           # TRUE pre compile needed, FALSE if not

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
# Create the static link module
#add_static_plugin_target(_socket socket.c)

# Add include directories - static version
# target_include_directories(_socket_static PRIVATE 
#     ${CMAKE_SOURCE_DIR}/include
#     ${CMAKE_SOURCE_DIR}/lib/include
#     ${CMAKE_SOURCE_DIR}/rxpa
# )
