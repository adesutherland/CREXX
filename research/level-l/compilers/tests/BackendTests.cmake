find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(llcompiler_say_measure
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-say.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/say-measure"
    --mode opt --vm rxbvm
  DEPENDS llcompiler_llrxc-c_opt llcompiler_llrxc-g_opt llcompiler_target_opt
    rxas rxlink rxbvm library classlib rxfnsc fs cstream
  USES_TERMINAL VERBATIM)
add_custom_target(llcompiler_step07_measure
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-step07.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/step07-measure"
    --mode opt --vm rxbvm --native-controls
  DEPENDS llcompiler_backend-step07_opt llcompiler_llrxc-c_opt llcompiler_llrxc-g_opt
    llcompiler_target_opt library classlib rxfnsc rxunicode_core fs cstream rxbvm rxas rxlink
  VERBATIM)
add_custom_target(llcompiler_poc_measure
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-poc.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/poc-measure"
    --mode opt --vm rxbvm
  DEPENDS llcompiler_backend-poc_opt llcompiler_llrxc-c_opt llcompiler_llrxc-g_opt
    llcompiler_target_opt library rxfnsc classlib rxunicode_core rxbvm rxas rxlink
  VERBATIM)
add_custom_target(llcompiler_backend_measure
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-backend.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/backend-measure"
    --mode noopt --vm rxbvm
  DEPENDS llcompiler_llrxc-c_noopt llcompiler_llrxc-g_noopt llcompiler_target_noopt llcompiler_backend-tests_noopt
    library rxfnsc classlib rxunicode_core rxbvm rxas rxlink
  VERBATIM)

# Retained isolated provenance measurement target; normal CTest lanes follow.
add_custom_target(llcompiler_provenance_measure
  COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-provenance.py"
    --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/provenance-measure"
    --mode noopt --vm rxbvm
  DEPENDS llcompiler_llrxc-c_noopt llcompiler_llrxc-g_noopt llcompiler_target_noopt llcompiler_backend-tests_noopt rxbvm rxas rxlink
  VERBATIM)

# Isolated measurements: 89s Debug (29 corpus cases), 263s maintained Apple
# ASan (31 cases), including nested tools. Serialize whole aggregates; 1800s
# is a hang guard with room for slower hosted machines, not a speed criterion.
foreach(_mode opt noopt)
  foreach(_vm rxbvm rxtvm)
    if(TARGET ${_vm})
      # SAY: isolated 40 commands measured at 24s Debug / 84s Apple ASan.
      # Serialize nested tools; deadlines are generous hang guards only.
      set(_say "llcompiler_say_${_mode}_${_vm}")
      add_test(NAME ${_say}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-say.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_say}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_say} PROPERTIES
        PASS_REGULAR_EXPRESSION "SAY PASS: 40 commands"
        TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_say}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_target_${_mode} library classlib rxfnsc fs cstream rxas rxlink ${_vm})
      set(_test "llcompiler_backend_${_mode}_${_vm}")
      add_test(NAME ${_test}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-backend.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_test}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_test} PROPERTIES
        PASS_REGULAR_EXPRESSION "BACKEND_E2E[|]cases=31[|]commands=232[|]failures=0"
        TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_test}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_backend-tests_${_mode} llcompiler_target_${_mode}
          library rxfnsc classlib rxunicode_core rxas rxlink ${_vm})

      # Isolated 90-command provenance proof: 28s Debug, 90s Apple ASan.
      # Keep nested tools serialized, with generous whole/inner hang guards.
      set(_provenance "llcompiler_provenance_${_mode}_${_vm}")
      add_test(NAME ${_provenance}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-provenance.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_provenance}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_provenance} PROPERTIES
        PASS_REGULAR_EXPRESSION "PROVENANCE_E2E[|]failures=0"
        TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_provenance}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_backend-tests_${_mode} llcompiler_target_${_mode}
          library rxfnsc rxas rxlink ${_vm})

      # STEP05 isolated 33-command aggregate: 32s Debug, 65s Apple ASan.
      # Compiler IR on/off is independent of the B host's opt/noopt build.
      set(_optimise "llcompiler_optimise_${_mode}_${_vm}")
      add_test(NAME ${_optimise}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-optimise.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_optimise}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_optimise} PROPERTIES
        PASS_REGULAR_EXPRESSION "OPTIMISE_E2E[|]commands=33[|]failures=0"
        TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_optimise}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_backend-optimise_${_mode} llcompiler_target_${_mode}
          library rxfnsc classlib rxunicode_core rxas rxlink ${_vm})

      # STEP06: 54 nested commands, isolated Debug 32s / Apple ASan 118s.
      # Serial whole tests and 300s inner / 900s outer limits are hang guards.
      set(_poc "llcompiler_poc_${_mode}_${_vm}")
      add_test(NAME ${_poc}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-poc.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_poc}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_poc} PROPERTIES
        PASS_REGULAR_EXPRESSION "POC_E2E[|]commands=54[|]failures=0"
        TIMEOUT 900 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_poc}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_backend-poc_${_mode} llcompiler_target_${_mode}
          library rxfnsc classlib rxunicode_core rxas rxlink ${_vm})

      # STEP07A: isolated Debug / maintained Apple ASan measured before
      # registration (see PLAN.md, STEP07A validation). Serial whole tests;
      # 300s inner / 1800s outer limits are hang guards, not speed criteria.
      set(_step07 "llcompiler_step07_${_mode}_${_vm}")
      add_test(NAME ${_step07}
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-step07.py"
          --build "${CMAKE_BINARY_DIR}" --work "${CMAKE_CURRENT_BINARY_DIR}/checks/${_step07}"
          --mode ${_mode} --vm ${_vm})
      set_tests_properties(${_step07} PROPERTIES
        PASS_REGULAR_EXPRESSION "STEP07_E2E[|]commands=93[|]failures=0"
        TIMEOUT 1800 RUN_SERIAL TRUE RESOURCE_LOCK llcompiler_backend
        LABELS "library;level-l;compiler-backend;optimizer-parity")
      crexx_register_test_prep_targets(TESTS ${_step07}
        TARGETS llcompiler_llrxc-c_${_mode} llcompiler_llrxc-g_${_mode}
          llcompiler_backend-step07_${_mode} llcompiler_target_${_mode}
          library classlib rxfnsc rxunicode_core fs cstream rxas rxlink ${_vm})
    endif()
  endforeach()
endforeach()
