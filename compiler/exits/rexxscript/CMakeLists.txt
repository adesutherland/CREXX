cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)
set(CLASSLIBPATH ${CMAKE_BINARY_DIR}/bin/classlib)
set(CLASSLIB_RXBIN ${CLASSLIBPATH}.rxbin)
set(RXFNSCPATH ${CMAKE_BINARY_DIR}/bin/rxfnsc)
set(RXFNSC_RXBIN ${RXFNSCPATH}.rxbin)
set(REXXSCRIPTPATH ${CMAKE_BINARY_DIR}/bin/rexxscript)
set(REXXSCRIPT_RXBIN ${REXXSCRIPTPATH}.rxbin)

crexx_add_compiler_exit_component(exit_rexxscript_bin RexxScript)

crexx_add_compiler_exit_runtime_test(
        test_rexxscript
        test_rexxscript.crexx
        EXTRA_DEPENDS ${CLASSLIB_RXBIN} ${RXFNSC_RXBIN} ${REXXSCRIPT_RXBIN}
        RUN_SUFFIX ${LIBRPATH} ${CLASSLIBPATH} ${RXFNSCPATH} ${REXXSCRIPTPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_rexxscript_direct
        test_rexxscript_direct.crexx
        EXTRA_DEPENDS exit_test_support_bin ${CLASSLIB_RXBIN} ${RXFNSC_RXBIN} ${REXXSCRIPT_RXBIN}
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${LIBRPATH} ${CLASSLIBPATH} ${RXFNSCPATH} ${REXXSCRIPTPATH}
)
