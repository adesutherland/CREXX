foreach(required_var RXC RXAS RXVM RXBVM SOURCE WORK)
  if(NOT DEFINED ${required_var})
    message(FATAL_ERROR "Missing required variable: ${required_var}")
  endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
get_filename_component(source_name "${SOURCE}" NAME)
file(COPY "${SOURCE}" DESTINATION "${WORK}")
set(expected "43\n43\n-1\n42\n42\n1\n-1\n1\n42\n-1\n43\n")

foreach(compiler_mode opt noopt)
  set(compiler_flags)
  if(compiler_mode STREQUAL "noopt")
    list(APPEND compiler_flags -n)
  endif()
  execute_process(
    COMMAND "${RXC}" ${compiler_flags} -o "${compiler_mode}" "${source_name}"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "rxc ${compiler_mode} failed (${res}): ${out}${err}")
  endif()

  if(compiler_mode STREQUAL "opt")
    file(READ "${WORK}/${compiler_mode}.rxas" rxas)
    set(prefix "§inline_receiver_caller_field.session.")
    foreach(method_and_next "nested;loop" "loop;sideeffect" "sideeffect;factorynested"
                            "factorynested;planat")
      list(GET method_and_next 0 method)
      list(GET method_and_next 1 next_method)
      string(FIND "${rxas}" "${prefix}${method}() .locals=" body_start)
      string(FIND "${rxas}" "${prefix}${next_method}() .locals=" body_end)
      if(body_start LESS 0 OR body_end LESS_EQUAL body_start)
        message(FATAL_ERROR "Could not isolate optimized ${method} receiver body")
      endif()
      math(EXPR body_length "${body_end} - ${body_start}")
      string(SUBSTRING "${rxas}" ${body_start} ${body_length} body)
      if(body MATCHES "call[0-9]*[ \t]+[^\n]*(plan\\.next|frame\\.(advance|§factory))\\(\\)")
        message(FATAL_ERROR "Supported nested receiver calls did not inline in ${method}:\n${body}")
      endif()
      # a1 is the Session receiver under the method ABI. The captured _plan
      # must use that owner before entering either foreign inline receiver.
      if(NOT body MATCHES "linkattr1[ \t]+[ra][0-9]+,a1,1")
        message(FATAL_ERROR "${method} lost the Session-owned _plan capture:\n${body}")
      endif()
    endforeach()
  endif()

  foreach(assembler_mode opt noopt)
    set(assembler_flags)
    if(assembler_mode STREQUAL "noopt")
      list(APPEND assembler_flags -n)
    endif()
    set(image "${compiler_mode}_${assembler_mode}.rxbin")
    execute_process(
      COMMAND "${RXAS}" ${assembler_flags} -o "${image}" "${compiler_mode}.rxas"
      WORKING_DIRECTORY "${WORK}"
      OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
    if(NOT res EQUAL 0)
      message(FATAL_ERROR "rxas ${compiler_mode}/${assembler_mode} failed (${res}): ${out}${err}")
    endif()
    foreach(vm RXVM RXBVM)
      execute_process(
        COMMAND "${${vm}}" "${WORK}/${image}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
      string(REPLACE "\r\n" "\n" out "${out}")
      if(NOT res EQUAL 0 OR NOT out STREQUAL expected OR NOT err STREQUAL "")
        message(FATAL_ERROR
          "${vm} ${compiler_mode}/${assembler_mode} receiver mismatch (${res}):\n${out}${err}")
      endif()
    endforeach()
  endforeach()
endforeach()
