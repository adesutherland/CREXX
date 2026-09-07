# A binary intrinsic's storage selector is syntax, not an object class. Use
# isolated import roots so this reproduces even without an ambient rxfnsg image.
function(run_checked label)
    execute_process(COMMAND ${ARGN} WORKING_DIRECTORY "${cell}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT "${rc}" STREQUAL "0")
        message(FATAL_ERROR "${label}: ${rc}\n${out}\n${err}")
    endif()
endfunction()

foreach(kind IN ITEMS sizeof at packed compare)
    set(cell "${WORK_ROOT}/${kind}")
    set(imports "${cell}/imports")
    set(sources "${cell}/sources")
    file(MAKE_DIRECTORY "${imports}" "${sources}" "${cell}/provider" "${cell}/runtime")
    file(COPY "${BIN}/library.rxbin" "${BIN}/classlib.rxbin" "${BIN}/rxcexits.rxbin"
        DESTINATION "${imports}")
    if(kind STREQUAL sizeof)
        set(expression "count * <sizeof..float>")
        set(expected 16)
    elseif(kind STREQUAL at)
        set(expression "<at..u8>(0) data")
        set(expected 7)
    elseif(kind STREQUAL packed)
        set(expression "<packed..float>(0) data")
        set(expected 3.5)
    else()
        set(expression "<compare..u8>(data, 0, 7)")
        set(expected 0)
    endif()
    file(WRITE "${cell}/provider/provider.crexx" "options levelb\nnamespace storage_probe expose layout\nlayout: class\n  *: factory\n    return\n  run: method = .float\n    arg data = .binary, count = .int\n    return ${expression}\n")
    run_checked("${kind} compile provider" "${RXC}" --no-exe-import -i "${imports}"
        -o "${cell}/provider/provider" "${cell}/provider/provider.crexx")
    run_checked("${kind} assemble provider" "${RXAS}" -o "${imports}/provider"
        "${cell}/provider/provider")
    # A second binary's public signature forces the class declaration to be
    # materialized without making storage_probe a source import of the consumer.
    file(WRITE "${cell}/provider/bridge.crexx" "options levelb\nnamespace storage_bridge expose identity\nidentity: procedure = .storage_probe..layout\n  arg value = .storage_probe..layout\n  return value\n")
    run_checked("${kind} compile bridge" "${RXC}" --no-exe-import -i "${imports}"
        -o "${cell}/provider/bridge" "${cell}/provider/bridge.crexx")
    run_checked("${kind} assemble bridge" "${RXAS}" -o "${imports}/bridge" "${cell}/provider/bridge")

    set(probe "${sources}/probe.crexx")
    # Inline declarations are inspected even if the consumer does not use them.
    # No namespace import or class use should make extension's body a dependency.
    file(WRITE "${probe}" "options levelb\nimport rxfnsb\nmain: procedure = .int\n  if abs(-7) <> 7 then return 1\n  return 0\n")
    set(consumer "${cell}/runtime/consumer.crexx")
    set(extension "${sources}/extension.crexx")
    # Exercise the same imported bodies separately: using this class makes its
    # namespace visible, so that is not an unused-source invalidation probe.
    file(WRITE "${consumer}" "options levelb\nimport storage_probe\nmain: procedure = .int\n  box = .layout()\n  data = '0700000000000000'x as .binary\n")
    if(kind STREQUAL packed)
        file(APPEND "${consumer}" "  <packed..float>(0) data = 3.5\n")
    endif()
    file(APPEND "${consumer}" "  if box.run(data, 2) <> ${expected} then return 1\n  return 0\n")

    foreach(mode IN ITEMS opt noopt)
        set(options)
        if(mode STREQUAL noopt)
            list(APPEND options -n)
        endif()
        file(WRITE "${extension}" "options levelb\nnamespace storage_probe expose unused_extension\nunused_extension: procedure = .int\n  return 1\n")
        set(snapshot "${cell}/${mode}.snapshot")
        set(arguments --no-exe-import ${options} -s "${sources}" -i "${imports}"
            -o "${cell}/probe_${mode}" "${probe}")
        run_checked("${kind}/${mode} compile" "${RXC}" --project-dependencies "${snapshot}" ${arguments})
        run_checked("${kind}/${mode} fresh snapshot" "${RXC}" --check-project-dependencies "${snapshot}" ${arguments})
        file(APPEND "${extension}" "\n/* private unused body edit */\n")
        run_checked("${kind}/${mode} unused source must not invalidate" "${RXC}" --check-project-dependencies "${snapshot}" ${arguments})
        run_checked("${kind}/${mode} compile runtime consumer" "${RXC}" --no-exe-import
            ${options} -i "${imports}" -o "${cell}/consumer_${mode}" "${consumer}")
        file(READ "${cell}/consumer_${mode}.rxas" assembly)
        if(mode STREQUAL opt AND assembly MATCHES "call[0-9]* [^\n]*storage_probe\\.layout\\.run\\(\\)")
            message(FATAL_ERROR "${kind}: supported method must remain inlined")
        endif()
        run_checked("${kind}/${mode} assemble" "${RXAS}" -o "${cell}/consumer_${mode}" "${cell}/consumer_${mode}")
        run_checked("${kind}/${mode} link" "${RXLINK}" -s -o "${cell}/program_${mode}"
            "${cell}/consumer_${mode}.rxbin" "${imports}/provider.rxbin"
            "${imports}/library.rxbin" "${imports}/classlib.rxbin")
        run_checked("${kind}/${mode} run" "${RXVME}" "${cell}/program_${mode}.rxbin")
    endforeach()
endforeach()
