cmake_minimum_required(VERSION 3.24)

foreach(required IN ITEMS RXC RXAS RXVM RXBVM LIBRARY PLUGIN_DIR SOURCE_DIR WORK_ROOT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "Missing required -D${required}=...")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}")
set(import_path "${LIBRARY}\;${PLUGIN_DIR}")

function(run_checked label)
  execute_process(
    COMMAND ${ARGN}
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE result
  )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${label} failed (${result}):\n${out}${err}")
  endif()
endfunction()

set(invalid_cases
    bad_unnamed:arguments
    bad_separator:arguments
    bad_arg_type:arguments
    bad_return_type:return
    bad_empty_component:arguments
    bad_trailing_component:arguments)

foreach(source IN ITEMS
    rxpa_signature_valid.crexx
    rxpa_signature_bad_unnamed.crexx
    rxpa_signature_bad_separator.crexx
    rxpa_signature_bad_arg_type.crexx
    rxpa_signature_bad_return_type.crexx
    rxpa_signature_bad_empty_component.crexx
    rxpa_signature_bad_trailing_component.crexx)
  file(COPY_FILE "${SOURCE_DIR}/${source}" "${WORK_ROOT}/${source}" ONLY_IF_DIFFERENT)
endforeach()

foreach(mode IN ITEMS noopt opt)
  if(mode STREQUAL "noopt")
    set(mode_flag -n)
  else()
    set(mode_flag)
  endif()

  set(base "${WORK_ROOT}/rxpa_signature_valid_${mode}")
  run_checked(
    "${mode}/valid compile"
    "${RXC}" ${mode_flag} -i "${import_path}"
    -o "${base}" "${WORK_ROOT}/rxpa_signature_valid.crexx")
  run_checked(
    "${mode}/valid assemble"
    "${RXAS}" ${mode_flag} -o "${base}.rxbin" "${base}.rxas")

  foreach(runner IN ITEMS RXVM RXBVM)
    execute_process(
      COMMAND "${${runner}}" "${base}.rxbin"
              "${PLUGIN_DIR}/rx_rxpa_bad_signatures" "${LIBRARY}/library"
      WORKING_DIRECTORY "${WORK_ROOT}"
      OUTPUT_VARIABLE out
      ERROR_VARIABLE err
      RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
      message(FATAL_ERROR "${mode}/${runner} failed (${result}):\n${out}${err}")
    endif()
    if(NOT out STREQUAL "RXPA_SIGNATURE_VALID_OK\n")
      message(FATAL_ERROR "${mode}/${runner} output mismatch:\n${out}${err}")
    endif()
  endforeach()

  foreach(case_entry IN LISTS invalid_cases)
    string(REPLACE ":" ";" case_parts "${case_entry}")
    list(GET case_parts 0 case_name)
    list(GET case_parts 1 expected_field)
    execute_process(
      COMMAND "${RXC}" ${mode_flag} --no-localisation
              -i "${import_path}"
              -o "${WORK_ROOT}/${case_name}_${mode}"
              "${WORK_ROOT}/rxpa_signature_${case_name}.crexx"
      WORKING_DIRECTORY "${WORK_ROOT}"
      OUTPUT_VARIABLE out
      ERROR_VARIABLE err
      RESULT_VARIABLE result)
    if(result EQUAL 0)
      message(FATAL_ERROR "${mode}/${case_name} unexpectedly compiled")
    endif()
    set(diagnostic "${out}${err}")
    if(diagnostic MATCHES "INTERNAL_ERROR")
      message(FATAL_ERROR "${mode}/${case_name} remained an internal error:\n${diagnostic}")
    endif()
    if(NOT diagnostic MATCHES "RXPA_IMPORT_SIGNATURE_INVALID")
      message(FATAL_ERROR "${mode}/${case_name} lacked RXPA signature diagnostic:\n${diagnostic}")
    endif()
    if(NOT diagnostic MATCHES "field=\\\"${expected_field}\\\"")
      message(FATAL_ERROR "${mode}/${case_name} lacked field=${expected_field}:\n${diagnostic}")
    endif()
    if(NOT diagnostic MATCHES "import_file=\\\"rx_rxpa_bad_signatures.rxplugin\\\"")
      message(FATAL_ERROR "${mode}/${case_name} lacked plugin identity:\n${diagnostic}")
    endif()
    if(NOT diagnostic MATCHES "@[ ]+5:[0-9]+")
      message(FATAL_ERROR "${mode}/${case_name} lacked consumer location:\n${diagnostic}")
    endif()
  endforeach()
endforeach()
