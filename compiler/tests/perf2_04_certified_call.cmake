file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

set(selected_expected "MIXED ÄÖ\nNAMED Ä\n56\n1\n日🙂\nbc\nBC\n1\nmixed äö\n0\n3\n2\né日\n日🙂\na🙂🙂\n🙂🙂a\n1\nabc\n1\nAlpha\nBeta\né日\n🙂\n1\n0\n1\n0\n0\n2\n")
set(fallback_expected "ené\nRené日🙂\nbc\n1\n...\nMIXED ÄÖ\nMiXeD äÖ\nmixed äö\n6\nRen\né日🙂\na..\n..a\nRené日🙂\n1\n9\n")
set(spoof_expected "mixed äö\n")

function(run_checked label)
    execute_process(
        COMMAND ${ARGN}
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE res)
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "${label} failed: ${out}${err}")
    endif()
endfunction()

function(check_selected_rxas label path optimized)
    file(READ "${path}" rxas_text)
    if(optimized)
        if(rxas_text MATCHES "call[^\n]*rxfnsb\\.(upper|lower|length|left|right|substr|word)\\(\\)" OR
           rxas_text MATCHES "[ \t](strupper|strlower|strlen|setstrpos|substring|padstr|fndnblnk|fndblnk)[ \t]")
            message(FATAL_ERROR "${label} retained runtime work for certified constant calls:\n${rxas_text}")
        endif()
        if(rxas_text MATCHES "[ \t]\\.srcstep[^\n]*\"(upper|lower|length|left|right|substr|word)\\.crexx\"")
            message(FATAL_ERROR "${label} attributed a folded result to the removed Level B body:\n${rxas_text}")
        endif()
        if(NOT rxas_text MATCHES "selected\\.crexx")
            message(FATAL_ERROR "${label} lost caller source identity:\n${rxas_text}")
        endif()
    else()
        foreach(bif upper lower length left right substr word)
            if(NOT rxas_text MATCHES "call[^\n]*rxfnsb\\.${bif}\\(\\)")
                message(FATAL_ERROR "${label} no-opt build did not retain ${bif} Level B call:\n${rxas_text}")
            endif()
        endforeach()
    endif()
endfunction()

function(check_fallback_rxas label path optimized)
    file(READ "${path}" rxas_text)
    if(optimized)
        foreach(op strupper strlower strlen substring padstr fndnblnk fndblnk)
            if(NOT rxas_text MATCHES "[ \t]${op}[ \t]")
                message(FATAL_ERROR "${label} removed uncertified ${op} dynamic fallback:\n${rxas_text}")
            endif()
        endforeach()
        foreach(bif upper lower length left right substr word)
            if(NOT rxas_text MATCHES "${bif}\\.crexx")
                message(FATAL_ERROR "${label} lost ${bif} Level B source identity:\n${rxas_text}")
            endif()
        endforeach()
    else()
        foreach(bif upper lower length left right substr word)
            if(NOT rxas_text MATCHES "call[^\n]*rxfnsb\\.${bif}\\(\\)")
                message(FATAL_ERROR "${label} no-opt fallback did not retain ${bif} normal call:\n${rxas_text}")
            endif()
        endforeach()
    endif()
endfunction()

function(run_dual_vm label expected upper_rxbin lower_rxbin length_rxbin
                     left_rxbin right_rxbin substr_rxbin word_rxbin main_rxbin)
    foreach(vm RXVM RXBVM)
        execute_process(
            COMMAND "${${vm}}" "${upper_rxbin}" "${lower_rxbin}"
                    "${length_rxbin}" "${left_rxbin}" "${right_rxbin}"
                    "${substr_rxbin}" "${word_rxbin}" "${main_rxbin}"
            OUTPUT_VARIABLE run_out
            ERROR_VARIABLE err
            RESULT_VARIABLE res)
        string(REPLACE "\r\n" "\n" run_out "${run_out}")
        if(NOT res EQUAL 0 OR NOT run_out STREQUAL expected)
            message(FATAL_ERROR "${label} ${vm} mismatch: expected [${expected}], got [${run_out}], stderr [${err}]")
        endif()
    endforeach()
endfunction()

