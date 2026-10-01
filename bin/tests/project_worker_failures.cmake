string(TIMESTAMP started "%s")
file(MAKE_DIRECTORY "${WORK_ROOT}/members")
file(COPY "${SOURCE}" DESTINATION "${WORK_ROOT}")
get_filename_component(worker_root "${WORKER}" DIRECTORY)
execute_process(COMMAND "${CREXX}" -i "${worker_root}" -l "${WORKER}" "${WORK_ROOT}/project_worker_failures.crexx" --args
                "${HELPER}" "${WORK_ROOT}/members"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "worker failure fixture returned ${rc}\n${out}\n${err}")
endif()
foreach(operation IN ITEMS "missing assembled project output" "write project stamp"
                           "rename project stamp" "PASS: all three worker failure operations")
    if(NOT "${out}" MATCHES "${operation}")
        message(FATAL_ERROR "missing diagnostic ${operation}\n${out}\n${err}")
    endif()
endforeach()
message(STATUS "${out}")

string(TIMESTAMP finished "%s")
math(EXPR elapsed "${finished} - ${started}")
message(STATUS "worker diagnostic matrix elapsed: ${elapsed} seconds")
