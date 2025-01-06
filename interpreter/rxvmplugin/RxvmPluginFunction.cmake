# Functions to add a rxvm plugin target
# Create a dynamic link module
function(add_dynamic_rxvmplugin_target plugin_name)
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    if(NOT sources)
        message(FATAL_ERROR "No source files provided for dynamic plugin ${plugin_name}")
    endif()

    # Create the plugin module
    add_library(${plugin_name} MODULE ${sources})
    target_include_directories(${plugin_name} PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
    target_compile_definitions(${plugin_name} PRIVATE BUILD_DLL)
    set_target_properties(${plugin_name} PROPERTIES PREFIX "rxvm_")
    set_target_properties(${plugin_name} PROPERTIES SUFFIX ".rxvmplugin")
endfunction()

# Create a static link module
function(add_static_rxvmplugin_target plugin_name)
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    # Check if sources are provided
    if(NOT sources)
        message(FATAL_ERROR "No source files provided for static plugin ${plugin_name}")
    endif()

    # Create a static library version of the plugin
    add_library(${plugin_name} STATIC ${sources})
    target_include_directories(${plugin_name} PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
    set_target_properties(${plugin_name} PROPERTIES PREFIX "rxvm_")
endfunction()

# Function to configure the linker for a static definition library ensuring the library is linked into the executable
function(configure_linker_for_static_rxvmplugin target pluginId)
    if(MSVC)
        # For Visual Studio Compiler
        target_link_libraries(${target} "${CMAKE_CURRENT_BINARY_DIR}/rxvm_${pluginId}.lib")
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:${pluginId}_register_rxvm_plugin")
    elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        # For GCC
        target_link_libraries(${target} "-Wl,--whole-archive \"${CMAKE_CURRENT_BINARY_DIR}/rxvm_${pluginId}.a\" -Wl,--no-whole-archive")
    elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
        # For Clang
        target_link_libraries(${target} "-Wl,-force_load,\"${CMAKE_CURRENT_BINARY_DIR}/rxvm_${pluginId}.a\"")
    endif()
endfunction()
