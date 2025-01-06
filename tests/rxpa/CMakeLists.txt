cmake_minimum_required(VERSION 3.24)
project(rxpatest C)

set(CMAKE_C_STANDARD 90)

# Including RXPlugin Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_rxpa_dynlink rxpa_dynlink.c)
add_dependencies(_rxpa_dynlink rxpa library rxvm rxc) # The test (not plugin) depends on rxpa, library, rxvm, and rxc

# Create a version of rxc that statically links the _rxpa_static library
# Create static link module for rxc
# The static declaration library
add_decl_plugin_target(_rxpalink rxpa_staticlink.c)
# The static link test - a version of rxc that statically links the _rxpa_decl library
add_executable(rxc_staticlink "${CMAKE_SOURCE_DIR}/compiler/rxc_main.c")
# Ensure the linker includes the plugin
configure_linker_for_decl_lib(rxc_staticlink _rxpalink)
# Add the dependencies and link libraries
add_dependencies(rxc_staticlink rxclib _rxpalink_decl rxpa library rxvm)
target_link_libraries(rxc_staticlink rxclib machine avl_tree db_decimal_manual platform db_decimal_manual rxaslib rxpa _rxpalink_decl m)

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
        ${CMAKE_SOURCE_DIR}/avl_tree/rxastree.h
        ${CMAKE_SOURCE_DIR}/utf8/utf.h
        ${CMAKE_SOURCE_DIR}/machine/rxbin.h
        ${CMAKE_SOURCE_DIR}/interpreter/rxspawn.c
        ${CMAKE_SOURCE_DIR}/interpreter/exitfunc.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxpafuncs.c)
# Set the performance properties
# set_property(TARGET testvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET testvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET testvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
# Ensure the linker includes the plugin
configure_linker_for_static_lib(testvm _rxpalink)
# Add the dependencies and link libraries
add_dependencies(testvm machine avl_tree rxpa platform _rxpalink_static)
target_link_libraries(testvm rxvmplugin machine avl_tree rxpa db_decimal_manual platform _rxpalink_static m)
target_include_directories(testvm PRIVATE
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/machine
        ${CMAKE_SOURCE_DIR}/assembler
        ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/utf8)

# Build Dynamic Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\" -o rxpa_dynlink_test \"${CMAKE_CURRENT_SOURCE_DIR}/rxpa_dynlink\" && ${CMAKE_BINARY_DIR}/assembler/rxas rxpa_dynlink_test
        DEPENDS rxas rxc library _rxpa_dynlink ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_dynlink.rexx
        OUTPUT rxpa_dynlink_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(rxpa_dynlink_test ALL
        DEPENDS rxpa_dynlink_test.rxbin
)

# Build Static Test Rexx
add_custom_command(
        COMMAND rxc_staticlink -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\" -o rxpa_staticlink_test \"${CMAKE_CURRENT_SOURCE_DIR}/rxpa_staticlink\" && ${CMAKE_BINARY_DIR}/assembler/rxas rxpa_staticlink_test
        DEPENDS rxas rxc_staticlink library ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_staticlink.rexx
        OUTPUT rxpa_staticlink_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(rxpa_staticlink_test ALL
        DEPENDS rxpa_staticlink_test.rxbin
)

# Enable testing functionality
enable_testing()

# Basic linking tests
# Dynamic link test
add_test(NAME rxpa_dynlink_test COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i "${CMAKE_BINARY_DIR}/lib/rxfnsb;${CMAKE_CURRENT_BINARY_DIR}" -o rxpa_dynlink ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_dynlink
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Static link test
add_test(NAME rxpa_staticlink_test COMMAND rxc_staticlink
        -i "${CMAKE_BINARY_DIR}/lib/rxfnsb" -o rxpa_staticlink ${CMAKE_CURRENT_SOURCE_DIR}/rxpa_staticlink
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Dynamic Execution Test
add_test(NAME rxpa_dynlink_run_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm rxpa_dynlink_test rx_rxpa_dynlink ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Static Execution Test
add_test(NAME rxpa_staticlink_run_test
        COMMAND testvm rxpa_staticlink_test ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Function Tests
# Create dynamic plugin module
add_dynamic_plugin_target(_testfuncs_dynamic rxpa_functests.c)
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
        ${CMAKE_SOURCE_DIR}/avl_tree/rxastree.h
        ${CMAKE_SOURCE_DIR}/utf8/utf.h
        ${CMAKE_SOURCE_DIR}/machine/rxbin.h
        ${CMAKE_SOURCE_DIR}/interpreter/rxspawn.c
        ${CMAKE_SOURCE_DIR}/interpreter/exitfunc.c
        ${CMAKE_SOURCE_DIR}/interpreter/rxpafuncs.c
)
# Set the performance properties
# set_property(TARGET funcvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET funcvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET funcvm PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
# Ensure the linker includes the plugin
configure_linker_for_static_lib(funcvm _testfuncs)
# Add the dependencies and link libraries
add_dependencies(funcvm machine avl_tree rxpa platform _testfuncs_static)
target_link_libraries(funcvm machine avl_tree rxpa db_decimal_manual platform rxvmplugin _testfuncs_static m)
target_include_directories(funcvm PRIVATE
        ${CMAKE_SOURCE_DIR}/interpreter
        ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin
        ${CMAKE_SOURCE_DIR}/machine
        ${CMAKE_SOURCE_DIR}/assembler
        ${CMAKE_SOURCE_DIR}/rxpa
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/avl_tree
        ${CMAKE_SOURCE_DIR}/utf8)

# Additional functional tests (of callback functions)
set(FUNC_TESTS
        signal_funcs
        string_funcs
        int_funcs
        float_funcs
)
set(OUTPUTS
        Signal
        OK
        OK
        OK
)

list(LENGTH FUNC_TESTS len)
math(EXPR len "${len} - 1")
foreach(idx RANGE ${len})
    list(GET FUNC_TESTS ${idx} _rexxtest)
    list(GET OUTPUTS ${idx} _expected_output)

    add_custom_command(
            COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i \"${CMAKE_BINARY_DIR}/lib/rxfnsb\;${CMAKE_CURRENT_BINARY_DIR}\" -o ${_rexxtest}_test ${CMAKE_CURRENT_SOURCE_DIR}/${_rexxtest} && ${CMAKE_BINARY_DIR}/assembler/rxas ${_rexxtest}_test
            DEPENDS rxas rxc library _testfuncs_dynamic ${CMAKE_CURRENT_SOURCE_DIR}/${_rexxtest}.rexx
            OUTPUT ${_rexxtest}_test.rxbin
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(${_rexxtest}_test ALL
            DEPENDS ${_rexxtest}_test.rxbin funcvm
    )

    add_test(NAME rxpa_${_rexxtest}_dynamic_test
            COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm ${_rexxtest}_test rx_testfuncs_dynamic ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(rxpa_${_rexxtest}_dynamic_test PROPERTIES PASS_REGULAR_EXPRESSION ".*${_expected_output}.*")

    add_test(NAME rxpa_${_rexxtest}_static_test
            COMMAND funcvm ${_rexxtest}_test ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(rxpa_${_rexxtest}_static_test PROPERTIES PASS_REGULAR_EXPRESSION ".*${_expected_output}.*")

endforeach()
