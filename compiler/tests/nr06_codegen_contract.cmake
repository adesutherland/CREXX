cmake_minimum_required(VERSION 3.20)

foreach(required IN ITEMS RXC BIN_DIR SOURCE_DIR WORK_DIR)
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
                    "${SOURCE_DIR}/nr06_call_window_scalar.crexx"
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

compile_rxas(nr06_opt)
compile_rxas(nr06_noopt -n)

foreach(image IN ITEMS nr06_opt nr06_noopt)
    file(READ "${WORK_DIR}/${image}.rxas" ${image})
endforeach()

assert_count(nr06_opt "\n[ \t]+swap " 2
             "NR-06 optimized swaps after exact placement")
assert_count(nr06_opt "\n[ \t]+(icopy|fcopy) " 0
             "NR-06 optimized typed call-window copies")
assert_count(nr06_opt "\n[ \t]+copy " 1
             "NR-06 optimized repeated-source snapshot")
assert_count(nr06_noopt "\n[ \t]+swap " 20
             "NR-06 no-opt baseline swaps")
assert_count(nr06_noopt "\n[ \t]+(icopy|fcopy) " 0
             "NR-06 no-opt typed call-window copies")
assert_count(nr06_noopt "\n[ \t]+copy " 1
             "NR-06 no-opt repeated-source snapshot")

assert_matches(nr06_opt "\nmain\\(\\) \\.locals=11\n"
               "NR-06 optimized main register high-water mark")
assert_matches(nr06_opt "\nconflictCases\\(\\) \\.locals=10\n"
               "NR-06 optimized fallback register high-water mark")
assert_matches(nr06_noopt "\nmain\\(\\) \\.locals=9\n"
               "NR-06 no-opt main register high-water mark")
assert_matches(nr06_noopt "\nconflictCases\\(\\) \\.locals=9\n"
               "NR-06 no-opt fallback register high-water mark")
assert_count(nr06_opt
             "\n[ \t]+load r0,2\n[ \t]+call [^\n]*,add\\(\\),r0"
             2
             "NR-06 exact two-argument call windows")
assert_matches(nr06_opt
               "\n[ \t]+settp r1,256\n[ \t]+call [^\n]*,optionalInt\\(\\),r0"
               "NR-06 exact optional argument window")
assert_matches(nr06_opt
               "\n[ \t]+settp r8,256\n[ \t]+call [^\n]*,echoText\\(\\),r7"
               "NR-06 exact string argument window")
assert_matches(nr06_opt
               "\n[ \t]+load r0,1\n[ \t]+call [^\n]*,incrementRef\\(\\),r0"
               "NR-06 exact reference argument window")
assert_matches(nr06_opt
               "\n[ \t]+copy r[0-9]+,r[0-9]+\n[ \t]+swap r[0-9]+,r[0-9]+\n[ \t]+call [^\n]*,add\\(\\),r[0-9]+\n[ \t]+swap r[0-9]+,r[0-9]+"
               "NR-06 repeated-source incompatible-window fallback")
assert_not_matches(nr06_opt
               "\n[ \t]+swap r[0-9]+,r[0-9]+\n[ \t]+call [^\n]*,optionalInt\\(\\),r[0-9]+\n[ \t]+swap r[0-9]+,r[0-9]+"
               "NR-06 optimized optional swap pair")
assert_not_matches(nr06_opt
               "\n[ \t]+swap r[0-9]+,r[0-9]+\n[ \t]+call [^\n]*,echoText\\(\\),r[0-9]+\n[ \t]+swap r[0-9]+,r[0-9]+"
               "NR-06 optimized string swap pair")
assert_not_matches(nr06_opt
               "\n[ \t]+swap r[0-9]+,r[0-9]+\n[ \t]+call [^\n]*,incrementRef\\(\\),r[0-9]+\n[ \t]+swap r[0-9]+,r[0-9]+"
               "NR-06 optimized reference swap pair")
assert_matches(nr06_noopt
               "\n[ \t]+swap r[0-9]+,r[0-9]+\n[ \t]+call [^\n]*,optionalInt\\(\\),r[0-9]+\n[ \t]+swap r[0-9]+,r[0-9]+"
               "NR-06 no-opt optional fallback restore pair")
assert_matches(nr06_noopt
               "\n[ \t]+swap r[0-9]+,r[0-9]+\n[ \t]+call [^\n]*,echoText\\(\\),r[0-9]+\n[ \t]+swap r[0-9]+,r[0-9]+"
               "NR-06 no-opt string fallback restore pair")
assert_matches(nr06_noopt
               "\n[ \t]+swap r[0-9]+,r[0-9]+\n[ \t]+call [^\n]*,incrementRef\\(\\),r[0-9]+\n[ \t]+swap r[0-9]+,r[0-9]+"
               "NR-06 no-opt reference fallback restore pair")
