if(NOT DEFINED RXPP_BIN OR NOT DEFINED INPUT_FILE OR
   NOT DEFINED OUTPUT_FILE OR NOT DEFINED MACLIB_FILE)
    message(FATAL_ERROR "RXPP_BIN, INPUT_FILE, OUTPUT_FILE and MACLIB_FILE are required")
endif()

execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp -I "${INPUT_FILE}" -o "${OUTPUT_FILE}" -m "${MACLIB_FILE}"
    RESULT_VARIABLE rxpp_res
    OUTPUT_VARIABLE rxpp_out
    ERROR_VARIABLE rxpp_err)
if(NOT rxpp_res EQUAL 0)
    message(FATAL_ERROR "rxpp failed with ${rxpp_res}:\n${rxpp_out}${rxpp_err}")
endif()

file(READ "${OUTPUT_FILE}" output)
foreach(expected IN ITEMS
        "fortran_source.1='      PROGRAM DEMO'"
        "fortran_source.2='C keep this fixed-format comment'"
        "fortran_source.3=\"  100 PRINT *, 'hello'\""
        "fortran_source.4='      END'"
        "fortran_source.5='      SUBROUTINE SCALE(A,N)'"
        "fortran_source.6='      END'")
    string(FIND "${output}" "${expected}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Missing preserved FORTRAN line ${expected}:\n${output}")
    endif()
endforeach()
string(REGEX MATCHALL "call fortran_run fortran_source" calls "${output}")
list(LENGTH calls call_count)
if(NOT call_count EQUAL 1)
    message(FATAL_ERROR "Expected one deferred FORTRAN call, got ${call_count}:\n${output}")
endif()
string(FIND "${output}" "fortran_source.6='      END'" last_source_pos)
string(FIND "${output}" "call fortran_run fortran_source" call_pos)
string(FIND "${output}" "say 'After FORTRAN blocks'" after_pos)
if(call_pos LESS last_source_pos OR after_pos LESS call_pos OR
   output MATCHES "END FORTRAN")
    message(FATAL_ERROR "FORTRAN blocks were not collected and deferred in order:\n${output}")
endif()
