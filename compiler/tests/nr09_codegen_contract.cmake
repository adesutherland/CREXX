cmake_minimum_required(VERSION 3.20)

foreach(required IN ITEMS RXC RXAS RXVM RXBVM BIN_DIR BENCH_DIR SOURCE_DIR WORK_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required -D${required}=...")
    endif()
endforeach()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

function(compile_source output_stem source)
    execute_process(
            COMMAND "${RXC}" -i "${BIN_DIR}" ${ARGN}
                    -o "${WORK_DIR}/${output_stem}" "${source}"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxc failed for ${output_stem}:\n${out}${err}")
    endif()
    execute_process(
            COMMAND "${RXAS}" -o "${WORK_DIR}/${output_stem}.rxbin"
                    "${WORK_DIR}/${output_stem}.rxas"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxas failed for ${output_stem}:\n${out}${err}")
    endif()
    file(READ "${WORK_DIR}/${output_stem}.rxas" image)
    set(${output_stem} "${image}" PARENT_SCOPE)
endfunction()

function(run_image output_stem)
    foreach(vm IN ITEMS RXVM RXBVM)
        execute_process(
                COMMAND "${${vm}}" "${WORK_DIR}/${output_stem}.rxbin"
                        "${BIN_DIR}/library.rxbin"
                OUTPUT_VARIABLE out
                ERROR_VARIABLE err
                RESULT_VARIABLE result)
        if(NOT result EQUAL 0 OR NOT out MATCHES "PASS:")
            message(FATAL_ERROR
                    "${vm} failed for fresh ${output_stem}:\n${out}${err}")
        endif()
    endforeach()
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

foreach(mode IN ITEMS noopt opt)
    set(flags)
    if(mode STREQUAL "noopt")
        list(APPEND flags -n)
    endif()
    compile_source(sieve_${mode} "${BENCH_DIR}/awfy_sieve.crexx" ${flags})
    compile_source(permute_${mode} "${BENCH_DIR}/awfy_permute.crexx" ${flags})
    compile_source(mandelbrot_${mode} "${BENCH_DIR}/awfy_mandelbrot.crexx" ${flags})
endforeach()
compile_source(casts_noopt "${SOURCE_DIR}/scalar_type_casts.crexx" -n)

foreach(image IN ITEMS sieve_noopt sieve_opt)
    assert_count(${image} "\n[ \t]+minlinkattr1 " 1
                 "${image} dynamic string attribute link")
    assert_count(${image} "\n[ \t]+stoi " 1
                 "${image} separate string-to-integer conversion")
    assert_count(${image} "\n[ \t]+isetunlink " 2
                 "${image} integer alias stores with cleanup")
    assert_count(${image} "\n[ \t]+setlinkiload " 2
                 "${image} promoted fixed-capacity link/load units")
    assert_count(${image} "\n[ \t]+itos " 3
                 "${image} separate integer-to-string conversions")
    assert_count(${image} "\n[ \t]+concat " 3
                 "${image} separate general concatenations")
    assert_count(${image} "\n[ \t]+sconcat " 2
                 "${image} retained constant concatenations")
endforeach()

foreach(image IN ITEMS mandelbrot_noopt mandelbrot_opt)
    assert_count(${image} "\n[ \t]+fdivsub " 2
                 "${image} float divide/subtract semantic units")
    assert_count(${image} "\n[ \t]+itos " 5
                 "${image} separate integer-to-string conversions")
    assert_count(${image} "\n[ \t]+concat " 6
                 "${image} separate general concatenations")
    assert_count(${image} "\n[ \t]+minlinkattr1 " 1
                 "${image} argv attribute link")
    assert_count(${image} "\n[ \t]+stoi " 1
                 "${image} separate argv conversion")
    assert_count(${image} "\n[ \t]+itof [arg][0-9]+,[arg][0-9]+" 4
                 "${image} direct integer-to-float destinations")
    assert_not_matches(${image}
            "\n[ \t]+fdiv ([arg][0-9]+),[^\n]+,\\1\n([^\n]*\n)*[ \t]+fsub "
            "${image} expanded divide/subtract chain")
endforeach()

foreach(image IN ITEMS permute_noopt permute_opt)
    assert_matches(${image} "\n[ \t]+isetattr1 "
                   "${image} direct integer class-attribute write")
    assert_count(${image} "\n[ \t]+minlinkattr1 " 1
                 "${image} argv attribute link")
    assert_count(${image} "\n[ \t]+stoi " 1
                 "${image} separate argv conversion")
endforeach()
assert_count(permute_noopt "\n[ \t]+linksetattrslinkadd " 4
             "permute_noopt promoted link/setattrs/link/add units")
# POSTPERF-05 stops recursive expansion once the cleaned site no longer beats
# the bounded call-plus-body reference. The retained two-level shape has twelve
# promoted units rather than the former recursively unrolled twenty.
assert_count(permute_opt "\n[ \t]+linksetattrslinkadd " 12
             "permute_opt promoted link/setattrs/link/add units")
assert_count(permute_noopt "\n[ \t]+setlinkattr1 " 0
             "permute_noopt unpromoted link/setattrs/link/add prefixes")
assert_count(permute_opt "\n[ \t]+setlinkattr1 " 0
             "permute_opt unpromoted link/setattrs/link/add prefixes")

foreach(image IN ITEMS sieve_noopt sieve_opt permute_noopt permute_opt
                       mandelbrot_noopt mandelbrot_opt)
    assert_not_matches(${image}
            "\n[ \t]+(minstoiattr1|itosconcat|sconcatitos) "
            "${image} withdrawn NR-09 opcode")
endforeach()

foreach(image IN ITEMS sieve_noopt sieve_opt permute_noopt permute_opt
                       mandelbrot_noopt mandelbrot_opt)
    run_image(${image})
endforeach()

assert_matches(casts_noopt "\n[ \t]+itof [arg][0-9]+,[arg][0-9]+"
               "no-opt direct integer-to-float cast")
assert_matches(casts_noopt "\n[ \t]+stoi [arg][0-9]+,[arg][0-9]+"
               "no-opt direct string-to-integer cast")
assert_not_matches(casts_noopt
        "\n[ \t]+icopy ([arg][0-9]+),[^\n]+\n[ \t]+itof \\1"
        "no-opt expanded integer-to-float cast")
assert_not_matches(casts_noopt
        "\n[ \t]+scopy ([arg][0-9]+),[^\n]+\n[ \t]+stoi \\1"
        "no-opt expanded string-to-integer cast")
