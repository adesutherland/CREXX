cmake_minimum_required(VERSION 3.24)

project(driver C)

set(CMAKE_C_STANDARD 90)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
get_filename_component(CREXX_NATIVE_C_COMPILER "${CMAKE_C_COMPILER}" NAME)
if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    set(CREXX_NATIVE_COMPILER_STYLE "MSVC")
else()
    set(CREXX_NATIVE_COMPILER_STYLE "GNU")
endif()
file(WRITE ${CMAKE_BINARY_DIR}/bin/crexx_native_compiler "${CREXX_NATIVE_C_COMPILER}\n")
file(WRITE ${CMAKE_BINARY_DIR}/bin/crexx_native_compiler_style "${CREXX_NATIVE_COMPILER_STYLE}\n")
set(CREXX_NATIVE_C_FLAGS_TEXT "${CMAKE_C_FLAGS}")
set(CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT "${CMAKE_EXE_LINKER_FLAGS}")
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" CREXX_NATIVE_BUILD_TYPE)
    string(APPEND CREXX_NATIVE_C_FLAGS_TEXT " ${CMAKE_C_FLAGS_${CREXX_NATIVE_BUILD_TYPE}}")
    string(APPEND CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT " ${CMAKE_EXE_LINKER_FLAGS_${CREXX_NATIVE_BUILD_TYPE}}")
endif()
if(CREXX_NATIVE_COMPILER_STYLE STREQUAL "MSVC")
    set(CREXX_NATIVE_MSVC_RUNTIME_FLAG "/MD")
    if(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreaded" AND
       NOT CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "DLL")
        set(CREXX_NATIVE_MSVC_RUNTIME_FLAG "/MT")
    endif()
    if(CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug$" AND
       (NOT CMAKE_MSVC_RUNTIME_LIBRARY OR CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "Debug"))
        string(APPEND CREXX_NATIVE_MSVC_RUNTIME_FLAG "d")
    endif()
    string(APPEND CREXX_NATIVE_C_FLAGS_TEXT " ${CREXX_NATIVE_MSVC_RUNTIME_FLAG}")
endif()
string(STRIP "${CREXX_NATIVE_C_FLAGS_TEXT}" CREXX_NATIVE_C_FLAGS_TEXT)
string(STRIP "${CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT}" CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT)
file(WRITE ${CMAKE_BINARY_DIR}/bin/crexx_native_cflags "${CREXX_NATIVE_C_FLAGS_TEXT}\n")
file(WRITE ${CMAKE_BINARY_DIR}/bin/crexx_native_ldflags "${CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT}\n")

function(crexx_write_native_arg_file output_path)
    file(WRITE "${output_path}" "")
    foreach(_arg IN LISTS ARGN)
        file(APPEND "${output_path}" "${_arg}\n")
    endforeach()
endfunction()

separate_arguments(CREXX_NATIVE_C_FLAGS_ARGS NATIVE_COMMAND "${CREXX_NATIVE_C_FLAGS_TEXT}")
separate_arguments(CREXX_NATIVE_EXE_LINKER_FLAGS_ARGS NATIVE_COMMAND "${CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT}")
set(CREXX_NATIVE_LINK_FLAGS_ARGS ${CREXX_NATIVE_RUNTIME_LINK_FLAGS_ARGS})
if(CREXX_NATIVE_COMPILER_STYLE STREQUAL "MSVC")
    separate_arguments(CREXX_NATIVE_STANDARD_LIBRARIES_ARGS NATIVE_COMMAND "${CMAKE_C_STANDARD_LIBRARIES}")
    list(APPEND CREXX_NATIVE_LINK_FLAGS_ARGS psapi.lib ${CREXX_NATIVE_STANDARD_LIBRARIES_ARGS})
endif()
string(JOIN " " CREXX_NATIVE_LINK_FLAGS_TEXT ${CREXX_NATIVE_LINK_FLAGS_ARGS})
file(WRITE ${CMAKE_BINARY_DIR}/bin/crexx_native_libs "${CREXX_NATIVE_LINK_FLAGS_TEXT}\n")
crexx_write_native_arg_file(
        ${CMAKE_BINARY_DIR}/bin/crexx_native_libs_argv
        ${CREXX_NATIVE_LINK_FLAGS_ARGS})
crexx_write_native_arg_file(
        ${CMAKE_BINARY_DIR}/bin/crexx_native_cflags_argv
        ${CREXX_NATIVE_C_FLAGS_ARGS})
crexx_write_native_arg_file(
        ${CMAKE_BINARY_DIR}/bin/crexx_native_ldflags_argv
        ${CREXX_NATIVE_EXE_LINKER_FLAGS_ARGS})

set(CREXX_DRIVER_IMPORT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/imports")
crexx_add_import_root(crexx_driver_imports
        DIRECTORY "${CREXX_DRIVER_IMPORT_ROOT}"
        OUTPUT_VARIABLE CREXX_DRIVER_IMPORT_FILES
        FILES
                "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                "${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin"
        DEPENDS
                library
                classlib_native
                rxfnsc
                rexxscript
                compiler_exit_bin
                fs
        COMMENT "Staging deterministic crexx driver imports ...")

set(_crexx_driver_source_root "${CMAKE_CURRENT_BINARY_DIR}/sources/crexx")
set(_crexx_driver_source "${_crexx_driver_source_root}/crexx.crexx")
add_custom_command(
        BYPRODUCTS
                ${CMAKE_CURRENT_BINARY_DIR}/crexx.rxbin
                ${CMAKE_CURRENT_BINARY_DIR}/crexx_linked.rxbin
                ${_crexx_driver_source}
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${_crexx_driver_source_root}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/crexx.crexx"
                "${_crexx_driver_source}"
        COMMAND $<TARGET_FILE:rxc>
                --no-exe-import
                -i "${CREXX_DRIVER_IMPORT_ROOT}"
                -o crexx
                "${_crexx_driver_source}"
        COMMAND $<TARGET_FILE:rxas> crexx
        COMMAND $<TARGET_FILE:rxlink> -s -o crexx_linked crexx ${CMAKE_BINARY_DIR}/bin/library
        COMMAND $<TARGET_FILE:rxcpack> -o crexx crexx_linked
        DEPENDS
                rxas
                rxc
                rxlink
                rxcpack
                rxpa
                fs
                fs_static
                crexx_driver_imports
                ${CREXX_DRIVER_IMPORT_FILES}
                crexx.crexx
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/crexx.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble, link and package crexx ..."
        VERBATIM
)

add_executable(crexx ${CMAKE_CURRENT_BINARY_DIR}/crexx.c)

configure_linker_for_static_lib(crexx fs)

target_link_libraries(crexx rxvml rxpashim)

add_dependencies(crexx rxvme rxvml rxpa avl_tree rxbin platform
        fs fs_static _hash _hash_static float float_static
        _treemap _treemap_static
        _llist _llist_static
        _keyaccess _keyaccess_static)

add_subdirectory(tests)
