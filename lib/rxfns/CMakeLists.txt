cmake_minimum_required(VERSION 3.18)

add_subdirectory(rexx)
add_subdirectory(rxas)
add_subdirectory(tests_functional)
add_subdirectory(tests_performance)

# Consolidate Rexx Library
set(SUBLIBS
        rexx/library.rxbin
        rxas/library.rxbin
        )

add_custom_target(rexxlibrary ALL
        DEPENDS lib_rexx lib_rxas)


execute_process (
        COMMAND ${CMAKE_COMMAND} -E cat ${SUBLIBS}
        OUTPUT_FILE library.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
