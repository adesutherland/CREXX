cmake_minimum_required(VERSION 3.24)
project(keyaccess_plugin C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_keyaccess keyaccess.c)

# Create static plugin module
add_static_plugin_target(_keyaccess keyaccess.c)

add_custom_target(keyaccess_test ALL)
crexx_add_rexx_opt_matrix(
        PASS_TEST_NAME_ARGUMENT
        NAME keyaccess_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/keyaccess_test.crexx
        TARGET_GROUP keyaccess_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _keyaccess
        RUNTIME_ARGS rx_keyaccess ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()
add_executable(keyaccess_compact_failure_test keyaccess_compact_failure_test.c)
set_target_properties(keyaccess_compact_failure_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(keyaccess_compact_failure_test PRIVATE
        ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_BINARY_DIR}/generated)
add_test(NAME keyaccess_compact_failure COMMAND keyaccess_compact_failure_test)
# Compaction currently uses shared temporary names within its working directory.
set_tests_properties(keyaccess_test_noopt keyaccess_test_opt keyaccess_compact_failure
        PROPERTIES RESOURCE_LOCK keyaccess_compaction TIMEOUT 300)
crexx_label_smoke_tests(keyaccess_test_noopt)
crexx_label_smoke_tests(keyaccess_compact_failure PREP_TARGETS keyaccess_compact_failure_test)
