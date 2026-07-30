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
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    if(NOT sources)
        message(FATAL_ERROR "No source files provided for dynamic rxpa plugin ${plugin_name}")
    endif()

    # Create the plugin module
    add_library(${plugin_name} MODULE ${sources})
    _crexx_configure_rxpa_plugin_target(${plugin_name})
    target_compile_definitions(${plugin_name} PRIVATE BUILD_DLL)
    target_compile_definitions(${plugin_name} PRIVATE "PLUGIN_ID=rx${plugin_name}")
    set_target_properties(${plugin_name} PROPERTIES PREFIX "rx")
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
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    if(NOT sources)
        message(FATAL_ERROR "No source files provided for static rxpa plugin ${plugin_name}")
    endif()

    # Create a static library version of the plugin
    add_library(${plugin_name}_static STATIC ${sources})
    _crexx_configure_rxpa_plugin_target(${plugin_name}_static)
    target_compile_definitions(${plugin_name}_static PRIVATE "PLUGIN_ID=rx${plugin_name}")
    set_target_properties(${plugin_name}_static PROPERTIES PREFIX "rx")
endfunction()

# Function to configure the linker for a static declaration library ensuring the library is linked into the executable
function(configure_linker_for_decl_lib target pluginId)
    if(MSVC)
        # For Visual Studio Compiler
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:${pluginId}_init")
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
    if(MSVC)
        # For Visual Studio Compiler
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:${pluginId}_init")
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
    if(MSVC)
        # For Visual Studio Compiler
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:${pluginId}_init")
    elseif(APPLE)
        # For Apple linkers
        target_link_libraries(${target} "-Wl,-force_load,\"$<TARGET_FILE:${pluginId}_static>\"")
    else()
        # For GNU-like ELF linkers, including GCC and Clang on Linux
        target_link_libraries(${target} "-Wl,--whole-archive \"$<TARGET_FILE:${pluginId}_static>\" -Wl,--no-whole-archive")
    endif()
endfunction()
