cmake_minimum_required(VERSION 3.24)
project(tests C)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(native_signals)
add_subdirectory(levelb)
add_subdirectory(benchmarks)
add_subdirectory(performance)
add_subdirectory(reference_iterators)
add_subdirectory(rxpa)

set(CMAKE_C_STANDARD 90)

enable_testing()

add_test(
        NAME crexx_expected_failure_contract
        COMMAND ${CMAKE_COMMAND}
                -DASSERTIONS_MODULE=${CMAKE_SOURCE_DIR}/cmake/CrexxTestAssertions.cmake
                -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/expected_failure_contract
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/expected_failure_contract.cmake
)
set_tests_properties(crexx_expected_failure_contract PROPERTIES LABELS "cmake;negative")

set(RXAS_OPTIMIZER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rxas_optimizer)

add_executable(test_rxinteger_contract test_rxinteger_contract.c)
target_include_directories(test_rxinteger_contract PRIVATE
        ../platform ../interpreter ../rxpa ../binutils/include ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(test_rxinteger_contract PRIVATE platform)
add_test(NAME rxinteger_contract COMMAND test_rxinteger_contract)
set_tests_properties(rxinteger_contract PROPERTIES LABELS "integer;abi;platform")

add_custom_target(rxas_optimizer_test_prereqs ALL
        DEPENDS rxas rxdas)

add_executable(test_rxop_metadata
        test_rxop_metadata.c
        ../binutils/include/opdata.c
        ../binutils/rxopmeta.c)
target_include_directories(test_rxop_metadata PRIVATE ../binutils/include)
add_test(NAME rxas_optimizer_metadata COMMAND test_rxop_metadata)
set_tests_properties(rxas_optimizer_metadata PROPERTIES LABELS "rxas;optimizer;metadata")

function(crexx_add_rxas_optimizer_check NAME SOURCE CASE)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs RXAS_FLAGS)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(CREXX_RXAS_FLAGS)
        string(JOIN ";" _rxas_optimizer_flags ${CREXX_RXAS_FLAGS})
    else()
        set(_rxas_optimizer_flags "")
    endif()

    add_test(
            NAME rxas_optimizer_${NAME}
            COMMAND ${CMAKE_COMMAND}
            -DRXAS=$<TARGET_FILE:rxas>
            -DRXDAS=$<TARGET_FILE:rxdas>
            -DSOURCE=${RXAS_OPTIMIZER_DIR}/${SOURCE}
            -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/rxas_optimizer
            -DCASE=${CASE}
            "-DRXAS_FLAGS=${_rxas_optimizer_flags}"
            -P ${RXAS_OPTIMIZER_DIR}/check_optimizer.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(rxas_optimizer_${NAME} PROPERTIES LABELS "rxas;optimizer")
endfunction()

crexx_add_rxas_optimizer_check(fixed_registers_opt fixed_registers.rxas fixed_registers_opt)
crexx_add_rxas_optimizer_check(fixed_registers_noopt fixed_registers.rxas fixed_registers_noopt RXAS_FLAGS -n)
crexx_add_rxas_optimizer_check(implicit_inc0_relevant implicit_inc0_relevant.rxas implicit_inc0_relevant)
crexx_add_rxas_optimizer_check(implicit_inc0_unrelated implicit_inc0_unrelated.rxas implicit_inc0_unrelated)
crexx_add_rxas_optimizer_check(loadint_relevant loadint_relevant.rxas loadint_relevant)
crexx_add_rxas_optimizer_check(loadint_unrelated loadint_unrelated.rxas loadint_unrelated)
crexx_add_rxas_optimizer_check(linkarg_relevant linkarg_relevant.rxas linkarg_relevant)
crexx_add_rxas_optimizer_check(linkarg_unrelated linkarg_unrelated.rxas linkarg_unrelated)
crexx_add_rxas_optimizer_check(copy_acopy_opt copy_acopy.rxas copy_acopy_opt)
crexx_add_rxas_optimizer_check(copy_acopy_noopt copy_acopy.rxas copy_acopy_noopt RXAS_FLAGS -n)
crexx_add_rxas_optimizer_check(compare_branch compare_branch.rxas compare_branch)
crexx_add_rxas_optimizer_check(compare_branch_noopt compare_branch.rxas compare_branch_noopt RXAS_FLAGS -n)
crexx_add_rxas_optimizer_check(duplicate_link_read duplicate_link_read.rxas duplicate_link_read)
crexx_add_rxas_optimizer_check(duplicate_link_read_meta_relevant duplicate_link_read_meta_relevant.rxas duplicate_link_read_meta_relevant)
crexx_add_rxas_optimizer_check(duplicate_link_read_meta_unrelated duplicate_link_read_meta_unrelated.rxas duplicate_link_read_meta_unrelated)
crexx_add_rxas_optimizer_check(duplicate_link_read_trace_relevant duplicate_link_read_trace_relevant.rxas duplicate_link_read_trace_relevant)
crexx_add_rxas_optimizer_check(duplicate_link_read_trace_unrelated duplicate_link_read_trace_unrelated.rxas duplicate_link_read_trace_unrelated)
crexx_add_rxas_optimizer_check(duplicate_link_read_bcopy duplicate_link_read_bcopy.rxas duplicate_link_read_bcopy)
crexx_add_rxas_optimizer_check(duplicate_linkattr_read duplicate_linkattr_read.rxas duplicate_linkattr_read)
crexx_add_rxas_optimizer_check(duplicate_linkattr_read_bcopy duplicate_linkattr_read_bcopy.rxas duplicate_linkattr_read_bcopy)
crexx_add_rxas_optimizer_check(duplicate_linkattr_different duplicate_linkattr_different.rxas duplicate_linkattr_different)
crexx_add_rxas_optimizer_check(barrier_call barrier_call.rxas barrier_call)
crexx_add_rxas_optimizer_check(barrier_dcall barrier_dcall.rxas barrier_dcall)
crexx_add_rxas_optimizer_check(barrier_srcfproc barrier_srcfproc.rxas barrier_srcfproc)
crexx_add_rxas_optimizer_check(barrier_asserttype barrier_asserttype.rxas barrier_asserttype)
crexx_add_rxas_optimizer_check(barrier_assertinitialized barrier_assertinitialized.rxas barrier_assertinitialized)
crexx_add_rxas_optimizer_check(barrier_ichkrng barrier_ichkrng.rxas barrier_ichkrng)
crexx_add_rxas_optimizer_check(barrier_signal barrier_signal.rxas barrier_signal)
crexx_add_rxas_optimizer_check(barrier_igtbr barrier_igtbr.rxas barrier_igtbr)
crexx_add_rxas_optimizer_check(barrier_iltbr barrier_iltbr.rxas barrier_iltbr)
crexx_add_rxas_optimizer_check(barrier_fgtbr barrier_fgtbr.rxas barrier_fgtbr)
crexx_add_rxas_optimizer_check(barrier_fltbr barrier_fltbr.rxas barrier_fltbr)
crexx_add_rxas_optimizer_check(barrier_dgtbr barrier_dgtbr.rxas barrier_dgtbr)
crexx_add_rxas_optimizer_check(barrier_dltbr barrier_dltbr.rxas barrier_dltbr)
crexx_add_rxas_optimizer_check(barrier_deqbr barrier_deqbr.rxas barrier_deqbr)
crexx_add_rxas_optimizer_check(barrier_metaloadmodule barrier_metaloadmodule.rxas barrier_metaloadmodule)
crexx_add_rxas_optimizer_check(nr09_class1 nr09_class1.rxas nr09_class1)
crexx_add_rxas_optimizer_check(nr09_class1_noopt nr09_class1.rxas nr09_class1_noopt RXAS_FLAGS -n)
crexx_add_rxas_optimizer_check(nr09_class1_boundaries nr09_class1_boundaries.rxas nr09_class1_boundaries)

crexx_label_smoke_tests(rxas_optimizer_copy_acopy_opt)

add_custom_target(linked_opt_sweep
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -L linked_opt
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS crexx rxlink
        USES_TERMINAL
        COMMENT "Running linked opt-runtime coverage through CTest"
)

set(ASMTESTBINS ascommon.rxbin asutf.rxbin)
set(REXXTESTBINS basic.rxbin levelbfunc.rxbin)

unset(_prev_dependency) # Clear the previous dependency (just in case)
foreach(_binfile ${ASMTESTBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_target(${_basename} ALL)
  crexx_add_rxas_opt_matrix(
          NAME ${_basename}
          SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
          TARGET_GROUP ${_basename}
          DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxas library compiler_exit_bin ${_prev_dependency}
  )
  set(_prev_dependency ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()

foreach(_binfile ${REXXTESTBINS})
   get_filename_component(_basename ${_binfile} NAME_WE)
   add_custom_target(${_basename} ALL)
   crexx_add_rexx_opt_matrix(
           NAME ${_basename}
           SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx
           TARGET_GROUP ${_basename}
           DEPENDS library compiler_exit_bin ${_prev_dependency}
   )
   set(_prev_dependency ${_basename}_noopt_artifact ${_basename}_opt_artifact)
endforeach()
