cmake_minimum_required(VERSION 3.20)

foreach(required IN ITEMS RXC BIN_DIR SOURCE_DIR WORK_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required -D${required}=...")
    endif()
endforeach()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

function(compile_rxas output_stem source_name)
    execute_process(
            COMMAND "${RXC}" -i "${BIN_DIR}" ${ARGN}
                    -o "${WORK_DIR}/${output_stem}"
                    "${SOURCE_DIR}/${source_name}.crexx"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
                "rxc failed for ${source_name}/${output_stem}:\n${out}${err}")
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

compile_rxas(nr06_opt nr06_call_window_scalar)
compile_rxas(nr06_noopt nr06_call_window_scalar -n)
compile_rxas(nr07_opt nr07_direct_compare_branch)
compile_rxas(nr07_noopt nr07_direct_compare_branch -n)

foreach(image IN ITEMS nr06_opt nr06_noopt nr07_opt nr07_noopt)
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

assert_count(nr07_opt "\n[ \t]+(beq|bne|blt|ble|bgt|bge) " 9
             "NR-07 optimized direct compare branches")
assert_count(nr07_noopt "\n[ \t]+(beq|bne|blt|ble|bgt|bge) " 0
             "NR-07 no-opt direct compare branches")
assert_count(nr07_opt "\n[ \t]+(ieq|ine|ilt|ilte|igt|igte) " 1
             "NR-07 optimized retained integer comparisons")
assert_count(nr07_noopt "\n[ \t]+(ieq|ine|ilt|ilte|igt|igte) " 10
             "NR-07 no-opt integer comparisons")
assert_count(nr07_opt "\n[ \t]+(brt|brf) " 5
             "NR-07 optimized fallback Boolean branches")
assert_count(nr07_noopt "\n[ \t]+(brt|brf) " 14
             "NR-07 no-opt Boolean branches")

assert_matches(nr07_opt
               "\n[ \t]+bge l[0-9]+iffalse,r[0-9]+,5"
               "NR-07 optimized constant-left normalization")
assert_matches(nr07_opt
               "\n[ \t]+bge l[0-9]+iffalse,r[0-9]+,r[0-9]+"
               "NR-07 optimized register comparison")
assert_matches(nr07_opt "\n[ \t]+seq r[0-9]+,[^\n]+\n[ \t]+\\.traceevent[^\n]+\n[ \t]+brf "
               "NR-07 strict comparison fallback")
assert_matches(nr07_opt "\n[ \t]+flt r[0-9]+,[^\n]+\n[ \t]+\\.traceevent[^\n]+\n[ \t]+brf "
               "NR-07 float comparison fallback")
assert_matches(nr07_opt "\n[ \t]+slt r[0-9]+,[^\n]+\n[ \t]+\\.traceevent[^\n]+\n[ \t]+brf "
               "NR-07 string comparison fallback")
