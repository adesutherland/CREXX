if(NOT DEFINED RXAS_SOURCE OR NOT EXISTS "${RXAS_SOURCE}")
  message(FATAL_ERROR "optimized HTTP policy RXAS was not provided")
endif()

file(READ "${RXAS_SOURCE}" rxas)
string(FIND "${rxas}" "if _scenario = 2 then do" scenario_start)
string(FIND "${rxas}" "if _scenario = 3 then do" scenario_end)
if(scenario_start LESS 0 OR scenario_end LESS_EQUAL scenario_start)
  message(FATAL_ERROR "could not isolate the scenario-2 reassignment proof")
endif()

math(EXPR scenario_length "${scenario_end} - ${scenario_start}")
string(SUBSTRING "${rxas}" ${scenario_start} ${scenario_length} scenario_body)
string(REGEX MATCH "sockaccept[ \t]+(r[0-9]+)," accept_match "${scenario_body}")
if(NOT accept_match)
  message(FATAL_ERROR "scenario-2 socket reassignment lost its inline accept result")
endif()
set(accepted_client "${CMAKE_MATCH_1}")

if(NOT scenario_body MATCHES
   "call[0-9]*[ \t]+[^\n]*read_request\\(\\),${accepted_client}([\n]|$)")
  message(FATAL_ERROR
    "read_request did not consume the reaching skipped-copy source ${accepted_client}:\n${scenario_body}")
endif()

message(STATUS
  "read_request consumes reaching skipped-copy source ${accepted_client}")
