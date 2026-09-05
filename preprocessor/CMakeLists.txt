cmake_minimum_required(VERSION 3.24)
project(preprocessor C)

set(CMAKE_C_STANDARD 99)

set(RXPP_SUPPORT_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/maclib.rexx
        ${CMAKE_CURRENT_SOURCE_DIR}/macsys.rexx
        ${CMAKE_CURRENT_SOURCE_DIR}/mathlib.rexx
        ${CMAKE_CURRENT_SOURCE_DIR}/syslib.rexx)
set(RXPP_STAGED_SUPPORT_FILES
        ${CMAKE_BINARY_DIR}/bin/maclib.rexx
        ${CMAKE_BINARY_DIR}/bin/macsys.rexx
        ${CMAKE_BINARY_DIR}/bin/mathlib.rexx
        ${CMAKE_BINARY_DIR}/bin/syslib.rexx)

add_custom_command(
        OUTPUT ${RXPP_STAGED_SUPPORT_FILES}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RXPP_SUPPORT_FILES} ${CMAKE_BINARY_DIR}/bin
        DEPENDS ${RXPP_SUPPORT_FILES}
        COMMENT "Stage rxpp macro support files ...")
add_custom_target(rxpp_support_files ALL DEPENDS ${RXPP_STAGED_SUPPORT_FILES})
install(FILES ${RXPP_SUPPORT_FILES} DESTINATION bin)

set(RXFNSB_RXBIN ${CMAKE_BINARY_DIR}/bin/library.rxbin)
set(CLASSLIB_RXBIN ${CMAKE_BINARY_DIR}/bin/classlib.rxbin)
set(CLASSLIB_NATIVE_RXBIN ${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin)
set(RXFNSC_RXBIN ${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin)
set(REXXSCRIPT_RXBIN ${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin)

# Include RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create the static link module
add_static_plugin_target(precomp precomp.c)

# Create the dynamic link module - this is needed for rxc to compile rxpp.crexx
add_dynamic_plugin_target(precomp precomp.c)

set(RXPP_IMPORT_ROOT ${CMAKE_CURRENT_BINARY_DIR}/imports/compiler)
set(RXPP_IMPORT_RXFNSB ${RXPP_IMPORT_ROOT}/library.rxbin)
set(RXPP_IMPORT_CLASSLIB ${RXPP_IMPORT_ROOT}/classlib.rxbin)
set(RXPP_IMPORT_CLASSLIB_NATIVE ${RXPP_IMPORT_ROOT}/classlib_native.rxbin)
set(RXPP_IMPORT_RXFNSC ${RXPP_IMPORT_ROOT}/rxfnsc.rxbin)
set(RXPP_IMPORT_REXXSCRIPT ${RXPP_IMPORT_ROOT}/rexxscript.rxbin)
set(RXPP_IMPORT_EXITS ${RXPP_IMPORT_ROOT}/rxcexits.rxbin)
set(RXPP_IMPORT_PRECOMP ${RXPP_IMPORT_ROOT}/rxprecomp.rxplugin)
add_custom_command(
        OUTPUT
                ${RXPP_IMPORT_RXFNSB}
                ${RXPP_IMPORT_CLASSLIB}
                ${RXPP_IMPORT_CLASSLIB_NATIVE}
                ${RXPP_IMPORT_RXFNSC}
                ${RXPP_IMPORT_REXXSCRIPT}
                ${RXPP_IMPORT_EXITS}
                ${RXPP_IMPORT_PRECOMP}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RXPP_IMPORT_ROOT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RXFNSB_RXBIN} ${RXPP_IMPORT_RXFNSB}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CLASSLIB_RXBIN} ${RXPP_IMPORT_CLASSLIB}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CLASSLIB_NATIVE_RXBIN} ${RXPP_IMPORT_CLASSLIB_NATIVE}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RXFNSC_RXBIN} ${RXPP_IMPORT_RXFNSC}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${REXXSCRIPT_RXBIN} ${RXPP_IMPORT_REXXSCRIPT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${RXPP_IMPORT_EXITS}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:precomp> ${RXPP_IMPORT_PRECOMP}
        DEPENDS
                precomp
                library classlib classlib_native rxfnsc rexxscript compiler_exit_bin
                ${RXFNSB_RXBIN}
                ${CLASSLIB_RXBIN}
                ${CLASSLIB_NATIVE_RXBIN}
                ${RXFNSC_RXBIN}
                ${REXXSCRIPT_RXBIN}
                ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
        COMMENT "Stage immutable rxpp compiler imports ..."
        VERBATIM)
add_custom_target(rxpp_compiler_imports
        DEPENDS
                ${RXPP_IMPORT_RXFNSB}
                ${RXPP_IMPORT_CLASSLIB}
                ${RXPP_IMPORT_CLASSLIB_NATIVE}
                ${RXPP_IMPORT_RXFNSC}
                ${RXPP_IMPORT_REXXSCRIPT}
                ${RXPP_IMPORT_EXITS}
                ${RXPP_IMPORT_PRECOMP})

