cmake_minimum_required(VERSION 3.24)

set(CREXX_VERSION_OVERRIDE "" CACHE STRING "Override CREXX base version; CI release builds pass the tag version without the leading v")
set(CREXX_BUILD_CHANNEL "local" CACHE STRING "Build channel embedded in the version string: release, dev-snapshot, or local")
set(CREXX_BUILD_TIMESTAMP "" CACHE STRING "UTC build timestamp for version metadata, formatted as YYYYMMDDTHHMMSSZ")
set(CREXX_BUILD_COMMIT "" CACHE STRING "Git commit SHA for version metadata")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" CREXX_VERSION_FILE_RAW)
string(STRIP "${CREXX_VERSION_FILE_RAW}" CREXX_VERSION_FILE)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")

if(CREXX_VERSION_OVERRIDE)
    string(STRIP "${CREXX_VERSION_OVERRIDE}" CREXX_VERSION_SOURCE)
else()
    set(CREXX_VERSION_SOURCE "${CREXX_VERSION_FILE}")
endif()

if(NOT CREXX_VERSION_SOURCE MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-[0-9A-Za-z.-]+)?(\\+([0-9A-Za-z.-]+))?$")
    message(FATAL_ERROR "CREXX version must be a semantic version such as 1.0.0, 1.0.0-beta.3, or 1.0.0-beta.3+build.7")
