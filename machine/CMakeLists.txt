cmake_minimum_required(VERSION 3.00)

project(Machine C)

set(CMAKE_C_STANDARD 90)
#set(CMAKE_C_FLAGS_RELEASE "-O3")

message("-- " "Generating instruction set include files")
#file(REMOVE instrset.h instrmiss.h)

#add_custom_command(
#        COMMAND ${REXX} ${CMAKE_CURRENT_SOURCE_DIR}/instructionGen.rexx ${CMAKE_HOME_DIRECTORY}
#        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/instrmiss.h ${CMAKE_CURRENT_SOURCE_DIR}/instrset.h
#        COMMENT "Generating Instructions Set for the Interpreter..."
#)

add_library(machine STATIC rxvminst.h rxvminst.c instrset.h instrmiss.h)
