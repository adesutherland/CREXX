# Minimum CMake version requirement
cmake_minimum_required(VERSION 3.24)

# Project name declaration (optional for subdirectories)
project(CREXX_PLUGIN_ARCHITECTURE)

find_package(Threads REQUIRED)

# Build library for the RX Plugin Architecture
add_library(rxpa STATIC rxpa.c rxpa.h crexxpa.h)
target_link_libraries(rxpa PUBLIC Threads::Threads)

# Public header-only development surface for external dynamic RXPA plugins.
# Dynamic plugins receive their callbacks from the VM at load time and do not
# link the internal rxpa archive.
add_library(crexx_rxpa_sdk INTERFACE)
add_library(CREXX::RXPA ALIAS crexx_rxpa_sdk)
set_target_properties(crexx_rxpa_sdk PROPERTIES EXPORT_NAME RXPA)
target_include_directories(crexx_rxpa_sdk INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/rxpa>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/rxpa>)

install(TARGETS crexx_rxpa_sdk EXPORT CREXXTargets)
install(FILES crexxpa.h DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rxpa")
install(FILES "${CMAKE_SOURCE_DIR}/platform/rxinteger.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/platform")
# Keep direct `-I<prefix>/include/rxpa` consumers self-contained while the
# imported target also exposes the canonical top-level generated header.
install(FILES "${CMAKE_BINARY_DIR}/generated/crexx_version.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/rxpa")

# rxpa is linked into shared objects such as libcrexxsaa on ELF platforms.
set_target_properties(rxpa PROPERTIES POSITION_INDEPENDENT_CODE ON)
