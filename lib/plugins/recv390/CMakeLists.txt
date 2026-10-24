cmake_minimum_required(VERSION 3.24)
project(strings_plugin C)

set(CMAKE_C_STANDARD 99)
# -------------------------------------------------------------------------------------------
# Set plugin_name and test_rexx to match the specific plugin and its corresponding test case.
# -------------------------------------------------------------------------------------------
set(plugin_name "recv390")       # add plugin name
set(test_rexx "recv390_test")    # add rexx to test plugin
set(use_precomp TRUE)           # TRUE pre compile needed, FALSE if not

# ===========================================================================================
# The following sequence is generic and does not require modification.
#   plugin_template(${plugin_name} ${test_rexx} PRECOMP)
# ===========================================================================================
include(${CMAKE_SOURCE_DIR}/rxpa/CMakePluginTemplate.txt)

plugin_template(${plugin_name} ${test_rexx} ${PRECOMP_ARG} ${use_precomp})

enable_testing()
#Create the static link module
add_static_plugin_target(_recv390 recv390.c)

#Add include directories - static version
target_include_directories(_recv390_static PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/include
    ${CMAKE_SOURCE_DIR}/rxpa
)

crexx_install_targets(_recv390 _recv390_static)

if(BUILD_TESTING)
    add_executable(test_recv390_directory_max_userdata
        tests/test_directory_max_userdata.c)
    target_link_libraries(test_recv390_directory_max_userdata PRIVATE
        _recv390_static rxpashim rxbvml)
    add_test(NAME recv390_directory_max_userdata
        COMMAND ${CMAKE_COMMAND}
            "-DRECV390_TEST=$<TARGET_FILE:test_recv390_directory_max_userdata>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_directory_max_userdata.cmake")
    set_tests_properties(recv390_directory_max_userdata PROPERTIES
        LABELS "unit;plugins;recv390;regression" TIMEOUT 30)
    crexx_register_test_prep_targets(TESTS recv390_directory_max_userdata
        TARGETS test_recv390_directory_max_userdata)
endif()
