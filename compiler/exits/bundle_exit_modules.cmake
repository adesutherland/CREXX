if(NOT DEFINED OUTPUT)
    message(FATAL_ERROR "OUTPUT is required")
endif()

if(NOT DEFINED TEMP_OUTPUT)
    message(FATAL_ERROR "TEMP_OUTPUT is required")
endif()

if(NOT DEFINED INPUTS)
    message(FATAL_ERROR "INPUTS is required")
endif()

if(NOT DEFINED CMAKE_COMMAND_PATH)
    set(CMAKE_COMMAND_PATH "${CMAKE_COMMAND}")
endif()

if(NOT INPUTS)
    message(FATAL_ERROR "No compiler exit modules were provided")
endif()

foreach(_input IN LISTS INPUTS)
    if(NOT EXISTS "${_input}")
        message(FATAL_ERROR "Compiler exit module is missing: ${_input}")
    endif()
endforeach()

get_filename_component(_output_dir "${OUTPUT}" DIRECTORY)
if(_output_dir)
    file(MAKE_DIRECTORY "${_output_dir}")
endif()

file(REMOVE "${TEMP_OUTPUT}")
execute_process(
        COMMAND "${CMAKE_COMMAND_PATH}" -E cat ${INPUTS}
        OUTPUT_FILE "${TEMP_OUTPUT}"
        RESULT_VARIABLE _bundle_result
        ERROR_VARIABLE _bundle_error
)

if(NOT _bundle_result EQUAL 0)
    file(REMOVE "${TEMP_OUTPUT}")
    message(FATAL_ERROR "Failed to bundle compiler exits: ${_bundle_error}")
endif()

file(SIZE "${TEMP_OUTPUT}" _bundle_size)
if(_bundle_size EQUAL 0)
    file(REMOVE "${TEMP_OUTPUT}")
    message(FATAL_ERROR "Bundled compiler exits file is empty")
endif()

file(REMOVE "${OUTPUT}")
file(RENAME "${TEMP_OUTPUT}" "${OUTPUT}")
