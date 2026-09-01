file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

foreach(mode IN ITEMS opt noopt)
  set(flag)
  if(mode STREQUAL "noopt")
    set(flag -n)
  endif()

  execute_process(
    COMMAND "${RXC}" -i "${LIBRARY_DIR}" ${flag}
            -o "${WORK}/codegen_${mode}" "${CODEGEN_SOURCE}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "${mode} codegen compile failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
  endif()

  execute_process(
    COMMAND "${RXAS}" -o "${WORK}/codegen_${mode}.rxbin" "${WORK}/codegen_${mode}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "${mode} codegen assemble failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
  endif()

  execute_process(
    COMMAND "${RXC}" -i "${LIBRARY_DIR}" ${flag}
            -o "${WORK}/semantics_${mode}" "${SEMANTICS_SOURCE}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "${mode} semantics compile failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
  endif()

  execute_process(
    COMMAND "${RXAS}" -o "${WORK}/semantics_${mode}.rxbin" "${WORK}/semantics_${mode}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "${mode} semantics assemble failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
  endif()
endforeach()

foreach(mode IN ITEMS opt noopt)
  file(READ "${WORK}/codegen_${mode}.rxas" codegen_rxas)
  if(NOT codegen_rxas MATCHES
     [=[\.meta "perf2_07_binary_byvalue_codegen\.readbyvalue"="\.inline" "I6;c,[^\n]*,7,0,400,3,0,400;f,]=])
    message(FATAL_ERROR "${mode} read-only binary summary did not export the exact non-escaping flag mask 400")
  endif()

  file(READ "${WORK}/semantics_${mode}.rxas" semantics_rxas)
  if(NOT semantics_rxas MATCHES
     [=[\.meta "perf2_07_binary_byvalue_semantics\.overwritelocal"="\.inline" "I6;c,[^\n]*,7,0,416;f,]=])
    message(FATAL_ERROR "${mode} writable binary summary did not export the written flag mask 416")
  endif()
endforeach()

file(READ "${WORK}/codegen_opt.rxas" opt_rxas)
string(REGEX MATCH
       "\\.meta \"perf2_07_binary_byvalue_codegen\\.main\\.data\"=\"b\" \"\\.binary\" (r[0-9]+)"
       data_meta "${opt_rxas}")
if(NOT data_meta)
  message(FATAL_ERROR "optimized RXAS did not expose the main.data register")
endif()
set(data_register "${CMAKE_MATCH_1}")

string(REGEX MATCH "bgetu32 [^,\n]+,${data_register}," direct_read "${opt_rxas}")
if(NOT direct_read)
  message(FATAL_ERROR "optimized read-only inline body did not read the caller binary register directly")
endif()

string(REGEX MATCH "(^|\n)[ \t]+copy [^,\n]+,${data_register}([\r\n]|$)" defensive_copy "${opt_rxas}")
if(defensive_copy)
  message(FATAL_ERROR "optimized read-only inline body retained a defensive binary copy: ${defensive_copy}")
endif()

string(REGEX MATCH "(^|\n)[ \t]+endlife ${data_register}([\r\n]|$)" aliased_endlife "${opt_rxas}")
if(aliased_endlife)
  message(FATAL_ERROR "optimized inline formal ended the caller binary lifetime")
endif()

file(READ "${WORK}/codegen_noopt.rxas" noopt_rxas)
string(FIND "${noopt_rxas}" "call2" noopt_call)
if(noopt_call EQUAL -1)
  message(FATAL_ERROR "unoptimized control did not retain the by-value call boundary")
endif()

execute_process(
  COMMAND "${RXC}" -i "${LIBRARY_DIR}"
          -o "${WORK}/perf2_07_binary_byvalue_import_dep" "${IMPORT_DEP_SOURCE}"
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "import dependency compile failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
endif()

file(READ "${WORK}/perf2_07_binary_byvalue_import_dep.rxas" import_dep_rxas)
if(NOT import_dep_rxas MATCHES
   [=[\.meta "perf2_07_binary_byvalue_import_dep\.readonly"="\.inline" "I6;c,[^\n]*,7,0,400;f,]=])
  message(FATAL_ERROR "import dependency did not export read-only binary flag mask 400")
endif()
if(NOT import_dep_rxas MATCHES
   [=[\.meta "perf2_07_binary_byvalue_import_dep\.mutate"="\.inline" "I6;c,[^\n]*,7,0,416;f,]=])
  message(FATAL_ERROR "import dependency did not export writable binary flag mask 416")
endif()

execute_process(
  COMMAND "${RXAS}" -o "${WORK}/perf2_07_binary_byvalue_import_dep.rxbin"
          "${WORK}/perf2_07_binary_byvalue_import_dep"
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "import dependency assemble failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
endif()

execute_process(
  COMMAND "${RXDAS}" -o "${WORK}/perf2_07_binary_byvalue_import_dep.roundtrip.rxas"
          "${WORK}/perf2_07_binary_byvalue_import_dep.rxbin"
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "import dependency disassemble failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
endif()

file(READ "${WORK}/perf2_07_binary_byvalue_import_dep.roundtrip.rxas" import_roundtrip_rxas)
foreach(expected IN ITEMS
        [=[perf2_07_binary_byvalue_import_dep\.readonly"="\.inline" "I6;c,[^\n]*,7,0,400;f,]=]
        [=[perf2_07_binary_byvalue_import_dep\.mutate"="\.inline" "I6;c,[^\n]*,7,0,416;f,]=])
  if(NOT import_roundtrip_rxas MATCHES "${expected}")
    message(FATAL_ERROR "RXDAS round trip lost binary callable-summary evidence: ${expected}")
  endif()
endforeach()

execute_process(
  COMMAND "${RXAS}" -o "${WORK}/perf2_07_binary_byvalue_import_dep.roundtrip.rxbin"
          "${WORK}/perf2_07_binary_byvalue_import_dep.roundtrip.rxas"
  OUTPUT_VARIABLE out
  ERROR_VARIABLE err
  RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  message(FATAL_ERROR "round-trip import dependency assemble failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
endif()
file(REMOVE "${WORK}/perf2_07_binary_byvalue_import_dep.rxbin")
file(RENAME
     "${WORK}/perf2_07_binary_byvalue_import_dep.roundtrip.rxbin"
     "${WORK}/perf2_07_binary_byvalue_import_dep.rxbin")

foreach(mode IN ITEMS opt noopt)
  set(flag)
  if(mode STREQUAL "noopt")
    set(flag -n)
  endif()

  execute_process(
    COMMAND "${RXC}" -i "${WORK}" -i "${LIBRARY_DIR}" ${flag}
            -o "${WORK}/import_main_${mode}" "${IMPORT_MAIN_SOURCE}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "${mode} import-main compile failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
  endif()

  execute_process(
    COMMAND "${RXAS}" -o "${WORK}/import_main_${mode}.rxbin"
            "${WORK}/import_main_${mode}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
    RESULT_VARIABLE res)
  if(NOT res EQUAL 0)
    message(FATAL_ERROR "${mode} import-main assemble failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
  endif()
endforeach()

file(READ "${WORK}/import_main_opt.rxas" import_opt_rxas)
if(import_opt_rxas MATCHES
   [=[call[^\n]*perf2_07_binary_byvalue_import_dep\.readonly\(\)]=])
  message(FATAL_ERROR "optimized imported read-only binary helper retained a call")
endif()
string(REGEX MATCH
       [=[\.meta "perf2_07_binary_byvalue_import_main\.main\.data"="b" "\.binary" (r[0-9]+)]=]
       import_data_meta "${import_opt_rxas}")
if(NOT import_data_meta)
  message(FATAL_ERROR "optimized import main did not expose its binary register")
endif()
set(import_data_register "${CMAKE_MATCH_1}")
if(NOT import_opt_rxas MATCHES "bgetu32 [^,\n]+,${import_data_register},")
  message(FATAL_ERROR "optimized imported read-only helper did not read the caller binary register")
endif()
if(NOT import_opt_rxas MATCHES "copy [^,\n]+,${import_data_register}")
  message(FATAL_ERROR "optimized imported writable helper did not isolate its binary formal")
endif()
if(import_opt_rxas MATCHES "endlife ${import_data_register}([\r\n]|$)")
  message(FATAL_ERROR "optimized imported inline formal ended the caller binary lifetime")
endif()

file(READ "${WORK}/import_main_noopt.rxas" import_noopt_rxas)
foreach(helper IN ITEMS readonly mutate)
  if(NOT import_noopt_rxas MATCHES
     "call[^\n]*perf2_07_binary_byvalue_import_dep\\.${helper}\\(\\)")
    message(FATAL_ERROR "unoptimized imported ${helper} helper did not retain its call boundary")
  endif()
endforeach()

foreach(vm IN ITEMS "${RXVM}" "${RXBVM}")
  foreach(mode IN ITEMS opt noopt)
    foreach(program IN ITEMS codegen semantics)
      execute_process(
        COMMAND "${vm}" "${WORK}/${program}_${mode}.rxbin" "${LIBRARY}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE res)
      if(NOT res EQUAL 0)
        message(FATAL_ERROR "${vm} ${program} ${mode} failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
      endif()
      if(NOT out MATCHES "PASS: PERF2-07 binary by-value")
        message(FATAL_ERROR "${vm} ${program} ${mode} missed PASS marker.\nSTDOUT:\n${out}\nSTDERR:\n${err}")
      endif()
      if(out MATCHES "FAIL:")
        message(FATAL_ERROR "${vm} ${program} ${mode} reported failure.\nSTDOUT:\n${out}\nSTDERR:\n${err}")
      endif()
    endforeach()
  endforeach()
endforeach()

foreach(vm IN ITEMS "${RXVM}" "${RXBVM}")
  foreach(mode IN ITEMS opt noopt)
    execute_process(
      COMMAND "${vm}" "${LIBRARY}"
              "${WORK}/perf2_07_binary_byvalue_import_dep.rxbin"
              "${WORK}/import_main_${mode}.rxbin"
      OUTPUT_VARIABLE out
      ERROR_VARIABLE err
      RESULT_VARIABLE res)
    if(NOT res EQUAL 0)
      message(FATAL_ERROR "${vm} imported ${mode} failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
    endif()
    if(NOT out MATCHES "PASS: PERF2-07 imported binary summary")
      message(FATAL_ERROR "${vm} imported ${mode} missed PASS marker.\nSTDOUT:\n${out}\nSTDERR:\n${err}")
    endif()
  endforeach()
endforeach()
