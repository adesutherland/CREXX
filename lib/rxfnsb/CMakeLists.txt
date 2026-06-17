cmake_minimum_required(VERSION 3.24)

add_subdirectory(rexx)
add_subdirectory(rxas)
# add_subdirectory(native)
add_subdirectory(tests_functional)
add_subdirectory(tests_performance)

# Consolidate Rexx Library through the linker so the shared constant pool is
# converged before downstream consumers package it.
set(_library_control_file ${CMAKE_CURRENT_BINARY_DIR}/library_link_$<CONFIG>.ctl)
string(CONCAT _library_control_text
        "* Auto-generated control file for the consolidated cREXX library\n"
        "INPUT ${RXXFNSB_REXX_ARCHIVE}\n"
        "INPUT ${RXXFNSB_RXAS_ARCHIVE}\n")

foreach(_selector ${RXXFNSB_REXX_SELECTORS})
    string(APPEND _library_control_text "INCLUDE ${_selector}\n")
endforeach()
foreach(_selector ${RXXFNSB_RXAS_SELECTORS})
    string(APPEND _library_control_text "INCLUDE ${_selector}\n")
endforeach()

string(APPEND _library_control_text
        "PRESERVE INLINE\n"
        "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
        "OUTPUT ${CMAKE_BINARY_DIR}/bin/library\n")

file(GENERATE OUTPUT ${_library_control_file} CONTENT "${_library_control_text}")

add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/bin/library.rxbin
        COMMAND $<TARGET_FILE:rxlink> -c ${_library_control_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS rxlink library-rexx library-rxas "${RXXFNSB_REXX_ARCHIVE}" "${RXXFNSB_RXAS_ARCHIVE}" ${_library_control_file}
        COMMENT "cREXX Linked Library Image ..."
)

add_custom_target(library ALL DEPENDS ${CMAKE_BINARY_DIR}/bin/library.rxbin)
