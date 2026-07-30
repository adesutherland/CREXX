file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

function(run_checked label)
    execute_process(
            COMMAND ${ARGN}
            RESULT_VARIABLE result
            OUTPUT_VARIABLE stdout
            ERROR_VARIABLE stderr)
    file(APPEND "${WORK}/commands-and-results.txt"
            "[${label}]\nstdout:\n${stdout}\nstderr:\n${stderr}\nexit=${result}\n\n")
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${label} failed (${result}): ${stderr}")
    endif()
endfunction()

run_checked(compile-noopt
        "${RXC}" -x -n -o "${WORK}/contract-noopt" "${SOURCE}")
run_checked(assemble-noopt
        "${RXAS}" -n -o "${WORK}/contract-noopt" "${WORK}/contract-noopt.rxas")
run_checked(compile-opt
        "${RXC}" -x -o "${WORK}/contract-opt" "${SOURCE}")
run_checked(assemble-opt
        "${RXAS}" -o "${WORK}/contract-opt" "${WORK}/contract-opt.rxas")

set(contract_args
        --operation operation_contract_test.operationcontract.execute
        --contract-version 1.0.0
        --nullable operation_contract_test.resultcontract.problem
        --optional-field operation_contract_test.errorcontract.details
        --error operation_contract_test.errorcontract)

run_checked(export-noopt
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        ${contract_args}
        --output "${WORK}/contract-noopt.rxcontract.json")
run_checked(export-opt
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-opt.rxbin"
        ${contract_args}
        --output "${WORK}/contract-opt.rxcontract.json")
run_checked(compare-optimized
        "${CMAKE_COMMAND}" -E compare_files
        "${WORK}/contract-noopt.rxcontract.json"
        "${WORK}/contract-opt.rxcontract.json")
run_checked(compare-golden
        "${CMAKE_COMMAND}" -E compare_files
        "${WORK}/contract-noopt.rxcontract.json"
        "${GOLDEN}")

run_checked(export-noerror-previous
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        --operation operation_contract_test.operationcontract.execute
        --contract-version 1.0.0
        --nullable operation_contract_test.resultcontract.problem
        --optional-field operation_contract_test.errorcontract.details
        --output "${WORK}/contract-noerror.rxcontract.json")
run_checked(compatibility-same
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        ${contract_args}
        --previous "${WORK}/contract-noopt.rxcontract.json"
        --output "${WORK}/contract-same.rxcontract.json")
run_checked(compatibility-additive-minor
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        --operation operation_contract_test.operationcontract.execute
        --contract-version 1.1.0
        --nullable operation_contract_test.resultcontract.problem
        --optional-field operation_contract_test.errorcontract.details
        --error operation_contract_test.errorcontract
        --previous "${WORK}/contract-noerror.rxcontract.json"
        --output "${WORK}/contract-minor.rxcontract.json")
run_checked(compatibility-breaking-major
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        --operation operation_contract_test.operationcontract.execute
        --contract-version 2.0.0
        --optional-field operation_contract_test.errorcontract.details
        --error operation_contract_test.errorcontract
        --previous "${WORK}/contract-noopt.rxcontract.json"
        --output "${WORK}/contract-major.rxcontract.json")

function(run_expected_failure label expected)
    execute_process(
            COMMAND ${ARGN}
            RESULT_VARIABLE result
            OUTPUT_VARIABLE stdout
            ERROR_VARIABLE stderr)
    file(APPEND "${WORK}/commands-and-results.txt"
            "[${label}]\nstdout:\n${stdout}\nstderr:\n${stderr}\nexit=${result}\n\n")
    if(result EQUAL 0 OR NOT stderr MATCHES "${expected}")
        message(FATAL_ERROR "${label} did not fail specifically: ${stderr}")
    endif()
endfunction()

run_expected_failure(compatibility-additive-without-minor
        "additive contract change requires a minor version increase"
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        ${contract_args}
        --previous "${WORK}/contract-noerror.rxcontract.json"
        --output "${WORK}/invalid-minor.rxcontract.json")
