cmake_minimum_required(VERSION 3.20)

foreach(required IN ITEMS RXC RXAS RXVM RXBVM BIN_DIR SOURCE_DIR WORK_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required -D${required}=...")
    endif()
endforeach()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

function(compile_rxas output_stem)
    execute_process(
            COMMAND "${RXC}" -i "${BIN_DIR}" ${ARGN}
                    -o "${WORK_DIR}/${output_stem}"
                    "${SOURCE_DIR}/nr09_numeric_context.crexx"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
                "rxc failed for ${output_stem}:\n${out}${err}")
    endif()
endfunction()

function(assert_count variable pattern expected description)
    string(REGEX MATCHALL "${pattern}" matches "${${variable}}")
    list(LENGTH matches actual)
    if(NOT actual EQUAL expected)
        message(FATAL_ERROR
                "${description}: expected ${expected} matches, found ${actual}")
    endif()
endfunction()

function(assert_matches variable pattern description)
    string(REGEX MATCH "${pattern}" match "${${variable}}")
    if(match STREQUAL "")
        message(FATAL_ERROR "${description}: expected pattern was absent")
    endif()
endfunction()

function(assert_not_matches variable pattern description)
    string(REGEX MATCH "${pattern}" match "${${variable}}")
    if(NOT match STREQUAL "")
        message(FATAL_ERROR "${description}: unexpected pattern was present")
    endif()
endfunction()

function(extract_procedure input_variable procedure next_procedure output_variable)
    string(FIND "${${input_variable}}" "\n${procedure}() " start)
    if(start EQUAL -1)
        message(FATAL_ERROR "Procedure ${procedure} was absent")
    endif()
    string(SUBSTRING "${${input_variable}}" ${start} -1 tail)
    if(next_procedure STREQUAL "")
        set(block "${tail}")
    else()
        string(FIND "${tail}" "\n${next_procedure}() " finish)
        if(finish EQUAL -1)
            message(FATAL_ERROR
                    "Procedure boundary ${next_procedure} after ${procedure} was absent")
        endif()
        string(SUBSTRING "${tail}" 0 ${finish} block)
    endif()
    set(${output_variable} "${block}" PARENT_SCOPE)
endfunction()

function(assemble_rxbin input_stem)
    execute_process(
            COMMAND "${RXAS}"
                    -o "${WORK_DIR}/${input_stem}.rxbin"
                    "${WORK_DIR}/${input_stem}.rxas"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
                "rxas failed for ${input_stem}:\n${out}${err}")
    endif()
endfunction()

set(expected_output
"default 18 0 scientific lower common\nscientific 9 0 scientific upper classic 0.333333333\nengineering 6 0 engineering lower common 0.333333\nnonzero-fuzz 6 2 scientific lower common\ninherited-digits 18 0 scientific lower common\nsmall-digits 4 0 scientific lower common 0.3333\n")

function(run_contract image runner runner_name plugin plugin_name)
    set(plugin_args)
    if(NOT plugin STREQUAL "")
        list(APPEND plugin_args -p "${plugin}")
    endif()
    execute_process(
            COMMAND "${runner}" ${plugin_args}
                    "${BIN_DIR}/library.rxbin"
                    "${WORK_DIR}/${image}.rxbin"
            WORKING_DIRECTORY "${BIN_DIR}"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
                "${runner_name}/${plugin_name}/${image} failed (${result}):\n${out}${err}")
    endif()
    string(REPLACE "\r\n" "\n" out "${out}")
    if(NOT out STREQUAL expected_output)
        message(FATAL_ERROR
                "${runner_name}/${plugin_name}/${image} output mismatch:\n"
                "Expected:\n${expected_output}\nActual:\n${out}\nStderr:\n${err}")
    endif()
endfunction()

compile_rxas(nr09_opt)
compile_rxas(nr09_noopt -n)

foreach(image IN ITEMS nr09_opt nr09_noopt)
    file(READ "${WORK_DIR}/${image}.rxas" ${image})

    assert_count(${image} "\n[ \t]+numsci " 3
                 "${image} combined scientific contexts")
    assert_count(${image} "\n[ \t]+numeng " 1
                 "${image} combined engineering contexts")
    assert_count(${image} "\n[ \t]+setnumdgts " 2
                 "${image} unfused digits controls")
    assert_count(${image} "\n[ \t]+setnumfuz " 3
                 "${image} unfused fuzz controls")
    assert_count(${image} "\n[ \t]+setnumfrm " 3
                 "${image} unfused form controls")
    assert_count(${image} "\n[ \t]+setnumcas " 3
                 "${image} unfused case controls")
    assert_count(${image} "\n[ \t]+setnumstd " 3
                 "${image} unfused standard controls")

    assert_count(${image} "\n[ \t]+numsci 18,1,1" 2
                 "${image} default combined contexts")
    assert_count(${image} "\n[ \t]+numsci 9,2,2" 1
                 "${image} explicit scientific context")
    assert_count(${image} "\n[ \t]+numeng 6,1,1" 1
                 "${image} explicit engineering context")

    extract_procedure(${image} scientificContext engineeringContext scientific_block)
    extract_procedure(${image} engineeringContext nonzeroFuzzContext engineering_block)
    extract_procedure(${image} nonzeroFuzzContext inheritedDigitsContext fuzz_block)
    extract_procedure(${image} inheritedDigitsContext smallDigitsContext inherited_block)
    extract_procedure(${image} smallDigitsContext "" small_block)

    assert_matches(scientific_block "\n[ \t]+numsci 9,2,2\n"
                   "${image} scientific prologue")
    assert_not_matches(scientific_block "\n[ \t]+setnum"
                       "${image} scientific legacy setters")
    assert_matches(engineering_block "\n[ \t]+numeng 6,1,1\n"
                   "${image} engineering prologue")
    assert_not_matches(engineering_block "\n[ \t]+setnum"
                       "${image} engineering legacy setters")
    assert_matches(fuzz_block
                   "\n[ \t]+setnumdgts 6\n[ \t]+setnumfuz 2\n[ \t]+setnumfrm 1\n[ \t]+setnumcas 1\n[ \t]+setnumstd 1\n"
                   "${image} nonzero-fuzz fail-closed prologue")
    assert_not_matches(fuzz_block "\n[ \t]+num(sci|eng)"
                       "${image} nonzero-fuzz combined context")
    assert_matches(inherited_block
                   "\n[ \t]+setnumfuz 0\n[ \t]+setnumfrm 1\n[ \t]+setnumcas 1\n[ \t]+setnumstd 1\n"
                   "${image} inherited-digits fail-closed prologue")
    assert_not_matches(inherited_block "\n[ \t]+(setnumdgts|num(sci|eng))"
                       "${image} inherited-digits overwrite")
    assert_matches(small_block
                   "\n[ \t]+setnumdgts 4\n[ \t]+setnumfuz 0\n[ \t]+setnumfrm 1\n[ \t]+setnumcas 1\n[ \t]+setnumstd 1\n"
                   "${image} digits-below-five compatibility prologue")
    assert_not_matches(small_block "\n[ \t]+num(sci|eng)"
                       "${image} invalid combined digits contract")

    assemble_rxbin(${image})
    foreach(runner_name IN ITEMS rxvm rxbvm)
        if(runner_name STREQUAL "rxvm")
            set(runner "${RXVM}")
        else()
            set(runner "${RXBVM}")
        endif()
        run_contract(${image} "${runner}" "${runner_name}" "" default)
        run_contract(${image} "${runner}" "${runner_name}"
                     rxvm_mc_decimal mc_decimal)
        run_contract(${image} "${runner}" "${runner_name}"
                     rxvm_db_decimal db_decimal)
    endforeach()
endforeach()
