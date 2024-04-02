cmake_minimum_required(VERSION 3.27)
project(rexxsaapoc C)

set(CMAKE_C_STANDARD 90)

include_directories(libevent-2.1.12-stable/include)
include_directories(libevent-2.1.12-stable/compat)

# add_subdirectory(libevent-2.1.12-stable)  # Add this line

add_executable(server server.c
        common.h
        common.c)

add_executable(client client.c
        common.h
        common.c
        crexxsaa.h)

add_executable(httpserver httpserver.c)
target_link_libraries(httpserver event)
add_executable(httpclient httpclient.c
        json.c
        httpclient.h)
