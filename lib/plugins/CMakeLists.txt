cmake_minimum_required(VERSION 3.24)

project(plugins NONE)

add_subdirectory(sysinfo)
add_subdirectory(arrays)
add_subdirectory(rxmath)
add_subdirectory(stack)
add_subdirectory(cipher)
add_subdirectory(rxuuid)
add_subdirectory(strings)
add_subdirectory(getpi)
add_subdirectory(precomp)
add_subdirectory(keyaccess)


if(ENABLE_ODBC)
    add_subdirectory(odbc)
    message(STATUS "Building with ODBC support enabled.")
else()
    message(STATUS "ODBC support is disabled, skipping subdirectory.")
endif()

if(ENABLE_GUI)
    add_subdirectory(gui)
    message(STATUS "Building with GTK Gui support enabled.")
else()
    message(STATUS "GTK Gui support is disabled, skipping subdirectory.")
endif()

if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")

endif()

if(WIN32)
	add_subdirectory(rxml)
#	add_subdirectory(regex)
#	add_subdirectory(pegparser)
	add_subdirectory(llist)
	add_subdirectory(fileio)
	add_subdirectory(matrix)
	add_subdirectory(rxtcp)
#	add_subdirectory(rxshm)
#	add_subdirectory(system)
#	add_subdirectory(pick)
#	add_subdirectory(vpool)
endif()

