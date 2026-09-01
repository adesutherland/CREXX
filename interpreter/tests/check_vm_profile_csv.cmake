if(NOT DEFINED VM OR NOT DEFINED INPUT OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "VM, INPUT, and OUTPUT are required")
endif()

file(REMOVE "${OUTPUT}" "${OUTPUT}.value-census.csv")
execute_process(
        COMMAND "${VM}" --profile-output "${OUTPUT}" "${INPUT}"
        RESULT_VARIABLE profile_result
        OUTPUT_VARIABLE program_output
        ERROR_VARIABLE profile_error)

if(NOT profile_result EQUAL 0)
    message(FATAL_ERROR
            "profiled VM failed with ${profile_result}\nstdout:\n${program_output}\nstderr:\n${profile_error}")
endif()
if(NOT EXISTS "${OUTPUT}")
    message(FATAL_ERROR "profile output was not created: ${OUTPUT}")
endif()

file(READ "${OUTPUT}" profile_csv)
if(NOT profile_csv MATCHES "^section,name,value,id,count,total_ns,average_ns,min_ns,max_ns,percent,selected,entries,resumes,terminals,module,kind,completed,unwound,return_type,args,bytes,max_bytes,high_water,status")
    message(FATAL_ERROR "profile output is not CSV based on its .CSV extension")
endif()
if(NOT profile_csv MATCHES "summary,schema_version,5")
    message(FATAL_ERROR "profile CSV does not identify schema version 5")
endif()
if(NOT profile_csv MATCHES "summary,profile_mode,timing")
    message(FATAL_ERROR "profile CSV does not identify timing mode")
endif()
if(NOT profile_csv MATCHES "instruction,CALL_FUNC,(inline|outline|mixed)")
    message(FATAL_ERROR
            "profile CSV does not contain effective handler placement")
endif()
if(NOT profile_csv MATCHES "transition,call_enter_frame")
    message(FATAL_ERROR "profile CSV does not contain transition rows")
endif()
if(NOT profile_csv MATCHES "procedure,\"procedure_profile.worker\",\"elapsed\",,2,")
    message(FATAL_ERROR "profile CSV does not contain the expected procedure call count")
endif()
if(NOT profile_csv MATCHES "procedure,\"procedure_profile.worker\",\"entry_overhead\"")
    message(FATAL_ERROR "profile CSV does not contain procedure entry overhead")
endif()
if(NOT profile_csv MATCHES "procedure,\"procedure_profile.worker\",\"native_child\"")
    message(FATAL_ERROR "profile CSV does not preserve native-child accounting")
endif()
if(NOT profile_csv MATCHES "allocation,frame_blocks,,,[1-9][0-9]*,0,0,0,0,0.*complete")
    message(FATAL_ERROR "profile CSV does not contain complete frame allocation data")
endif()
if(NOT profile_csv MATCHES "allocation,value_slots,,,[1-9][0-9]*,0,0,0,0,0.*complete")
    message(FATAL_ERROR "profile CSV does not contain value-slot allocation data")
endif()
if(NOT profile_csv MATCHES "allocation,frame_activations,,,[1-9][0-9]*,0,0,0,0,0.*,[1-9][0-9]*,complete")
    message(FATAL_ERROR "profile CSV does not contain a complete frame high-water row")
endif()
if(NOT profile_csv MATCHES "value_operation,\"clear\"")
    message(FATAL_ERROR "profile CSV does not contain value-operation rows")
endif()
if(NOT profile_csv MATCHES "frame_entry,\"local_relink\"")
    message(FATAL_ERROR "profile CSV does not contain frame-entry phase rows")
endif()
if(NOT profile_csv MATCHES "status,\"branch_sites\",\"complete\"")
    message(FATAL_ERROR "profile CSV does not contain complete branch-site status")
endif()
if(NOT profile_csv MATCHES "census,\"call_path\",\"direct_bytecode\",,2,")
    message(FATAL_ERROR "profile CSV does not contain the expected direct-call census")
endif()
if(NOT profile_csv MATCHES "census,\"arity\",\"0\",,2,")
    message(FATAL_ERROR "profile CSV does not contain exact zero-arity rows")
endif()
if(NOT profile_csv MATCHES "census,\"frame_disposition\",\"reused\",,1,")
    message(FATAL_ERROR "profile CSV does not relate recycler reuse to calls")
endif()
if(NOT profile_csv MATCHES "return,\"placement\",\"terminal_external\",,1,")
    message(FATAL_ERROR "profile CSV does not contain return placement")
endif()
if(NOT profile_csv MATCHES "mechanics,\"call_window\",\"attribution_degraded\",,0,")
    message(FATAL_ERROR "profile CSV does not expose attribution status")
endif()
if(NOT profile_csv MATCHES "unwind,\"signal\",\"restoration_failures\",,0,")
    message(FATAL_ERROR "profile CSV does not expose signal restoration status")
