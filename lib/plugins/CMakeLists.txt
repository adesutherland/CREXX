cmake_minimum_required(VERSION 3.24)

project(plugins NONE)

#
# These are the plugins that are built for Windows, macOS and Linux
#
add_subdirectory(sysinfo)
add_subdirectory(arrays)
add_subdirectory(rxmath)
add_subdirectory(stack)
add_subdirectory(cipher)
add_subdirectory(strings)
add_subdirectory(getpi)
add_subdirectory(precomp)
add_subdirectory(keyaccess)
add_subdirectory(llist)
add_subdirectory(treemap)
add_subdirectory(id)
add_subdirectory(recv390)
add_subdirectory(map)
add_subdirectory(rxtcp)
add_subdirectory(socket)
add_subdirectory(system)

#
# These are the plugins that are additionally built for Linux
#
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # add_subdirectory(map)
#  add_subdirectory(console) probably misses xwindows libs
endif()

#
# These are the plugins that are additionally built for macOS
#
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#  add_subdirectory(console) probably misses xwindows libs
endif()

#
# These are the plugins that are additionally built for Windows (X64)
#
if(WIN32)
	add_subdirectory(rxml)
#	add_subdirectory(regex)
#	add_subdirectory(pegparser)
	add_subdirectory(fileio)
	add_subdirectory(matrix)
#	add_subdirectory(rxtcp)
#	add_subdirectory(rxshm)
#	add_subdirectory(pick)
#       add_subdirectory(pipe)
#       add_subdirectory(socket)
#       add_subdirectory(process)
#       add_subdirectory(subtask)
#	add_subdirectory(vpool)
        # add_subdirectory(map)
endif()

#
# These plugins are conditionally built, because they require external libraries
#

# specify -DENABLE_GTK=ON on the cmake commandline to build with GTK GUI support
if(ENABLE_GTK)
    add_subdirectory(gui)
    message(STATUS "Building with GTK Gui support enabled.")
else()
    message(STATUS "GTK Gui support is disabled, skipping subdirectory.")
endif()

# speficy -DENABLE_ODBC=ON on the cmake commandline to build with ODBC support
if(ENABLE_ODBC)
    add_subdirectory(odbc)
    message(STATUS "Building with ODBC support enabled.")
else()
    message(STATUS "ODBC support is disabled, skipping subdirectory.")
endif()
