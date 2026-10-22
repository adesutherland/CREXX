cmake_minimum_required(VERSION 3.24)

# Execution is optional and absent from grammar bootstrap/static targets.
set_property(GLOBAL PROPERTY RXFNSL_EXECUTE_SELECTORS llirexecute)
set(_imports "${CMAKE_CURRENT_BINARY_DIR}/imports")
crexx_add_import_root(rxfnsl_execute_imports DIRECTORY "${_imports}"
  OUTPUT_VARIABLE _import_files
  FILES "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxfnsl-analysis.rxbin"
  DEPENDS library rxfnsl rxfnsl_analysis)
set(_flags)
if(CREXX_LEVEL_L_NOOPT)
  list(APPEND _flags -n)
endif()
crexx_add_fixture_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/llirexecute.rxbin"
  COMMAND $<TARGET_FILE:rxc> ${_flags} -x --no-exe-import -i "${_imports}"
    -o llirexecute "${CMAKE_CURRENT_SOURCE_DIR}/llirexecute.crexx"
  COMMAND $<TARGET_FILE:rxas> ${_flags} llirexecute
  DEPENDS rxc rxas rxfnsl_execute_imports ${_import_files}
    "${CMAKE_CURRENT_SOURCE_DIR}/llirexecute.crexx"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
set(_control "${CMAKE_CURRENT_BINARY_DIR}/execute.ctl")
file(GENERATE OUTPUT "${_control}" CONTENT
  "INPUT ${CMAKE_CURRENT_BINARY_DIR}/llirexecute.rxbin\nINCLUDE llirexecute\nPRESERVE INLINE\nOUTPUT ${CMAKE_BINARY_DIR}/bin/rxfnsl-execute\n")
crexx_add_fixture_command(OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsl-execute.rxbin"
  COMMAND $<TARGET_FILE:rxlink> -c "${_control}"
  DEPENDS rxlink "${CMAKE_CURRENT_BINARY_DIR}/llirexecute.rxbin" "${_control}" VERBATIM)
add_custom_target(rxfnsl_execute ALL DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsl-execute.rxbin")
crexx_install_bytecode(rxfnsl_execute "${CMAKE_BINARY_DIR}/bin/rxfnsl-execute.rxbin")
