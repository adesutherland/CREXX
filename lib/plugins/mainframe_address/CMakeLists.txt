cmake_minimum_required(VERSION 3.24)
project(mainframe_address C)
set(CMAKE_C_STANDARD 99)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)
# Host metadata compiler for matching native consumers. The desktop product
# does not register a mainframe command provider.
add_decl_plugin_target(mainframe_address mainframe_address.c)
target_include_directories(mainframe_address_decl PRIVATE
    ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/interpreter)
set_target_properties(mainframe_address_decl PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_executable(rxc_mainframe_address EXCLUDE_FROM_ALL ${CMAKE_SOURCE_DIR}/compiler/rxc_main.c)
target_include_directories(rxc_mainframe_address PRIVATE ${CMAKE_SOURCE_DIR}/platform)
configure_linker_for_decl_lib(rxc_mainframe_address mainframe_address)
target_link_libraries(rxc_mainframe_address rxclib)
# Source manifests come from the owning targets, so native builds track the
# current compiler/assembler split rather than a historical Lab source list.
function(mainframe_source_manifest output)
    set(content "")
    foreach(owner IN LISTS ARGN)
        get_target_property(units ${owner} SOURCES)
        get_target_property(directory ${owner} SOURCE_DIR)
        foreach(unit IN LISTS units)
            if(NOT IS_ABSOLUTE "${unit}")
                set(unit "${directory}/${unit}")
            endif()
            get_filename_component(unit "${unit}" ABSOLUTE)
            get_source_file_property(header ${unit} TARGET_DIRECTORY ${owner} HEADER_FILE_ONLY)
            if(unit MATCHES "\\.c$" AND NOT header)
                string(APPEND content "${unit}\n")
            endif()
        endforeach()
    endforeach()
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/mainframe/${output} CONTENT "${content}")
endfunction()
mainframe_source_manifest(compiler_sources.txt rxc_backend_objects rxc_typed_objects rxc_classic_objects)
mainframe_source_manifest(assembler_sources.txt rxaslib)

