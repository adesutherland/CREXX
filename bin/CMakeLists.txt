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
                "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
                "${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin"
                "${CMAKE_BINARY_DIR}/bin/rx_hash.rxplugin"
        DEPENDS
                library
                classlib
                classlib_native
                rxfnsc
                rexxscript
                compiler_exit_bin
                fs
                _hash
        COMMENT "Staging deterministic crexx driver imports ...")

set(_crexx_build_worker_root
        "${CMAKE_CURRENT_BINARY_DIR}/crexx-build-worker")
set(_crexx_build_worker_source
        "${_crexx_build_worker_root}/source/crexx_build_worker.crexx")
set(_crexx_build_worker_rxbin
        "${_crexx_build_worker_root}/crexx_build_worker.rxbin")
set(_crexx_build_worker_import_root
        "${_crexx_build_worker_root}/controller-imports")
set(_crexx_build_worker_import
        "${_crexx_build_worker_import_root}/crexx_build_worker.rxbin")
add_custom_command(
        OUTPUT
                ${_crexx_build_worker_rxbin}
                ${_crexx_build_worker_import}
        BYPRODUCTS
                ${_crexx_build_worker_source}
                ${_crexx_build_worker_root}/crexx_build_worker.rxas
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${_crexx_build_worker_root}/source"
                "${_crexx_build_worker_import_root}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/crexx_build_worker.crexx"
                "${_crexx_build_worker_source}"
        COMMAND $<TARGET_FILE:rxc>
                --no-exe-import
                -i "${CREXX_DRIVER_IMPORT_ROOT}"
                -o crexx_build_worker
                "${_crexx_build_worker_source}"
        COMMAND $<TARGET_FILE:rxas>
                -o crexx_build_worker crexx_build_worker
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_crexx_build_worker_rxbin}"
                "${_crexx_build_worker_import}"
        DEPENDS
                rxc
                rxas
                crexx_driver_imports
                ${CREXX_DRIVER_IMPORT_FILES}
                crexx_build_worker.crexx
        WORKING_DIRECTORY "${_crexx_build_worker_root}"
        COMMENT "Bootstrapping the installed crexx project-build worker ..."
        VERBATIM)
add_custom_target(crexx_build_worker
        DEPENDS ${_crexx_build_worker_rxbin} ${_crexx_build_worker_import})

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
                -i "${_crexx_build_worker_import_root}"
                -o crexx
                "${_crexx_driver_source}"
        COMMAND $<TARGET_FILE:rxas> crexx
        COMMAND $<TARGET_FILE:rxlink> -s -o crexx_linked
                crexx
                "${_crexx_build_worker_rxbin}"
                "${CMAKE_BINARY_DIR}/bin/library"
                "${CMAKE_BINARY_DIR}/bin/classlib"
        COMMAND $<TARGET_FILE:rxcpack> -o crexx crexx_linked
        DEPENDS
                rxas
                rxc
                rxlink
                rxcpack
                rxpa
                fs
                fs_static
                _hash
                _hash_static
                crexx_driver_imports
                crexx_build_worker
                ${CREXX_DRIVER_IMPORT_FILES}
                ${_crexx_build_worker_rxbin}
                ${_crexx_build_worker_import}
                crexx.crexx
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/crexx.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble, link and package crexx ..."
        VERBATIM
)

add_executable(crexx ${CMAKE_CURRENT_BINARY_DIR}/crexx.c)

configure_linker_for_static_lib(crexx fs)
configure_linker_for_static_lib(crexx _hash)

target_link_libraries(crexx rxvml rxpashim)

add_dependencies(crexx rxvme rxvml rxpa avl_tree rxbin platform
        fs fs_static _hash _hash_static float float_static
        _treemap _treemap_static
        _llist _llist_static
        _keyaccess _keyaccess_static)

add_subdirectory(tests)
