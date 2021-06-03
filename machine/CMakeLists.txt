cmake_minimum_required(VERSION 3.00)

project(Machine C)

set(CMAKE_C_STANDARD 90)

find_program(REXX regina)
if(NOT REXX)
    message(STATUS "NOTE: Regina not found - using existing instrmiss.h and instrset.h (if they exist!)")
else()
    message(STATUS "NOTE: REXX Regina found - using ${REXX}")
    add_custom_command(
        COMMAND ${REXX} ${CMAKE_CURRENT_SOURCE_DIR}/rxvmigen.rexx ${CMAKE_HOME_DIRECTORY}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rxvminst.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../interpreter/rxvmintp.c
            ${CMAKE_CURRENT_SOURCE_DIR}/rxvmigen.rexx
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/instrmiss.h ${CMAKE_CURRENT_SOURCE_DIR}/instrset.h
        COMMENT "Generating Instructions Set for the Interpreter... using"
)
endif()

add_library(machine STATIC rxvminst.h rxvminst.c instrset.h instrmiss.h)
