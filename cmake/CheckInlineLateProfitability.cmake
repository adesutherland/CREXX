if(NOT DEFINED RXC OR NOT EXISTS "${RXC}")
  message(FATAL_ERROR "rxc executable was not provided")
endif()
if(NOT DEFINED RXAS OR NOT EXISTS "${RXAS}")
  message(FATAL_ERROR "rxas executable was not provided")
endif()
if(NOT DEFINED RXVM OR NOT EXISTS "${RXVM}")
  message(FATAL_ERROR "rxvm executable was not provided")
endif()
if(NOT DEFINED RXBVM OR NOT EXISTS "${RXBVM}")
  message(FATAL_ERROR "rxbvm executable was not provided")
endif()
if(NOT DEFINED SOURCE OR NOT EXISTS "${SOURCE}")
  message(FATAL_ERROR "late-profitability source was not provided")
endif()
if(NOT DEFINED INCLUDE_DIR OR NOT IS_DIRECTORY "${INCLUDE_DIR}")
  message(FATAL_ERROR "compiler include directory was not provided")
endif()
if(NOT DEFINED LIBRARY OR NOT EXISTS "${LIBRARY}")
  message(FATAL_ERROR "runtime library was not provided")
endif()
if(NOT DEFINED WORK)
  message(FATAL_ERROR "compiler inspection work directory was not provided")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

foreach(mode IN ITEMS opt noopt)
  set(output_base "${WORK}/inline_late_profitability_guard_${mode}")
  set(options)
  if(mode STREQUAL "noopt")
    list(APPEND options -n)
  endif()

  execute_process(
    COMMAND "${RXC}" ${options} -i "${INCLUDE_DIR}" -o "${output_base}" "${SOURCE}"
    OUTPUT_VARIABLE compiler_output
    ERROR_VARIABLE compiler_error
    RESULT_VARIABLE compiler_result)
  if(NOT compiler_result EQUAL 0)
    message(FATAL_ERROR
      "${mode} late-profitability compile failed:\n${compiler_output}${compiler_error}")
  endif()

  if(mode STREQUAL "opt")
    file(READ "${output_base}.rxas" optimized_rxas)
    if(optimized_rxas MATCHES
       "call[0-9]*[^\n]*inline_late_profitability_guard\\.addone\\(\\)")
      message(FATAL_ERROR "tiny addone() body did not retain the bounded inline path")
    endif()
    if(NOT optimized_rxas MATCHES
       "call[0-9]*[^\n]*inline_late_profitability_guard\\.probe\\.updateandsum\\(\\)")
      message(FATAL_ERROR
        "expansion-heavy updateandsum() site bypassed the late profitability gate")
    endif()
  endif()

  execute_process(
    COMMAND "${RXAS}" -o "${output_base}.rxbin" "${output_base}"
    OUTPUT_VARIABLE assembler_output
    ERROR_VARIABLE assembler_error
    RESULT_VARIABLE assembler_result)
  if(NOT assembler_result EQUAL 0)
    message(FATAL_ERROR
      "${mode} late-profitability assembly failed:\n${assembler_output}${assembler_error}")
  endif()

  foreach(vm IN ITEMS "${RXVM}" "${RXBVM}")
    execute_process(
      COMMAND "${vm}" "${output_base}.rxbin" "${LIBRARY}"
      OUTPUT_VARIABLE runtime_output
      ERROR_VARIABLE runtime_error
      RESULT_VARIABLE runtime_result)
    string(REPLACE "\r\n" "\n" runtime_output "${runtime_output}")
    if(NOT runtime_result EQUAL 0 OR
       NOT runtime_output STREQUAL "PASS: late inline profitability\n")
      message(FATAL_ERROR
        "${mode} late-profitability runtime mismatch for ${vm}:\n${runtime_output}${runtime_error}")
    endif()
  endforeach()
endforeach()

message(STATUS "late inline profitability admission and fallback are correct")
