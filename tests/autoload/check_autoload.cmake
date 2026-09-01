if(NOT DEFINED RXC OR NOT DEFINED RXAS OR NOT DEFINED RXDAS OR
   NOT DEFINED RXLINK OR NOT DEFINED RXVM OR NOT DEFINED RXBVM OR
   NOT DEFINED PROVIDER_SOURCE OR NOT DEFINED CONSUMER_SOURCE OR
   NOT DEFINED WORK)
  message(FATAL_ERROR "RXBIN autoload contract is missing required inputs")
endif()

function(run_checked label)
  execute_process(
          COMMAND ${ARGN}
          RESULT_VARIABLE rc
          OUTPUT_VARIABLE stdout
          ERROR_VARIABLE stderr)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR
            "${label} failed (${rc})\nstdout:\n${stdout}\nstderr:\n${stderr}")
  endif()
  set(last_stdout "${stdout}" PARENT_SCOPE)
  set(last_stderr "${stderr}" PARENT_SCOPE)
endfunction()

function(require_text label text pattern)
  if(NOT "${text}" MATCHES "${pattern}")
    message(FATAL_ERROR "${label} did not contain ${pattern}\n${text}")
  endif()
endfunction()

function(reject_text label text pattern)
  if("${text}" MATCHES "${pattern}")
    message(FATAL_ERROR "${label} unexpectedly contained ${pattern}\n${text}")
  endif()
endfunction()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/binary" "${WORK}/source" "${WORK}/linked")

set(provider_stem "${WORK}/binary/packaged_provider")
set(consumer_stem "${WORK}/binary/consumer")
set(explicit_stem "${WORK}/binary/consumer_explicit")
set(nohint_stem "${WORK}/binary/consumer_nohint")
set(source_stem "${WORK}/source/consumer_source")
get_filename_component(provider_source_dir "${PROVIDER_SOURCE}" DIRECTORY)

run_checked("compile packaged provider"
        "${RXC}" -n -x --no-exe-import -o "${provider_stem}" "${PROVIDER_SOURCE}")
run_checked("assemble packaged provider"
        "${RXAS}" -o "${provider_stem}" "${provider_stem}.rxas")

run_checked("compile binary-import consumer"
        "${RXC}" -n -x --no-exe-import -i "${WORK}/binary"
        -o "${consumer_stem}" "${CONSUMER_SOURCE}")
file(READ "${consumer_stem}.rxas" consumer_rxas)
require_text("binary-import RXAS" "${consumer_rxas}"
        "\\.meta \\\"autoloaddep\\.hello\\\"=\\\"\\.autoload\\\" \\\"packaged_provider\\\"")
run_checked("assemble binary-import consumer"
        "${RXAS}" -o "${consumer_stem}" "${consumer_stem}.rxas")

run_checked("compile explicit-autoload consumer"
        "${RXC}" -n -x --autoload --no-exe-import -i "${WORK}/binary"
        -o "${explicit_stem}" "${CONSUMER_SOURCE}")
file(READ "${explicit_stem}.rxas" explicit_rxas)
require_text("explicit --autoload RXAS" "${explicit_rxas}"
        "\\.meta \\\"autoloaddep\\.hello\\\"=\\\"\\.autoload\\\" \\\"packaged_provider\\\"")

run_checked("disassemble autoload metadata"
        "${RXDAS}" "${consumer_stem}.rxbin")
require_text("disassembled RXBIN" "${last_stdout}" "\\.autoload")
require_text("disassembled RXBIN" "${last_stdout}" "packaged_provider")
file(WRITE "${WORK}/binary/consumer_disassembled.rxas" "${last_stdout}")
run_checked("reassemble disassembled autoload metadata"
        "${RXAS}" -o "${WORK}/binary/consumer_disassembled"
        "${WORK}/binary/consumer_disassembled.rxas")

run_checked("compile no-autoload-hint consumer"
        "${RXC}" -n -x --no-autoload --no-exe-import -i "${WORK}/binary"
        -o "${nohint_stem}" "${CONSUMER_SOURCE}")
file(READ "${nohint_stem}.rxas" nohint_rxas)
reject_text("--no-autoload RXAS" "${nohint_rxas}" "\\.autoload")

run_checked("compile source-import consumer"
        "${RXC}" -n -x --autoload --no-exe-import
        -s "${provider_source_dir}" -o "${source_stem}" "${CONSUMER_SOURCE}")
file(READ "${source_stem}.rxas" source_rxas)
reject_text("source-import RXAS" "${source_rxas}" "\\.autoload")

foreach(vm IN ITEMS "${RXVM}" "${RXBVM}")
  run_checked("default autoload with ${vm}"
          "${vm}" -l "${WORK}/binary" "${consumer_stem}.rxbin")
  require_text("default autoload with ${vm}" "${last_stdout}"
          "PASS: RXBIN autoload")

  run_checked("explicit autoload with ${vm}"
          "${vm}" --autoload -l "${WORK}/binary" "${consumer_stem}.rxbin")
  require_text("explicit autoload with ${vm}" "${last_stdout}"
          "PASS: RXBIN autoload")

  execute_process(
          COMMAND "${vm}" --no-autoload -l "${WORK}/binary"
                  "${consumer_stem}.rxbin"
          RESULT_VARIABLE noauto_rc
          OUTPUT_VARIABLE noauto_stdout
          ERROR_VARIABLE noauto_stderr)
  if(noauto_rc EQUAL 0)
    message(FATAL_ERROR
            "--no-autoload unexpectedly succeeded with ${vm}\n${noauto_stdout}\n${noauto_stderr}")
  endif()
endforeach()

run_checked("link consumer and provider"
        "${RXLINK}" -o "${WORK}/linked/autoload_linked"
        "${consumer_stem}.rxbin" "${provider_stem}.rxbin")
foreach(vm IN ITEMS "${RXVM}" "${RXBVM}")
  run_checked("linked dependency suppresses autoload with ${vm}"
          "${vm}" "${WORK}/linked/autoload_linked.rxbin")
  require_text("linked dependency suppresses autoload with ${vm}"
          "${last_stdout}" "PASS: RXBIN autoload")
endforeach()