run_expected_failure(compatibility-breaking-without-major
        "breaking contract change requires a major version increase"
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        --operation operation_contract_test.operationcontract.execute
        --contract-version 1.1.0
        --optional-field operation_contract_test.errorcontract.details
        --error operation_contract_test.errorcontract
        --previous "${WORK}/contract-noopt.rxcontract.json"
        --output "${WORK}/invalid-major.rxcontract.json")

file(WRITE "${WORK}/malformed.rxcontract.json" "{\"format\":")
run_expected_failure(compatibility-malformed-previous
        "malformed previous contract JSON"
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        ${contract_args}
        --previous "${WORK}/malformed.rxcontract.json"
        --output "${WORK}/invalid-malformed.rxcontract.json")

file(WRITE "${WORK}/malformed-separator.rxcontract.json"
        "{\"format\" \"crexx.operation-contract\"}")
run_expected_failure(compatibility-malformed-separator
        "malformed previous contract JSON"
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        ${contract_args}
        --previous "${WORK}/malformed-separator.rxcontract.json"
        --output "${WORK}/invalid-malformed-separator.rxcontract.json")

run_expected_failure(unknown-nullable
        "nullable field.*is not present"
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        --operation operation_contract_test.operationcontract.execute
        --contract-version 1.0.0
        --nullable operation_contract_test.resultcontract.absent
        --output "${WORK}/invalid.rxcontract.json")

run_checked(compile-invalid-fixture
        "${RXC}" -x -n -o "${WORK}/invalid-contract" "${INVALID_SOURCE}")
run_checked(assemble-invalid-fixture
        "${RXAS}" -n -o "${WORK}/invalid-contract" "${WORK}/invalid-contract.rxas")
run_checked(assemble-ambiguous-fixture
        "${RXAS}" -n -o "${WORK}/ambiguous-contract" "${AMBIGUOUS_RXAS}")

function(run_invalid_operation member expected)
    run_expected_failure("invalid-${member}" "${expected}"
            "${CONTRACT_TOOL}"
            --rxbin "${WORK}/invalid-contract.rxbin"
            --operation "operation_contract_invalid.operationcontract.${member}"
            --contract-version 1.0.0
            --output "${WORK}/invalid-${member}.rxcontract.json")
endfunction()

run_invalid_operation(classinput "is a concrete class")
run_invalid_operation(refinput "uses a reference or vararg")
run_invalid_operation(vararginput "uses a reference or vararg")
run_invalid_operation(fixedinput "not a one-dimensional dynamic array")
run_invalid_operation(multidiminput "not a one-dimensional dynamic array")
run_invalid_operation(objectinput "outside the fail-closed format-1 contract slice")
run_invalid_operation(badmemberresult "fields must be zero-argument methods")
run_invalid_operation(defaultresult "is not an abstract field method")
run_invalid_operation(factoryresult "is not an abstract field method")
run_invalid_operation(defaultoperation "must be an abstract interface method")

run_expected_failure(ambiguous-relative-type
        "type '.payload' is ambiguous"
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/ambiguous-contract.rxbin"
        --operation ambiguous_contract.operationcontract.execute
        --contract-version 1.0.0
        --output "${WORK}/invalid-ambiguous.rxcontract.json")
run_expected_failure(unknown-operation-owner
        "is not a compiled Level B interface"
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        --operation operation_contract_test.absent.execute
        --contract-version 1.0.0
        --output "${WORK}/invalid-owner.rxcontract.json")
run_expected_failure(unknown-operation-member
        "is not declared by its interface"
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        --operation operation_contract_test.operationcontract.absent
        --contract-version 1.0.0
        --output "${WORK}/invalid-member.rxcontract.json")
run_expected_failure(duplicate-error
        "is declared more than once"
        "${CONTRACT_TOOL}"
        --rxbin "${WORK}/contract-noopt.rxbin"
        --operation operation_contract_test.operationcontract.execute
        --contract-version 1.0.0
        --error operation_contract_test.errorcontract
        --error operation_contract_test.errorcontract
        --output "${WORK}/invalid-duplicate-error.rxcontract.json")
