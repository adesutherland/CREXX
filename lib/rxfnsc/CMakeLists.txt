cmake_minimum_required(VERSION 3.24)

add_subdirectory(rexx)
add_subdirectory(rxas)
add_subdirectory(tests_functional)
add_subdirectory(tests_performance)

# Consolidate Rexx Library
set(SUBLIBS
        rexx/libraryc-rexx
        rxas/libraryc-rxas
)

foreach (LIB ${SUBLIBS})
    set(LIB_FILES ${LIB_FILES} ${LIB}.rxbin)
endforeach ()

add_custom_command(
        OUTPUT libraryc.rxbin
        COMMAND ${CMAKE_COMMAND} -E cat ${LIB_FILES} > libraryc.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS libraryc-rexx libraryc-rxas rexx/libraryc-rexx.rxbin rxas/libraryc-rxas.rxbin
        COMMENT "cREXX Combined Library Archive ..."
)

add_custom_target(libraryc ALL DEPENDS libraryc.rxbin)
