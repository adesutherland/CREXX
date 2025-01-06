cmake_minimum_required(VERSION 3.24)

project(db_decimal C)

set(CMAKE_C_STANDARD 90)

# Including RXVM Build System
include(${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/RxvmPluginFunction.cmake)

# Add the dynamic plugin library
add_dynamic_rxvmplugin_target(db_decimal_dynamic db_decimal.c ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/rxvmplugin.h)
add_dependencies(db_decimal_dynamic rxvmplugin) # Note: For build safety we want rxvm to be built first
target_include_directories(db_decimal_dynamic PRIVATE .)
target_include_directories(db_decimal_dynamic PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(db_decimal_dynamic PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
# Set the output name to db_decimal
set_target_properties(db_decimal_dynamic PROPERTIES OUTPUT_NAME db_decimal)

# Add the static plugin library - with manual linking and initialising
add_static_rxvmplugin_target(db_decimal_manual db_decimal.c ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin/rxvmplugin.h)
add_dependencies(db_decimal_manual rxvmplugin) # Note: For build safety we want rxvm to be built first
target_include_directories(db_decimal_manual PRIVATE .)
target_include_directories(db_decimal_manual PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(db_decimal_manual PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
# add MANUAL_PLUGIN_LINK to the compile definitions
target_compile_definitions(db_decimal_manual PRIVATE MANUAL_PLUGIN_LINK)

# Add the test executable
add_executable(db_decimal_tests db_decimal_tests.c)
# Add a DYNAMIC define to pass to the compile
target_compile_definitions(db_decimal_tests PRIVATE DYNAMIC)
target_include_directories(db_decimal_tests PRIVATE ${CMAKE_SOURCE_DIR}/interpreter)
target_include_directories(db_decimal_tests PRIVATE ${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin)
target_link_libraries(db_decimal_tests rxvmplugin db_decimal_manual)
#add_dependencies(db_decimal_tests db_decimal_dynamic)
#add_dependencies(db_decimal_tests db_decimal_manual)

# Enable Testing
enable_testing()

# DB Decimal Test
add_test(db_decimal_tests db_decimal_tests)
