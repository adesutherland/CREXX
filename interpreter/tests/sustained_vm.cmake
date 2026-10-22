# Included from langtools/tests after the existing callable-service owners.
# This file defines buildable measurement targets first. Call the registration
# function only after isolated normal Debug measurement fixes workload sizes
# and the generous functional hang backstop (VM-REV-AC02).
find_package(Threads REQUIRED)
# The owning function adds shared platform objects, but its directory-local
# backend library list must also be visible when called from langtools/tests.
get_directory_property(_sustained_socket_libraries
  DIRECTORY "${CMAKE_SOURCE_DIR}/interpreter" DEFINITION CREXX_SOCKET_TARGET_LIBS)
if("${_sustained_socket_libraries}" MATCHES "OpenSSL::")
  find_package(OpenSSL REQUIRED)
endif()
include("${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake")
set(_sustained_source "${CMAKE_SOURCE_DIR}/interpreter/tests")
set(_sustained_dir "${CMAKE_CURRENT_BINARY_DIR}/sustained-vm")
file(MAKE_DIRECTORY "${_sustained_dir}")
add_dynamic_plugin_target(_sustainedvm "${_sustained_source}/sustained_vm_provider.c")
add_static_plugin_target(_sustainedvm "${_sustained_source}/sustained_vm_provider.c")
set_property(SOURCE "${_sustained_source}/sustained_vm_provider.c" APPEND PROPERTY
  OBJECT_DEPENDS "${CMAKE_SOURCE_DIR}/tests/rxpa/rxpa_objects.c")
set_target_properties(_sustainedvm PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${_sustained_dir}/provider"
  RUNTIME_OUTPUT_DIRECTORY "${_sustained_dir}/provider")

# The existing native object fixture owns all provider algorithms/counters.
# Its tiny wrapper publishes the same declared namespace from this private
# test provider. No production provider or public metric is introduced.
foreach(_scenario values providers levell)
  set(_sustained_files "${CMAKE_BINARY_DIR}/bin/library.rxbin")
  set(_sustained_dependencies library)
  set(_sustained_native_imports)
  set(_sustained_provider_inputs)
  if(_scenario STREQUAL "providers")
    list(APPEND _sustained_files "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
      "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin")
    list(APPEND _sustained_dependencies classlib rxfnsg _sustainedvm)
    list(APPEND _sustained_native_imports "${_sustained_dir}/provider/rx_sustainedvm.rxplugin")
    list(APPEND _sustained_provider_inputs "${CMAKE_SOURCE_DIR}/tests/rxpa/rxpa_objects.c"
      "${_sustained_source}/sustained_vm_provider.c")
  elseif(_scenario STREQUAL "levell")
    set(_sustained_files ${_service_files})
    set(_sustained_dependencies library rxfnsl rxlanglib llrxastext llbinmeta llfileoutput fs)
    list(APPEND _sustained_native_imports "${CMAKE_BINARY_DIR}/bin/rxfs.rxplugin")
  endif()
  crexx_add_import_root(sustained_vm_${_scenario}_imports
    DIRECTORY "${_sustained_dir}/imports-${_scenario}" OUTPUT_VARIABLE _sustained_imports
    FILES ${_sustained_files} ${_sustained_native_imports}
    DEPENDS ${_sustained_dependencies})
  crexx_add_fixture_command(OUTPUT "${_sustained_dir}/${_scenario}-linked.rxbin"
    BYPRODUCTS "${_sustained_dir}/${_scenario}.rxas" "${_sustained_dir}/${_scenario}.rxbin"
    COMMAND $<TARGET_FILE:rxc> -x --no-exe-import -i "${_sustained_dir}/imports-${_scenario}"
      -o "${_sustained_dir}/${_scenario}" "${_sustained_source}/rexx/sustained_vm_${_scenario}.crexx"
    COMMAND $<TARGET_FILE:rxas> "${_sustained_dir}/${_scenario}"
    COMMAND $<TARGET_FILE:rxlink> -r "${_scenario}" -s -o "${_sustained_dir}/${_scenario}-linked"
      "${_sustained_dir}/${_scenario}" ${_sustained_files}
    DEPENDS rxc rxas rxlink sustained_vm_${_scenario}_imports ${_sustained_imports}
      ${_sustained_provider_inputs}
      "${_sustained_source}/rexx/sustained_vm_${_scenario}.crexx"
    VERBATIM)
  add_custom_target(sustained_vm_${_scenario}_bin
    DEPENDS "${_sustained_dir}/${_scenario}-linked.rxbin")
