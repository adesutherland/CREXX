# Functions to add a plugin target
# Create a dynamic link module
function(add_dynamic_plugin_target plugin_name)
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    # Create the plugin module
    add_library(${plugin_name} MODULE ${sources})
    target_include_directories(${plugin_name} PRIVATE ${CMAKE_SOURCE_DIR}/rxpa)
    target_compile_definitions(${plugin_name} PRIVATE BUILD_DLL)
    target_compile_definitions(${plugin_name} PRIVATE "PLUGIN_ID=rx${plugin_name}")
    set_target_properties(${plugin_name} PROPERTIES PREFIX "rx")
    set_target_properties(${plugin_name} PROPERTIES SUFFIX ".rxplugin")
endfunction()

# Create a static link module - declaration only
function(add_decl_plugin_target plugin_name)
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    # Create a static library version of the plugin declaration
    add_library(${plugin_name}_decl STATIC ${sources})
    target_include_directories(${plugin_name}_decl PRIVATE ${CMAKE_SOURCE_DIR}/rxpa)
    target_compile_definitions(${plugin_name}_decl PRIVATE "PLUGIN_ID=rx${plugin_name}")
    target_compile_definitions(${plugin_name}_decl PRIVATE "DECL_ONLY")
    set_target_properties(${plugin_name}_decl PROPERTIES PREFIX "rx")
endfunction()

# Create a static link module - declaration and definition/implementation
function(add_static_plugin_target plugin_name)
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    # Create a static library version of the plugin
    add_library(${plugin_name}_static STATIC ${sources})
    target_include_directories(${plugin_name}_static PRIVATE ${CMAKE_SOURCE_DIR}/rxpa)
    target_compile_definitions(${plugin_name}_static PRIVATE "PLUGIN_ID=rx${plugin_name}")
    set_target_properties(${plugin_name}_static PROPERTIES PREFIX "rx")
endfunction()

# Function to configure the linker for a static declaration library ensuring the library is linked into the executable
function(configure_linker_for_decl_lib target pluginId)
    if(MSVC)
        # For Visual Studio Compiler
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:${pluginId}_init")
    elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        # For GCC
        target_link_libraries(${target} "-Wl,--whole-archive \"${CMAKE_CURRENT_BINARY_DIR}/rx${pluginId}_decl.a\" -Wl,--no-whole-archive")
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        # For Clang
        target_link_libraries(${target} "-Wl,-force_load,\"${CMAKE_CURRENT_BINARY_DIR}/rx${pluginId}_decl.a\"")
    endif()
endfunction()

# Function to configure the linker for a static definition library ensuring the library is linked into the executable
function(configure_linker_for_static_lib target pluginId)
    if(MSVC)
        # For Visual Studio Compiler
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:${pluginId}_init")
    elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        # For GCC
        target_link_libraries(${target} "-Wl,--whole-archive \"${CMAKE_CURRENT_BINARY_DIR}/rx${pluginId}_static.a\" -Wl,--no-whole-archive")
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        # For Clang
        target_link_libraries(${target} "-Wl,-force_load,\"${CMAKE_CURRENT_BINARY_DIR}/rx${pluginId}_static.a\"")
    endif()
endfunction()
