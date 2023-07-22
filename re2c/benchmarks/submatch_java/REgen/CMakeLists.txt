cmake_minimum_required(VERSION 3.0)

find_package(Java REQUIRED)
include(UseJava)

project(REgen)

set(CMAKE_JAVA_COMPILE_FLAGS "-encoding" "UTF8")

add_jar(REgen
    REgen.java
    REgenMain.java
    ENTRY_POINT REgenMain
)
