cmake_minimum_required(VERSION 3.24)

foreach(required IN ITEMS RXC RXAS RXVM RXBVM LIBRARY SOURCE_DIR WORK_ROOT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "Missing required -D${required}=...")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}")

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

foreach(import_mode IN ITEMS source binary)
  foreach(compile_mode IN ITEMS noopt opt)
    if(compile_mode STREQUAL "noopt")
      set(mode_flag -n)
    else()
      set(mode_flag)
    endif()

    set(work "${WORK_ROOT}/${import_mode}-${compile_mode}")
    file(MAKE_DIRECTORY "${work}")
    foreach(source IN ITEMS
        imported_record_contract.crexx
        imported_record_return_levelb.crexx
        imported_record_return_levelg.crexx
        imported_record_runner_levelb.crexx
        imported_record_runner_levelg.crexx
        imported_record_return_wrong_type.crexx)
      file(COPY_FILE "${SOURCE_DIR}/${source}" "${work}/${source}" ONLY_IF_DIFFERENT)
    endforeach()

    if(import_mode STREQUAL "binary")
      run_checked(
        "${import_mode}/${compile_mode} provider compile"
        "${RXC}" ${mode_flag} -i "${LIBRARY}" -o "${work}/imported_record_contract"
        "${work}/imported_record_contract.crexx"
      )
      run_checked(
        "${import_mode}/${compile_mode} provider assemble"
        "${RXAS}" ${mode_flag} -o "${work}/imported_record_contract.rxbin"
        "${work}/imported_record_contract"
      )
      file(REMOVE "${work}/imported_record_contract.crexx")
    endif()

    foreach(level IN ITEMS levelb levelg)
      run_checked(
        "${import_mode}/${compile_mode}/${level} consumer compile"
        "${RXC}" ${mode_flag} -i "${work}" -i "${LIBRARY}"
        -o "${work}/facade-${level}"
        "${work}/imported_record_return_${level}.crexx"
      )
      run_checked(
        "${import_mode}/${compile_mode}/${level} consumer assemble"
        "${RXAS}" ${mode_flag} -o "${work}/facade-${level}.rxbin"
        "${work}/facade-${level}"
      )
    endforeach()

    if(import_mode STREQUAL "source")
      run_checked(
        "${import_mode}/${compile_mode} provider compile"
        "${RXC}" ${mode_flag} -i "${LIBRARY}" -o "${work}/imported_record_contract"
        "${work}/imported_record_contract.crexx"
      )
      run_checked(
        "${import_mode}/${compile_mode} provider assemble"
        "${RXAS}" ${mode_flag} -o "${work}/imported_record_contract.rxbin"
        "${work}/imported_record_contract"
      )
    endif()

    foreach(level IN ITEMS levelb levelg)
      file(REMOVE "${work}/imported_record_return_${level}.crexx")
      run_checked(
        "${import_mode}/${compile_mode}/${level} runner compile"
        "${RXC}" ${mode_flag} -i "${work}" -i "${LIBRARY}"
        -o "${work}/main-${level}"
        "${work}/imported_record_runner_${level}.crexx"
      )
      run_checked(
        "${import_mode}/${compile_mode}/${level} runner assemble"
        "${RXAS}" ${mode_flag} -o "${work}/main-${level}.rxbin"
        "${work}/main-${level}"
      )
    endforeach()

    foreach(runner IN ITEMS RXVM RXBVM)
      foreach(level IN ITEMS levelb levelg)
        execute_process(
          COMMAND "${${runner}}" "${LIBRARY}/library.rxbin"
                  "${work}/imported_record_contract.rxbin"
                  "${work}/facade-${level}.rxbin"
                  "${work}/main-${level}.rxbin"
          WORKING_DIRECTORY "${work}"
          OUTPUT_VARIABLE out
          ERROR_VARIABLE err
          RESULT_VARIABLE result
        )
        if(NOT result EQUAL 0)
          message(FATAL_ERROR
            "${import_mode}/${compile_mode}/${level}/${runner} failed (${result}):\n${out}${err}")
        endif()
        if(NOT out STREQUAL "IMPORTED_RECORD_RETURN_OK level=${level}\n")
          message(FATAL_ERROR
            "${import_mode}/${compile_mode}/${level}/${runner} output mismatch:\n${out}${err}")
        endif()
      endforeach()
    endforeach()

    execute_process(
      COMMAND "${RXC}" ${mode_flag} -i "${work}" -i "${LIBRARY}"
              -o "${work}/wrong-type"
              "${work}/imported_record_return_wrong_type.crexx"
      WORKING_DIRECTORY "${work}"
      OUTPUT_VARIABLE out
      ERROR_VARIABLE err
      RESULT_VARIABLE result
    )
    if(result EQUAL 0)
      message(FATAL_ERROR
        "${import_mode}/${compile_mode} wrong-record return unexpectedly compiled")
    endif()
    set(diagnostic "${out}${err}")
    if(NOT diagnostic MATCHES "TYPE_MISMATCH")
      message(FATAL_ERROR
        "${import_mode}/${compile_mode} wrong-record return lacked TYPE_MISMATCH:\n${diagnostic}")
    endif()
  endforeach()
endforeach()