set(RXPP_MEMBER_ROOT ${CMAKE_CURRENT_BINARY_DIR}/members/rxpp)
set(RXPP_MEMBER_SOURCE_ROOT ${RXPP_MEMBER_ROOT}/source)
set(RXPP_MEMBER_SOURCE ${RXPP_MEMBER_SOURCE_ROOT}/rxpp.crexx)
set(RXPP_MEMBER_RXAS ${RXPP_MEMBER_ROOT}/rxpp.rxas)
set(RXPP_MEMBER_RXBIN ${RXPP_MEMBER_ROOT}/rxpp.rxbin)
add_custom_command(
        OUTPUT ${RXPP_MEMBER_RXBIN}
        BYPRODUCTS ${RXPP_MEMBER_SOURCE} ${RXPP_MEMBER_RXAS}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RXPP_MEMBER_SOURCE_ROOT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/rxpp.crexx ${RXPP_MEMBER_SOURCE}
        COMMAND ${CMAKE_COMMAND} -E chdir ${RXPP_MEMBER_ROOT}
                $<TARGET_FILE:rxc>
                --no-exe-import
                -i ${RXPP_IMPORT_ROOT}
                -o rxpp
                ${RXPP_MEMBER_SOURCE}
        COMMAND ${CMAKE_COMMAND} -E chdir ${RXPP_MEMBER_ROOT}
                $<TARGET_FILE:rxas> rxpp
        DEPENDS
                ${CMAKE_CURRENT_SOURCE_DIR}/rxpp.crexx
                rxpp_compiler_imports
                ${RXPP_IMPORT_RXFNSB}
                ${RXPP_IMPORT_CLASSLIB}
                ${RXPP_IMPORT_RXFNSC}
                ${RXPP_IMPORT_REXXSCRIPT}
                ${RXPP_IMPORT_EXITS}
                ${RXPP_IMPORT_PRECOMP}
                rxc rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compile and assemble rxpp in its private member directory ..."
        VERBATIM)

set(RXPP_LINK_ROOT ${CMAKE_CURRENT_BINARY_DIR}/linked/rxpp)
set(RXPP_LINK_IMAGE ${RXPP_LINK_ROOT}/rxpp_linked.rxbin)
add_custom_command(
        OUTPUT ${RXPP_LINK_IMAGE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RXPP_LINK_ROOT}
        COMMAND $<TARGET_FILE:rxlink>
                -s
                -o ${RXPP_LINK_IMAGE}
                ${RXPP_MEMBER_RXBIN}
                ${RXFNSB_RXBIN}
                ${CLASSLIB_RXBIN}
                ${CLASSLIB_NATIVE_RXBIN}
                ${RXFNSC_RXBIN}
                ${REXXSCRIPT_RXBIN}
        DEPENDS
                rxlink
                ${RXPP_MEMBER_RXBIN}
                ${RXFNSB_RXBIN}
                ${CLASSLIB_RXBIN}
                ${CLASSLIB_NATIVE_RXBIN}
                ${RXFNSC_RXBIN}
                ${REXXSCRIPT_RXBIN}
        COMMENT "Link the private rxpp runtime image ..."
        VERBATIM)

set(RXPP_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated/rxpp)
set(RXPP_GENERATED_C ${RXPP_GENERATED_ROOT}/rxpp.c)
set(RXPP_GENERATED_C_TEMP_STEM ${RXPP_GENERATED_ROOT}/rxpp.tmp)
set(RXPP_GENERATED_C_TEMP ${RXPP_GENERATED_C_TEMP_STEM}.c)
add_custom_command(
        OUTPUT ${RXPP_GENERATED_C}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RXPP_GENERATED_ROOT}
        COMMAND $<TARGET_FILE:rxcpack>
                -o ${RXPP_GENERATED_C_TEMP_STEM}
                ${RXPP_LINK_ROOT}/rxpp_linked
        COMMAND ${CMAKE_COMMAND} -E rename
                ${RXPP_GENERATED_C_TEMP}
                ${RXPP_GENERATED_C}
        DEPENDS rxcpack ${RXPP_LINK_IMAGE}
        COMMENT "Package the private rxpp runtime image as C ..."
        VERBATIM)

###
add_executable(rxpp ${RXPP_GENERATED_C})

#
# RXPA providers statically required by the packaged RXPP executable.
#
# precomp is RXPP's own native provider.
#
# The SYSTEM/Os class is contained in classlib_native.rxbin and references
# the filesystem and platform RXPA providers.  Since RXPP is packaged as a
# standalone native executable, those providers must also be statically
# available to the executable.
#

configure_linker_for_static_lib(rxpp precomp)
configure_linker_for_static_lib(rxpp fs)
configure_linker_for_static_lib(rxpp _platform
        PROVIDER_ID rxplatform)

# Runtime libraries required by the packaged executable.
#
# configure_linker_for_static_lib() already links the corresponding
# *_static provider libraries and force-loads their RXPA init symbols.
#
target_link_libraries(rxpp
        rxvml
        rxpashim)

add_dependencies(rxpp
        rxvml
        rxpa
        avl_tree
        platform
        fs
        fs_static
        _platform
        _platform_static)

add_dependencies(rxpp
        rxpp_support_files)

if (ENABLE_PARSER_MODE)
        configure_file(
                ${CMAKE_CURRENT_SOURCE_DIR}/rxpp_sh_config.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/rxpp_sh_config.h
                @ONLY)
        add_executable(rxpp_sh rxpp_sh.c)
        set_target_properties(rxpp_sh PROPERTIES OUTPUT_NAME "rxpp-sh")
        target_compile_definitions(rxpp_sh PRIVATE ENABLE_PARSER_MODE)
        target_include_directories(rxpp_sh PRIVATE
                ${CMAKE_SOURCE_DIR}/compiler
                ${CMAKE_BINARY_DIR}/compiler
                ${CMAKE_CURRENT_BINARY_DIR}
                ${CMAKE_SOURCE_DIR}/platform
                ${CMAKE_SOURCE_DIR}/utf8)
        target_link_libraries(rxpp_sh PRIVATE rxclib)
        add_dependencies(rxpp_sh rxpp)
        install(TARGETS rxpp_sh DESTINATION bin)
endif()

# Tests for the rxpp preprocessor tool
add_subdirectory(tests)
