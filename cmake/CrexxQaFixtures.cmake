include_guard(GLOBAL)

# An ordinary donor is deliberately opt-in. Original/full builds retain every
# instrumented producer invocation. Only audited callers use this helper.
set(CREXX_QA_FIXTURE_TREE "" CACHE PATH "Prepared ordinary bytecode donor for runtime sanitizer QA")
set(CREXX_QA_FIXTURE_HELPER "${CMAKE_CURRENT_LIST_DIR}/../tools/sanitizer-fixtures.py")
string(TOUPPER "${CMAKE_BUILD_TYPE}" _qa_configuration)
set(_qa_native_flags "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS_${_qa_configuration}} ${CMAKE_CXX_FLAGS_${_qa_configuration}}")
if(_qa_native_flags MATCHES "-fsanitize=address")
    set(CREXX_QA_AUDIT_NATIVE_SOURCES TRUE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()
if(CREXX_QA_FIXTURE_TREE)
    if(CMAKE_CONFIGURATION_TYPES OR NOT CMAKE_GENERATOR STREQUAL "Ninja")
        message(FATAL_ERROR "External QA fixtures require the qualified single-config Ninja build")
    endif()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    get_filename_component(CREXX_QA_FIXTURE_TREE "${CREXX_QA_FIXTURE_TREE}" ABSOLUTE)
    file(WRITE "${CMAKE_BINARY_DIR}/.crexx-external-fixtures-used" "${CREXX_QA_FIXTURE_TREE}\n")
    add_custom_target(qa-fixture-identity
        COMMAND "${Python3_EXECUTABLE}" "${CREXX_QA_FIXTURE_HELPER}"
            validate --build-dir "${CMAKE_BINARY_DIR}"
        VERBATIM)
endif()

function(crexx_add_fixture_command)
    # Preserve argument boundaries when forwarding the ordinary command,
    # including literal/escaped semicolons in import paths.
    set(_original)
    math(EXPR _last "${ARGC} - 1")
    foreach(_i RANGE 0 ${_last})
        string(REPLACE ";" "\\;" _item "${ARGV${_i}}")
        list(APPEND _original "${_item}")
    endforeach()
    cmake_parse_arguments(PARSE_ARGV 0 FIX "VERBATIM;COMMAND_EXPAND_LISTS;USES_TERMINAL"
        "WORKING_DIRECTORY;COMMENT;JOB_POOL" "OUTPUT;BYPRODUCTS;DEPENDS;COMMAND")
    if(NOT FIX_OUTPUT)
        message(FATAL_ERROR "Audited QA fixture command requires OUTPUT")
    endif()
    set(_files)
    foreach(_file IN LISTS FIX_OUTPUT FIX_BYPRODUCTS)
        if(NOT _file MATCHES "\\.(rxbin|rxas|crexx|rexx|c|qa-output)$" OR _file MATCHES "\\$<")
            message(FATAL_ERROR "Unaudited QA fixture output: ${_file}")
        endif()
        get_filename_component(_absolute "${_file}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        file(RELATIVE_PATH _relative "${CMAKE_BINARY_DIR}" "${_absolute}")
        if(_relative MATCHES "^\\.\\./")
            message(FATAL_ERROR "QA fixture must belong to the build tree: ${_file}")
        endif()
        list(APPEND _files "${_relative}")
    endforeach()
    set_property(GLOBAL APPEND PROPERTY CREXX_QA_FIXTURE_FILES ${_files})
    if(NOT CREXX_QA_FIXTURE_TREE)
        add_custom_command(${_original})
        return()
    endif()
    if(FIX_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unaudited QA fixture command options: ${FIX_UNPARSED_ARGUMENTS}")
    endif()
    # TARGET_FILE creates implicit native prerequisites in the original rule.
    # Retain those prerequisites even though the execution is now a copy.
    string(REGEX MATCHALL "\\$<TARGET_FILE:[A-Za-z0-9_.+-]+>" _tools "${ARGN}")
    set(_depends ${FIX_DEPENDS} qa-fixture-identity
        "${CREXX_QA_FIXTURE_TREE}/qa-fixtures.json")
    foreach(_tool IN LISTS _tools)
        string(REGEX REPLACE "^\\$<TARGET_FILE:(.*)>$" "\\1" _target "${_tool}")
        list(APPEND _depends "${_target}")
    endforeach()
    foreach(_file IN LISTS _files)
        list(APPEND _depends "${CREXX_QA_FIXTURE_TREE}/${_file}")
    endforeach()
    add_custom_command(OUTPUT ${FIX_OUTPUT} BYPRODUCTS ${FIX_BYPRODUCTS}
        COMMAND "${Python3_EXECUTABLE}" "${CREXX_QA_FIXTURE_HELPER}"
            copy --build-dir "${CMAKE_BINARY_DIR}" --files ${_files}
        DEPENDS ${_depends}
        COMMENT "Taking checked ordinary QA fixtures"
        VERBATIM)
endfunction()

function(_crexx_qa_native_outputs directory)
    get_property(_targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_target IN LISTS _targets)
        get_target_property(_type "${_target}" TYPE)
        set(_path "")
        if(_type MATCHES "^(EXECUTABLE|STATIC_LIBRARY|SHARED_LIBRARY|MODULE_LIBRARY)$")
            set(_path "$<TARGET_FILE:${_target}>")
        endif()
        set_property(GLOBAL APPEND PROPERTY CREXX_QA_BUILD_TARGETS "${_target}\t${_type}\t${_path}")
        if(CREXX_QA_AUDIT_NATIVE_SOURCES AND NOT _type MATCHES "^(UTILITY|INTERFACE_LIBRARY)$")
            set_property(TARGET "${_target}" PROPERTY EXPORT_COMPILE_COMMANDS ON)
        endif()
        if(_type MATCHES "^(EXECUTABLE|STATIC_LIBRARY|SHARED_LIBRARY|MODULE_LIBRARY)$")
            set_property(GLOBAL APPEND PROPERTY CREXX_QA_NATIVE_OUTPUTS "$<TARGET_FILE:${_target}>")
        endif()
    endforeach()
    get_property(_children DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES)
    foreach(_child IN LISTS _children)
        _crexx_qa_native_outputs("${_child}")
    endforeach()
endfunction()

# CTest semantic PASS expressions can override a late nonzero sanitizer exit.
# Keep those semantic oracles and reject diagnostic headers before CTest output
# truncation, appending to any owning failure expressions. Ordinary builds retain
# their original properties.
function(crexx_apply_sanitizer_failure_guards)
    if(NOT CREXX_QA_AUDIT_NATIVE_SOURCES)
        return()
    endif()
    get_property(_tests DIRECTORY PROPERTY TESTS)
    foreach(_test IN LISTS _tests)
        set_property(TEST "${_test}" APPEND PROPERTY
            FAIL_REGULAR_EXPRESSION
            "ERROR: (AddressSanitizer|LeakSanitizer):"
            "SUMMARY: (AddressSanitizer|LeakSanitizer):"
            "AddressSanitizer:DEADLYSIGNAL"
            "AddressSanitizer: CHECK failed"
            "LeakSanitizer has encountered a fatal error")
    endforeach()
endfunction()

function(crexx_finalize_qa_fixtures)
    get_property(_files GLOBAL PROPERTY CREXX_QA_FIXTURE_FILES)
    list(REMOVE_DUPLICATES _files)
    list(SORT _files)
    string(JOIN "\n" _file_text ${_files})
    file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/qa-fixture-files.txt" CONTENT "${_file_text}\n")
    _crexx_qa_native_outputs("${CMAKE_SOURCE_DIR}")
    get_property(_build_targets GLOBAL PROPERTY CREXX_QA_BUILD_TARGETS)
    if(TARGET rxvm)
        list(APPEND _build_targets "rxvm\tUTILITY\t${CMAKE_BINARY_DIR}/bin/rxvm${CMAKE_EXECUTABLE_SUFFIX}")
        # Replace the utility entry with the concrete product alias entry.
        list(REMOVE_ITEM _build_targets "rxvm\tUTILITY\t")
    endif()
    string(JOIN "\n" _build_target_text ${_build_targets})
    set(_build_registry "${CMAKE_BINARY_DIR}/qa-build-targets.tsv")
    if(CMAKE_CONFIGURATION_TYPES)
        set(_build_registry "${CMAKE_BINARY_DIR}/qa-build-targets-$<CONFIG>.tsv")
    endif()
    file(GENERATE OUTPUT "${_build_registry}" CONTENT "${_build_target_text}\n")
    get_property(_native GLOBAL PROPERTY CREXX_QA_NATIVE_OUTPUTS)
    if(TARGET rxvm)
        list(APPEND _native "${CMAKE_BINARY_DIR}/bin/rxvm${CMAKE_EXECUTABLE_SUFFIX}")
    endif()
    list(REMOVE_DUPLICATES _native)
    string(JOIN "\n" _native_text ${_native})
    set(_native_inventory "${CMAKE_BINARY_DIR}/qa-native-files.txt")
    if(CMAKE_CONFIGURATION_TYPES)
        set(_native_inventory "${CMAKE_BINARY_DIR}/qa-native-files-$<CONFIG>.txt")
    endif()
    file(GENERATE OUTPUT "${_native_inventory}" CONTENT "${_native_text}\n")

    set(_config "{}")
    get_cmake_property(_variables VARIABLES)
    list(APPEND _variables CMAKE_BUILD_TYPE CMAKE_GENERATOR CMAKE_MAKE_PROGRAM CMAKE_BINARY_DIR CMAKE_HOME_DIRECTORY
        CMAKE_SYSTEM_NAME CMAKE_SYSTEM_PROCESSOR CMAKE_SIZEOF_VOID_P
        CMAKE_C_COMPILER CMAKE_C_COMPILER_ID CMAKE_C_COMPILER_VERSION CMAKE_C_COMPILER_TARGET
        CMAKE_CXX_COMPILER CMAKE_CXX_COMPILER_ID CMAKE_CXX_COMPILER_VERSION CMAKE_CXX_COMPILER_TARGET
        CMAKE_OSX_ARCHITECTURES CMAKE_OSX_DEPLOYMENT_TARGET CMAKE_OSX_SYSROOT
        DSLSH_SOURCE_DIR CREXX_QA_FIXTURE_TREE CREXX_VERSION_OVERRIDE
        CREXX_VERSION CREXX_VERSION_CORE CREXX_VERSION_DISPLAY CREXX_BUILD_CHANNEL
        CREXX_BUILD_TIMESTAMP CREXX_BUILD_COMMIT CREXX_BUILD_DIRTY)
    list(REMOVE_DUPLICATES _variables)
    foreach(_variable IN LISTS _variables)
        if(_variable MATCHES "^(CMAKE_(C|CXX|EXE_LINKER|SHARED_LINKER|MODULE_LINKER)_FLAGS($|_)|ENABLE_|CREXX_LEVEL_|CREXX_VM_)" OR
           _variable MATCHES "^(CMAKE_(BUILD_TYPE|GENERATOR|MAKE_PROGRAM|BINARY_DIR|HOME_DIRECTORY|SYSTEM_NAME|SYSTEM_PROCESSOR|SIZEOF_VOID_P|(C|CXX)_COMPILER(_ID|_VERSION|_TARGET)?|OSX_.*)|DSLSH_SOURCE_DIR|CREXX_QA_FIXTURE_TREE|CREXX_VERSION.*|CREXX_BUILD_(CHANNEL|TIMESTAMP|COMMIT|DIRTY))$")
            set(_value "${${_variable}}")
            string(REPLACE "\\" "\\\\" _value "${_value}")
            string(REPLACE "\"" "\\\"" _value "${_value}")
            string(REPLACE "\n" "\\n" _value "${_value}")
            string(JSON _config SET "${_config}" "${_variable}" "\"${_value}\"")
        endif()
    endforeach()
    file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/qa-fixture-config.json" CONTENT "${_config}\n")
    if(BUILD_TESTING)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        add_test(NAME sanitizer_strategy_contract
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/tests/test_sanitizer_strategy.py")
        set_tests_properties(sanitizer_strategy_contract PROPERTIES
            CREXX_PREP_NONE TRUE RUN_SERIAL TRUE TIMEOUT 300 LABELS "essential;sanitizer;contract"
            ENVIRONMENT "CREXX_QA_REQUIRE_ASAN=${CREXX_QA_AUDIT_NATIVE_SOURCES}")
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        add_test(NAME sanitizer_fixture_integrity
            COMMAND "${Python3_EXECUTABLE}" "${CREXX_QA_FIXTURE_HELPER}"
                validate --build-dir "${CMAKE_BINARY_DIR}" --artifacts)
        set_tests_properties(sanitizer_fixture_integrity PROPERTIES
            CREXX_PREP_NONE TRUE FIXTURES_SETUP qa-fixture-integrity TIMEOUT 120 LABELS "essential;sanitizer;contract")
    endif()
    crexx_apply_sanitizer_failure_guards()
endfunction()
