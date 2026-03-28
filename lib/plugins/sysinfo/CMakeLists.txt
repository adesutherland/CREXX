cmake_minimum_required(VERSION 3.24)
project(sysinfo_plugin C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_sysinfo sysinfo.c)

# Create the static link module
add_static_plugin_target(_sysinfo sysinfo.c)

add_custom_target(sysinfo_test ALL)
crexx_add_rexx_opt_matrix(
        NAME sysinfo_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/sysinfo_test.rexx
        TARGET_GROUP sysinfo_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _sysinfo
        RUNTIME_ARGS rx_sysinfo ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()

# add_custom_target(sysinfocopy ALL DEPENDS rx_sysinfo_static.a)

# add_custom_command(TARGET sysinfocopy
#   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib/plugins/sysinfo/rx_sysinfo_static.a ${CMAKE_BINARY_DIR}/bin/
#   )