endif()
if(NOT profile_csv MATCHES "call,\"procedure_profile.worker\",\"direct_bytecode\"")
    message(FATAL_ERROR "profile CSV does not contain per-site call rows")
endif()
if(profile_error MATCHES "VM PROFILE")
    message(FATAL_ERROR "profile table leaked to stderr despite file output")
endif()

set(value_census_output "${OUTPUT}.value-census.csv")
if(NOT EXISTS "${value_census_output}")
    message(FATAL_ERROR
            "profile value-census sidecar was not created: ${value_census_output}")
endif()
file(READ "${value_census_output}" value_census_csv)
if(NOT value_census_csv MATCHES "summary,schema_version,,,0,1,1,1")
    message(FATAL_ERROR "value census does not identify schema version 1")
endif()
if(NOT value_census_csv MATCHES
        "summary,unique_value_addresses.*sizeof_value=[1-9][0-9]*.*tracking=complete")
    message(FATAL_ERROR "value census does not contain complete shape identity")
endif()
if(NOT value_census_csv MATCHES "origin,unique_values")
    message(FATAL_ERROR "value census does not contain value origins")
endif()
if(NOT value_census_csv MATCHES "reclaimable_peak,string_capacity")
    message(FATAL_ERROR "value census does not expose reclaimable capacity")
endif()

set(counts_output "${OUTPUT}.counts.csv")
set(counts_repeat "${OUTPUT}.counts-repeat.csv")
file(REMOVE
        "${counts_output}" "${counts_repeat}"
        "${counts_output}.value-census.csv"
        "${counts_repeat}.value-census.csv")
execute_process(
        COMMAND "${VM}" --profile=counts --profile-output "${counts_output}" "${INPUT}"
        RESULT_VARIABLE counts_result
        OUTPUT_VARIABLE counts_program_output
        ERROR_VARIABLE counts_error)
execute_process(
        COMMAND "${VM}" --profile=counts --profile-output "${counts_repeat}" "${INPUT}"
        RESULT_VARIABLE counts_repeat_result
        OUTPUT_VARIABLE counts_repeat_program_output
        ERROR_VARIABLE counts_repeat_error)
if(NOT counts_result EQUAL 0 OR NOT counts_repeat_result EQUAL 0)
    message(FATAL_ERROR
            "counts profile failed (${counts_result}/${counts_repeat_result})\n${counts_error}\n${counts_repeat_error}")
endif()
execute_process(
        COMMAND "${CMAKE_COMMAND}" -E compare_files "${counts_output}" "${counts_repeat}"
        RESULT_VARIABLE counts_compare)
if(NOT counts_compare EQUAL 0)
    message(FATAL_ERROR "counts-only profiles are not deterministic")
endif()
execute_process(
        COMMAND "${CMAKE_COMMAND}" -E compare_files
                "${counts_output}.value-census.csv"
                "${counts_repeat}.value-census.csv"
        RESULT_VARIABLE counts_census_compare)
if(NOT counts_census_compare EQUAL 0)
    message(FATAL_ERROR "counts-only value censuses are not deterministic")
endif()
file(READ "${counts_output}" counts_csv)
if(NOT counts_csv MATCHES "summary,profile_mode,counts")
    message(FATAL_ERROR "counts CSV does not identify counts mode")
endif()
if(NOT counts_csv MATCHES "instruction,CALL_FUNC,(inline|outline|mixed),[0-9]+,[1-9][0-9]*,0,0,0,0,0")
    message(FATAL_ERROR "counts CSV did not zero instruction timing fields")
endif()

set(table_output "${OUTPUT}.txt")
file(REMOVE "${table_output}")
execute_process(
        COMMAND "${VM}" --profile-output "${table_output}" "${INPUT}"
        RESULT_VARIABLE table_result
        OUTPUT_VARIABLE table_program_output
        ERROR_VARIABLE table_error)
if(NOT table_result EQUAL 0)
    message(FATAL_ERROR
            "table-profiled VM failed with ${table_result}\nstdout:\n${table_program_output}\nstderr:\n${table_error}")
endif()
file(READ "${table_output}" profile_table)
if(NOT profile_table MATCHES
        "VM PROFILE.*Instructions.*Procedures and methods.*procedure_profile.worker.*Value operations.*Frame-entry phases.*Branch sites.*Call-path census.*Observed exact call rows.*Return placement.*Dynamic selection.*Call-window attribution.*Signal-unwind call-window restoration")
    message(FATAL_ERROR "non-CSV filename did not select table output")
endif()
if(NOT profile_table MATCHES
        "opcode[ ]+handler[ ]+count.*CALL_FUNC[ ]+(inline|outline|mixed)")
    message(FATAL_ERROR
            "profile table does not contain effective handler placement")
endif()