foreach(mode opt noopt)
    set(mode_dir "${WORK_DIR}/${mode}")
    file(MAKE_DIRECTORY "${mode_dir}")
    if(mode STREQUAL "noopt")
        set(mode_args -n)
        set(optimized FALSE)
    else()
        set(mode_args)
        set(optimized TRUE)
    endif()

    foreach(bif upper lower length left right substr word)
        run_checked("${mode} ${bif} dependency compile"
            "${RXC}" --no-exe-import ${mode_args}
            -o "${mode_dir}/${bif}" "${BIF_SOURCE_DIR}/${bif}.crexx")
        run_checked("${mode} ${bif} dependency assemble"
            "${RXAS}" -o "${mode_dir}/${bif}.rxbin" "${mode_dir}/${bif}.rxas")
    endforeach()

    run_checked("${mode} selected source-import compile"
        "${RXC}" --no-exe-import ${mode_args} -s "${BIF_SOURCE_DIR}"
        -o "${mode_dir}/selected_source" "${SELECTED_SOURCE}")
    check_selected_rxas("${mode} selected source import"
        "${mode_dir}/selected_source.rxas" ${optimized})
    run_checked("${mode} selected source-import assemble"
        "${RXAS}" -o "${mode_dir}/selected_source.rxbin"
        "${mode_dir}/selected_source.rxas")
    run_dual_vm("${mode} selected source import" "${selected_expected}"
        "${mode_dir}/upper.rxbin" "${mode_dir}/lower.rxbin"
        "${mode_dir}/length.rxbin" "${mode_dir}/left.rxbin"
        "${mode_dir}/right.rxbin" "${mode_dir}/substr.rxbin"
        "${mode_dir}/word.rxbin"
        "${mode_dir}/selected_source.rxbin")

    run_checked("${mode} selected binary-import compile"
        "${RXC}" --no-exe-import ${mode_args} -i "${mode_dir}"
        -o "${mode_dir}/selected_binary" "${SELECTED_SOURCE}")
    check_selected_rxas("${mode} selected binary import"
        "${mode_dir}/selected_binary.rxas" ${optimized})
    run_checked("${mode} selected binary-import assemble"
        "${RXAS}" -o "${mode_dir}/selected_binary.rxbin"
        "${mode_dir}/selected_binary.rxas")
    run_dual_vm("${mode} selected binary import" "${selected_expected}"
        "${mode_dir}/upper.rxbin" "${mode_dir}/lower.rxbin"
        "${mode_dir}/length.rxbin" "${mode_dir}/left.rxbin"
        "${mode_dir}/right.rxbin" "${mode_dir}/substr.rxbin"
        "${mode_dir}/word.rxbin"
        "${mode_dir}/selected_binary.rxbin")

    run_checked("${mode} fallback source-import compile"
        "${RXC}" --no-exe-import ${mode_args} -s "${BIF_SOURCE_DIR}"
        -o "${mode_dir}/fallback" "${FALLBACK_SOURCE}")
    check_fallback_rxas("${mode} fallback" "${mode_dir}/fallback.rxas" ${optimized})
    run_checked("${mode} fallback assemble"
        "${RXAS}" -o "${mode_dir}/fallback.rxbin" "${mode_dir}/fallback.rxas")
    run_dual_vm("${mode} fallback" "${fallback_expected}"
        "${mode_dir}/upper.rxbin" "${mode_dir}/lower.rxbin"
        "${mode_dir}/length.rxbin" "${mode_dir}/left.rxbin"
        "${mode_dir}/right.rxbin" "${mode_dir}/substr.rxbin"
        "${mode_dir}/word.rxbin"
        "${mode_dir}/fallback.rxbin")
endforeach()

run_checked("bounded-result fallback compile"
    "${RXC}" --no-exe-import -s "${BIF_SOURCE_DIR}"
    -o "${WORK_DIR}/huge_width" "${HUGE_WIDTH_SOURCE}")
file(READ "${WORK_DIR}/huge_width.rxas" huge_width_rxas)
if(NOT huge_width_rxas MATCHES "left\\.crexx" OR
   NOT huge_width_rxas MATCHES "[ \t]padstr[ \t]")
    message(FATAL_ERROR "bounded compiler-result policy did not retain LEFT fallback:\n${huge_width_rxas}")
endif()

run_checked("spoof provider compile"
    "${RXC}" --no-exe-import -o "${WORK_DIR}/spoof_upper" "${SPOOF_SOURCE}")
