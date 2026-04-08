cmake_minimum_required(VERSION 3.24)
project(CREXX)

# Print the CMake version
message(STATUS "CMake Version: ${CMAKE_VERSION}")

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
option(ENABLE_PARSER_MODE "Enable Parser Mode (requires DSL-Syntax-Highlighter)" ON)

if (ENABLE_PARSER_MODE)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../DSL-Syntax-Highlighter/CMakeLists.txt")
        message(STATUS "Found DSL-Syntax-Highlighter as sibling directory.")
        set(DSL_SH_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../DSL-Syntax-Highlighter")
        add_subdirectory(${DSL_SH_SOURCE_DIR} dsl-syntax-highlighter)
    else()
        message(STATUS "Downloading DSL-Syntax-Highlighter from GitHub...")
        include(FetchContent)
        FetchContent_Declare(
            dsl_syntax_highlighter
            GIT_REPOSITORY https://github.com/adesutherland/DSL-Syntax-Highlighter.git
            GIT_TAG        main
        )
        FetchContent_MakeAvailable(dsl_syntax_highlighter)
        set(DSL_SH_SOURCE_DIR ${dsl_syntax_highlighter_SOURCE_DIR})
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
foreach(ext_tgt IN ITEMS re2c re2go)
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

add_subdirectory(platform)
add_subdirectory(avl_tree)
add_subdirectory(cpacker)
add_subdirectory(assembler)
add_subdirectory(compiler)
add_subdirectory(disassembler)
add_subdirectory(debugger)
add_subdirectory(interpreter)
add_subdirectory(rxpa)
add_subdirectory(lib)
add_subdirectory(tests)
add_subdirectory(bin crexx_driver)

set(SOURCE_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)


# set the destination directory for your scripts in the build directory
set(DEST_SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # Fallback to local user bin to avoid sudo permission errors during testing
  if(WIN32)
    set(CMAKE_INSTALL_PREFIX "$ENV{USERPROFILE}/.local" CACHE PATH "Default install prefix" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Default install prefix" FORCE)
  endif()
endif()

# Install target to copy binaries to /usr/local/bin
install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/
        DESTINATION bin
        FILES_MATCHING
        PATTERN "*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
