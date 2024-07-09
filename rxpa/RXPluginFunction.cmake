# Function to add a plugin target
function(add_plugin_target plugin_name)
    # Assuming the rest of the source files are passed as additional arguments
    set(sources ${ARGN})

    # Create a static library version of the plugin
    add_library(${plugin_name}_static STATIC ${sources})
    target_include_directories(${plugin_name}_static PRIVATE ${CMAKE_SOURCE_DIR}/rxpa)
    target_compile_definitions(${plugin_name}_static PRIVATE "PLUGIN_ID=rx${plugin_name}")
    set_target_properties(${plugin_name}_static PROPERTIES PREFIX "rx")

    # Create the plugin module
    add_library(${plugin_name} MODULE ${sources})
    target_include_directories(${plugin_name} PRIVATE ${CMAKE_SOURCE_DIR}/rxpa)
    target_compile_definitions(${plugin_name} PRIVATE BUILD_DLL)
    target_compile_definitions(${plugin_name} PRIVATE "PLUGIN_ID=rx${plugin_name}")
    set_target_properties(${plugin_name} PROPERTIES PREFIX "rx")
    set_target_properties(${plugin_name} PROPERTIES SUFFIX ".rxplugin")
endfunction()
