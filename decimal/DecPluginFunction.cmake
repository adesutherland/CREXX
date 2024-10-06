# Functions to add a dec plugin target
# Create a dynamic link module
function(add_dynamic_decplugin_target plugin_name)
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    # Create the plugin module
    add_library(${plugin_name} MODULE ${sources})
    target_include_directories(${plugin_name} PRIVATE ${CMAKE_SOURCE_DIR}/decimal)
    target_compile_definitions(${plugin_name} PRIVATE BUILD_DLL)
    set_target_properties(${plugin_name} PROPERTIES PREFIX "rxdec_")
    set_target_properties(${plugin_name} PROPERTIES SUFFIX ".decplugin")
endfunction()

# Create a static link module
function(add_static_decplugin_target plugin_name)
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    # Create a static library version of the plugin
    add_library(${plugin_name} STATIC ${sources})
    target_include_directories(${plugin_name} PRIVATE ${CMAKE_SOURCE_DIR}/decimal)
    set_target_properties(${plugin_name} PROPERTIES PREFIX "rxdec_")
endfunction()

# Function to configure the linker for a static definition library ensuring the library is linked into the executable
function(configure_linker_for_static_decplugin target pluginId)
    if(MSVC)
        # For Visual Studio Compiler
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:${pluginId}_register_dec_plugin")
    elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        # For GCC
        target_link_libraries(${target} "-Wl,--whole-archive \"${CMAKE_CURRENT_BINARY_DIR}/rxdec_${pluginId}.a\" -Wl,--no-whole-archive")
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        # For Clang
        target_link_libraries(${target} "-Wl,-force_load,\"${CMAKE_CURRENT_BINARY_DIR}/rxdec_${pluginId}.a\"")
    endif()
endfunction()