endforeach()

foreach(_engine rxbvm rxtvm)
  if(NOT TARGET ${_engine}_core_objects)
    continue()
  endif()
  add_executable(sustained_vm_${_engine}
    "${_sustained_source}/test_sustained_vm.c"
    "${CMAKE_SOURCE_DIR}/interpreter/rxvml.c"
    $<TARGET_OBJECTS:${_engine}_core_objects>)
  # Private VM headers expose C11 atomics. The interpreter directory's settings
  # do not propagate to these targets, which are defined in langtools/tests.
  target_compile_features(sustained_vm_${_engine} PRIVATE c_std_11)
  if(MSVC)
    target_compile_options(sustained_vm_${_engine} PRIVATE /experimental:c11atomics)
  endif()
  target_include_directories(sustained_vm_${_engine} PRIVATE
    "${CMAKE_SOURCE_DIR}/interpreter" "${CMAKE_SOURCE_DIR}/interpreter/rxvmplugin"
    "${CMAKE_SOURCE_DIR}/interpreter/rxvml" "${CMAKE_SOURCE_DIR}/assembler"
    "${CMAKE_SOURCE_DIR}/binutils/include" "${CMAKE_SOURCE_DIR}/rxpa"
    "${CMAKE_SOURCE_DIR}/platform" "${CMAKE_SOURCE_DIR}/third_party/avl_tree"
    "${CMAKE_SOURCE_DIR}/third_party/utf8")
  # Force-loaded providers introduce RXPA references. Keep them before the
  # support archives so ELF linkers resolve those references in one scan.
  configure_linker_for_static_lib(sustained_vm_${_engine} _sustainedvm)
  # Callable publication uses the existing fs provider. Link its instrumented
  # static owner so no host-dependent provider search can select another DSO.
  configure_linker_for_static_lib(sustained_vm_${_engine} fs)
  target_link_libraries(sustained_vm_${_engine} rxvmplugin mc_decimal_manual
    avl_tree rxpa rxvmref rxbin rxpashim Threads::Threads)
  crexx_enable_vm_platform(sustained_vm_${_engine})
  target_link_libraries(sustained_vm_${_engine} ${_sustained_socket_libraries})
  if(_engine STREQUAL "rxbvm")
    target_compile_definitions(sustained_vm_${_engine} PRIVATE NTHREADED=1)
  endif()
  if(CREXX_VM_PROFILING)
    target_compile_definitions(sustained_vm_${_engine} PRIVATE CREXX_VM_PROFILING=1)
  endif()
endforeach()

function(crexx_register_sustained_vm_tests value_operations provider_operations levell_requests hang_timeout)
  # Explicit values, after isolated measurement; no silent platform count oracle.
  foreach(_engine rxbvm rxtvm)
    if(NOT TARGET sustained_vm_${_engine})
      continue()
    endif()
    foreach(_scenario values providers levell)
      if(_scenario STREQUAL "values")
        set(_count ${value_operations})
      elseif(_scenario STREQUAL "providers")
        set(_count ${provider_operations})
      else()
        set(_count ${levell_requests})
      endif()
      set(_arguments ${_scenario} "${_sustained_dir}/${_scenario}-linked.rxbin" ${_count})
      if(_scenario STREQUAL "levell")
        list(APPEND _arguments "${CMAKE_SOURCE_DIR}/langtools/tests/example.levell"
          "${_sustained_dir}/output-${_engine}.rxas")
      endif()
      set(_name sustained_vm_${_scenario}_${_engine})
      add_test(NAME ${_name} COMMAND $<TARGET_FILE:sustained_vm_${_engine}> ${_arguments})
      set_tests_properties(${_name} PROPERTIES RUN_SERIAL TRUE TIMEOUT ${hang_timeout}
        LABELS "interpreter;ownership;sustained-state")
      crexx_register_test_prep_targets(TESTS ${_name}
        TARGETS sustained_vm_${_engine} sustained_vm_${_scenario}_bin _sustainedvm_static fs_static)
    endforeach()
  endforeach()
endfunction()
