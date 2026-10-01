cmake_minimum_required(VERSION 3.24)
project(provider_package_copy_control C)
include(CTest)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
get_filename_component(CREXX_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
add_library(copy_dependency SHARED dependency.c)
set_target_properties(copy_dependency PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS TRUE OUTPUT_NAME copy-dependency
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/a"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/a")
if(APPLE)
    set(origin "@loader_path")
    set_target_properties(copy_dependency PROPERTIES INSTALL_NAME_DIR "@rpath")
else()
    set(origin "$ORIGIN")
endif()
foreach(name a b)
    add_library(copy_root_${name} SHARED root.c)
    target_link_libraries(copy_root_${name} PRIVATE copy_dependency)
    set_target_properties(copy_root_${name} PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS TRUE
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${name}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${name}"
        BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${origin}")
endforeach()
set_target_properties(copy_dependency PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
add_custom_command(TARGET copy_root_b POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_if_different "$<TARGET_FILE:copy_dependency>"
    "$<TARGET_FILE_DIR:copy_root_b>/$<TARGET_FILE_NAME:copy_dependency>")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/input-$<CONFIG>.cmake" CONTENT
"set(platform \"${CMAKE_SYSTEM_NAME}\")\nset(provider copy_control)\nset(arch \"${CMAKE_SYSTEM_PROCESSOR}\")\nset(engine fixture)\nset(runtime_files \"$<TARGET_FILE:copy_root_a>\" \"$<TARGET_FILE:copy_root_b>\" \"$<TARGET_FILE:copy_dependency>\")\n")
add_test(NAME provider_package_copy COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/check.py"
    "${CMAKE_BINARY_DIR}/input-$<CONFIG>.cmake"
    "${CREXX_SOURCE}/rxpa/WriteProviderPackage.cmake"
    "${CMAKE_BINARY_DIR}/proof")
set_tests_properties(provider_package_copy PROPERTIES RUN_SERIAL TRUE TIMEOUT 300)
