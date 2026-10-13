cmake_minimum_required(VERSION 3.24)
project(provider_package_copy_control C)
include(CTest)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
get_filename_component(CREXX_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
if(NOT DEFINED CREXX_PACKAGE_HELPER_DIR)
    set(CREXX_PACKAGE_HELPER_DIR "${CREXX_SOURCE}/rxpa")
endif()
include("${CREXX_PACKAGE_HELPER_DIR}/RXPluginFunction.cmake")
add_library(copy_dependency SHARED dependency.c)
set_target_properties(copy_dependency PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS TRUE OUTPUT_NAME copy-dependency
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/a"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/a")
if(APPLE)
    set(origin "@loader_path")
    set_target_properties(copy_dependency PROPERTIES INSTALL_NAME_DIR "@rpath")
else()
    set(origin "$ORIGIN")
endif()
foreach(name a b)
    if(name STREQUAL "a")
        add_library(copy_root_${name} MODULE root.c)
        set_target_properties(copy_root_${name} PROPERTIES SUFFIX ".rxplugin")
    else()
        add_library(copy_root_${name} SHARED root.c)
    endif()
    target_link_libraries(copy_root_${name} PRIVATE copy_dependency)
    set_target_properties(copy_root_${name} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${name}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${name}"
        BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${origin}")
endforeach()
set_target_properties(copy_dependency PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
add_custom_command(TARGET copy_root_b POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_if_different "$<TARGET_FILE:copy_dependency>"
    "$<TARGET_FILE_DIR:copy_root_b>/$<TARGET_FILE_NAME:copy_dependency>")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/input-$<CONFIG>.cmake" CONTENT
"set(platform \"${CMAKE_SYSTEM_NAME}\")\nset(provider copy_control)\nset(arch \"${CMAKE_SYSTEM_PROCESSOR}\")\nset(engine fixture)\nset(runtime_files \"$<TARGET_FILE:copy_root_a>\" \"$<TARGET_FILE:copy_root_b>\" \"$<TARGET_FILE:copy_dependency>\")\n")
add_test(NAME provider_package_copy COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/check.py"
    "${CMAKE_BINARY_DIR}/input-$<CONFIG>.cmake"
    "${CREXX_PACKAGE_HELPER_DIR}/WriteProviderPackage.cmake"
    "${CMAKE_BINARY_DIR}/proof")
set_tests_properties(provider_package_copy PROPERTIES RUN_SERIAL TRUE TIMEOUT 300)

# Exercise the public package helper with both target-owned and concrete linker
# files. An INTERFACE SDK target cannot supply TARGET_LINKER_FILE itself.
add_library(copy_link_archive STATIC dependency.c)
add_dynamic_plugin_target(copy_provider root.c)
add_static_plugin_target(copy_provider root.c)
foreach(target copy_provider copy_provider_static)
    target_link_libraries(${target} PRIVATE copy_dependency)
    set_target_properties(${target} PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${origin}")
endforeach()
add_rxpa_provider_package(copy_provider
    LINK_TARGETS copy_link_archive
    LINK_FILES "$<TARGET_LINKER_FILE:copy_dependency>"
    RUNTIME_TARGETS copy_provider
    RUNTIME_SEARCH_DIRECTORIES "$<TARGET_FILE_DIR:copy_dependency>")
add_test(NAME provider_package_file_links
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/check-file-links.py"
        "${CMAKE_BINARY_DIR}/bin/providers/rxcopy_provider.native.json"
        "$<TARGET_LINKER_FILE:copy_link_archive>"
        "$<TARGET_LINKER_FILE:copy_dependency>"
        "$<TARGET_FILE:copy_provider>"
        "$<TARGET_FILE:copy_dependency>"
        "${CMAKE_BINARY_DIR}/bin/providers/rxcopy_provider${CMAKE_STATIC_LIBRARY_SUFFIX}")
set_tests_properties(provider_package_file_links PROPERTIES TIMEOUT 300)

# A partial flat package declares one DLL, while another transitive DLL is
# initially found in the SDK. Both routes encounter identical dependency bytes
# under different source paths, as in a GTK DLL closure on Windows.
add_library(copy_branch SHARED branch.c)
target_link_libraries(copy_branch PRIVATE copy_dependency)
set_target_properties(copy_branch PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/a"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/a"
    BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${origin}")
if(APPLE)
    set_target_properties(copy_branch PROPERTIES INSTALL_NAME_DIR "@rpath")
endif()
add_library(copy_root_partial MODULE root.c)
target_compile_definitions(copy_root_partial PRIVATE PACKAGE_TRANSITIVE_ROOT)
target_link_libraries(copy_root_partial PRIVATE copy_dependency copy_branch)
set_target_properties(copy_root_partial PROPERTIES SUFFIX ".rxplugin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/partial"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/partial"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "${origin};$<TARGET_FILE_DIR:copy_branch>")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/partial-$<CONFIG>.cmake" CONTENT
"set(platform \"${CMAKE_SYSTEM_NAME}\")\nset(provider partial_control)\nset(arch \"${CMAKE_SYSTEM_PROCESSOR}\")\nset(engine fixture)\nset(runtime_files \"$<TARGET_FILE:copy_root_partial>\" \"$<TARGET_FILE:copy_dependency>\")\nset(runtime_search_directories \"$<TARGET_FILE_DIR:copy_branch>\")\nset(branch \"$<TARGET_FILE:copy_branch>\")\n")
add_custom_target(provider_package_partial_measure
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/check-partial.py"
        "${CMAKE_BINARY_DIR}/partial-$<CONFIG>.cmake"
        "${CREXX_PACKAGE_HELPER_DIR}/WriteProviderPackage.cmake"
        "${CMAKE_BINARY_DIR}/partial-proof"
    DEPENDS copy_root_partial copy_dependency copy_branch
    VERBATIM)
# Measured before registration: Debug 0.178 s, Apple-ASan flags 0.207 s.
add_test(NAME provider_package_partial
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/check-partial.py"
        "${CMAKE_BINARY_DIR}/partial-$<CONFIG>.cmake"
        "${CREXX_PACKAGE_HELPER_DIR}/WriteProviderPackage.cmake"
        "${CMAKE_BINARY_DIR}/partial-ctest-proof")
set_tests_properties(provider_package_partial PROPERTIES RUN_SERIAL TRUE TIMEOUT 300)
