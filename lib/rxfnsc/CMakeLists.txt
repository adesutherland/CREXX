cmake_minimum_required(VERSION 3.24)

add_subdirectory(rexx)
add_subdirectory(rxas)
add_subdirectory(tests_functional)
add_subdirectory(tests_performance)

# Consolidate Rexx Library
set(SUBLIBS
        rexx/rxfnsc-rexx
        rxas/rxfnsc-rxas
)

foreach (LIB ${SUBLIBS})
    set(LIB_FILES ${LIB_FILES} ${LIB}.rxbin)
endforeach ()

add_custom_command(
        OUTPUT rxfnsc.rxbin
        COMMAND ${CMAKE_COMMAND} -E cat ${LIB_FILES} > rxfnsc.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS rxfnsc-rexx rxfnsc-rxas rexx/rxfnsc-rexx.rxbin rxas/rxfnsc-rxas.rxbin
        COMMENT "cREXX Combined Library Archive ..."
)

add_custom_target(rxfnsc ALL DEPENDS rxfnsc.rxbin)
