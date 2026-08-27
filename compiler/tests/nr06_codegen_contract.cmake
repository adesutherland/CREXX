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

assert_count(nr06_opt "\n[ \t]+swap " 0
             "fixed-call optimized call-window swaps")
assert_count(nr06_opt "\n[ \t]+(icopy|fcopy) " 0
             "fixed-call optimized typed call-window copies")
assert_count(nr06_opt "\n[ \t]+copy " 0
             "fixed-call optimized repeated-source snapshots")
assert_count(nr06_noopt "\n[ \t]+swap " 0
             "fixed-call no-opt call-window swaps")
assert_count(nr06_noopt "\n[ \t]+(icopy|fcopy) " 0
             "fixed-call no-opt typed call-window copies")
assert_count(nr06_noopt "\n[ \t]+copy " 0
             "fixed-call no-opt repeated-source snapshots")

assert_matches(nr06_opt "\nmain\\(\\) \\.locals=11\n"
               "NR-06 optimized main register high-water mark")
assert_matches(nr06_opt "\nconflictCases\\(\\) \\.locals=10\n"
               "NR-06 optimized fallback register high-water mark")
assert_matches(nr06_noopt "\nmain\\(\\) \\.locals=8\n"
               "NR-06 no-opt main register high-water mark")
assert_matches(nr06_noopt "\nconflictCases\\(\\) \\.locals=9\n"
               "NR-06 no-opt fallback register high-water mark")
assert_count(nr06_opt "\n[ \t]+call1 " 7
             "fixed arity-one optimized calls")
assert_count(nr06_opt "\n[ \t]+call2 " 4
             "fixed arity-two optimized calls")
assert_count(nr06_noopt "\n[ \t]+call1 " 7
             "fixed arity-one no-opt calls")
assert_count(nr06_noopt "\n[ \t]+call2 " 4
             "fixed arity-two no-opt calls")
assert_matches(nr06_opt
               "\n[ \t]+settp r1,256\n[ \t]+call1 [^\n]*,optionalInt\\(\\),r1"
               "fixed-call optional argument status")
assert_matches(nr06_opt
               "\n[ \t]+settp r8,256\n[ \t]+call1 [^\n]*,echoText\\(\\),r8"
               "fixed-call string argument status")
assert_matches(nr06_opt
               "\n[ \t]+call1 [^\n]*,incrementRef\\(\\),r1"
               "fixed-call reference argument")
assert_matches(nr06_opt
               "\n[ \t]+call2 [^\n]*,add\\(\\),r1,r1"
               "fixed-call repeated scalar register")
assert_matches(nr06_noopt
               "\n[ \t]+settp r1,256\n[ \t]+call1 [^\n]*,optionalInt\\(\\),r1"
               "fixed-call no-opt optional argument status")
assert_matches(nr06_noopt
               "\n[ \t]+settp r3,256\n[ \t]+call1 [^\n]*,echoText\\(\\),r3"
               "fixed-call no-opt string argument status")
assert_matches(nr06_noopt
               "\n[ \t]+call2 [^\n]*,add\\(\\),r2,r2"
               "fixed-call no-opt repeated scalar register")
