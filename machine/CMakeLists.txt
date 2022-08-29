cmake_minimum_required(VERSION 3.00)

project(Machine C)

set(CMAKE_C_STANDARD 90)

find_program(REXX rexx)
if(NOT REXX)
    message(STATUS "NOTE: rexx not found - using existing instrmiss.h and instrset.h (in source tree, if they exist!)")
    add_custom_command(
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/instrmiss.h ${CMAKE_CURRENT_BINARY_DIR}/instrmiss.h
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/instrset.h ${CMAKE_CURRENT_BINARY_DIR}/instrset.h
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/instrmiss.h ${CMAKE_CURRENT_SOURCE_DIR}/instrset.h
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/instrmiss.h ${CMAKE_CURRENT_BINARY_DIR}/instrset.h
        COMMENT "Copying Instructions Set to binary directory ..."
    )
else()
    message(STATUS "NOTE: REXX found - using ${REXX}")
    add_custom_command(
        COMMAND ${REXX} ${CMAKE_CURRENT_SOURCE_DIR}/rxvmigen.rexx "${CMAKE_SOURCE_DIR},${CMAKE_BINARY_DIR}"
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rxvminst.c ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter/rxvmintp.c
            ${CMAKE_CURRENT_SOURCE_DIR}/rxvmigen.rexx
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/instrmiss.h ${CMAKE_CURRENT_BINARY_DIR}/instrset.h
        COMMENT "Generating Instructions Set for the Interpreter ..."
    )
endif()

add_custom_target(instrset ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/instrset.h)

add_library(machine STATIC rxvminst.h rxvminst.c ${CMAKE_CURRENT_BINARY_DIR}/instrset.h
        ${CMAKE_CURRENT_BINARY_DIR}/instrmiss.h ../platform/platform.h rxbin.h)
target_include_directories(machine PUBLIC . ../platform ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(machine instrset)
