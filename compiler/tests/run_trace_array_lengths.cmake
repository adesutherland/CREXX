foreach(required IN ITEMS RXC RXAS RXVM BIN_DIR LIBRARY SOURCE WORK)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is required")
    endif()
endforeach()
if(NOT DEFINED RXVM_TIMEOUT)
    set(RXVM_TIMEOUT 60)
endif()
file(MAKE_DIRECTORY "${WORK}")

function(array_trace_rxas_clause clause output)
    set(marker "\"${clause}\"\n")
    string(FIND "${assembly}" "${marker}" start)
    if(start LESS 0)
        message(FATAL_ERROR "Missing RXAS source anchor: ${clause}")
    endif()
    string(SUBSTRING "${assembly}" 0 ${start} before_source)
    string(REGEX MATCH "[^\n]*$" header "${before_source}")
    if(NOT header MATCHES "^[ \t]*[.]srcstep ([0-9]+) ([0-9]+) ")
        message(FATAL_ERROR "Invalid RXAS source owner for ${clause}: ${header}")
    endif()
    set(owner_step "${CMAKE_MATCH_1}")
    set(owner_clause "${CMAKE_MATCH_2}")
    string(LENGTH "${marker}" marker_length)
    math(EXPR start "${start} + ${marker_length}")
    string(SUBSTRING "${assembly}" ${start} -1 body)
    string(FIND "${body}" "   .srcstep " next)
    if(NOT next LESS 0)
        string(SUBSTRING "${body}" 0 ${next} body)
    endif()
    # Source identities are relational: every observation belongs to this
    # authored step/clause, independent of its numeric ids or register choice.
    string(REGEX MATCHALL "[.]traceevent [^\n]*" events "${body}")
    if(events STREQUAL "")
        message(FATAL_ERROR "Missing RXAS observations for ${clause}")
    endif()
    foreach(event IN LISTS events)
        if(NOT event MATCHES "^[.]traceevent \"[^\"]+\" [0-9]+ \"[^\"]*\" \"[^\"]*\" \"[^\"]*\" -?[0-9]+ ([0-9]+) ([0-9]+) ")
            message(FATAL_ERROR "Malformed RXAS observation for ${clause}: ${event}")
        endif()
        if(NOT "${CMAKE_MATCH_1}" STREQUAL "${owner_step}" OR
           NOT "${CMAKE_MATCH_2}" STREQUAL "${owner_clause}")
            message(FATAL_ERROR "Wrong RXAS source owner for ${clause}: ${event}")
        endif()
    endforeach()
    set(${output} "${body}" PARENT_SCOPE)
endfunction()

set(expected_values [=[>V>   "10"
>=>   "10"
>V>   "5"
>=>   "5"
>V>   "4"
>=>   "4"
>V>   "1"
>C>   "1"
>V>   "4"
>=>   "4"
>V>   "2"
>=>   "2"
>V>   "2"
>=>   "2"
>V>   "1"
>C>   "1"
>V>   "3"
>=>   "3"
>V>   "1"
>C>   "1"
>V>   "one"
>=>   "one"
>C>   "live"
>V>   "named"
>=>   "named"
]=])
set(expected_sources
    "fixed_length = fixed[]" "zero_length = zero[0]"
    "grid_length = grid[1, ]" "indexed_length = grid[index, ]"
    "open_length = open[]" "open_zero_length = open[0]"
    "matrix_length = matrix[index, ]" "value = open[index]"
    "stem_value = names.live")

