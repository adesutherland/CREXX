foreach(required IN ITEMS RXC SOURCE WORK)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
execute_process(
    COMMAND "${RXC}" -n -x --no-exe-import -o "${WORK}/array_index_live_receiver" "${SOURCE}"
    RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Array index lifetime compile failed (${result}):\n${out}${err}")
endif()
file(READ "${WORK}/array_index_live_receiver.rxas" assembly)

# Derive storage identities from the emitted access rather than snapshotting
# register numbers. The receiver is linked before the index expression, so
# none of that expression's call results may overwrite it before consumption.
foreach(method IN ITEMS read write readargument readwindow readgrid)
    string(REGEX MATCH "indexstore\\.${method}\\(\\) \\.locals=[^\n]*\n([^\n]*\n)*" body "${assembly}")
    # A function header begins at column zero; metadata/instructions are indented.
    string(REGEX REPLACE "\n[^ \t\n][^\n]*([\n].*)?$" "" body "${body}")
    string(REGEX MATCH "linkattr1 (r[0-9]+),a1,[0-9]+" receiver_link "${body}")
    if(NOT receiver_link)
        message(FATAL_ERROR "Missing ${method} array receiver link")
    endif()
    set(receiver "${CMAKE_MATCH_1}")
    string(REGEX MATCH "minlinkattr1 r[0-9]+,${receiver}," access "${body}")
    if(NOT access)
        message(FATAL_ERROR "Missing ${method} access through linked receiver ${receiver}")
    endif()
    string(FIND "${body}" "${access}" access_at)
    string(SUBSTRING "${body}" 0 ${access_at} before_access)
    if(before_access MATCHES "call[0-9]* ${receiver},")
        message(FATAL_ERROR "${method} index call overwrites live receiver ${receiver}")
    endif()
endforeach()

# A multidimensional access links its first dimension before evaluating the
# next index. Protect that intermediate in both attribute and local arrays.
foreach(method IN ITEMS readgrid checklocal)
    string(REGEX MATCH "indexstore\\.${method}\\(\\) \\.locals=[^\n]*\n([^\n]*\n)*" body "${assembly}")
    string(REGEX REPLACE "\n[^ \t\n][^\n]*([\n].*)?$" "" body "${body}")
    # checklocal initializes its local first; select only the return expression.
    string(REGEX MATCH "[.]srcstep[^\n]*return [^\n]*\n([^\n]*\n)*" returned "${body}")
    string(REGEX MATCH "minlinkattr1 (r[0-9]+),r[0-9]+,r[0-9]+,0" first_link "${returned}")
    if(NOT first_link)
        message(FATAL_ERROR "Missing ${method} first dimension")
    endif()
    set(intermediate "${CMAKE_MATCH_1}")
    string(FIND "${returned}" "${first_link}" first_at)
    string(LENGTH "${first_link}" first_length)
    math(EXPR next_at "${first_at} + ${first_length}")
    string(SUBSTRING "${returned}" ${next_at} -1 after_first)
    string(REGEX MATCH "minlinkattr1 ${intermediate},${intermediate}," second_link "${after_first}")
    if(NOT second_link)
        message(FATAL_ERROR "Missing ${method} next dimension through ${intermediate}")
    endif()
    string(FIND "${after_first}" "${second_link}" second_at)
    string(SUBSTRING "${after_first}" 0 ${second_at} next_index)
    if(next_index MATCHES "call[0-9]* ${intermediate},")
        message(FATAL_ERROR "${method} next index overwrites linked dimension ${intermediate}")
    endif()
endforeach()

# A literal single-dimensional read needs just the property link and element
# link. Reserving storage earlier must not introduce snapshots or call machinery.
string(REGEX MATCH "indexstore\\.first\\(\\) \\.locals=[^\n]*\n([^\n]*\n)*" body "${assembly}")
string(REGEX REPLACE "\n[^ \t\n][^\n]*([\n].*)?$" "" body "${body}")
if(NOT body MATCHES "linkattr1 r[0-9]+,a1,[0-9]+" OR
   NOT body MATCHES "minlinkattr1 r[0-9]+,r[0-9]+,1" OR
   body MATCHES "[ \t](copy|swap|call[0-9]*) ")
    message(FATAL_ERROR "Literal array read gained unexpected receiver/call machinery")
endif()
message(STATUS "Array index receiver and intermediate lifetime shapes passed")
