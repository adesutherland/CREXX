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
option(ENABLE_GUI  "Enable GUI support"  OFF)

include(CTest)
add_subdirectory(re2c)
add_subdirectory(lemon)
add_subdirectory(platform)
add_subdirectory(avl_tree)
add_subdirectory(cpacker)
add_subdirectory(compiler)
add_subdirectory(machine)
add_subdirectory(assembler)
add_subdirectory(disassembler)
add_subdirectory(debugger)
add_subdirectory(interpreter)
add_subdirectory(rxpa)
add_subdirectory(lib)
add_subdirectory(tests)
add_subdirectory(bin)

set(SOURCE_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)


# set the destination directory for your scripts in the build directory
set(DEST_SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR})
