# Functions to add a plugin target
function(_crexx_configure_rxpa_plugin_target target)
    if(TARGET CREXX::RXPA)
        target_link_libraries(${target} PRIVATE CREXX::RXPA)
    elseif(TARGET crexx_rxpa_sdk)
        target_link_libraries(${target} PRIVATE crexx_rxpa_sdk)
    else()
        # Compatibility path for existing source-tree users that include this
        # helper before the RXPA SDK target exists.
        target_include_directories(${target} PRIVATE
                "${CMAKE_SOURCE_DIR}/rxpa"
                "${CMAKE_BINARY_DIR}/generated")
    endif()
endfunction()

function(_crexx_set_dynamic_plugin_output target)
    if(DEFINED CREXX_RXPA_PLUGIN_OUTPUT_DIRECTORY AND
       NOT CREXX_RXPA_PLUGIN_OUTPUT_DIRECTORY STREQUAL "")
        set(_crexx_plugin_output "${CREXX_RXPA_PLUGIN_OUTPUT_DIRECTORY}")
    else()
        set(_crexx_plugin_output "${CMAKE_CURRENT_BINARY_DIR}/bin")
    endif()
    set_target_properties(${target} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${_crexx_plugin_output}"
            RUNTIME_OUTPUT_DIRECTORY "${_crexx_plugin_output}")
endfunction()

# Create a dynamic link module
function(add_dynamic_plugin_target plugin_name)
    # A test or alternate implementation may deliberately publish the stable
    # identity of another provider while retaining a distinct CMake target.
    cmake_parse_arguments(RXPA_PLUGIN "" "PROVIDER_ID" "" ${ARGN})
    set(sources ${RXPA_PLUGIN_UNPARSED_ARGUMENTS})
    if(RXPA_PLUGIN_PROVIDER_ID)
        set(_crexx_plugin_id "${RXPA_PLUGIN_PROVIDER_ID}")
    else()
        set(_crexx_plugin_id "rx${plugin_name}")
    endif()

    if(NOT sources)
        message(FATAL_ERROR "No source files provided for dynamic rxpa plugin ${plugin_name}")
    endif()

    # Create the plugin module
    add_library(${plugin_name} MODULE ${sources})
    _crexx_configure_rxpa_plugin_target(${plugin_name})
    target_compile_definitions(${plugin_name} PRIVATE BUILD_DLL)
    target_compile_definitions(${plugin_name} PRIVATE "PLUGIN_ID=${_crexx_plugin_id}")
    if(RXPA_PLUGIN_PROVIDER_ID)
        set_target_properties(${plugin_name} PROPERTIES
                PREFIX "" OUTPUT_NAME "${_crexx_plugin_id}")
    else()
        set_target_properties(${plugin_name} PROPERTIES PREFIX "rx")
    endif()
    set_target_properties(${plugin_name} PROPERTIES SUFFIX ".rxplugin")
    _crexx_set_dynamic_plugin_output(${plugin_name})

    # Virtual target rx{plugin_name} to rx{plugin_name}.rxplugin
    # add_custom_target(rx${plugin_name} ALL DEPENDS ${plugin_name})
endfunction()

# Create a static link module - declaration only
function(add_decl_plugin_target plugin_name)
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    if(NOT sources)
        message(FATAL_ERROR "No source files provided for static rxpa plugin decl ${plugin_name}")
    endif()

    # Create a static library version of the plugin declaration
    add_library(${plugin_name}_decl STATIC ${sources})
    _crexx_configure_rxpa_plugin_target(${plugin_name}_decl)
    target_compile_definitions(${plugin_name}_decl PRIVATE "PLUGIN_ID=rx${plugin_name}")
    target_compile_definitions(${plugin_name}_decl PRIVATE "DECL_ONLY")
    set_target_properties(${plugin_name}_decl PROPERTIES PREFIX "rx")
endfunction()

# Create a static link module - declaration and definition/implementation
function(add_static_plugin_target plugin_name)
    # A static provider can use a stable provider identity which differs from
    # its CMake target name (for example when the natural target name is
    # already occupied by a core library).
    cmake_parse_arguments(RXPA_PLUGIN "" "PROVIDER_ID" "" ${ARGN})
    set(sources ${RXPA_PLUGIN_UNPARSED_ARGUMENTS})
    if(RXPA_PLUGIN_PROVIDER_ID)
        set(_crexx_plugin_id "${RXPA_PLUGIN_PROVIDER_ID}")
    else()
        set(_crexx_plugin_id "rx${plugin_name}")
    endif()

    if(NOT sources)
        message(FATAL_ERROR "No source files provided for static rxpa plugin ${plugin_name}")
    endif()

    # Create a static library version of the plugin
    add_library(${plugin_name}_static STATIC ${sources})
    _crexx_configure_rxpa_plugin_target(${plugin_name}_static)
    target_compile_definitions(${plugin_name}_static PRIVATE "PLUGIN_ID=${_crexx_plugin_id}")
    if(RXPA_PLUGIN_PROVIDER_ID)
        set_target_properties(${plugin_name}_static PROPERTIES
                PREFIX "" OUTPUT_NAME "${_crexx_plugin_id}_static")
    else()
        set_target_properties(${plugin_name}_static PROPERTIES PREFIX "rx")
    endif()
endfunction()

