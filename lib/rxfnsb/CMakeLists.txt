cmake_minimum_required(VERSION 3.24)

add_subdirectory(rexx)
add_subdirectory(rxas)
# add_subdirectory(native)
add_subdirectory(tests_functional)
add_subdirectory(tests_performance)

# Consolidate Rexx Library
set(SUBLIBS
        rexx/library-rexx
        rxas/library-rxas
)

foreach (LIB ${SUBLIBS})
    set(LIB_FILES ${LIB_FILES} ${LIB}.rxbin)
endforeach ()

add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/bin/library.rxbin
        COMMAND ${CMAKE_COMMAND} -E cat ${LIB_FILES} > ${CMAKE_BINARY_DIR}/bin/library.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library-rexx library-rxas rexx/library-rexx.rxbin rxas/library-rxas.rxbin
        COMMENT "cREXX Combined Library Archive ..."
)

add_custom_target(library ALL DEPENDS ${CMAKE_BINARY_DIR}/bin/library.rxbin)
