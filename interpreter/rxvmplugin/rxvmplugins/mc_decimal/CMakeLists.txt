cmake_minimum_required(VERSION 3.24)

project(mc_decimal C)

set(CMAKE_C_STANDARD 90)

# Add the decnumber library
add_library(decnumber STATIC
        decnumber/decNumber.c
        decnumber/decContext.c
)
target_include_directories(decnumber PUBLIC decnumber)

# Add the test executable for the decnumber library (changed to use the decnumber library)
add_executable(decnumber_test
        decnumber_test.c
)
target_include_directories(decnumber_test PRIVATE decnumber)
target_link_libraries(decnumber_test decnumber)

# Including RXVM Build System
include(${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/RxvmPluginFunction.cmake)

# Add the decnumber dynamic plugin library
add_dynamic_rxvmplugin_target(mc_decimal_dynamic mc_decimal.c ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/rxvmplugin.h)
add_dependencies(mc_decimal_dynamic rxvmplugin rxvm) # Note: For build safety we want rxvm to be built first
target_include_directories(mc_decimal_dynamic PRIVATE .)
target_include_directories(mc_decimal_dynamic PRIVATE decnumber)
target_include_directories(mc_decimal_dynamic PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(mc_decimal_dynamic PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
target_link_libraries(mc_decimal_dynamic decnumber)
# Set the output name to mc_decimal
set_target_properties(mc_decimal_dynamic PROPERTIES OUTPUT_NAME mc_decimal)

# Add the decnumber static plugin library - with auto linking and initialising of the plugin
add_static_rxvmplugin_target(mc_decimal mc_decimal.c ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/rxvmplugin.h)
add_dependencies(mc_decimal rxvmplugin rxvm) # Note: For build safety we want rxvm to be built first
target_include_directories(mc_decimal PRIVATE .)
target_include_directories(mc_decimal PRIVATE decnumber)
target_include_directories(mc_decimal PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(mc_decimal PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
target_link_libraries(mc_decimal decnumber)

# Add the static test harness executable - with auto linking and initialising of the plugin
add_executable(mc_decimal_test_harness mc_decimal_sanity_harness_test.c)
target_include_directories(mc_decimal_test_harness PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(mc_decimal_test_harness PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
# Ensure the linker includes the plugin
configure_linker_for_static_rxvmplugin(mc_decimal_test_harness mc_decimal)
target_link_libraries(mc_decimal_test_harness rxvmplugin mc_decimal)

# Add the dynamic test harness executable
add_executable(mc_decimal_dyn_test_harness mc_decimal_sanity_harness_test.c)
# Add a DYNAMIC define to pass to the compile
target_compile_definitions(mc_decimal_dyn_test_harness PRIVATE DYNAMIC)
target_include_directories(mc_decimal_dyn_test_harness PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(mc_decimal_dyn_test_harness PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
target_link_libraries(mc_decimal_dyn_test_harness rxvmplugin)
add_dependencies(mc_decimal_dyn_test_harness mc_decimal_dynamic)

# Add the decnumber static plugin library - with manual linking and initialising
add_static_rxvmplugin_target(mc_decimal_manual mc_decimal.c ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/rxvmplugin.h)
add_dependencies(mc_decimal_manual rxvmplugin rxvm) # Note: For build safety we want rxvm to be built first
target_include_directories(mc_decimal_manual PRIVATE .)
target_include_directories(mc_decimal_manual PRIVATE decnumber)
target_include_directories(mc_decimal_manual PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(mc_decimal_manual PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
target_link_libraries(mc_decimal_manual decnumber)
# add MANUAL_PLUGIN_LINK to the compile definitions
target_compile_definitions(mc_decimal_manual PRIVATE MANUAL_PLUGIN_LINK)

# Add the static test harness executable - with auto linking and initialising of the plugin
add_executable(mc_decimal_manual_test_harness mc_decimal_sanity_harness_test.c)
target_include_directories(mc_decimal_manual_test_harness PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(mc_decimal_manual_test_harness PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
target_link_libraries(mc_decimal_manual_test_harness rxvmplugin mc_decimal_manual)
target_compile_definitions(mc_decimal_manual_test_harness PRIVATE MANUAL_PLUGIN_LINK)
add_dependencies(mc_decimal_manual_test_harness mc_decimal_manual)

# Add the dynamic test2 executable
add_executable(mc_decimal_full_tests mc_decimal_full_tests.c)
# Add a DYNAMIC define to pass to the compile
target_compile_definitions(mc_decimal_full_tests PRIVATE DYNAMIC)
target_include_directories(mc_decimal_full_tests PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(mc_decimal_full_tests PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
target_link_libraries(mc_decimal_full_tests rxvmplugin)
add_dependencies(mc_decimal_full_tests mc_decimal_dynamic)

# Enable Testing
enable_testing()

# First run the test for decnumber library changes we made for the plugin
add_test(decnumber_test decnumber_test)

# Run the test harness sanity tests using tthe harness for dynamic, and manual and auto static linking
# Add the first test - 32 digits, with arguments 1, 2 and check that the output are:
# ADD 3
# SUB -1
# MUL 2
# DIV 0.5
add_test(mc_decimal_test1 mc_decimal_test_harness 32 1 2)
set_tests_properties(mc_decimal_test1 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:3:.*SUB:-1:.*MUL:2:.*DIV:0\\.5:")

# Add the second test - 32 digits, with 32 digit arguments and check output
add_test(mc_decimal_test2 mc_decimal_test_harness 32 10000000000000000000000000000000 20000000000000000000000000000000)
set_tests_properties(mc_decimal_test2 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:30000000000000000000000000000000:.*SUB:-10000000000000000000000000000000:.*MUL:2E\\+62:.*DIV:0\\.5:")

# Add the third test - 100 significant digits, with 100 digit results
add_test(mc_decimal_test3 mc_decimal_test_harness 100
        12345678901234567890123456789012345678901234567890
        98765432109876543210987654321098765432109876543210)
# Set the PASS_REGULAR_EXPRESSION with the new patterns
set_tests_properties(mc_decimal_test3 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:111111111011111111101111111110111111111011111111100:.*SUB:-86419753208641975320864197532086419753208641975320:.*MUL:1219326311370217952261850327338667885945115073915611949397448712086533622923332237463801111263526900:.*DIV:0.1249999988609375000142382812498220214843772247314452846908569339413642883257329463959283381700508958:")

add_test(mc_decimal_dyn_test1 mc_decimal_dyn_test_harness 32 1 2)
set_tests_properties(mc_decimal_dyn_test1 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:3:.*SUB:-1:.*MUL:2:.*DIV:0\\.5:")

add_test(mc_decimal_manual_test1 mc_decimal_manual_test_harness 32 1 2)
set_tests_properties(mc_decimal_manual_test1 PROPERTIES PASS_REGULAR_EXPRESSION
        "ADD:3:.*SUB:-1:.*MUL:2:.*DIV:0\\.5:")

# Decimal Test 2
add_test(mc_decimal_full_tests mc_decimal_full_tests)
