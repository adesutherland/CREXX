file(MAKE_DIRECTORY "${WORK}")
file(WRITE "${WORK}/provider.crexx" [=[options levelg srcmap
namespace mapped_provider expose makebox optionalbox
makebox: procedure = .optionalbox
  return .optionalbox(1)
optionalbox: class
  _value = .int
  *: factory
@"provider.rxpp"
@8l"    arg value = 7"
    arg value = 7
    _value = value
    return
  value: method = .int
    return _value
]=])
file(WRITE "${WORK}/consumer.crexx" [=[options levelg
import mapped_provider
main: procedure = .int
  box = .optionalbox()
  if box.value() <> 7 then return 1
  return 0
]=])
execute_process(COMMAND "${RXC}" --no-exe-import --diagnostics raw
        -o "${WORK}/consumer" "${WORK}/consumer.crexx"
        WORKING_DIRECTORY "${WORK}" RESULT_VARIABLE rc
        OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT "${rc}" STREQUAL "0")
    message(FATAL_ERROR "Source-mapped import failed (${rc}): ${out}${err}")
endif()
file(WRITE "${WORK}/badargs.crexx" [=[options levelg
makebox: procedure = .badbox
  return .badbox(1)
badbox: class
  *: factory
    nop
    arg value = 7
    return
]=])
execute_process(COMMAND "${RXC}" --no-exe-import --diagnostics raw
        -o "${WORK}/badargs" "${WORK}/badargs.crexx"
        WORKING_DIRECTORY "${WORK}" RESULT_VARIABLE rc
        OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT "${rc}" STREQUAL "2" OR NOT "${out}${err}" MATCHES "ARG_NOT_FIRST_INST")
    message(FATAL_ERROR "Invalid ARG must diagnose, not crash (${rc}): ${out}${err}")
endif()
# A malformed mapped import must fail closed and retain its source-map error.
file(WRITE "${WORK}/provider.crexx" [=[options levelg srcmap
namespace mapped_provider expose optionalbox
@1+2{bad
]=])
execute_process(COMMAND "${RXC}" --no-exe-import --diagnostics raw
        -o "${WORK}/consumer_bad" "${WORK}/consumer.crexx"
        WORKING_DIRECTORY "${WORK}" RESULT_VARIABLE rc
        OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT "${rc}" STREQUAL "2" OR NOT "${out}${err}" MATCHES "SRCMAP_UNBALANCED")
    message(FATAL_ERROR "Malformed mapped import must diagnose (${rc}): ${out}${err}")
endif()
