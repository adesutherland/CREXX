if(NOT DEFINED RXPP OR NOT DEFINED MACLIB OR NOT DEFINED SOURCE OR NOT DEFINED SCRATCH)
    message(FATAL_ERROR "RXPP, MACLIB, SOURCE and SCRATCH are required")
endif()

file(READ "${SOURCE}" authored)
file(MAKE_DIRECTORY "${SCRATCH}")

function(check_invalid name original replacement expected)
    string(REPLACE "${original}" "${replacement}" mutated "${authored}")
    if(mutated STREQUAL authored)
        message(FATAL_ERROR "${name}: fixture did not change")
    endif()
    set(input "${SCRATCH}/${name}.rxpp")
    file(WRITE "${input}" "${mutated}")
    string(FIND "${mutated}" "${replacement}" offset)
    string(SUBSTRING "${mutated}" 0 ${offset} prefix)
    string(REGEX MATCHALL "\n" newlines "${prefix}")
    list(LENGTH newlines line)
    math(EXPR line "${line} + 1")
    execute_process(COMMAND "${RXPP}" rxprecomp -I "${input}"
            -o "${SCRATCH}/${name}.crexx" -m "${MACLIB}"
            RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
    set(diagnostic "${output}${error}")
    string(FIND "${diagnostic}" "${input}:${line}:" location)
    string(FIND "${diagnostic}" "RXPP_HANDLER_VALIDATION" code)
    string(FIND "${diagnostic}" "${expected}" explanation)
    if(NOT result EQUAL 8 OR location EQUAL -1 OR code EQUAL -1 OR explanation EQUAL -1)
        message(FATAL_ERROR "${name}: wrong result/location/diagnostic: ${result} ${diagnostic}")
    endif()
endfunction()

check_invalid(unknown "  LABEL path\n" "  MENU path\n" "unsupported DEFINE-GUI declaration")
check_invalid(duplicate "    LABEL lines\n" "    LABEL path\n" "duplicate or invalid control ID")
check_invalid(conflict
        "BUTTON open_again ACTION document.open EVENT document.open.requested SHORTCUT o"
        "BUTTON open_again ACTION document.open EVENT document.clear.requested SHORTCUT o"
        "conflicting command definition")
check_invalid(binding "  BIND path path\n" "  BIND path absent\n" "BIND references an unknown")
check_invalid(event "END DEFINE-GUI\n" "  ON CHANGE path FORWARD\nEND DEFINE-GUI\n" "text input ON requires")
check_invalid(row "  ROW\n" "  ROW extra\n" "malformed or nested ROW")
check_invalid(button "BUTTON clear ACTION document.clear EVENT document.clear.requested SHORTCUT c"
        "BUTTON clear ACTION document.clear EVENT document.clear.requested"
        "BUTTON requires ACTION, EVENT and SHORTCUT")
check_invalid(identifier "  LABEL path\n" "  LABEL path\"\n" "duplicate or invalid control ID")

set(first "${SCRATCH}/regeneration-a.crexx")
set(second "${SCRATCH}/regeneration-b.crexx")
foreach(output IN ITEMS "${first}" "${second}")
    execute_process(COMMAND "${RXPP}" rxprecomp -I "${SOURCE}" -o "${output}" -m "${MACLIB}"
            RESULT_VARIABLE result OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "positive regeneration failed: ${stdout}${stderr}")
    endif()
endforeach()
file(READ "${first}" a)
file(READ "${second}" b)
string(REGEX REPLACE "PRE Compiled on[^\n]*" "PRE Compiled on <timestamp>" a "${a}")
string(REGEX REPLACE "PRE Compiled on[^\n]*" "PRE Compiled on <timestamp>" b "${b}")
if(NOT a STREQUAL b)
    message(FATAL_ERROR "DEFINE-GUI regeneration differs with unchanged input")
endif()
string(FIND "${a}" "source.node_text(index)" current_value)
string(FIND "${a}" "return _delegate.update(event)" forwarding)
if(NOT a MATCHES "options[^\n]*levelg[^\n]*srcmap" OR
   current_value EQUAL -1 OR forwarding EQUAL -1)
    message(FATAL_ERROR "generated source lost source maps, current values or direct forwarding")
endif()
message(STATUS "PASS: DEFINE-GUI validation, authored locations and regeneration")
