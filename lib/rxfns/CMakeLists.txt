cmake_minimum_required(VERSION 3.18)

add_subdirectory(rexx)
add_subdirectory(rxas)
add_subdirectory(native)
add_subdirectory(tests_functional)
add_subdirectory(tests_performance)

# Consolidate Rexx Library
set(SUBLIBS
        rexx/library-rexx
        rxas/library-rxas
)

foreach (LIB ${SUBLIBS})
    set(LIB_FILES ${LIB_FILES} ${LIB}".rxbin")
endforeach ()

add_custom_command(
        OUTPUT library.rxbin
        COMMAND ${CMAKE_COMMAND} -E cat ${LIB_FILES} > library.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${SUBLIBS}
        COMMENT "cREXX Combined Library Archive ..."
)

add_custom_target(library ALL DEPENDS library.rxbin)
