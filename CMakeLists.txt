cmake_minimum_required(VERSION 3.16)
project(CREXX)

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
add_subdirectory(interpreter)
add_subdirectory(lib)
add_subdirectory(debugger)
add_subdirectory(tests)

