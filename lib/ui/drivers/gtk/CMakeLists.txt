cmake_minimum_required(VERSION 3.24)

project(crexx_ui_gtk_driver C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
set(_ui_library_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
if(MSVC)
    # MSYS2 supplies both GNU static archives and Windows DLL import archives.
    # A native MSVC consumer uses the DLL C ABI, not the GNU C++ static objects.
    set(CMAKE_FIND_LIBRARY_SUFFIXES .dll.a .lib .a)
endif()
pkg_check_modules(CREXX_UI_GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
set(CMAKE_FIND_LIBRARY_SUFFIXES ${_ui_library_suffixes})

include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(_ui_gtk_native ui_gtk.c)
add_static_plugin_target(_ui_gtk_native ui_gtk.c)
foreach(target IN ITEMS _ui_gtk_native _ui_gtk_native_static)
    target_link_libraries(${target} PRIVATE PkgConfig::CREXX_UI_GTK3)
    # A GNU-built GTK SDK includes its runtime headers in pkg-config's root
    # include directory. Keep the selected MSVC SDK first when consuming that
    # C ABI from a Visual Studio developer environment.
    if(MSVC AND DEFINED ENV{INCLUDE})
        target_include_directories(${target} BEFORE PRIVATE $ENV{INCLUDE})
    endif()
endforeach()
set(CREXX_UI_GTK_RUNTIME_SEARCH_DIRECTORIES "" CACHE STRING
        "Build-time GTK runtime DLL search directories (not installed search paths)")
set(_ui_gtk_runtime_directories ${CREXX_UI_GTK_RUNTIME_SEARCH_DIRECTORIES})
if(WIN32 AND CREXX_UI_GTK3_PREFIX)
    list(APPEND _ui_gtk_runtime_directories "${CREXX_UI_GTK3_PREFIX}/bin")
endif()
set(_ui_gtk_link_files ${CREXX_UI_GTK3_LINK_LIBRARIES})
set(_ui_gtk_runtime_files)
if(WIN32 AND CREXX_UI_GTK3_PREFIX)
    file(GLOB _ui_gtk_runtime_files CONFIGURE_DEPENDS
            "${CREXX_UI_GTK3_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders/*.dll")
    set(_ui_gtk_data "${CMAKE_BINARY_DIR}/bin/providers/rx_ui_gtk_native.data")
    add_custom_command(OUTPUT "${_ui_gtk_data}/complete.stamp"
            COMMAND ${CMAKE_COMMAND} "-DPREFIX=${CREXX_UI_GTK3_PREFIX}"
                    "-DOUTPUT=${_ui_gtk_data}"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/StageWindowsGtkData.cmake"
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/StageWindowsGtkData.cmake"
                    "${CREXX_UI_GTK3_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache"
                    "${CREXX_UI_GTK3_PREFIX}/share/glib-2.0/schemas/gschemas.compiled"
                    ${_ui_gtk_runtime_files}
            VERBATIM COMMENT "Staging portable GTK data and relative loader cache")
    add_custom_target(_ui_gtk_native_data DEPENDS "${_ui_gtk_data}/complete.stamp")
endif()
if(MSVC)
    # The native compiler SDK supplies Windows system import libraries.
    list(FILTER _ui_gtk_link_files INCLUDE REGEX "\\.(dll\\.a|lib)$")
endif()
add_rxpa_provider_package(_ui_gtk_native
        LINK_FILES ${_ui_gtk_link_files}
        RUNTIME_TARGETS _ui_gtk_native
        RUNTIME_FILES ${_ui_gtk_runtime_files}
        RUNTIME_SEARCH_DIRECTORIES ${_ui_gtk_runtime_directories}
        ENGINE_ID "GTK-${CREXX_UI_GTK3_VERSION}")
if(TARGET _ui_gtk_native_data)
    add_dependencies(_ui_gtk_native_runtime_package _ui_gtk_native_data)
endif()

if(BUILD_TESTING)
    add_dynamic_plugin_target(_ui_gtk_test ui_gtk_test.c)
    add_static_plugin_target(_ui_gtk_test ui_gtk_test.c)
    foreach(target IN ITEMS _ui_gtk_test _ui_gtk_test_static)
        target_link_libraries(${target} PRIVATE PkgConfig::CREXX_UI_GTK3)
        if(MSVC AND DEFINED ENV{INCLUDE})
            target_include_directories(${target} BEFORE PRIVATE $ENV{INCLUDE})
        endif()
    endforeach()
    add_rxpa_provider_package(_ui_gtk_test)
endif()
