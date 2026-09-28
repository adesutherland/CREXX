if(NOT DEFINED RXC OR NOT DEFINED RXAS OR NOT DEFINED RXBVM OR
   NOT DEFINED FIXTURE OR NOT DEFINED WORK_ROOT)
    message(FATAL_ERROR "source-root namespace test arguments are incomplete")
endif()

file(MAKE_DIRECTORY "${WORK_ROOT}")
foreach(order IN ITEMS first-second second-first)
    if(order STREQUAL "first-second")
        set(root0 "${FIXTURE}/first")
        set(root1 "${FIXTURE}/second")
        set(expected "ORDER PASS 35")
    else()
        set(root0 "${FIXTURE}/second")
        set(root1 "${FIXTURE}/first")
        set(expected "ORDER PASS 38")
    endif()
    set(stem "${WORK_ROOT}/${order}")
    execute_process(
        COMMAND "${RXC}" -x -s "${root0}" -s "${root1}"
                -o "${stem}" "${FIXTURE}/main.crexx"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "rxc ${order}: ${rc}\n${out}\n${err}")
    endif()
    execute_process(
        COMMAND "${RXAS}" -o "${stem}" "${stem}.rxas"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "rxas ${order}: ${rc}\n${out}\n${err}")
    endif()
    execute_process(
        COMMAND "${RXBVM}" "${stem}.rxbin"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT rc EQUAL 0 OR NOT out MATCHES "${expected}")
        message(FATAL_ERROR "rxbvm ${order}: ${rc}\n${out}\n${err}")
    endif()
endforeach()
