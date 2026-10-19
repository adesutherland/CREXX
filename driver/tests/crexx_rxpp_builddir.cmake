if(NOT DEFINED CREXX OR NOT DEFINED WORK_ROOT)
    message(FATAL_ERROR "CREXX and WORK_ROOT are required")
endif()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}/source")
file(WRITE "${WORK_ROOT}/source/builddir.rxpp" [=[##BUILDDIR output/relative
options levelb
say "BUILDDIR_OK"
]=])

execute_process(
        COMMAND "${CREXX}" "source/builddir.rxpp"
        WORKING_DIRECTORY "${WORK_ROOT}"
        RESULT_VARIABLE crexx_result
        OUTPUT_VARIABLE crexx_output
        ERROR_VARIABLE crexx_error)
if(NOT crexx_result EQUAL 0)
    message(FATAL_ERROR
            "crexx ##BUILDDIR pipeline failed with ${crexx_result}:\n"
            "stdout:\n${crexx_output}\nstderr:\n${crexx_error}")
endif()
if(NOT crexx_output MATCHES "BUILDDIR_OK")
    message(FATAL_ERROR
            "crexx ##BUILDDIR pipeline did not execute the program:\n${crexx_output}")
endif()

foreach(extension IN ITEMS crexx rxas rxbin)
    set(expected "${WORK_ROOT}/output/relative/builddir.${extension}")
    if(NOT EXISTS "${expected}")
        message(FATAL_ERROR "##BUILDDIR did not produce ${expected}")
    endif()
    if(EXISTS "${WORK_ROOT}/source/builddir.${extension}")
        message(FATAL_ERROR
                "##BUILDDIR left builddir.${extension} beside the source")
    endif()
endforeach()
