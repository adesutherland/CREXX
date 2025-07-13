cmake_minimum_required(VERSION 3.24)
project(odbc_plugin C)

set(CMAKE_C_STANDARD 99)

# Find ODBC libraries
if(WIN32)
    find_library(ODBC32_LIBRARY odbc32)
    find_library(ODBCCP32_LIBRARY odbccp32)
    if(NOT ODBC32_LIBRARY OR NOT ODBCCP32_LIBRARY)
        message(FATAL_ERROR "ODBC libraries not found on Windows")
    endif()
    set(ODBC_LIBS ${ODBC32_LIBRARY} ${ODBCCP32_LIBRARY})
else()
    find_package(ODBC REQUIRED)
    find_path(ODBC_INCLUDE_DIR sql.h PATHS /usr/include /usr/local/include /opt/homebrew/include)
    find_library(ODBC_LIBRARY odbc PATHS /usr/lib /usr/local/lib /opt/homebrew/lib)
    if(NOT ODBC_INCLUDE_DIR OR NOT ODBC_LIBRARY)
        message(FATAL_ERROR "ODBC headers (sql.h) or library (libodbc) not found! Install unixODBC.")
    endif()
    set(ODBC_LIBS ${ODBC_LIBRARY})
endif()

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create the static link module
#add_static_plugin_target(_odbc odbc.c)

# Create dynamic plugin module
add_dynamic_plugin_target(_odbc odbc.c)

# Add include directories
target_include_directories(_odbc PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/include
    ${CMAKE_SOURCE_DIR}/rxpa
    ${CMAKE_SOURCE_DIR}
    ${ODBC_INCLUDE_DIR}  # Ensure ODBC headers are included
)

# Link libraries
target_link_libraries(_odbc PRIVATE ${ODBC_LIBS})

# Set output name
set_target_properties(_odbc PROPERTIES
    OUTPUT_NAME "_odbc"
    SUFFIX ".rxplugin"
)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o odbc_test \"${CMAKE_CURRENT_SOURCE_DIR}/odbc_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas odbc_test

        DEPENDS rxas rxc library _odbc ${CMAKE_CURRENT_SOURCE_DIR}/odbc_test.rexx
        OUTPUT odbc_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(odbc_test ALL
        DEPENDS odbc_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run odbc_test.rxbin with the odbc plugin
add_test(NAME odbc_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm odbc_test rx_odbc ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

