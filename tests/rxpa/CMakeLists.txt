cmake_minimum_required(VERSION 3.24)
project(rxpatest C)
add_subdirectory(multplugins)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPlugin Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

function(localize_test_plugin_output target)
    # Keep test-only plugins out of the shared release bin directory.
    set_target_properties(${target} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endfunction()

# Create dynamic plugin module
add_dynamic_plugin_target(_rxpa_dynlink rxpa_dynlink.c)
localize_test_plugin_output(_rxpa_dynlink)
add_dependencies(_rxpa_dynlink rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

# Create a version of rxc that statically links the _rxpa_static library
# Create static link module for rxc
# The static declaration library
add_decl_plugin_target(_rxpalink rxpa_staticlink.c)
# The static link test - a version of rxc that statically links the _rxpa_decl library
add_executable(rxc_staticlink "${CMAKE_SOURCE_DIR}/compiler/rxc_main.c")
target_include_directories(rxc_staticlink PRIVATE ${CMAKE_SOURCE_DIR}/platform)
# Ensure the linker includes the plugin
configure_linker_for_decl_lib(rxc_staticlink _rxpalink)
# Add the dependencies and link libraries
add_dependencies(rxc_staticlink rxclib _rxpalink_decl rxpa library rxvm)
target_link_libraries(rxc_staticlink rxclib _rxpalink_decl)

# Create static link module for rxvm
# The static definition library
add_static_plugin_target(_rxpalink rxpa_staticlink.c)

# Create a version of rxvm that statically links the _rxpa_static library
# Basic Build
add_executable(testvm
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmmain.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmintp.h
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmintp.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmload.c
        ${CMAKE_SOURCE_DIR}/interpreter/interrupt.c
        ${CMAKE_SOURCE_DIR}/avl_tree/rxastree.h
        ${CMAKE_SOURCE_DIR}/utf8/utf.h
        ${CMAKE_SOURCE_DIR}/binutils/include/rxbin.h
        ${CMAKE_SOURCE_DIR}/interpreter/rxspawn.c
        ${CMAKE_SOURCE_DIR}/interpreter/exitfunc.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxpafuncs.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxvml/rxvm_run.c)
# Set the performance properties
# set_property(TARGET testvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET testvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET testvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
# Ensure the linker includes the plugin
configure_linker_for_static_lib(testvm _rxpalink)
# Add the dependencies and link libraries
add_dependencies(testvm avl_tree rxpa platform _rxpalink_static)
target_link_libraries(testvm rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim _rxpalink_static)
target_include_directories(testvm PRIVATE
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/binutils/include
        ${CMAKE_SOURCE_DIR}/assembler
        ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/utf8
        ${CMAKE_SOURCE_DIR}/inc)

add_custom_target(rxpa_dynlink_run ALL)
crexx_add_rexx_opt_matrix(
        NAME rxpa_dynlink_run
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_dynlink.rexx
        TARGET_GROUP rxpa_dynlink_run
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _rxpa_dynlink
        RUNTIME_ARGS rx_rxpa_dynlink ${CMAKE_BINARY_DIR}/bin/library
)

add_custom_target(rxpa_staticlink_run ALL)
crexx_add_rexx_opt_matrix(
        NAME rxpa_staticlink_run
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_staticlink.rexx
        TARGET_GROUP rxpa_staticlink_run
        COMPILER_TARGET rxc_staticlink
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library compiler_exit_bin
        RUNNER testvm
        RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
)

# Enable testing functionality
enable_testing()

foreach(_mode IN ITEMS noopt opt)
    set(_mode_flag)
    if(_mode STREQUAL "noopt")
        set(_mode_flag -n)
    endif()

    add_test(
            NAME rxpa_dynlink_compile_${_mode}
            COMMAND $<TARGET_FILE:rxc> -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}" ${_mode_flag} -o rxpa_dynlink_compile_${_mode} ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_dynlink
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_test(
            NAME rxpa_staticlink_compile_${_mode}
            COMMAND rxc_staticlink -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_CURRENT_BINARY_DIR}" ${_mode_flag} -o rxpa_staticlink_compile_${_mode} ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_staticlink
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endforeach()

# Function Tests
# Create dynamic plugin module
add_dynamic_plugin_target(_testfuncs_dynamic rxpa_functests.c)
localize_test_plugin_output(_testfuncs_dynamic)
add_dependencies(_testfuncs_dynamic rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

# Create static plugin module for rxvm
add_static_plugin_target(_testfuncs rxpa_functests.c)

# Create a version of rxvm that statically links the _testfuncs_static library
# Basic Build
add_executable(funcvm
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmmain.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmintp.h
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmintp.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmload.c
        ${CMAKE_SOURCE_DIR}/interpreter/interrupt.c
        ${CMAKE_SOURCE_DIR}/avl_tree/rxastree.h
        ${CMAKE_SOURCE_DIR}/utf8/utf.h
        ${CMAKE_SOURCE_DIR}/binutils/include/rxbin.h
        ${CMAKE_SOURCE_DIR}/interpreter/rxspawn.c
        ${CMAKE_SOURCE_DIR}/interpreter/exitfunc.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxpafuncs.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxvml/rxvm_run.c
)
# Set the performance properties
# set_property(TARGET funcvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET funcvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET funcvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
# Ensure the linker includes the plugin
configure_linker_for_static_lib(funcvm _testfuncs)
# Add the dependencies and link libraries
add_dependencies(funcvm avl_tree rxpa platform _testfuncs_static)
target_link_libraries(funcvm rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim _testfuncs_static)
target_include_directories(funcvm PRIVATE
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/binutils/include
        ${CMAKE_SOURCE_DIR}/assembler
        ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/utf8
        ${CMAKE_SOURCE_DIR}/inc)

# Additional functional tests (of callback functions)
set(FUNC_TESTS
        signal_funcs
        string_funcs
        int_funcs
        float_funcs
        array_push_funcs
)
set(OUTPUTS
        Signal
        OK
        OK
        OK
        OK
)

list(LENGTH FUNC_TESTS num_tests)
math(EXPR num_tests "${num_tests} - 1") # Correct upper bound for RANGE
foreach(idx RANGE 0 ${num_tests})  # Iterate from 0 to len-1
    list(GET FUNC_TESTS ${idx} _rexxtest)
    list(GET OUTPUTS ${idx} _expected_output)
    set(_rexxtest_source ${_rexxtest}.rexx)
    add_custom_target(rxpa_${_rexxtest}_dynamic ALL)
    crexx_add_rexx_opt_matrix(
            NAME rxpa_${_rexxtest}_dynamic
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_rexxtest_source}
            TARGET_GROUP rxpa_${_rexxtest}_dynamic
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS library funcvm _testfuncs_dynamic
            RUNTIME_ARGS rx_testfuncs_dynamic ${CMAKE_BINARY_DIR}/bin/library
    )
    set_tests_properties(
            rxpa_${_rexxtest}_dynamic_noopt
            rxpa_${_rexxtest}_dynamic_opt
            PROPERTIES PASS_REGULAR_EXPRESSION ".*${_expected_output}.*"
    )

    add_custom_target(rxpa_${_rexxtest}_static ALL)
    crexx_add_rexx_opt_matrix(
            NAME rxpa_${_rexxtest}_static
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_rexxtest_source}
            TARGET_GROUP rxpa_${_rexxtest}_static
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS library funcvm _testfuncs_dynamic
            RUNNER funcvm
            RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
    )
    set_tests_properties(
            rxpa_${_rexxtest}_static_noopt
            rxpa_${_rexxtest}_static_opt
            PROPERTIES PASS_REGULAR_EXPRESSION ".*${_expected_output}.*"
    )

endforeach()
