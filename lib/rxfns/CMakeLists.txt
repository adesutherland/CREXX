cmake_minimum_required(VERSION 3.18)

add_subdirectory(rexx)
add_subdirectory(rxas)
add_subdirectory(tests_functional)
add_subdirectory(tests_performance)

# Consolidate Rexx Library
set(SUBLIBS
        rexx/library-rexx.rxbin
        rxas/library-rxas.rxbin
        )

add_custom_command(
        OUTPUT library.rxbin
        COMMAND ${CMAKE_COMMAND} -E cat ${SUBLIBS} > library.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${SUBLIBS} rxas rxc
)
