cmake_minimum_required(VERSION 3.27)
project(rexxsaa C)

set(CMAKE_C_STANDARD 90)

add_executable( httpserver httpserver.c
                jsnparse.c jsnemit.c
                httpclient.h
        httpcommon.c
        httpcommon.h
        jsnemit.h
        httpserver.h)

add_executable( httpclient httpclient.c
                jsnparse.c jsnemit.c
                httpclient.h
        httpcommon.c
        httpcommon.h
        jsnemit.h)

# Create an executable for the test
add_executable(test_parseJSON test_parseJSON.c jsnparse.c httpcommon.c)
add_executable(test_emitJSON test_emitJSON.c jsnemit.c httpcommon.c)
add_executable(test_emitparseJSON test_emitparseJSON.c jsnemit.c jsnparse.c httpcommon.c)
add_executable(test_httpcommon test_httpcommon.c httpcommon.c)
add_executable(test_httpclient test_httpclient.c httpclient.c jsnemit.c jsnparse.c httpcommon.c)

# Add the test
add_test(NAME test_parseJSON COMMAND test_parseJSON)
add_test(NAME test_emitJSON COMMAND test_emitJSON)
add_test(NAME test_emitparseJSON COMMAND test_emitparseJSON)
add_test(NAME test_httpcommon COMMAND test_httpcommon)
add_test(NAME test_httpclient COMMAND test_httpclient)
