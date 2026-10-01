foreach(required_var CPRAG_RXC CPRAG_RXAS CPRAG_RXVME CPRAG_RXBVM
        CPRAG_CREXX_BIN_DIR CPRAG_PLUGIN_DIR CPRAG_MODEL CPRAG_CONFIG
        CPRAG_FILE CPRAG_CONFIG_FILE_MODULE CPRAG_GLOSSARY_MODULE CPRAG_PROFILE_MODULE
        CPRAG_PROFILE_FILE_MODULE CPRAG_SCENARIO CPRAG_FIXTURE
        CPRAG_SUBSCRIPTION_FIXTURE CPRAG_APPLICATION CPRAG_WORK_DIR)
    if(NOT DEFINED ${required_var} OR "${${required_var}}" STREQUAL "")
        message(FATAL_ERROR "${required_var} is required")
    endif()
endforeach()

file(REMOVE_RECURSE "${CPRAG_WORK_DIR}")
file(MAKE_DIRECTORY "${CPRAG_WORK_DIR}")
set(base_import "${CPRAG_PLUGIN_DIR};${CPRAG_CREXX_BIN_DIR}")
set(program_import "${CPRAG_WORK_DIR};${base_import}")
set(report "${CPRAG_WORK_DIR}/result.txt")
file(WRITE "${report}"
    "test=configuration-contract\nformat=crexx-rag.config/3\nformat_1_2_compatibility=verified\nprovider_calls=0\ncredential_reads=0\n")
file(WRITE "${CPRAG_WORK_DIR}/glossary-valid.tsv"
    "format\tcrexx-rag.glossary/1\nconcept\tBillingService\tapplication-component\tBilling Service\nconcept\tCustomerDatabase\tdata-store\tCustomer DB\nexclude\tDeprecatedSystem\n")
file(WRITE "${CPRAG_WORK_DIR}/glossary-duplicate.tsv"
    "format\tcrexx-rag.glossary/1\nconcept\tBillingService\tapplication-component\nconcept\tBillingService\tapplication-component\n")
file(WRITE "${CPRAG_WORK_DIR}/glossary-alias.tsv"
    "format\tcrexx-rag.glossary/1\nconcept\tBillingService\tapplication-component\tShared Alias\nconcept\tCustomerDatabase\tdata-store\tShared Alias\n")
file(WRITE "${CPRAG_WORK_DIR}/glossary-exclusion.tsv"
    "format\tcrexx-rag.glossary/1\nconcept\tBillingService\tapplication-component\tBilling Service\nexclude\tBilling Service\n")
file(WRITE "${CPRAG_WORK_DIR}/glossary-missing-format.tsv" "# no data records\n")
file(WRITE "${CPRAG_WORK_DIR}/profile-valid.tsv"
    "format\tcrexx-rag.profile/1\nprofile\tscottish-history-profile\t1\nconcept\tperson\nconcept\tplace\nrelationship\trelated-to\trelated-to\tfalse\nchunk\t1400\t180\tplain,markdown\nweight\tlexical\t1000000\nprompt\tadvisory\t1\tscottish-history-advisory\nprompt\textractor\t1\tscottish-history-extractor\nvalidator\tdirection-required\n")
file(WRITE "${CPRAG_WORK_DIR}/profile-mismatch.tsv"
    "format\tcrexx-rag.profile/1\nprofile\twrong-profile\t1\nconcept\tperson\nrelationship\trelated-to\trelated-to\tfalse\nchunk\t1400\t180\tplain\nweight\tlexical\t1000000\nprompt\tadvisory\t1\twrong-advisory\nprompt\textractor\t1\twrong-extractor\nvalidator\tdirection-required\n")

