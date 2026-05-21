cmake_minimum_required(VERSION 3.24)

set(CREXX_LLM_DEMO_OUTPUT_DIR "${CMAKE_BINARY_DIR}/demos/llm")
set(CREXX_LLM_GENERATE_DEMOS
  ollama_generate
  openai_generate
  anthropic_generate
  gemini_generate
)

add_custom_target(llm_generate_demos_bin)

foreach(_demo IN LISTS CREXX_LLM_GENERATE_DEMOS)
  set(_source "${CMAKE_CURRENT_SOURCE_DIR}/${_demo}.crexx")
  set(_output_base "${CREXX_LLM_DEMO_OUTPUT_DIR}/${_demo}")
  set(_rxbin "${_output_base}.rxbin")

  add_custom_command(
    OUTPUT "${_rxbin}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CREXX_LLM_DEMO_OUTPUT_DIR}"
    COMMAND $<TARGET_FILE:rxc> -i "${CMAKE_BINARY_DIR}/bin" -o "${_output_base}" "${_source}"
    COMMAND $<TARGET_FILE:rxas> -o "${_output_base}" "${_output_base}"
    DEPENDS rxc rxas library rxfnsg compiler_exit_bin "${_source}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Compiling ${_demo}.crexx ..."
    VERBATIM
  )

  add_custom_target("llm_${_demo}_demo_bin" DEPENDS "${_rxbin}")
  add_dependencies(llm_generate_demos_bin "llm_${_demo}_demo_bin")
endforeach()

set(CREXX_LLM_ADDRESS_DEMOS
  llm_address_environment
  llm_address_demo
)

set(CREXX_LLM_ADDRESS_DEMO_BINS)
foreach(_demo IN LISTS CREXX_LLM_ADDRESS_DEMOS)
  set(_source "${CMAKE_CURRENT_SOURCE_DIR}/${_demo}.crexx")
  set(_output_base "${CREXX_LLM_DEMO_OUTPUT_DIR}/${_demo}")
  set(_rxbin "${_output_base}.rxbin")
  list(APPEND CREXX_LLM_ADDRESS_DEMO_BINS "${_rxbin}")

  add_custom_command(
    OUTPUT "${_rxbin}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CREXX_LLM_DEMO_OUTPUT_DIR}"
    COMMAND $<TARGET_FILE:rxc> -i "${CMAKE_BINARY_DIR}/bin" -o "${_output_base}" "${_source}"
    COMMAND $<TARGET_FILE:rxas> -o "${_output_base}" "${_output_base}"
    DEPENDS rxc rxas library rxfnsg compiler_exit_bin "${_source}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Compiling ${_demo}.crexx ..."
    VERBATIM
  )
endforeach()

add_custom_target(llm_address_demo_bin
  DEPENDS
    "${CREXX_LLM_DEMO_OUTPUT_DIR}/llm_address_environment.rxbin"
    "${CREXX_LLM_DEMO_OUTPUT_DIR}/llm_address_demo.rxbin")
add_custom_target(llm_address_demos_bin DEPENDS ${CREXX_LLM_ADDRESS_DEMO_BINS})