# Actual SDK-backed host checks are opt-in: a release SDK may predate command
# services, and the repository does not carry a second copy of their ABI.
set(CREXX_MAINFRAME_COMMAND_ADAPTERS "" CACHE PATH "SDK adapter source for native ADDRESS host checks")
if(BUILD_TESTING AND EXISTS "${CREXX_MAINFRAME_COMMAND_ADAPTERS}/command.h")
    foreach(profile IN ITEMS cms tso)
        string(TOUPPER ${profile} upper)
        add_library(mainframe_address_${profile}_static STATIC mainframe_address.c
            ${CMAKE_SOURCE_DIR}/platform/mainframe_command.c
            ${CMAKE_SOURCE_DIR}/platform/mainframe_command_${profile}.c
            ${CMAKE_SOURCE_DIR}/platform/text_codec.c
            ${CREXX_MAINFRAME_COMMAND_ADAPTERS}/${profile}/command.c
            ${CREXX_MAINFRAME_COMMAND_ADAPTERS}/command-text.c
            ${CREXX_MAINFRAME_COMMAND_ADAPTERS}/cms/text-codec.c
            tests/native_boundary.c)
        target_include_directories(mainframe_address_${profile}_static PRIVATE
            ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/interpreter
            ${CREXX_MAINFRAME_COMMAND_ADAPTERS})
        target_compile_definitions(mainframe_address_${profile}_static PRIVATE
            PLUGIN_ID=rxmainframe_address CREXX_MAINFRAME_ADDRESS=1 CREXX_${upper}_ELF=1 MF_COMMAND_HOST_TEST=1)
        if(profile STREQUAL "cms")
            target_sources(mainframe_address_${profile}_static PRIVATE
                ${CREXX_MAINFRAME_COMMAND_ADAPTERS}/cms/adapter.c)
            target_compile_definitions(mainframe_address_${profile}_static PRIVATE
                LAB_CMS_HOST_TEST=1 LAB_CMS_APPLICATION=1)
        endif()
        set(vm mainframe_address_${profile}_vm)
        add_executable(${vm} EXCLUDE_FROM_ALL ${CMAKE_SOURCE_DIR}/interpreter/rxvmmain.c
            $<TARGET_OBJECTS:rxbvm_core_objects> $<TARGET_OBJECTS:rxvm_platform_notls_objects>)
        target_compile_definitions(${vm} PRIVATE NTHREADED=1)
        target_include_directories(${vm} PRIVATE ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/interpreter ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
            ${CMAKE_SOURCE_DIR}/binutils/include ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/rxpa ${CMAKE_SOURCE_DIR}/avl_tree ${CMAKE_SOURCE_DIR}/utf8 ${CMAKE_SOURCE_DIR}/inc)
        # Explicit stable identity: the CMake target carries the selected mock.
        configure_linker_for_static_lib(${vm} mainframe_address_${profile} PROVIDER_ID rxmainframe_address)
        target_link_libraries(${vm} rxvmplugin rxvmref rxbin mc_decimal_manual avl_tree rxpa rxpashim)
        set(embed mainframe_address_${profile}_embed)
        add_executable(${embed} EXCLUDE_FROM_ALL tests/native_embed.c
            ${CMAKE_SOURCE_DIR}/interpreter/rxvml.c
            $<TARGET_OBJECTS:rxbvm_core_objects> $<TARGET_OBJECTS:rxvm_platform_notls_objects>)
        get_target_property(includes ${vm} INCLUDE_DIRECTORIES)
        target_include_directories(${embed} PRIVATE ${includes})
        target_compile_definitions(${embed} PRIVATE NTHREADED=1)
        configure_linker_for_static_lib(${embed} mainframe_address_${profile} PROVIDER_ID rxmainframe_address)
        target_link_libraries(${embed} rxvmplugin rxvmref rxbin mc_decimal_manual avl_tree rxpa rxpashim)
        add_executable(mainframe_address_${profile}_policy EXCLUDE_FROM_ALL tests/native_policy.c)
        target_include_directories(mainframe_address_${profile}_policy PRIVATE ${CMAKE_SOURCE_DIR}/platform)
        target_compile_definitions(mainframe_address_${profile}_policy PRIVATE CREXX_${upper}_ELF=1)
        target_link_libraries(mainframe_address_${profile}_policy mainframe_address_${profile}_static)
        add_test(NAME mainframe_address_${profile}_policy COMMAND mainframe_address_${profile}_policy)
    endforeach()
    set(fixture_dir ${CMAKE_CURRENT_BINARY_DIR}/fixtures)
    # Force new native reads after ADDRESS rather than only checking stdio's
    # prefetched tail. Both selected SDK newlib profiles use 1024-byte BUFSIZ.
    string(REPEAT "X" 65536 caller_padding)
    file(GENERATE OUTPUT ${fixture_dir}/keepin.DATA CONTENT "ALPHA\n${caller_padding}\nOMEGA\n")
    # Match the four-library guest collection instead of scanning every
    # unrelated plugin staged beside the host compiler.
    set(fixture_imports ${fixture_dir}/imports)
    set(import_files)
    foreach(member IN ITEMS library rxfnsc rxfnsg rxcexits)
        list(APPEND import_files ${fixture_imports}/${member}.rxbin)
    endforeach()
    add_custom_command(OUTPUT ${import_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${fixture_imports}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE_DIR:rxas>/library.rxbin
            $<TARGET_FILE_DIR:rxas>/rxfnsc.rxbin
            $<TARGET_FILE_DIR:rxas>/rxfnsg.rxbin
            $<TARGET_FILE_DIR:rxas>/rxcexits.rxbin ${fixture_imports}
        DEPENDS library rxfnsc rxfnsg compiler_exit_bin)
    foreach(level IN ITEMS b c)
        set(source ${CMAKE_CURRENT_SOURCE_DIR}/tests/native_address.crexx)
        set(libraries $<TARGET_FILE_DIR:rxas>/library.rxbin)
        if(level STREQUAL "c")
            set(source ${CMAKE_CURRENT_SOURCE_DIR}/tests/native_address_classic.rexx)
            list(APPEND libraries $<TARGET_FILE_DIR:rxas>/rxfnsc.rxbin $<TARGET_FILE_DIR:rxas>/rxfnsg.rxbin)
        endif()
        set(fixture ${fixture_dir}/native_${level})
        add_custom_command(OUTPUT ${fixture}_linked.rxbin
            COMMAND ${CMAKE_COMMAND} -E make_directory ${fixture_dir}
            COMMAND rxc_mainframe_address --no-exe-import -n -i ${fixture_imports} -l ${fixture_imports} -o ${fixture} ${source}
            COMMAND rxas -o ${fixture} ${fixture}
            COMMAND rxlink -o ${fixture}_linked ${fixture} ${libraries}
            DEPENDS ${source} rxc_mainframe_address rxas rxlink library rxfnsc ${libraries} ${import_files})
        list(APPEND fixtures ${fixture}_linked.rxbin)
        foreach(profile IN ITEMS cms tso)
            string(TOUPPER ${profile} upper)
            set(capture nocapture)
            if(profile STREQUAL "cms")
                set(capture capture)
            endif()
            if(level STREQUAL "c")
                add_test(NAME mainframe_address_${profile}_${level} COMMAND mainframe_address_${profile}_vm
                    ${fixture}_linked.rxbin -a "${upper} ${capture} - QUALIFY")
            else()
                add_test(NAME mainframe_address_${profile}_${level} COMMAND mainframe_address_${profile}_vm
                    ${fixture}_linked.rxbin -a ${upper} ${capture} -
                    $<IF:$<STREQUAL:${profile},cms>,${fixture_dir}/keepin,${fixture_dir}/keepin.DATA> QUALIFY)
                add_test(NAME mainframe_address_${profile}_embed COMMAND mainframe_address_${profile}_embed
                    ${fixture}_linked.rxbin ${upper} ${capture})
            endif()
        endforeach()
    endforeach()
    add_custom_target(mainframe_address_fixtures DEPENDS ${fixtures})
    add_custom_target(mainframe_address_checks DEPENDS mainframe_address_fixtures
        mainframe_address_cms_vm mainframe_address_tso_vm mainframe_address_cms_embed mainframe_address_tso_embed
        mainframe_address_cms_policy mainframe_address_tso_policy)
    set_tests_properties(mainframe_address_cms_policy mainframe_address_tso_policy
        mainframe_address_cms_b mainframe_address_tso_b mainframe_address_cms_c mainframe_address_tso_c
        mainframe_address_cms_embed mainframe_address_tso_embed PROPERTIES LABELS "mainframe;address" TIMEOUT 60)
    crexx_register_test_prep_targets(
        TESTS mainframe_address_cms_policy mainframe_address_tso_policy
            mainframe_address_cms_b mainframe_address_tso_b mainframe_address_cms_c mainframe_address_tso_c
            mainframe_address_cms_embed mainframe_address_tso_embed
        TARGETS mainframe_address_checks)
    crexx_schedule_directory_qa_tier_finalization()
endif()