file(READ "${WORK_DIR}/spoof_upper.rxas" spoof_rxas)
if(NOT spoof_rxas MATCHES "[ \t]strlower[ \t]" OR
   spoof_rxas MATCHES "[ \t]strupper[ \t]")
    message(FATAL_ERROR "contradictory same-summary provider bypassed body certification:\n${spoof_rxas}")
endif()
run_checked("spoof provider assemble"
    "${RXAS}" -o "${WORK_DIR}/spoof_upper.rxbin" "${WORK_DIR}/spoof_upper.rxas")
foreach(vm RXVM RXBVM)
    execute_process(
        COMMAND "${${vm}}" "${WORK_DIR}/spoof_upper.rxbin"
        OUTPUT_VARIABLE run_out
        ERROR_VARIABLE err
        RESULT_VARIABLE res)
    string(REPLACE "\r\n" "\n" run_out "${run_out}")
    if(NOT res EQUAL 0 OR NOT run_out STREQUAL spoof_expected)
        message(FATAL_ERROR "spoof provider ${vm} mismatch: expected [${spoof_expected}], got [${run_out}], stderr [${err}]")
    endif()
endforeach()

run_checked("spoof word provider compile"
    "${RXC}" --no-exe-import -s "${SPOOF_WORD_DIR}"
    -o "${WORK_DIR}/spoof_word" "${SPOOF_WORD_DIR}/main.crexx")
file(READ "${WORK_DIR}/spoof_word.rxas" spoof_word_rxas)
if(NOT spoof_word_rxas MATCHES "[ \t]fndnblnk[ \t]" OR
   NOT spoof_word_rxas MATCHES "[ \t]fndblnk[ \t]")
    message(FATAL_ERROR "contradictory WORD same-summary provider bypassed body certification:\n${spoof_word_rxas}")
endif()
run_checked("spoof word provider assemble"
    "${RXAS}" -o "${WORK_DIR}/spoof_word.rxbin" "${WORK_DIR}/spoof_word.rxas")
foreach(vm RXVM RXBVM)
    execute_process(
        COMMAND "${${vm}}" "${WORK_DIR}/spoof_word.rxbin"
        OUTPUT_VARIABLE spoof_word_out
        ERROR_VARIABLE spoof_word_err
        RESULT_VARIABLE spoof_word_res)
    string(REPLACE "\r\n" "\n" spoof_word_out "${spoof_word_out}")
    if(NOT spoof_word_res EQUAL 0 OR NOT spoof_word_out STREQUAL "Alpha\n")
        message(FATAL_ERROR "spoof word provider ${vm} mismatch: [${spoof_word_out}] [${spoof_word_err}]")
    endif()
endforeach()

run_checked("spoof lower provider compile"
    "${RXC}" --no-exe-import -o "${WORK_DIR}/spoof_lower" "${SPOOF_LOWER_SOURCE}")
file(READ "${WORK_DIR}/spoof_lower.rxas" spoof_lower_rxas)
if(NOT spoof_lower_rxas MATCHES "[ \t]strupper[ \t]" OR
   spoof_lower_rxas MATCHES "[ \t]strlower[ \t]")
    message(FATAL_ERROR "contradictory LOWER same-summary provider bypassed body certification:\n${spoof_lower_rxas}")
endif()
run_checked("spoof lower provider assemble"
    "${RXAS}" -o "${WORK_DIR}/spoof_lower.rxbin" "${WORK_DIR}/spoof_lower.rxas")
foreach(vm RXVM RXBVM)
    execute_process(
        COMMAND "${${vm}}" "${WORK_DIR}/spoof_lower.rxbin"
        OUTPUT_VARIABLE spoof_lower_out
        ERROR_VARIABLE spoof_lower_err
        RESULT_VARIABLE spoof_lower_res)
    string(REPLACE "\r\n" "\n" spoof_lower_out "${spoof_lower_out}")
    if(NOT spoof_lower_res EQUAL 0 OR NOT spoof_lower_out STREQUAL "MIXED ÄÖ\n")
        message(FATAL_ERROR "spoof lower provider ${vm} mismatch: [${spoof_lower_out}] [${spoof_lower_err}]")
    endif()
endforeach()
