cmake_minimum_required(VERSION 3.24)
project(strings_plugin C)

set(CMAKE_C_STANDARD 99)
if(WIN32)
    link_libraries(ws2_32 wsock32)
endif()

# -------------------------------------------------------------------------------------------
# Plugin configuration
# -------------------------------------------------------------------------------------------
set(plugin_name "socket")       # plugin name
set(test_rexx "socket_test")    # Rexx test name
set(use_precomp FALSE)          # TRUE = precompile needed, FALSE otherwise

# ===========================================================================================
# Generic plugin setup (defines the target)
# ===========================================================================================
include(${CMAKE_SOURCE_DIR}/rxpa/CMakePluginTemplate.txt)
plugin_template(${plugin_name} ${test_rexx} ${PRECOMP_ARG} ${use_precomp})

# -------------------------------------------------------------------------------------------
# Link OpenSSL to the plugin (after target exists)
# -------------------------------------------------------------------------------------------

option(CREXX_FORCE_SYSTEM_OPENSSL
       "Force use of system OpenSSL without local override"
       OFF)

if(NOT CREXX_FORCE_SYSTEM_OPENSSL)
set(OPENSSL_ROOT_DIR "C:/msys64/mingw64")
endif()

set(OPENSSL_USE_STATIC_LIBS FALSE)
find_package(OpenSSL REQUIRED)

if(OPENSSL_FOUND)
    message(STATUS "Found OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "Found OpenSSL libraries: ${OPENSSL_LIBRARIES}")
    include_directories(${OPENSSL_INCLUDE_DIR})
    # NOTE: the plugin target is probably named "_socket"
    target_link_libraries(_${plugin_name} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
else()
    message(FATAL_ERROR "OpenSSL not found! Please install mingw-w64-x86_64-openssl via pacman.")
endif()

# -------------------------------------------------------------------------------------------
# Enable testing
# -------------------------------------------------------------------------------------------
enable_testing()
add_test(NAME ${test_rexx}
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm ${test_rexx} rx_${plugin_name} ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
