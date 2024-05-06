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
        crexxsaa.h
        jsnemit.c
        jsnparse.c)

add_executable(httpserver httpserver.c)
target_link_libraries(httpserver event)
add_executable(httpclient httpclient.c
        jsnparse.c
        httpclient.h)

# Create an executable for the test
add_executable(test_parseJSON test_parseJSON.c jsnparse.c)
add_executable(test_emitJSON test_emitJSON.c jsnemit.c)
add_executable(test_emitparseJSON test_emitparseJSON.c jsnemit.c jsnparse.c)

# Add the test
add_test(NAME test_parseJSON COMMAND test_parseJSON)
add_test(NAME test_emitJSON COMMAND test_emitJSON)
add_test(NAME test_emitparseJSON COMMAND test_emitparseJSON)
