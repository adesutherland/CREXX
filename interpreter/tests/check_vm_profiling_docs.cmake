if(NOT DEFINED VM OR NOT DEFINED RXSEQ OR NOT DEFINED INPUT OR NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "VM, RXSEQ, INPUT, and OUTPUT_DIR are required")
endif()

set(TABLE "${OUTPUT_DIR}/profiling-docs.txt")
set(CSV "${OUTPUT_DIR}/profiling-docs.csv")
set(SEQUENCE "${OUTPUT_DIR}/profiling-docs.rxseq")
set(CANDIDATE_CSV "${OUTPUT_DIR}/profiling-docs-candidates.csv")
file(REMOVE "${TABLE}" "${CSV}" "${SEQUENCE}" "${CANDIDATE_CSV}")

execute_process(
        COMMAND "${VM}" --profile-output "${TABLE}" "${INPUT}"
        RESULT_VARIABLE table_rc
        OUTPUT_VARIABLE table_stdout
        ERROR_VARIABLE table_stderr)
if(NOT table_rc EQUAL 0)
    message(FATAL_ERROR
            "documented table profile failed (${table_rc})\nstdout:\n${table_stdout}\nstderr:\n${table_stderr}")
endif()
file(READ "${TABLE}" table_content)
if(NOT table_content MATCHES
        "Instructions.*Transitions.*Procedures and methods.*profiling_demo.worker[ ]+procedure[ ]+2[ ]+2[ ]+0.*Call mechanics.*Call-path census.*Return placement.*Call-window attribution.*Signal-unwind call-window restoration.*Interrupt sub-phases")
    message(FATAL_ERROR "documented table profile is missing expected sections or procedure counts")
endif()

execute_process(
        COMMAND "${VM}" --profile-output "${CSV}" "${INPUT}"
        RESULT_VARIABLE csv_rc
        OUTPUT_VARIABLE csv_stdout
        ERROR_VARIABLE csv_stderr)
if(NOT csv_rc EQUAL 0)
    message(FATAL_ERROR
            "documented CSV profile failed (${csv_rc})\nstdout:\n${csv_stdout}\nstderr:\n${csv_stderr}")
endif()
file(READ "${CSV}" csv_content)
if(NOT csv_content MATCHES
        "^section,name,value,id,count,total_ns,average_ns,min_ns,max_ns,percent,selected,entries,resumes,terminals,module,kind,completed,unwound,return_type,args")
    message(FATAL_ERROR "documented timing CSV header changed")
endif()
if(NOT csv_content MATCHES "summary,schema_version,5")
    message(FATAL_ERROR "documented timing CSV is not schema version 5")
endif()
if(NOT csv_content MATCHES "instruction,CALL_FUNC,(inline|outline|mixed)")
    message(FATAL_ERROR
            "documented timing CSV is missing effective handler placement")
endif()
if(NOT csv_content MATCHES
        "procedure,\"profiling_demo.worker\",\"elapsed\",,2,.*procedure,\"profiling_demo.worker\",\"entry_overhead\"")
    message(FATAL_ERROR "documented timing CSV is missing worker metrics")
endif()
if(NOT csv_content MATCHES
        "census,\"call_path\",\"direct_bytecode\",,2,.*census,\"arity\",\"0\",,2,.*return,\"placement\".*mechanics,\"call_window\".*unwind,\"signal\"")
    message(FATAL_ERROR "documented timing CSV is missing call-census sections")
endif()

execute_process(
        COMMAND "${VM}" --sequence-count=2 --sequence-output "${SEQUENCE}" "${INPUT}"
        RESULT_VARIABLE sequence_rc
        OUTPUT_VARIABLE sequence_stdout
        ERROR_VARIABLE sequence_stderr)
if(NOT sequence_rc EQUAL 0)
    message(FATAL_ERROR
            "documented sequence capture failed (${sequence_rc})\nstdout:\n${sequence_stdout}\nstderr:\n${sequence_stderr}")
endif()

execute_process(
        COMMAND "${RXSEQ}" "${SEQUENCE}" "${INPUT}"
        RESULT_VARIABLE table_analysis_rc
        OUTPUT_VARIABLE candidate_table
        ERROR_VARIABLE table_analysis_stderr)
if(NOT table_analysis_rc EQUAL 0)
    message(FATAL_ERROR
            "documented table analysis failed (${table_analysis_rc}): ${table_analysis_stderr}")
endif()
if(NOT candidate_table MATCHES
        "RXSEQ CANDIDATES length=2.*candidate[ ]+IADD_REG_REG_INT.*COPY_REG_REG")
    message(FATAL_ERROR "documented candidate table is missing the expected normalized pattern")
endif()

execute_process(
        COMMAND "${RXSEQ}" "${SEQUENCE}" "${INPUT}" --output "${CANDIDATE_CSV}"
        RESULT_VARIABLE csv_analysis_rc
        OUTPUT_VARIABLE csv_analysis_stdout
        ERROR_VARIABLE csv_analysis_stderr)
if(NOT csv_analysis_rc EQUAL 0)
    message(FATAL_ERROR
            "documented CSV analysis failed (${csv_analysis_rc}): ${csv_analysis_stderr}")
endif()
file(READ "${CANDIDATE_CSV}" candidate_csv)
if(NOT candidate_csv MATCHES
        "^rank,count,sites,modules,symbols,status,pattern,mapping,example_module,example_start")
    message(FATAL_ERROR "documented candidate CSV header changed")
endif()
if(NOT candidate_csv MATCHES
        ",candidate,\"IADD_REG_REG_INT.*COPY_REG_REG")
    message(FATAL_ERROR "documented candidate CSV is missing the expected normalized pattern")
endif()
