cmake_minimum_required(VERSION 3.20)

foreach(required IN ITEMS RXC RXAS RXVM RXBVM BIN_DIR SOURCE_DIR WORK_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required -D${required}=...")
    endif()
endforeach()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

function(compile_flow output_stem)
    execute_process(
            COMMAND "${RXC}" -i "${BIN_DIR}" ${ARGN}
                    -o "${WORK_DIR}/${output_stem}"
                    "${SOURCE_DIR}/nr26_flow_analysis.crexx"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxc failed for ${output_stem}:\n${out}${err}")
    endif()
    file(READ "${WORK_DIR}/${output_stem}.rxas" image)
    set(${output_stem} "${image}" PARENT_SCOPE)
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

function(assert_count variable pattern expected description)
    string(REGEX MATCHALL "${pattern}" matches "${${variable}}")
    list(LENGTH matches actual)
    if(NOT actual EQUAL expected)
        message(FATAL_ERROR
                "${description}: expected ${expected} matches, found ${actual}\n"
                "Procedure RXAS:\n${${variable}}")
    endif()
endfunction()

function(assemble_flow image)
    execute_process(
            COMMAND "${RXAS}" -o "${WORK_DIR}/${image}.rxbin"
                    "${WORK_DIR}/${image}.rxas"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxas failed for ${image}:\n${out}${err}")
    endif()
endfunction()

function(run_flow image runner runner_name)
    execute_process(
            COMMAND "${runner}" "${BIN_DIR}/library.rxbin"
                    "${WORK_DIR}/${image}.rxbin"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    string(REPLACE "\r\n" "\n" out "${out}")
    if(NOT result EQUAL 0 OR NOT out STREQUAL "ok\n")
        message(FATAL_ERROR
                "${runner_name}/${image} failed (${result}):\n${out}${err}")
    endif()
endfunction()

compile_flow(nr26_opt)
compile_flow(nr26_noopt -n)

foreach(image IN ITEMS nr26_opt nr26_noopt)
    extract_procedure(${image} localStraight localBranchBoth ${image}_local_straight)
    extract_procedure(${image} localBranchBoth localBranchOne ${image}_local_branch_both)
    extract_procedure(${image} localBranchOne localLoopZero ${image}_local_branch_one)
    extract_procedure(${image} localLoopZero localNestedLoop ${image}_local_loop_zero)
    extract_procedure(${image} localNestedLoop localEarly ${image}_local_nested_loop)
    extract_procedure(${image} localEarly localReferencePositive ${image}_local_early)
    extract_procedure(${image} localReferencePositive localReferenceTarget ${image}_local_ref_positive)
    extract_procedure(${image} localReferenceTarget localTracePositive ${image}_local_ref_target)
    extract_procedure(${image} localTracePositive localSignalNegative ${image}_local_trace)
    extract_procedure(${image} localSignalNegative localSignalPositive ${image}_local_signal)
    extract_procedure(${image} localSignalPositive maybeSignal ${image}_local_signal_positive)
    extract_procedure(${image} argStraight argBranchBoth ${image}_arg_straight)
    extract_procedure(${image} argBranchBoth argConditional ${image}_arg_branch_both)
    extract_procedure(${image} argConditional argLoopZero ${image}_arg_conditional)
    extract_procedure(${image} argLoopZero argEarly ${image}_arg_loop_zero)
    extract_procedure(${image} argEarly argReadThenWrite ${image}_arg_early)
    extract_procedure(${image} argReadThenWrite argReadThenCompute ${image}_arg_read_then_write)
    extract_procedure(${image} argReadThenCompute copyStraight ${image}_arg_read_then_compute)
    extract_procedure(${image} copyStraight copyBranchSame ${image}_copy_straight)
    extract_procedure(${image} copyBranchSame copySourceOverwrite ${image}_copy_branch_same)
    extract_procedure(${image} copySourceOverwrite copySourceOverwriteUsed ${image}_copy_source_overwrite)
    extract_procedure(${image} copySourceOverwriteUsed copySourceComputeUsed ${image}_copy_source_overwrite_used)
    extract_procedure(${image} copySourceComputeUsed copyJoinDifferent ${image}_copy_source_compute_used)
    extract_procedure(${image} copyJoinDifferent copyLoopStable ${image}_copy_join_different)
    extract_procedure(${image} copyLoopStable copyLoopBackedgeKill ${image}_copy_loop_stable)
    extract_procedure(${image} copyLoopBackedgeKill copyDispatchSelector ${image}_copy_loop_backedge_kill)
    extract_procedure(${image} copyDispatchSelector copyFloat ${image}_copy_dispatch_selector)
    extract_procedure(${image} copyFloat copyBoolean ${image}_copy_float)
    extract_procedure(${image} copyBoolean copyPromotionSourceLive ${image}_copy_boolean)
    extract_procedure(${image} copyPromotionSourceLive copyPromotionSourceDead ${image}_copy_promotion_live)
    extract_procedure(${image} copyPromotionSourceDead deadCopy ${image}_copy_promotion_dead)
    extract_procedure(${image} deadCopy deadEffect ${image}_dead_copy)
    extract_procedure(${image} deadEffect incrementCounter ${image}_dead_effect)
    extract_procedure(${image} incrementCounter __rxtrace_handler ${image}_increment_counter)
endforeach()

# F1 positives: every path to each first read has a safe scalar write.
foreach(block IN ITEMS nr26_opt_local_straight nr26_opt_local_branch_both
                       nr26_opt_local_nested_loop nr26_opt_local_early
                       nr26_opt_local_trace nr26_opt_local_signal_positive)
    assert_count(${block} "\n[ \t]+null " 0
                 "${block} proved-overwritten local default")
endforeach()
# The unrelated reference locals retain two defaults; only `value` is removed.
assert_count(nr26_opt_local_ref_positive "\n[ \t]+null " 2
             "unrelated references must not poison the eligible local")

# F1 negatives: an uncovered path, zero-trip loop, alias target, or handler read
# keeps the language default.
assert_count(nr26_opt_local_branch_one "\n[ \t]+null " 1
             "one-branch overwrite default")
assert_count(nr26_opt_local_loop_zero "\n[ \t]+null " 1
             "zero-trip loop default")
assert_count(nr26_opt_local_ref_target "\n[ \t]+null " 2
             "reference-target and reference defaults")
assert_count(nr26_opt_local_signal "\n[ \t]+null " 1
             "signal-handler-visible default")

# F2 positives and negatives use the incoming first argument slot (`a1`) as an
# exact structural discriminator for the private-formal entry copy.
foreach(block IN ITEMS nr26_opt_arg_straight nr26_opt_arg_branch_both
                       nr26_opt_arg_early)
    assert_count(${block} "\n[ \t]+icopy [arg][0-9]+,a1\n" 0
                 "${block} proved-overwritten entry copy")
endforeach()
foreach(block IN ITEMS nr26_opt_arg_conditional nr26_opt_arg_loop_zero
                       nr26_opt_arg_read_then_compute)
    assert_count(${block} "\n[ \t]+icopy [arg][0-9]+,a1\n" 1
                 "${block} required entry copy")
endforeach()

# A semantically writable formal may still share the incoming slot when every
# physical write is an elided exact copy. A computed write remains private.
assert_count(nr26_opt_arg_read_then_write "\n[ \t]+icopy [arg][0-9]+,a1\n" 0
             "copy-forwarded formal incoming-slot share")

# `-n` is the untouched reference path for both production transformations.
foreach(block IN ITEMS nr26_noopt_local_straight nr26_noopt_local_branch_both
                       nr26_noopt_local_nested_loop nr26_noopt_local_early
                       nr26_noopt_local_trace nr26_noopt_local_signal_positive)
    assert_count(${block} "\n[ \t]+null " 1
                 "${block} no-opt local default")
endforeach()
foreach(block IN ITEMS nr26_noopt_arg_straight nr26_noopt_arg_branch_both
                       nr26_noopt_arg_early nr26_noopt_arg_conditional
                       nr26_noopt_arg_loop_zero nr26_noopt_arg_read_then_write
                       nr26_noopt_arg_read_then_compute)
    assert_count(${block} "\n[ \t]+icopy [arg][0-9]+,a1\n" 1
                 "${block} no-opt entry copy")
endforeach()

# F3 propagates exact small-scalar copy equalities through straight-line and
# same-source joins, then drops only stores made dead by the rewritten uses.
assert_count(nr26_noopt_copy_straight "\n[ \t]+icopy " 1
             "straight copy no-opt reference")
assert_count(nr26_opt_copy_straight "\n[ \t]+icopy " 0
             "straight copy propagation")
assert_count(nr26_noopt_copy_branch_same "\n[ \t]+icopy " 2
             "same-source branch no-opt reference")
assert_count(nr26_opt_copy_branch_same "\n[ \t]+icopy " 0
             "same-source branch copy propagation")
assert_count(nr26_noopt_copy_loop_stable "\n[ \t]+icopy " 2
             "loop-stable copy no-opt reference")
assert_count(nr26_opt_copy_loop_stable "\n[ \t]+icopy " 1
             "loop-stable copy propagation")
assert_count(nr26_noopt_copy_loop_backedge_kill "\n[ \t]+icopy " 1
             "counted-loop backedge no-opt reference")
assert_count(nr26_opt_copy_loop_backedge_kill "\n[ \t]+icopy " 1
             "counted-loop backedge equality kill")
assert_count(nr26_noopt_copy_dispatch_selector "\n[ \t]+icopy " 1
             "dispatch selector no-opt reference")
assert_count(nr26_opt_copy_dispatch_selector "\n[ \t]+icopy " 1
             "implicit dispatch selector read")
assert_count(nr26_opt_copy_dispatch_selector "\n[ \t]+jumpi " 1
             "integer dispatch lowering")
assert_count(nr26_noopt_copy_float "\n[ \t]+fcopy " 1
             "float copy no-opt reference")
assert_count(nr26_opt_copy_float "\n[ \t]+fcopy " 0
             "float copy propagation")
assert_count(nr26_noopt_copy_boolean "\n[ \t]+icopy " 1
             "boolean copy no-opt reference")
assert_count(nr26_opt_copy_boolean "\n[ \t]+icopy " 0
             "boolean copy propagation")
assert_count(nr26_noopt_copy_promotion_live "\n[ \t]+icopy " 1
             "destructive-promotion live-source no-opt reference")
assert_count(nr26_opt_copy_promotion_live "\n[ \t]+icopy " 1
             "destructive-promotion live-source guard")
assert_count(nr26_noopt_copy_promotion_dead "\n[ \t]+icopy " 1
             "destructive-promotion dead-source no-opt reference")
assert_count(nr26_opt_copy_promotion_dead "\n[ \t]+icopy " 0
             "destructive-promotion dead-source propagation")

# A dead source write is omitted first, after which fixed-point propagation can
# reuse the unchanged physical source for the old copied value. When the new
# source value is itself a copy, later source reads can use the replacement
# register too. A computed overwrite still kills the old equality. Different
# branch sources do not meet.
assert_count(nr26_noopt_copy_source_overwrite "\n[ \t]+icopy " 3
             "source-overwrite no-opt reference")
assert_count(nr26_opt_copy_source_overwrite "\n[ \t]+icopy " 0
             "dead-source-write fixed-point propagation")
assert_count(nr26_noopt_copy_source_overwrite_used "\n[ \t]+icopy " 3
             "reused source-overwrite no-opt reference")
assert_count(nr26_opt_copy_source_overwrite_used "\n[ \t]+icopy " 0
             "two-value fixed-point propagation")
assert_count(nr26_noopt_copy_source_compute_used "\n[ \t]+icopy " 2
             "computed source-overwrite no-opt reference")
assert_count(nr26_opt_copy_source_compute_used "\n[ \t]+icopy " 2
             "computed source-overwrite equality kill")
assert_count(nr26_noopt_copy_join_different "\n[ \t]+icopy " 2
             "different-source join no-opt reference")
assert_count(nr26_opt_copy_join_different "\n[ \t]+icopy " 2
             "different-source join must retain stores")
assert_count(nr26_noopt_dead_copy "\n[ \t]+icopy " 1
             "dead copy no-opt reference")
assert_count(nr26_opt_dead_copy "\n[ \t]+icopy " 0
             "dead copy store")

# A dead destination never suppresses evaluation of its RHS. The call remains
# in both modes and the dual-VM runtime check verifies the exposed side effect.
assert_count(nr26_noopt_dead_effect "\n[ \t]+call " 1
             "dead effect no-opt RHS call")
assert_count(nr26_opt_dead_effect "\n[ \t]+call " 1
             "dead effect optimized RHS call")

# TRACE keeps the authored target name while reading the proved-equal source
# register. No synthetic trace suppression is used to obtain the copy saving.
assert_count(nr26_opt_copy_straight
             "\\.traceevent \"A\"[^\n]*\"a\" 1 [^\n]*\"value\""
             1 "copy assignment TRACE retarget")
assert_count(nr26_opt_copy_straight
             "\\.traceevent \"V\"[^\n]*\"a\" 1 [^\n]*\"value\""
             1 "copy read TRACE retarget")

foreach(image IN ITEMS nr26_opt nr26_noopt)
    assemble_flow(${image})
    run_flow(${image} "${RXVM}" rxvm)
    run_flow(${image} "${RXBVM}" rxbvm)
endforeach()
