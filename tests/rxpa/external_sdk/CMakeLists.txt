cmake_minimum_required(VERSION 3.24)
project(crexx_rxpa_external_sdk_consumer C CXX)

if(NOT DEFINED EXPECTED_CREXX_VERSION OR EXPECTED_CREXX_VERSION STREQUAL "")
    message(FATAL_ERROR "EXPECTED_CREXX_VERSION is required")
endif()
if(NOT DEFINED EXPECTED_CREXX_CORE_VERSION OR EXPECTED_CREXX_CORE_VERSION STREQUAL "")
    message(FATAL_ERROR "EXPECTED_CREXX_CORE_VERSION is required")
endif()
if(NOT DEFINED EXPECTED_CREXX_PREFIX OR EXPECTED_CREXX_PREFIX STREQUAL "")
    message(FATAL_ERROR "EXPECTED_CREXX_PREFIX is required")
endif()

find_package(CREXX ${EXPECTED_CREXX_CORE_VERSION} EXACT CONFIG REQUIRED)

if(NOT CREXX_VERSION_STRING STREQUAL EXPECTED_CREXX_VERSION)
    message(FATAL_ERROR
            "CREXX SDK version mismatch: expected ${EXPECTED_CREXX_VERSION}, found ${CREXX_VERSION_STRING}")
endif()
if(NOT CREXX_PACKAGE_PREFIX STREQUAL EXPECTED_CREXX_PREFIX)
    message(FATAL_ERROR
            "CREXX SDK prefix mismatch: expected ${EXPECTED_CREXX_PREFIX}, found ${CREXX_PACKAGE_PREFIX}")
endif()
if(NOT TARGET CREXX::RXPA OR NOT COMMAND add_dynamic_plugin_target)
    message(FATAL_ERROR "CREXX RXPA imported target or plugin helper is missing")
endif()

get_target_property(_crexx_rxpa_includes CREXX::RXPA INTERFACE_INCLUDE_DIRECTORIES)
foreach(_crexx_rxpa_include IN LISTS _crexx_rxpa_includes)
    cmake_path(IS_PREFIX EXPECTED_CREXX_PREFIX "${_crexx_rxpa_include}"
            NORMALIZE _crexx_installed_include)
    if(NOT _crexx_installed_include)
        message(FATAL_ERROR
                "Installed RXPA target exposes a non-prefix include: ${_crexx_rxpa_include}")
    endif()
endforeach()

set(_crexx_tool_manifest "")
foreach(_crexx_tool IN ITEMS rxc rxas rxvm rxbvm)
    if(NOT TARGET CREXX::${_crexx_tool})
        message(FATAL_ERROR "Installed CREXX::${_crexx_tool} target is missing")
    endif()
    get_target_property(_crexx_tool_path CREXX::${_crexx_tool} IMPORTED_LOCATION)
    if(NOT EXISTS "${_crexx_tool_path}")
        message(FATAL_ERROR "Installed CREXX::${_crexx_tool} path is missing: ${_crexx_tool_path}")
    endif()
    string(APPEND _crexx_tool_manifest "${_crexx_tool}=${_crexx_tool_path}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/crexx-sdk-paths.txt"
        "version=${CREXX_VERSION_STRING}\n"
        "prefix=${CREXX_PACKAGE_PREFIX}\n"
        "import_dir=${CREXX_IMPORT_DIR}\n"
        "plugin_dir=${CREXX_PLUGIN_DIR}\n"
        "runtime_dir=${CREXX_RUNTIME_DIR}\n"
        "includes=${_crexx_rxpa_includes}\n"
        "${_crexx_tool_manifest}")

set(CREXX_RXPA_PLUGIN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugin")
add_dynamic_plugin_target(_cri07_sdk_probe sdk_probe.c)

if(MSVC)
    target_compile_options(_cri07_sdk_probe PRIVATE /W4 /WX)
else()
    target_compile_options(_cri07_sdk_probe PRIVATE
            -Wall -Wextra -Werror -Wwrite-strings)
endif()

add_dynamic_plugin_target(_cri08_const_cpp sdk_const_probe.cpp)
target_compile_features(_cri08_const_cpp PRIVATE cxx_std_17)

add_library(_cri08_const_cpp_static OBJECT sdk_const_probe.cpp)
target_link_libraries(_cri08_const_cpp_static PRIVATE CREXX::RXPA)
target_compile_features(_cri08_const_cpp_static PRIVATE cxx_std_17)

foreach(_cri08_cpp_target IN ITEMS _cri08_const_cpp _cri08_const_cpp_static)
    if(MSVC)
        target_compile_options(${_cri08_cpp_target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${_cri08_cpp_target} PRIVATE -Wall -Wextra -Werror)
    endif()
endforeach()
add_custom_target(_cri08_const_consumers
        DEPENDS _cri08_const_cpp _cri08_const_cpp_static)