set(secret_marker "CONFIG_SECRET_MUST_NOT_APPEAR_8A21")
foreach(mode IN ITEMS noopt opt)
    set(mode_flag)
    if(mode STREQUAL "noopt")
        set(mode_flag --nooptimize)
    endif()
    # The frozen donor's explicit VM list still names retired rx_system.
    # Use the supported installed project route and its provider metadata.
    execute_process(COMMAND "${CPRAG_CREXX_BIN_DIR}/crexx"
        --program "${CPRAG_WORK_DIR}/scenario-${mode}"
        "${CPRAG_SCENARIO}" "${CPRAG_MODEL}" "${CPRAG_CONFIG}"
        "${CPRAG_FILE}" "${CPRAG_CONFIG_FILE_MODULE}" "${CPRAG_GLOSSARY_MODULE}"
        "${CPRAG_PROFILE_MODULE}" "${CPRAG_PROFILE_FILE_MODULE}"
        ${mode_flag} --jobs 1 --noexec --nocolor --verbose1 -i "${base_import}"
        RESULT_VARIABLE project_result OUTPUT_VARIABLE project_out ERROR_VARIABLE project_err
        TIMEOUT 180)
    if(NOT project_result EQUAL 0)
        message(FATAL_ERROR "${mode} project build failed:\n${project_out}${project_err}")
    endif()

    foreach(runtime_name IN ITEMS rxvme rxbvm)
        if(runtime_name STREQUAL "rxvme")
            set(runtime "${CPRAG_RXVME}")
        else()
            set(runtime "${CPRAG_RXBVM}")
        endif()
        set(cell "${mode}-${runtime_name}")
        execute_process(COMMAND "${CMAKE_COMMAND}" -E env
            "GEMINI_API_KEY=${secret_marker}"
            "${runtime}" --provider-path "${CPRAG_PLUGIN_DIR}" -l "${program_import}"
            "${CPRAG_WORK_DIR}/scenario-${mode}"
            -a "${cell}" "${CPRAG_FIXTURE}"
                "${CPRAG_WORK_DIR}/glossary-valid.tsv"
                "${CPRAG_WORK_DIR}/glossary-duplicate.tsv"
                "${CPRAG_WORK_DIR}/glossary-alias.tsv"
                "${CPRAG_WORK_DIR}/glossary-exclusion.tsv"
                "${CPRAG_WORK_DIR}/glossary-missing-format.tsv"
                "${CPRAG_WORK_DIR}/profile-valid.tsv"
                "${CPRAG_WORK_DIR}/profile-mismatch.tsv"
            RESULT_VARIABLE vm_result OUTPUT_VARIABLE vm_out ERROR_VARIABLE vm_err
            TIMEOUT 30)
        if(NOT vm_result EQUAL 0 OR NOT vm_out MATCHES
                "CONFIG_CONTRACT_OK cell=${cell} formats=1,2,3 identities=split settings=typed-and-bounded providers=2 gemini=2 env_refs=2 literal_secrets=0 executable_modules=0 glossary=validated profile=validated provider_calls=0")
            message(FATAL_ERROR "${cell} config scenario failed:\n${vm_out}${vm_err}")
        endif()
        if(vm_out MATCHES "${secret_marker}" OR vm_err MATCHES "${secret_marker}")
            message(FATAL_ERROR "${cell} exposed a resolved credential")
        endif()
        file(APPEND "${report}" "${cell}: ${vm_out}${vm_err}")
    endforeach()
endforeach()

execute_process(COMMAND "${CMAKE_COMMAND}" -E env
    "GEMINI_API_KEY=${secret_marker}"
    "${CPRAG_RXVME}" "${CPRAG_APPLICATION}" -a
    --config-file "${CPRAG_FIXTURE}"
    --profile generic-profile --format json doctor
    RESULT_VARIABLE cli_result OUTPUT_VARIABLE cli_out ERROR_VARIABLE cli_err
    TIMEOUT 30)
if(NOT cli_result EQUAL 0 OR NOT cli_out MATCHES
        "\"operation\":\"doctor\",\"status\":\"ok\"" OR
   NOT cli_out MATCHES "\"config_count\":1" OR
   NOT cli_out MATCHES "\"provider_id\":\"gemini-generate\"")
    message(FATAL_ERROR "linked CLI config-file smoke failed:\n${cli_out}${cli_err}")
