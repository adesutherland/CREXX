cmake_minimum_required(VERSION 3.24)

project(RXVMPlugin C)

set(CMAKE_C_STANDARD 90)

# Add subdirectories first if needed
add_subdirectory(rxvmplugins)

# Add the rxvmplugin support library
add_library(rxvmplugin rxvmplugin_framework.c rxvmplugin.h rxvmplugin_framework.h)
target_include_directories(rxvmplugin PRIVATE .. ${CMAKE_SOURCE_DIR}/third_party/utf8 ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/binutils/include)
target_link_libraries(rxvmplugin PUBLIC platform PRIVATE rxvmref Threads::Threads)

if(BUILD_TESTING AND NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    include(${CMAKE_CURRENT_SOURCE_DIR}/RxvmPluginFunction.cmake)
    add_dynamic_rxvmplugin_target(sync_legacy_decimal tests/legacy_plugin.c)
    target_compile_definitions(sync_legacy_decimal PRIVATE LEGACY_PLUGIN_TYPE=1)
    add_dynamic_rxvmplugin_target(sync_legacy_unicode tests/legacy_plugin.c)
    target_compile_definitions(sync_legacy_unicode PRIVATE LEGACY_PLUGIN_TYPE=2)
    add_dynamic_rxvmplugin_target(sync_incompatible_decimal tests/incompatible_decimal.c)
    add_dynamic_rxvmplugin_target(sync_dual_registration tests/dual_registration.c)
    add_executable(test_decimal_registration tests/test_decimal_registration.c)
    target_include_directories(test_decimal_registration PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/interpreter
            ${CMAKE_SOURCE_DIR}/platform ${CMAKE_SOURCE_DIR}/third_party/utf8)
    target_link_libraries(test_decimal_registration PRIVATE rxvmplugin)
    add_dependencies(test_decimal_registration mc_decimal_dynamic db_decimal_dynamic
            sync_legacy_decimal sync_legacy_unicode sync_incompatible_decimal sync_dual_registration)
    add_test(NAME decimal_plugin_registration_v3
            COMMAND test_decimal_registration $<TARGET_FILE_DIR:mc_decimal_dynamic>
                rxvm_mc_decimal rxvm_db_decimal rxvm_sync_legacy_decimal
                rxvm_sync_legacy_unicode rxvm_sync_incompatible_decimal rxvm_sync_dual_registration)
    set_tests_properties(decimal_plugin_registration_v3 PROPERTIES TIMEOUT 30
            LABELS "interpreter;decimal;plugin;compatibility;regression"
            PASS_REGULAR_EXPRESSION "PASS: decimal ABI 3 registration and value-only synchronization")
    crexx_register_test_prep_targets(TESTS decimal_plugin_registration_v3
            TARGETS test_decimal_registration mc_decimal_dynamic db_decimal_dynamic
                sync_legacy_decimal sync_legacy_unicode sync_incompatible_decimal sync_dual_registration)
endif()

# Published products owned by this component.
crexx_install_targets(rxvmplugin)
