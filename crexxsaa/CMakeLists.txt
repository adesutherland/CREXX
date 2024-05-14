cmake_minimum_required(VERSION 3.27)
project(rexxsaa C)

set(CMAKE_C_STANDARD 90)

add_executable( httpserver httpserver.c
                jsnparse.c jsnemit.c
                httpclient.h
        httpcommon.c
        httpcommon.h)

add_executable( httpclient httpclient.c
                jsnparse.c jsnemit.c
                httpclient.h
        httpcommon.c
        httpcommon.h)

# Create an executable for the test
add_executable(test_parseJSON test_parseJSON.c jsnparse.c)
add_executable(test_emitJSON test_emitJSON.c jsnemit.c)
add_executable(test_emitparseJSON test_emitparseJSON.c jsnemit.c jsnparse.c)
add_executable(test_httpcommon test_httpcommon.c httpcommon.c)

# Add the test
add_test(NAME test_parseJSON COMMAND test_parseJSON)
add_test(NAME test_emitJSON COMMAND test_emitJSON)
add_test(NAME test_emitparseJSON COMMAND test_emitparseJSON)
add_test(NAME test_httpcommon COMMAND test_httpcommon)
