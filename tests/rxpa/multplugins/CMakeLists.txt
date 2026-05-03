cmake_minimum_required(VERSION 3.24)
project(rxpatest C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPlugin Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

function(localize_test_plugin_output target)
    # Keep test-only plugins out of the shared release bin directory.
    set_target_properties(${target} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endfunction()

# Create dynamic plugin 1
add_dynamic_plugin_target(_plugin1 plugin1.c)
localize_test_plugin_output(_plugin1)
add_dependencies(_plugin1 rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

# Create dynamic plugin 2
add_dynamic_plugin_target(_plugin2 plugin2.c)
localize_test_plugin_output(_plugin2)
add_dependencies(_plugin2 rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

add_custom_target(rxpa_mult_test ALL)
crexx_add_rexx_opt_matrix(
        NAME rxpa_mult_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_mult_test.rexx
        TARGET_GROUP rxpa_mult_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _plugin1 _plugin2
        RUNTIME_ARGS rx_plugin1 rx_plugin2 ${CMAKE_BINARY_DIR}/bin/library
)

# Enable testing functionality
enable_testing()

# Set the property to check for expected output
set_tests_properties(rxpa_mult_test_noopt rxpa_mult_test_opt PROPERTIES
        PASS_REGULAR_EXPRESSION "plugin1[^\\1]*plugin2"
)