endif()
if(cli_out MATCHES "${secret_marker}" OR cli_err MATCHES "${secret_marker}")
    message(FATAL_ERROR "linked CLI exposed a resolved credential")
endif()

execute_process(COMMAND "${CMAKE_COMMAND}" -E env
    "GEMINI_API_KEY=${secret_marker}"
    "${CPRAG_RXVME}" "${CPRAG_APPLICATION}" -a
    --config-file "${CPRAG_FIXTURE}"
    --profile generic-profile --format json config explain
    RESULT_VARIABLE explain_result OUTPUT_VARIABLE explain_out ERROR_VARIABLE explain_err
    TIMEOUT 30)
string(JSON explain_evidence_ceiling ERROR_VARIABLE explain_evidence_error GET
    "${explain_out}" records 4 fields maximum_evidence_bytes)
string(JSON explain_maintenance_length ERROR_VARIABLE explain_maintenance_error LENGTH
    "${explain_out}" records 5 fields)
string(JSON explain_maintenance_batch ERROR_VARIABLE explain_maintenance_batch_error GET
    "${explain_out}" records 5 fields batch_items)
if(NOT explain_result EQUAL 0 OR
   NOT explain_evidence_error STREQUAL "NOTFOUND" OR
   NOT explain_evidence_ceiling STREQUAL "262144" OR
   NOT explain_maintenance_error STREQUAL "NOTFOUND" OR
   NOT explain_maintenance_length EQUAL 11 OR
   NOT explain_maintenance_batch_error STREQUAL "NOTFOUND" OR
   NOT explain_maintenance_batch STREQUAL "1000" OR
   NOT explain_out MATCHES "\"narrative_output_tokens\":4096")
    message(FATAL_ERROR
        "linked configuration explanation was not bounded and isolated:\n${explain_out}${explain_err}")
endif()

# A data profile is selected by id in ordinary configuration and loaded only
# from the corresponding bounded data file.  No executable module name is
# accepted from operator configuration.
file(READ "${CPRAG_FIXTURE}" profile_config_text)
string(REPLACE "profiles = generic-profile,it-architecture-profile"
    "profiles = scottish-history-profile"
    profile_config_text "${profile_config_text}")
string(APPEND profile_config_text
    "\nprofile.scottish-history-profile.file = ${CPRAG_WORK_DIR}/profile-valid.tsv\n")
set(profile_config "${CPRAG_WORK_DIR}/profile-config.conf")
file(WRITE "${profile_config}" "${profile_config_text}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E env
    "GEMINI_API_KEY=${secret_marker}"
    "${CPRAG_RXVME}" "${CPRAG_APPLICATION}" -a
    --config-file "${profile_config}"
    --profile scottish-history-profile --format json profile show
    RESULT_VARIABLE profile_cli_result
    OUTPUT_VARIABLE profile_cli_out ERROR_VARIABLE profile_cli_err
    TIMEOUT 30)
string(JSON profile_maximum_chunk ERROR_VARIABLE profile_maximum_chunk_error
    GET "${profile_cli_out}" records 0 fields maximum_chunk_characters)
string(JSON profile_overlap ERROR_VARIABLE profile_overlap_error
    GET "${profile_cli_out}" records 0 fields overlap_characters)
if(NOT profile_cli_result EQUAL 0 OR
   NOT profile_cli_out MATCHES "\"profile_id\":\"scottish-history-profile\"" OR
   NOT profile_cli_out MATCHES "\"valid\":true" OR
   NOT profile_maximum_chunk_error STREQUAL "NOTFOUND" OR
   NOT profile_overlap_error STREQUAL "NOTFOUND" OR
   NOT profile_maximum_chunk STREQUAL "1400" OR
   NOT profile_overlap STREQUAL "180")
    message(FATAL_ERROR
        "linked data-defined profile smoke failed:\n${profile_cli_out}${profile_cli_err}")
endif()

message(STATUS "Offline configuration compile/runtime and CLI checks passed; lifecycle/ingestion section excluded")