endif()
set(CREXX_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(CREXX_VERSION_MINOR "${CMAKE_MATCH_2}")
set(CREXX_VERSION_PATCH "${CMAKE_MATCH_3}")
set(CREXX_VERSION_PRERELEASE "${CMAKE_MATCH_4}")
set(CREXX_VERSION_BUILD_METADATA_BASE "${CMAKE_MATCH_6}")
set(CREXX_VERSION_CORE "${CREXX_VERSION_MAJOR}.${CREXX_VERSION_MINOR}.${CREXX_VERSION_PATCH}")
set(CREXX_VERSION_WITHOUT_BUILD "${CREXX_VERSION_CORE}${CREXX_VERSION_PRERELEASE}")

string(TOLOWER "${CREXX_BUILD_CHANNEL}" CREXX_BUILD_CHANNEL)
if(NOT CREXX_BUILD_CHANNEL MATCHES "^[0-9a-z][0-9a-z.-]*$")
    message(FATAL_ERROR "CREXX_BUILD_CHANNEL must contain only ASCII letters, digits, dots, and hyphens")
endif()

if(NOT CREXX_BUILD_TIMESTAMP)
    string(TIMESTAMP CREXX_BUILD_TIMESTAMP "%Y%m%dT%H%M%SZ" UTC)
    # Keep the auto-generated timestamp stable for this build tree. Otherwise
    # every CMake reconfigure rewrites crexx_version.h and forces broad rebuilds.
    set(CREXX_BUILD_TIMESTAMP "${CREXX_BUILD_TIMESTAMP}" CACHE STRING "UTC build timestamp for version metadata, formatted as YYYYMMDDTHHMMSSZ" FORCE)
endif()
if(NOT CREXX_BUILD_TIMESTAMP MATCHES "^[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]T[0-9][0-9][0-9][0-9][0-9][0-9]Z$")
    message(FATAL_ERROR "CREXX_BUILD_TIMESTAMP must be formatted as YYYYMMDDTHHMMSSZ")
endif()

if(NOT CREXX_BUILD_COMMIT)
    find_package(Git QUIET)
    if(Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
        execute_process(
                COMMAND "${GIT_EXECUTABLE}" rev-parse --verify HEAD
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE CREXX_BUILD_COMMIT
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
    endif()
endif()

set(CREXX_BUILD_COMMIT_SHORT "")
if(CREXX_BUILD_COMMIT)
    string(LENGTH "${CREXX_BUILD_COMMIT}" CREXX_BUILD_COMMIT_LENGTH)
    if(CREXX_BUILD_COMMIT MATCHES "^[0-9A-Fa-f]+$" AND CREXX_BUILD_COMMIT_LENGTH GREATER_EQUAL 7)
        if(CREXX_BUILD_COMMIT_LENGTH LESS 12)
            set(CREXX_BUILD_COMMIT_SHORT_LENGTH "${CREXX_BUILD_COMMIT_LENGTH}")
        else()
            set(CREXX_BUILD_COMMIT_SHORT_LENGTH 12)
        endif()
        string(SUBSTRING "${CREXX_BUILD_COMMIT}" 0 "${CREXX_BUILD_COMMIT_SHORT_LENGTH}" CREXX_BUILD_COMMIT_SHORT)
    else()
        message(WARNING "CREXX_BUILD_COMMIT is not a git SHA and will not be embedded: ${CREXX_BUILD_COMMIT}")
    endif()
endif()

set(CREXX_BUILD_DIRTY 0)
if(CREXX_BUILD_CHANNEL STREQUAL "local")
    find_package(Git QUIET)
    if(Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
        execute_process(
                COMMAND "${GIT_EXECUTABLE}" diff --quiet --ignore-submodules HEAD --
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                RESULT_VARIABLE CREXX_GIT_DIFF_RESULT
                OUTPUT_QUIET
                ERROR_QUIET)
        if(NOT CREXX_GIT_DIFF_RESULT EQUAL 0)
            set(CREXX_BUILD_DIRTY 1)
        endif()
    endif()
endif()

set(CREXX_VERSION_BUILD_METADATA_EXTRA "")
if(NOT CREXX_BUILD_CHANNEL STREQUAL "release")
    set(CREXX_VERSION_BUILD_METADATA_EXTRA "${CREXX_BUILD_CHANNEL}")
    if(CREXX_BUILD_COMMIT_SHORT)
        string(APPEND CREXX_VERSION_BUILD_METADATA_EXTRA ".g${CREXX_BUILD_COMMIT_SHORT}")
    endif()
    if(CREXX_BUILD_DIRTY)
        string(APPEND CREXX_VERSION_BUILD_METADATA_EXTRA ".dirty")
    endif()
endif()

if(CREXX_VERSION_BUILD_METADATA_EXTRA)
    if(CREXX_VERSION_BUILD_METADATA_BASE)
        set(CREXX_VERSION "${CREXX_VERSION_WITHOUT_BUILD}+${CREXX_VERSION_BUILD_METADATA_BASE}.${CREXX_VERSION_BUILD_METADATA_EXTRA}")
    else()
        set(CREXX_VERSION "${CREXX_VERSION_WITHOUT_BUILD}+${CREXX_VERSION_BUILD_METADATA_EXTRA}")
    endif()
else()
    if(CREXX_VERSION_BUILD_METADATA_BASE)
        set(CREXX_VERSION "${CREXX_VERSION_WITHOUT_BUILD}+${CREXX_VERSION_BUILD_METADATA_BASE}")
    else()
        set(CREXX_VERSION "${CREXX_VERSION_WITHOUT_BUILD}")
    endif()
endif()

set(CREXX_VERSION_SEMVER "${CREXX_VERSION}")
set(CREXX_VERSION_DISPLAY "crexx-${CREXX_VERSION}")
set(CREXX_BUILDINFO_PATH "${CMAKE_BINARY_DIR}/generated/BUILDINFO")
set(CREXX_VERSION_FILE_PATH "${CMAKE_BINARY_DIR}/generated/VERSION")

project(CREXX VERSION ${CREXX_VERSION_CORE})
set(CREXX_VERSION "${CREXX_VERSION_SEMVER}")

if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    set(CREXX_MATH_LIBRARY)
else()
    set(CREXX_MATH_LIBRARY m)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CREXX_CMAKE_PACKAGE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/CREXX")
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CREXXConfig.cmake.in"
        "${CMAKE_BINARY_DIR}/generated/CREXXConfig.cmake"
        INSTALL_DESTINATION "${CREXX_CMAKE_PACKAGE_INSTALL_DIR}")
write_basic_package_version_file(
        "${CMAKE_BINARY_DIR}/generated/CREXXConfigVersion.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMajorVersion)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # Fallback to local user bin to avoid sudo permission errors during testing
  if(WIN32)
    set(CMAKE_INSTALL_PREFIX "$ENV{USERPROFILE}/.local" CACHE PATH "Default install prefix" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Default install prefix" FORCE)
  endif()
endif()

# Print the CMake version
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "CREXX Version: ${CREXX_VERSION_DISPLAY}")
message(STATUS "CREXX Build Channel: ${CREXX_BUILD_CHANNEL}")
message(STATUS "CREXX Build Timestamp: ${CREXX_BUILD_TIMESTAMP}")
if(CREXX_BUILD_COMMIT_SHORT)
    message(STATUS "CREXX Build Commit: ${CREXX_BUILD_COMMIT_SHORT}")
endif()

# Print the CMake gererator
message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CrexxBuildResources.cmake)

# Debugging output - make cmake print all the commands it executes
#set(CMAKE_VERBOSE_MAKEFILE ON)

# Global Platform Options
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    if ("${CMAKE_CXX_PLATFORM_ID}" STREQUAL "MinGW")
        # Standards based printfs not MSVCRT
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__USE_MINGW_ANSI_STDIO=1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_MINGW_ANSI_STDIO=1")
    endif ()
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
endif ()

if(WIN32)
    set(EXE_SUFFIX ".exe")
else()
    set(EXE_SUFFIX "")
endif()

option(ENABLE_ODBC "Enable ODBC support" OFF)
option(ENABLE_GTK  "Enable GTK support"  OFF)
option(CREXX_ALLOW_NETWORK_DOWNLOADS
       "Allow explicitly enabled build features to download pinned dependencies"
       OFF)
option(CREXX_BUILD_SQLITE_ADDRESS_DEMO
       "Build the native SQLite ADDRESS demo; requires CREXX_ALLOW_NETWORK_DOWNLOADS=ON"
       OFF)
# These are source-controlled dependency pins, not user build options. Remove
# stale cache entries left by earlier builds where they were CACHE variables.
unset(CREXX_SQLITE_AMALGAMATION_YEAR CACHE)
unset(CREXX_SQLITE_AMALGAMATION_VERSION CACHE)
unset(CREXX_SQLITE_AMALGAMATION_SHA3_256 CACHE)
set(CREXX_SQLITE_AMALGAMATION_YEAR "2026")
set(CREXX_SQLITE_AMALGAMATION_VERSION "3530200")
set(CREXX_SQLITE_AMALGAMATION_SHA3_256 "81142986038e18f96c4a54e1a72562ae17e502a916f2a7701eff43388cbf1a40")
set(DSLSH_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../DSL-Syntax-Highlighter"
        CACHE PATH "Path to a local DSL-Syntax-Highlighter checkout")
option(DSLSH_PREFER_LOCAL "Prefer a local/sibling DSL-Syntax-Highlighter checkout when present" ON)
if(MSVC)
    set(CREXX_PARSER_MODE_DEFAULT OFF)
elseif(DSLSH_PREFER_LOCAL AND EXISTS "${DSLSH_LOCAL_DIR}/codebuffer/CMakeLists.txt")
    set(CREXX_PARSER_MODE_DEFAULT ON)
elseif(CREXX_ALLOW_NETWORK_DOWNLOADS)
    set(CREXX_PARSER_MODE_DEFAULT ON)
else()
    set(CREXX_PARSER_MODE_DEFAULT OFF)
endif()
option(ENABLE_PARSER_MODE "Enable Parser Mode (requires DSL-Syntax-Highlighter)"
        ${CREXX_PARSER_MODE_DEFAULT})
set(DSLSH_GIT_REPOSITORY "https://github.com/adesutherland/DSL-Syntax-Highlighter.git"
        CACHE STRING "DSL-Syntax-Highlighter Git repository")
set(DSLSH_GIT_TAG "develop"
        CACHE STRING "DSL-Syntax-Highlighter Git branch, tag, or commit SHA")

if (ENABLE_PARSER_MODE)
    if(DSLSH_PREFER_LOCAL AND EXISTS "${DSLSH_LOCAL_DIR}/codebuffer/CMakeLists.txt")
        message(STATUS "Using local DSL-Syntax-Highlighter at ${DSLSH_LOCAL_DIR}")
        set(DSLSH_SOURCE_DIR "${DSLSH_LOCAL_DIR}")
        add_subdirectory("${DSLSH_SOURCE_DIR}/codebuffer" "${CMAKE_CURRENT_BINARY_DIR}/dslsyntax-codebuffer" EXCLUDE_FROM_ALL)
    else()
        if(NOT CREXX_ALLOW_NETWORK_DOWNLOADS)
            message(FATAL_ERROR
                    "ENABLE_PARSER_MODE=ON requires a local DSL-Syntax-Highlighter checkout or CREXX_ALLOW_NETWORK_DOWNLOADS=ON")
        endif()
        include(FetchContent)
        message(STATUS "Fetching DSL-Syntax-Highlighter ${DSLSH_GIT_TAG} from ${DSLSH_GIT_REPOSITORY}")
        FetchContent_Declare(
            dsl_syntax_highlighter
            GIT_REPOSITORY ${DSLSH_GIT_REPOSITORY}
            GIT_TAG        ${DSLSH_GIT_TAG}
            SOURCE_SUBDIR  codebuffer
        )
        FetchContent_MakeAvailable(dsl_syntax_highlighter)
        FetchContent_GetProperties(dsl_syntax_highlighter)
        set(DSLSH_SOURCE_DIR "${dsl_syntax_highlighter_SOURCE_DIR}")
    endif()

    add_subdirectory("${DSLSH_SOURCE_DIR}/tools" "${CMAKE_CURRENT_BINARY_DIR}/dslsyntax-tools")
    if(TARGET dslsyntax_editor)
        target_include_directories(dslsyntax_editor PUBLIC "${DSLSH_SOURCE_DIR}/codebuffer")
    endif()
    if(TARGET dslsyntax_parser)
        target_include_directories(dslsyntax_parser PUBLIC "${DSLSH_SOURCE_DIR}/codebuffer")
    endif()
    if(NOT TARGET DSLSH::editor AND TARGET dslsyntax_editor)
        add_library(DSLSH::editor ALIAS dslsyntax_editor)
    endif()
    if(NOT TARGET DSLSH::parser AND TARGET dslsyntax_parser)
        add_library(DSLSH::parser ALIAS dslsyntax_parser)
    endif()
endif()

include(CTest)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CrexxQaTiers.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CrexxImportRoots.cmake)

# CREXX only uses re2c itself for scanner generation; do not build the
# language alias binaries by default.
foreach(re2c_alias IN ITEMS D GO HS JAVA JS OCAML PY RUST SWIFT V ZIG)
    set(RE2C_BUILD_RE2${re2c_alias} OFF CACHE BOOL "Build re2c language alias" FORCE)
endforeach()
add_subdirectory(re2c)
add_subdirectory(lemon)

# Suppress warnings for external dependencies (re2c, lemon) across compilers
# We do this here (top-level) so it applies regardless of how their subdirs are configured
if (TARGET lemon)
    target_compile_options(lemon PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/w>
        $<$<NOT:$<C_COMPILER_ID:MSVC>>:-w>
    )
endif()
foreach(ext_tgt IN ITEMS re2c re2go re2c_objects_autogen re2c_objects_autogen_ver_to_vernum)
    if (TARGET ${ext_tgt})
        target_compile_options(${ext_tgt} PRIVATE
            $<$<C_COMPILER_ID:MSVC>:/w>
            $<$<NOT:$<C_COMPILER_ID:MSVC>>:-w>
        )
    endif()
endforeach()

# Set global output directory for all binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CREXX_RXPA_PLUGIN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# The portable switch VM is the Clang/MSVC product default; GCC retains the
# measured direct-threaded default.  This is defined before subdirectories so
# every compiler, linker and runtime test can execute the stable rxvm product
# path without knowing which concrete engine supplied it.
set(CREXX_THREADED_VM_SUPPORTED OFF)
if(CMAKE_C_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
    set(CREXX_THREADED_VM_SUPPORTED ON)
endif()
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CREXX_DEFAULT_VM_TARGET rxtvm)
else()
    set(CREXX_DEFAULT_VM_TARGET rxbvm)
endif()
set(CREXX_RXVM_EXECUTABLE
        "$<TARGET_FILE_DIR:${CREXX_DEFAULT_VM_TARGET}>/rxvm${CMAKE_EXECUTABLE_SUFFIX}")

file(GLOB CREXX_MESSAGE_CATALOGS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/messages/*.msg")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/messages")
foreach(_crexx_message_catalog IN LISTS CREXX_MESSAGE_CATALOGS)
    get_filename_component(_crexx_message_catalog_name "${_crexx_message_catalog}" NAME)
    configure_file(
            "${_crexx_message_catalog}"
            "${CMAKE_BINARY_DIR}/bin/messages/${_crexx_message_catalog_name}"
            COPYONLY)
endforeach()
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/messages/"
        DESTINATION share/crexx/messages
        FILES_MATCHING PATTERN "*.msg")

option(CREXX_WINDOWS_LINK_REMOVE_RETRY
        "On Windows, remove existing link outputs with retry before relinking to tolerate transient test/AV locks"
        ON)
set(CREXX_WINDOWS_LINK_REMOVE_RETRIES "120" CACHE STRING
        "Retry count for removing locked Windows link outputs before relinking")
set(CREXX_WINDOWS_LINK_REMOVE_DELAY "0.5" CACHE STRING
        "Delay in seconds between Windows link-output remove retries")

if(WIN32 AND CREXX_WINDOWS_LINK_REMOVE_RETRY)
    set(CREXX_WINDOWS_LINK_REMOVE_COMMAND
            "\"${CMAKE_COMMAND}\""
            "-DREMOVE_PATHS=<TARGET>"
            "-DREMOVE_RETRIES=${CREXX_WINDOWS_LINK_REMOVE_RETRIES}"
            "-DREMOVE_DELAY=${CREXX_WINDOWS_LINK_REMOVE_DELAY}"
            "-DREMOVE_CONTEXT=Windows_relink_output"
            "-P" "\"${CMAKE_CURRENT_SOURCE_DIR}/cmake/CrexxRemoveWithRetry.cmake\"")
    string(REPLACE ";" " " CREXX_WINDOWS_LINK_REMOVE_PREFIX "${CREXX_WINDOWS_LINK_REMOVE_COMMAND}")

    set(CMAKE_C_LINK_EXECUTABLE
            "${CREXX_WINDOWS_LINK_REMOVE_PREFIX} && ${CMAKE_C_LINK_EXECUTABLE}")
    set(CMAKE_C_CREATE_SHARED_LIBRARY
            "${CREXX_WINDOWS_LINK_REMOVE_PREFIX} && ${CMAKE_C_CREATE_SHARED_LIBRARY}")
    set(CMAKE_C_CREATE_SHARED_MODULE
            "${CREXX_WINDOWS_LINK_REMOVE_PREFIX} && ${CMAKE_C_CREATE_SHARED_MODULE}")

    set(CMAKE_CXX_LINK_EXECUTABLE
            "${CREXX_WINDOWS_LINK_REMOVE_PREFIX} && ${CMAKE_CXX_LINK_EXECUTABLE}")
    set(CMAKE_CXX_CREATE_SHARED_LIBRARY
            "${CREXX_WINDOWS_LINK_REMOVE_PREFIX} && ${CMAKE_CXX_CREATE_SHARED_LIBRARY}")
    set(CMAKE_CXX_CREATE_SHARED_MODULE
            "${CREXX_WINDOWS_LINK_REMOVE_PREFIX} && ${CMAKE_CXX_CREATE_SHARED_MODULE}")
endif()

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/crexx_version.h.in
        ${CMAKE_BINARY_DIR}/generated/crexx_version.h
        @ONLY)
file(WRITE "${CREXX_VERSION_FILE_PATH}" "${CREXX_VERSION_DISPLAY}\n")
file(WRITE "${CREXX_BUILDINFO_PATH}"
        "display=${CREXX_VERSION_DISPLAY}\n"
        "version=${CREXX_VERSION}\n"
        "base_version=${CREXX_VERSION_SOURCE}\n"
        "version_file=${CREXX_VERSION_FILE}\n"
        "channel=${CREXX_BUILD_CHANNEL}\n"
        "timestamp=${CREXX_BUILD_TIMESTAMP}\n"
        "commit=${CREXX_BUILD_COMMIT}\n"
        "dirty=${CREXX_BUILD_DIRTY}\n")
include_directories(${CMAKE_BINARY_DIR}/generated)
install(FILES ${CMAKE_BINARY_DIR}/generated/crexx_version.h DESTINATION include)
install(FILES "${CREXX_VERSION_FILE_PATH}" "${CREXX_BUILDINFO_PATH}" DESTINATION .)

# Shared generated-artifact contracts used across subdirectory configuration
# boundaries. The compiler tests are declared before lib/classlib, so both the
# producer and its consumers must derive the native member path from here.
set(CREXX_CLASSLIB_NATIVE_MEMBER_ROOT
        "${CMAKE_BINARY_DIR}/lib/classlib/members/native")

if(BUILD_TESTING)
    # Tier preparation targets must exist before test directories are added so
    # their deferred tier finalizers can attach exact artifact prerequisites.
    add_custom_target(qa-prep)
    add_custom_target(qa-prep-essential)
    add_custom_target(qa-prep-smoke)
    add_custom_target(qa-prep-comprehensive)
    add_custom_target(qa-prep-qualification)
    add_custom_target(qa-prep-stress)
    add_custom_target(qa-prep-measurement)
    add_dependencies(qa-prep-smoke qa-prep-essential)
    add_dependencies(qa-prep-comprehensive qa-prep-smoke)
    add_dependencies(qa-prep
            qa-prep-comprehensive
            qa-prep-qualification
            qa-prep-stress)
endif()

add_subdirectory(platform)
add_subdirectory(avl_tree)
add_subdirectory(cpacker)
add_subdirectory(binutils)
add_subdirectory(contract)
add_subdirectory(assembler)
add_subdirectory(compiler)
add_subdirectory(linker)
add_subdirectory(disassembler)
add_subdirectory(debugger)
add_subdirectory(interpreter)
add_subdirectory(rxpa)
add_subdirectory(preprocessor)
add_subdirectory(lib)
add_subdirectory(rexxscript)
add_subdirectory(demos)
add_subdirectory(examples)
add_subdirectory(tests)
add_subdirectory(bin crexx_driver)

# These non-owning aggregate targets expose the product stages without
# replacing or duplicating the direct dependencies declared by their members.
include(${CMAKE_SOURCE_DIR}/cmake/CrexxBuildStages.cmake)

# Test directories register their exact generated-artifact requirements while
# assigning tiers. Attach those dependencies only after all producers and the
# non-owning stage aggregates exist.
if(BUILD_TESTING)
    crexx_finalize_qa_prep_targets()

    # Legacy hand-written tests share these explicit family closures. Generated
    # opt/noopt fixtures are already attached test-by-test by CrexxTestModes;
    # this list covers the direct harnesses and interpreter fixture families.
    crexx_add_qa_prep_dependencies(comprehensive
            REQUIRED_TARGETS
                stage-product
                crexx_test_driver
            OPTIONAL_TARGETS
                crexxsaa
                cri17_attached_provider_control-rxbvml
                cri17_attached_provider_control-rxvml
                db_decimal_tests
                decnumber_test
                address_llm_fixture_bin
                dynamic_interface_fixture_bin
                linein_stdin_harness
                linein_tty_harness
                mc_decimal_dyn_test_harness
                mc_decimal_full_tests
                mc_decimal_manual_test_harness
                mc_decimal_test_harness
                module_initializers-rxbvml
                module_initializers-rxtvml
                module_initializers-rxvml
                native_kv_address_demo
                platform_terminal_harness
                persistent_worker_executor-rxbvml
                persistent_worker_executor-rxtvml
                persistent_worker_executor-rxvml
                program_generation_control-rxbvml
                program_generation_control-rxtvml
                program_generation_control-rxvml
                rxbvm_instrumented
                rxc_classdecl_static
                rxc_staticlink
                signal_tester
                test_bridge
                test_e2_active_context
                test_float_pool_format
                test_highlight_cache
                test_highlight_editor_diagnostics
                test_import_discovery_snapshot
                test_import_resolution_report
                test_jtable_policy
                test_rxas_flow_graph
                test_rxcp_super_combiner
                test_rxcrexxcmd_flush
                test_rxgraph
                test_rxinteger_contract
                test_rxop_metadata
                test_rxpa_concurrency
                test_rxpa_odbc
                test_rxpa_odbc_old_host
                test_rxseqfile
                test_rxspawn_diagnostics
                test_rxvmactive
                test_rxvmbyteendpoint
                test_rxvmchannel_byte
                test_rxvmchannel_process
                test_rxvmchannel_registry
                test_rxvml_utf_boundaries
                test_rxvmmemory
                test_rxvmplugin_context
                test_rxvmstem
                test_rxvmstem_allocator_family
                test_rxvmworker
                test_signal_mask
                test_trace_env_bin
                test_trace_env_ignored_bin
                test_trace_env_invalid_bin
                test_trace_event_batch_bin
                test_trace_file_bin
                test_trace_normal_command_bin
                test_trace_normal_quiet_bin
                test_trace_source_jump_bin
                test_trace_stderr_bin
                tests_linked_srcfproc_artifact
                tests_linked_srcmethod_artifact
                rxdas_roundtrip_basic_artifact
                rxdas_roundtrip_binary_artifact
                rxdas_roundtrip_compact_format_artifact
                rxdas_roundtrip_float_pool_artifact
                rxdas_roundtrip_imported_artifact
                rxdas_roundtrip_int64_contract_artifact
                rxdas_roundtrip_interfaces_artifact
                rxdas_roundtrip_jtable_hash_artifact
                rxdas_roundtrip_jtable_linear_artifact
                rxdas_roundtrip_jtable_rexx_strings_artifact
                rxdas_roundtrip_metadata_artifact
                rxdas_roundtrip_wide_operands_artifact
                ts_linein_stdin_artifact
                odbc_mock_prepared_runtime
                _rxpa_bad_signatures
                _rxpa_classdecl
                _rxpa_invalid_manifest
                run_rxasjtablecorruptacphslot
                run_rxasjtablecorruptacphtarget
                run_rxasjtablecorruptalgorithm
                run_rxasjtablecorruptheader
                run_rxasjtablecorruptlinearkey
                run_rxasjtablecorruptlineartarget
                run_rxasjtablecorruptopenslot
                run_tests_attributes
                run_tests_basic
                run_tests_binary
                run_tests_binary_memory
                run_tests_breakpoint_address
                run_tests_compact_format
                run_tests_conversion
                run_tests_decimal
                run_tests_float_math
                run_tests_float_pool
                run_tests_instruction_gaps
                run_tests_int_contract
                run_tests_int_math
                run_tests_interfaces
                run_tests_jump_table
                run_tests_jump_table_acph_sources
                run_tests_jump_table_auto_edges
                run_tests_jump_table_auto_sources
                run_tests_jump_table_boundaries
                run_tests_jump_table_hash_edges
                run_tests_jump_table_linear_sources
                run_tests_jump_table_openhash_sources
                run_tests_jump_table_rexx_strings
                run_tests_logic_flow
                run_tests_nr09_superinstructions
                run_tests_object_initialization
                run_tests_objects
                run_tests_panic_source
                run_tests_perf2_exact_relink
                run_tests_perf2_partial_reference_materialization
                run_tests_perf2_reference_guards
                run_tests_references
                run_tests_reflection
                run_tests_register_flags
                run_tests_selector_descriptors
                run_tests_signal_actions
                run_tests_signal_branch_value
                run_tests_signal_call_attrs
                run_tests_signal_call_unwind
                run_tests_signal_frame_scope
                run_tests_signal_handler_modes
                run_tests_signal_mask_bounds
                run_tests_signal_nested_pending
                run_tests_signal_pushpop
                run_tests_signal_retry_retired
                run_tests_signal_silent_halt
                run_tests_signals
                run_tests_socket
                run_tests_string
                run_tests_system_io
                run_tests_utf_flags
                run_tests_wide_operands
                test_db_decimal_archive_link_bin
                test_gate_f_channel_local_bin
                test_gate_f_channel_process_bin
                test_module_initializer_bins
                test_program_generation_control_bins
                test_reentrancy_bin
                test_retired_process_opcode_bin
                test_persistent_worker_executor_bin
                test_persistent_worker_sparse_bin)

    crexx_add_qa_prep_dependencies(qualification
            REQUIRED_TARGETS stage-product performance_decimal_gate1)
    crexx_add_qa_prep_dependencies(stress
            REQUIRED_TARGETS rxvm run_tests_stress
            OPTIONAL_TARGETS
                persistent_worker_executor-rxvml
                persistent_worker_executor-rxbvml
                persistent_worker_executor-rxtvml
                test_persistent_worker_executor_bin
                test_persistent_worker_sparse_bin)
    crexx_add_qa_prep_dependencies(measurement
            OPTIONAL_TARGETS
                performance_binary_byvalue
                performance_binary_fastpath
                performance_binary_packed_float
                performance_classlib_iterator
                performance_classlib_treemap
                performance_inline
                performance_jump_dispatch
                performance_nr15_stem_access
                performance_nr15_stem_semantics
                performance_nr16_17_stable_state
                performance_packed_numeric_class
                performance_postperf04_scalar_access
                performance_rcc5f_rxstats
                performance_reference_iterator
                performance_runtime_lookup
                performance_rxjson_numeric
                performance_rxjson_parser
                performance_rxvector01
                performance_tinyexpr_dispatch)
endif()

if(BUILD_TESTING)
    # Build preparation is an explicit graph phase. CTest executes tests only;
    # it must not start a nested build in the active test worker pool.
    if(WIN32)
        set(_crexx_concurrency_default_test_jobs 1)
    else()
        set(_crexx_concurrency_default_test_jobs 10)
    endif()
    set(CREXX_CONCURRENCY_CTEST_JOBS
            "${_crexx_concurrency_default_test_jobs}" CACHE STRING
            "Parallel CTest jobs used by the deterministic concurrency QA target")
    if(NOT CREXX_CONCURRENCY_CTEST_JOBS MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR
                "CREXX_CONCURRENCY_CTEST_JOBS must be a positive integer")
    endif()

    add_custom_target(concurrency_test_artifacts)
    set(_crexx_concurrency_required_targets
            rxc rxas rxlink rxdas rxvm rxbvm
            library classlib rxfnsg
            linked_opt_runtime_artifacts
            test_rxgraph test_linked_format test_e2_active_context
            test_rxvmmemory test_rxvmworker test_rxvmchannel_registry
            test_rxvmchannel_process test_rxvmbyteendpoint
            test_rxvmchannel_byte test_rxvmactive
            test_rxvmplugin_context test_rxpa_concurrency
            test_gate_f_channel_local_bin test_gate_f_channel_process_bin
            test_retired_process_opcode_bin run_tests_basic
            program_generation_control-rxvml
            program_generation_control-rxbvml
            _rxpa_dynlink _rxpa_invalid_manifest
            testConcurrency testTaskContextEndpoint
            concurrency_doc_examples
            ts_address_array_lifecycle ts_address_spawn_transfer
            address_spawn_transfer_fixture
            ts_http_service_contract ts_llm_ollama ts_llm_providers
            ts_http_server ts_http_server_failures ts_http_pooled
            ts_http_streaming ts_http_crexx_rag ts_http_tls_live
            ts_http_policy ts_http_codec address_llm_fixture_bin)
    foreach(_crexx_concurrency_target IN LISTS
            _crexx_concurrency_required_targets)
        if(NOT TARGET "${_crexx_concurrency_target}")
            message(FATAL_ERROR
                    "Required concurrency QA target is missing: ${_crexx_concurrency_target}")
        endif()
        add_dependencies(concurrency_test_artifacts
                "${_crexx_concurrency_target}")
    endforeach()

    if(APPLE OR WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
        foreach(_crexx_concurrency_carrier_target IN ITEMS
                persistent_worker_executor-rxvml
                persistent_worker_executor-rxbvml)
            if(NOT TARGET "${_crexx_concurrency_carrier_target}")
                message(FATAL_ERROR
                        "Required concurrency carrier target is missing: ${_crexx_concurrency_carrier_target}")
            endif()
            add_dependencies(concurrency_test_artifacts
                    "${_crexx_concurrency_carrier_target}")
        endforeach()
    endif()

    foreach(_crexx_concurrency_optional_target IN ITEMS
            rxtvm test_import_discovery_snapshot
            program_generation_control-rxtvml
            persistent_worker_executor-rxtvml)
        if(TARGET "${_crexx_concurrency_optional_target}")
            add_dependencies(concurrency_test_artifacts
                    "${_crexx_concurrency_optional_target}")
        endif()
    endforeach()

    set(_crexx_concurrency_ctest_configuration)
    if(CMAKE_CONFIGURATION_TYPES)
        set(_crexx_concurrency_ctest_configuration -C "$<CONFIG>")
    endif()
    add_custom_target(concurrency-qa
            COMMAND ${CMAKE_CTEST_COMMAND}
                    --test-dir "${CMAKE_BINARY_DIR}"
                    ${_crexx_concurrency_ctest_configuration}
                    --parallel "${CREXX_CONCURRENCY_CTEST_JOBS}"
                    --output-on-failure
                    -L "^concurrency"
            DEPENDS concurrency_test_artifacts
            USES_TERMINAL
            COMMENT "Running the initial concurrency solution-point matrix")
endif()

install(EXPORT CREXXTargets
        FILE CREXXTargets.cmake
        NAMESPACE CREXX::
        DESTINATION "${CREXX_CMAKE_PACKAGE_INSTALL_DIR}")
install(FILES
        "${CMAKE_BINARY_DIR}/generated/CREXXConfig.cmake"
        "${CMAKE_BINARY_DIR}/generated/CREXXConfigVersion.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CrexxOperationContract.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/rxpa/RXPluginFunction.cmake"
        DESTINATION "${CREXX_CMAKE_PACKAGE_INSTALL_DIR}")

if(TARGET linked_opt_runtime_artifacts)
    # Linked-opt tests consume the shared runtime images; classlib member
    # rebuilds remove the consolidated image until the classlib target relinks it.
    foreach(_crexx_linked_opt_runtime_dependency IN ITEMS library classlib rxfnsc rexxscript)
        if(TARGET ${_crexx_linked_opt_runtime_dependency})
            add_dependencies(linked_opt_runtime_artifacts ${_crexx_linked_opt_runtime_dependency})
        endif()
    endforeach()

    add_custom_target(qa-prep-linked-opt-runtime)
    add_dependencies(qa-prep-linked-opt-runtime linked_opt_runtime_artifacts)
    if(TARGET qa-prep-compiler-exits)
        add_dependencies(qa-prep-comprehensive qa-prep-compiler-exits)
    endif()
    if(TARGET qa-prep-examples)
        add_dependencies(qa-prep-comprehensive qa-prep-examples)
    endif()

    if(TARGET linked_opt_sweep)
        add_dependencies(linked_opt_sweep qa-prep-linked-opt-runtime)
    endif()
endif()

if(BUILD_TESTING)
    # SAN-006 consumes both the native KeyDB member image and the published
    # native-backed aggregate. Keep those clean-tree inputs inside qa-prep
    # rather than relying on a preceding default build or a stale legacy path.
    add_custom_target(qa-prep-compiler-import-regressions)
    add_dependencies(qa-prep-compiler-import-regressions
            classlib_native)
    add_dependencies(qa-prep-comprehensive
            qa-prep-compiler-import-regressions)
endif()

if(BUILD_TESTING)
    if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
        set(_crexx_qa_default_jobs 30)
    else()
        set(_crexx_qa_default_jobs 5)
    endif()
    set(CREXX_QA_CTEST_JOBS "${_crexx_qa_default_jobs}" CACHE STRING
            "Parallel CTest jobs used by the named correctness QA targets")
    if(NOT CREXX_QA_CTEST_JOBS MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR "CREXX_QA_CTEST_JOBS must be a positive integer")
    endif()

    set(_crexx_qa_ctest_configuration)
    if(CMAKE_CONFIGURATION_TYPES)
        set(_crexx_qa_ctest_configuration -C "$<CONFIG>")
    endif()

    foreach(_crexx_measurement_target IN ITEMS
            test_persistent_worker_executor_bin
            persistent_worker_executor-rxvml
            persistent_worker_executor-rxbvml
            persistent_worker_executor-rxtvml)
        if(TARGET "${_crexx_measurement_target}")
            add_dependencies(qa-prep-measurement "${_crexx_measurement_target}")
        endif()
    endforeach()

    add_custom_target(qa-essential
            COMMAND ${CMAKE_CTEST_COMMAND}
                    --test-dir "${CMAKE_BINARY_DIR}"
                    ${_crexx_qa_ctest_configuration}
                    --parallel "${CREXX_QA_CTEST_JOBS}"
                    --output-on-failure
                    --label-regex "^essential$"
            DEPENDS qa-prep-essential
            USES_TERMINAL
            VERBATIM
            COMMENT "Running essential QA")
    add_custom_target(qa-smoke
            COMMAND ${CMAKE_CTEST_COMMAND}
                    --test-dir "${CMAKE_BINARY_DIR}"
                    ${_crexx_qa_ctest_configuration}
                    --parallel "${CREXX_QA_CTEST_JOBS}"
                    --output-on-failure
                    --label-regex "^(essential|smoke)$"
            DEPENDS qa-prep-smoke
            USES_TERMINAL
            VERBATIM
            COMMENT "Running essential and smoke QA")
    add_custom_target(qa-comprehensive
            COMMAND ${CMAKE_CTEST_COMMAND}
                    --test-dir "${CMAKE_BINARY_DIR}"
                    ${_crexx_qa_ctest_configuration}
                    --parallel "${CREXX_QA_CTEST_JOBS}"
                    --output-on-failure
                    --label-regex "^(essential|smoke|comprehensive)$"
            DEPENDS qa-prep-comprehensive
            USES_TERMINAL
            VERBATIM
            COMMENT "Running comprehensive correctness QA without stress or measurement")
    add_custom_target(qa-qualification
            COMMAND ${CMAKE_CTEST_COMMAND}
                    --test-dir "${CMAKE_BINARY_DIR}"
                    ${_crexx_qa_ctest_configuration}
                    --parallel "${CREXX_QA_CTEST_JOBS}"
                    --output-on-failure
                    --label-regex "^qualification$"
            DEPENDS qa-prep-qualification
            USES_TERMINAL
            VERBATIM
            COMMENT "Running qualification QA")
    add_custom_target(qa-stress
            COMMAND ${CMAKE_CTEST_COMMAND}
                    --test-dir "${CMAKE_BINARY_DIR}"
                    ${_crexx_qa_ctest_configuration}
                    --parallel "${CREXX_QA_CTEST_JOBS}"
                    --output-on-failure
                    --label-regex "^stress$"
            DEPENDS qa-prep-stress
            USES_TERMINAL
            VERBATIM
            COMMENT "Running explicit stress QA")
    add_custom_target(qa-measurement
            COMMAND ${CMAKE_CTEST_COMMAND}
                    --test-dir "${CMAKE_BINARY_DIR}"
                    ${_crexx_qa_ctest_configuration}
                    --parallel 1
                    --output-on-failure
                    --label-regex "^performance-measurement$"
            DEPENDS qa-prep-measurement
            USES_TERMINAL
            VERBATIM
            COMMENT "Running measurement tests serially; requires a quiescent host")
endif()

set(SOURCE_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)


# set the destination directory for your scripts in the build directory
set(DEST_SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Install target to copy binaries to /usr/local/bin
install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/
        DESTINATION bin
        FILES_MATCHING
        PATTERN "*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        # These exact names were historical test-only performance controls in
        # the shared build bin directory. Keep incremental stale artifacts out
        # of installs after their targets moved to tests/performance/.
        PATTERN "rx_rcc5f_stats_boxed.rxplugin" EXCLUDE
        PATTERN "rx_rcc5f_stats_direct.rxplugin" EXCLUDE
        PATTERN "rx_rxvector01_direct.rxplugin" EXCLUDE)
