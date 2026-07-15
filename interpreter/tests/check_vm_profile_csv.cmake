if(NOT DEFINED VM OR NOT DEFINED INPUT OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "VM, INPUT, and OUTPUT are required")
endif()

file(REMOVE "${OUTPUT}")
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
if(NOT profile_csv MATCHES "summary,schema_version,3")
    message(FATAL_ERROR "profile CSV does not identify schema version 3")
endif()
if(NOT profile_csv MATCHES "instruction,CALL_FUNC")
    message(FATAL_ERROR "profile CSV does not contain instruction rows")
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
if(profile_error MATCHES "VM PROFILE")
    message(FATAL_ERROR "profile table leaked to stderr despite file output")
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
if(NOT profile_table MATCHES "VM PROFILE.*Instructions.*Procedures and methods.*procedure_profile.worker")
    message(FATAL_ERROR "non-CSV filename did not select table output")
endif()
