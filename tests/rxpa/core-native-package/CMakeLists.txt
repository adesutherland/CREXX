cmake_minimum_required(VERSION 3.24)
project(core_native_package_fixture C)
if(NOT DEFINED CREXX_SOURCE OR NOT DEFINED CREXX_GENERATED)
    message(FATAL_ERROR "Fixture requires matching core source/generated headers")
endif()
include("${CREXX_SOURCE}/rxpa/RXPluginFunction.cmake")
add_library(core_dependency SHARED dependency.c)
set_target_properties(core_dependency PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dependency"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dependency")
if(APPLE)
    set(origin "@loader_path")
    set_target_properties(core_dependency PROPERTIES INSTALL_NAME_DIR "@rpath")
else()
    set(origin "$ORIGIN")
endif()
set_target_properties(core_dependency PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
add_dynamic_plugin_target(core_package provider.c)
add_static_plugin_target(core_package provider.c)
foreach(target core_package core_package_static)
    target_include_directories(${target} PRIVATE "${CREXX_SOURCE}/rxpa" "${CREXX_GENERATED}")
    target_link_libraries(${target} PRIVATE core_dependency)
    set_target_properties(${target} PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${origin}")
endforeach()
add_rxpa_provider_package(core_package
    LINK_TARGETS core_dependency RUNTIME_TARGETS core_dependency
    ENGINE_ID core-fixture)