foreach(mode IN ITEMS noopt opt)
    set(flags)
    if(mode STREQUAL "noopt")
        list(APPEND flags -n)
    endif()
    set(output_base "${WORK}/trace_array_lengths_${mode}")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env "RXCP_EXIT_MODULE=rxcexits"
                "${RXC}" -i "${BIN_DIR}" ${flags} -o "${output_base}" "${SOURCE}"
        WORKING_DIRECTORY "${WORK}"
        RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${mode} array TRACE compile failed (${result}):\n${out}${err}")
    endif()
    file(READ "${output_base}.rxas" assembly)
    string(REPLACE "\r\n" "\n" assembly "${assembly}")

    # Length selectors have no tail value. Preserve the integer result and
    # assignment observations, without accepting an unwritten NOVAL temporary.
    foreach(access IN ITEMS "fixed_length = fixed[]" "zero_length = zero[0]"
                            "grid_length = grid[1, ]" "open_length = open[]"
                            "open_zero_length = open[0]")
        array_trace_rxas_clause("  ${access}" body)
        if(body MATCHES "[.]traceevent \"C\" " OR
           NOT body MATCHES "[.]traceevent \"V\" 6 \"R\" \"I\" " OR
           NOT body MATCHES "[.]traceevent \"A\" 6 \"R\" \"I\" ")
            message(FATAL_ERROR "${mode} array length observations are invalid: ${access}\n${body}")
        endif()
    endforeach()

    # A real first index remains visible when a later dimension is omitted.
    # Follow its register identity instead of snapshotting register allocation.
    foreach(access IN ITEMS "indexed_length = grid[index, ]"
                            "matrix_length = matrix[index, ]" "value = open[index]")
        array_trace_rxas_clause("  ${access}" body)
        string(REGEX MATCH "[.]traceevent \"V\" 6 \"R\" \"I\" \"([rag])\" ([0-9]+) [^\n]* \"index\" \"\"" index_event "${body}")
        if(index_event STREQUAL "")
            message(FATAL_ERROR "${mode} real array index observation is missing: ${access}")
        endif()
        set(index_type "${CMAKE_MATCH_1}")
        set(index_register "${CMAKE_MATCH_2}")
        string(REGEX MATCHALL "[.]traceevent \"C\" [^\n]*" compounds "${body}")
        list(LENGTH compounds compound_count)
        if(NOT compound_count EQUAL 1 OR
           NOT body MATCHES "[.]traceevent \"C\" 4 \"R\" \"I\" \"${index_type}\" ${index_register} ")
            message(FATAL_ERROR "${mode} real array tail is not the sole live compound observation: ${access}\n${body}")
        endif()
    endforeach()
    array_trace_rxas_clause("  matrix_length = matrix[index, ]" body)
    string(REGEX MATCH "getattrs (r[0-9]+),(r[0-9]+),0\n[ \t]+unlink (r[0-9]+)\n[ \t]+icopy (r[0-9]+),(r[0-9]+)" cleanup "${body}")
    if(cleanup STREQUAL "" OR
       NOT "${CMAKE_MATCH_2}" STREQUAL "${CMAKE_MATCH_3}" OR
       NOT "${CMAKE_MATCH_2}" STREQUAL "${CMAKE_MATCH_4}" OR
       NOT "${CMAKE_MATCH_1}" STREQUAL "${CMAKE_MATCH_5}" OR
       "${CMAKE_MATCH_1}" STREQUAL "${CMAKE_MATCH_2}")
        message(FATAL_ERROR "${mode} open later-dimension length lost its linked-owner cleanup\n${body}")
    endif()
    array_trace_rxas_clause("  stem_value = names.live" body)
    if(NOT body MATCHES "[.]traceevent \"C\" 4 \"R\" \"S\" ")
        message(FATAL_ERROR "${mode} stem compound observation is missing\n${body}")
    endif()

    execute_process(
        COMMAND "${RXAS}" -o "${output_base}.rxbin" "${output_base}"
        WORKING_DIRECTORY "${WORK}"
        RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${mode} array TRACE assembly failed (${result}):\n${out}${err}")
    endif()
    execute_process(
        COMMAND "${RXVM}" "${LIBRARY}" "${output_base}.rxbin" -a 1
        WORKING_DIRECTORY "${WORK}"
        TIMEOUT "${RXVM_TIMEOUT}"
        RESULT_VARIABLE result OUTPUT_VARIABLE runtime_output ERROR_VARIABLE err)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${mode} array TRACE execution failed (${result}):\n${runtime_output}${err}")
    endif()
    string(REPLACE "\r\n" "\n" runtime_output "${runtime_output}")

    # TRACE drains pending values before presenting the next source header.
    # Compare the complete ordered value projection independently of headers;
    # extra C/L/O/F/P (or other value-prefix) records must fail this oracle.
    string(REGEX MATCHALL "(^|\n)[ \t]+>[^ \t\n]+>[ \t]+[^\n]*" value_lines "${runtime_output}")
    set(actual_values "")
    foreach(line IN LISTS value_lines)
        string(STRIP "${line}" line)
        string(APPEND actual_values "${line}\n")
    endforeach()
    if(NOT actual_values STREQUAL expected_values)
        message(FATAL_ERROR "${mode} ordered TRACE value mismatch\nExpected:\n${expected_values}\nActual:\n${actual_values}")
    endif()

    # Every authored access appears once and in order. The terminal TRACE OFF
    # header may be visible before observation is disabled.
    string(REGEX MATCHALL "(^|\n)[ ]*[0-9]+ [*]-[*] [^\n]*" source_lines "${runtime_output}")
    set(actual_sources)
    foreach(line IN LISTS source_lines)
        string(REGEX REPLACE "^[ \n]*[0-9]+ [*]-[*] " "" line "${line}")
        string(STRIP "${line}" line)
        list(APPEND actual_sources "${line}")
    endforeach()
    list(LENGTH actual_sources source_count)
    if(source_count GREATER 0)
        list(GET actual_sources -1 last_source)
        if(last_source STREQUAL "trace off")
            list(POP_BACK actual_sources)
        endif()
    endif()
    if(NOT actual_sources STREQUAL expected_sources)
        message(FATAL_ERROR "${mode} authored TRACE source order/count mismatch\nExpected:\n${expected_sources}\nActual:\n${actual_sources}")
    endif()
    if(NOT runtime_output MATCHES "\nRESULT 10 5 4 4 2 2 3 one named\n")
        message(FATAL_ERROR "${mode} independent array length/value result is incorrect\n${runtime_output}")
    endif()
endforeach()