# Publish a dynamic/static RXPA pair as one declarative provider.  The stable
# provider ID is both the PLUGIN_ID compiled into the targets and the canonical
# artifact stem: <id>.rxplugin for dynamic loading and <id>.a/.lib for native
# packaging.  The historical <id>_static.a/.lib archive remains available for
# compatibility with existing consumers.
function(add_rxpa_provider_package plugin_name)
    cmake_parse_arguments(RXPA_PROVIDER "" "OUTPUT_DIRECTORY;PROVIDER_ID" "" ${ARGN})
    set(_crexx_dynamic_target ${plugin_name})
    set(_crexx_static_target ${plugin_name}_static)
    if(RXPA_PROVIDER_PROVIDER_ID)
        set(_crexx_provider_id "${RXPA_PROVIDER_PROVIDER_ID}")
    else()
        set(_crexx_provider_id "rx${plugin_name}")
    endif()
    if(RXPA_PROVIDER_OUTPUT_DIRECTORY)
        set(_crexx_provider_dir "${RXPA_PROVIDER_OUTPUT_DIRECTORY}")
    else()
        set(_crexx_provider_dir "${CMAKE_BINARY_DIR}/bin/providers")
    endif()
    set(_crexx_provider_static
            "${_crexx_provider_dir}/${_crexx_provider_id}${CMAKE_STATIC_LIBRARY_SUFFIX}")

    if(NOT TARGET ${_crexx_dynamic_target} OR
       NOT TARGET ${_crexx_static_target})
        message(FATAL_ERROR
                "add_rxpa_provider_package(${plugin_name}) requires both dynamic and static targets")
    endif()

    file(MAKE_DIRECTORY "${_crexx_provider_dir}")
    # This helper owned the version-1 sidecar in older build trees.  Remove
    # that exact generated artifact so an incremental build/install cannot
    # accidentally republish the retired discovery mechanism.
    file(REMOVE "${_crexx_provider_dir}/${_crexx_provider_id}.rxprovider")
    add_custom_command(TARGET ${_crexx_dynamic_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${_crexx_provider_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${_crexx_dynamic_target}>"
                    "${_crexx_provider_dir}/$<TARGET_FILE_NAME:${_crexx_dynamic_target}>")
    add_custom_command(TARGET ${_crexx_static_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${_crexx_provider_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${_crexx_static_target}>"
                    "${_crexx_provider_static}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${_crexx_static_target}>"
                    "${_crexx_provider_dir}/$<TARGET_FILE_NAME:${_crexx_static_target}>")
endfunction()

# Function to configure the linker for a static declaration library ensuring the library is linked into the executable
function(configure_linker_for_decl_lib target pluginId)
    if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
        # For Visual Studio Compiler
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_crexx_plugin_init_symbol "rx${pluginId}_init_")
        else()
            set(_crexx_plugin_init_symbol "_rx${pluginId}_init_")
        endif()
        target_link_libraries(${target} ${pluginId}_decl)
        target_link_options(${target} PRIVATE "/INCLUDE:${_crexx_plugin_init_symbol}")
    elseif(APPLE)
        # For Apple linkers
        target_link_libraries(${target} "-Wl,-force_load,\"$<TARGET_FILE:${pluginId}_decl>\"")
    else()
        # For GNU-like ELF linkers, including GCC and Clang on Linux
        target_link_libraries(${target} "-Wl,--whole-archive \"$<TARGET_FILE:${pluginId}_decl>\" -Wl,--no-whole-archive")
    endif()
endfunction()

# Function to configure the linker for a static definition library ensuring the library is linked into the executable
function(configure_linker_for_static_lib target pluginId)
    cmake_parse_arguments(RXPA_STATIC_LINK "" "PROVIDER_ID" "" ${ARGN})
    if(RXPA_STATIC_LINK_PROVIDER_ID)
        set(_crexx_static_provider_id "${RXPA_STATIC_LINK_PROVIDER_ID}")
    else()
        set(_crexx_static_provider_id "rx${pluginId}")
    endif()
    if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
        # For Visual Studio Compiler
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_crexx_plugin_init_symbol "${_crexx_static_provider_id}_init_")
        else()
            set(_crexx_plugin_init_symbol "_${_crexx_static_provider_id}_init_")
        endif()
        target_link_libraries(${target} ${pluginId}_static)
        target_link_options(${target} PRIVATE "/INCLUDE:${_crexx_plugin_init_symbol}")
    elseif(APPLE)
        # For Apple linkers
        target_link_libraries(${target} "-Wl,-force_load,\"$<TARGET_FILE:${pluginId}_static>\"")
    else()
        # For GNU-like ELF linkers, including GCC and Clang on Linux
        target_link_libraries(${target} "-Wl,--whole-archive \"$<TARGET_FILE:${pluginId}_static>\" -Wl,--no-whole-archive")
    endif()
endfunction()


# Function to configure the linker for a static definition library ensuring the library is linked into the executable
function(configure_linker_for_static_lib_rel target dirId pluginId)
    if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
        # For Visual Studio Compiler
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_crexx_plugin_init_symbol "rx${pluginId}_init_")
        else()
            set(_crexx_plugin_init_symbol "_rx${pluginId}_init_")
        endif()
        target_link_libraries(${target} ${pluginId}_static)
        target_link_options(${target} PRIVATE "/INCLUDE:${_crexx_plugin_init_symbol}")
    elseif(APPLE)
        # For Apple linkers
        target_link_libraries(${target} "-Wl,-force_load,\"$<TARGET_FILE:${pluginId}_static>\"")
    else()
        # For GNU-like ELF linkers, including GCC and Clang on Linux
        target_link_libraries(${target} "-Wl,--whole-archive \"$<TARGET_FILE:${pluginId}_static>\" -Wl,--no-whole-archive")
    endif()
endfunction()
