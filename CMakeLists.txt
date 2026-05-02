cmake_minimum_required(VERSION 3.24)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" CREXX_VERSION_RAW)
string(STRIP "${CREXX_VERSION_RAW}" CREXX_VERSION)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
if(NOT CREXX_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)([-+][0-9A-Za-z.-]+)?$")
    message(FATAL_ERROR "VERSION must be a semantic version such as 1.0.0 or 1.0.0-beta.1")
endif()
set(CREXX_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(CREXX_VERSION_MINOR "${CMAKE_MATCH_2}")
set(CREXX_VERSION_PATCH "${CMAKE_MATCH_3}")
set(CREXX_VERSION_CORE "${CREXX_VERSION_MAJOR}.${CREXX_VERSION_MINOR}.${CREXX_VERSION_PATCH}")
set(CREXX_VERSION_DISPLAY "crexx-${CREXX_VERSION}")

project(CREXX VERSION ${CREXX_VERSION_CORE})

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

# Print the CMake gererator
message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")

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
option(CREXX_BUILD_SQLITE_ADDRESS_DEMO "Build the native SQLite ADDRESS demo; downloads the pinned SQLite amalgamation" ON)
set(CREXX_SQLITE_AMALGAMATION_YEAR "2026" CACHE STRING "SQLite amalgamation download year")
set(CREXX_SQLITE_AMALGAMATION_VERSION "3530000" CACHE STRING "SQLite amalgamation version code")
set(CREXX_SQLITE_AMALGAMATION_SHA3_256 "c2325c53b3b41761469f91cfb078e96882ac5d85bac10c11b0bd8f253b031e5b" CACHE STRING "SQLite amalgamation SHA3-256")
option(ENABLE_PARSER_MODE "Enable Parser Mode (requires DSL-Syntax-Highlighter)" ON)
set(DSLSH_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../DSL-Syntax-Highlighter"
        CACHE PATH "Path to a local DSL-Syntax-Highlighter checkout")
option(DSLSH_PREFER_LOCAL "Prefer a local/sibling DSL-Syntax-Highlighter checkout when present" ON)
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

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/crexx_version.h.in
        ${CMAKE_BINARY_DIR}/generated/crexx_version.h
        @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated)
install(FILES ${CMAKE_BINARY_DIR}/generated/crexx_version.h DESTINATION include)

add_subdirectory(platform)
add_subdirectory(avl_tree)
add_subdirectory(cpacker)
add_subdirectory(assembler)
add_subdirectory(compiler)
add_subdirectory(linker)
add_subdirectory(disassembler)
add_subdirectory(debugger)
add_subdirectory(interpreter)
add_subdirectory(rxpa)
add_subdirectory(lib)
add_subdirectory(demos)
add_subdirectory(tests)
add_subdirectory(bin crexx_driver)

if(TARGET linked_opt_runtime_artifacts)
    add_test(
            NAME linked_opt_runtime_artifacts_build
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target linked_opt_runtime_artifacts
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    # This test invokes a nested build; keep it out of the parallel test pool so
    # generated artifacts cannot race with tests that use the same build tree.
    set_tests_properties(linked_opt_runtime_artifacts_build PROPERTIES
            FIXTURES_SETUP linked_opt_runtime_artifacts
            RUN_SERIAL TRUE)
    if(TARGET linked_opt_sweep)
        add_dependencies(linked_opt_sweep linked_opt_runtime_artifacts)
    endif()
endif()

set(SOURCE_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)


# set the destination directory for your scripts in the build directory
set(DEST_SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Install target to copy binaries to /usr/local/bin
install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/
        DESTINATION bin
        FILES_MATCHING
        PATTERN "*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
