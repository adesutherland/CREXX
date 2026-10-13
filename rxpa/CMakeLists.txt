# Minimum CMake version requirement
cmake_minimum_required(VERSION 3.24)

# Project name declaration (optional for subdirectories)
project(CREXX_PLUGIN_ARCHITECTURE)

find_package(Threads REQUIRED)

# The native driver consumes generic .native.json packages even without llama.
# Reuse the existing shared validator/copier unchanged; its JSON header is the
# pinned MIT-licensed single-header dependency retained in this core source.
add_executable(crexx-provider-package
        "${CMAKE_SOURCE_DIR}/lib/plugins/llama/provider_package.cpp"
        "${CMAKE_SOURCE_DIR}/binutils/rxsha256.c")
target_compile_features(crexx-provider-package PRIVATE cxx_std_17)
# This standalone C++ tool needs no generated cREXX headers. On case-insensitive
# filesystems generated/VERSION otherwise shadows the standard <version> header.
set_target_properties(crexx-provider-package PROPERTIES INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/vendor;${CMAKE_SOURCE_DIR}/binutils/include")
target_compile_definitions(crexx-provider-package PRIVATE
        CREXX_LLAMA_PLATFORM="${CMAKE_SYSTEM_NAME}"
        CREXX_LLAMA_ARCH="${CMAKE_SYSTEM_PROCESSOR}")
if(WIN32)
    target_link_libraries(crexx-provider-package PRIVATE shell32)
endif()
set_target_properties(crexx-provider-package PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
install(TARGETS crexx-provider-package RUNTIME DESTINATION bin)
install(FILES vendor/nlohmann/LICENSE vendor/nlohmann/provenance.json
        DESTINATION share/crexx/licenses/nlohmann-json)

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
