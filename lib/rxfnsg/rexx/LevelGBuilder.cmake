# CMake bootstraps one Level B program; that program owns the complete Level G
# graph and its parallel dependency waves; shared Unicode has its own producer.

set(_stage_g_root "${CMAKE_CURRENT_BINARY_DIR}/stage-g")
set(_stage_g_bootstrap_root "${_stage_g_root}/bootstrap")
set(_stage_g_bootstrap_import_root "${_stage_g_bootstrap_root}/imports")
set(_stage_g_controller_import_root
  "${_stage_g_bootstrap_root}/controller-imports")
set(_stage_g_work_root "${_stage_g_root}/work")
set(_stage_g_members_root "${_stage_g_work_root}/members")
if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
  set(_stage_g_default_jobs 30)
else()
  set(_stage_g_default_jobs 5)
endif()
set(CREXX_LEVEL_G_BUILD_JOBS "${_stage_g_default_jobs}" CACHE STRING
  "Parallel jobs used inside the Level B Level G builder")
if(NOT CREXX_LEVEL_G_BUILD_JOBS MATCHES "^[1-9][0-9]*$")
  message(FATAL_ERROR "CREXX_LEVEL_G_BUILD_JOBS must be a positive integer")
endif()

set(_stage_g_controller_source
  "${CMAKE_SOURCE_DIR}/tools/build-system/levelg/build_stage_g.crexx")
set(_stage_g_worker_source
  "${CMAKE_SOURCE_DIR}/tools/build-system/levelg/build_stage_g_worker.crexx")
set(_stage_g_controller_copy
  "${_stage_g_bootstrap_root}/controller-source/build_stage_g.crexx")
set(_stage_g_worker_copy
  "${_stage_g_bootstrap_root}/worker-source/build_stage_g_worker.crexx")
set(_stage_g_controller_rxas
  "${_stage_g_bootstrap_root}/build_stage_g.rxas")
set(_stage_g_controller_rxbin
  "${_stage_g_bootstrap_root}/build_stage_g.rxbin")
set(_stage_g_worker_rxas
  "${_stage_g_bootstrap_root}/build_stage_g_worker.rxas")
set(_stage_g_worker_rxbin
  "${_stage_g_bootstrap_root}/build_stage_g_worker.rxbin")
set(_stage_g_linked_builder
  "${_stage_g_bootstrap_root}/build_stage_g_linked.rxbin")

crexx_add_import_root(stage_g_builder_imports
  DIRECTORY "${_stage_g_bootstrap_import_root}"
  OUTPUT_VARIABLE _stage_g_builder_import_files
  FILES
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rx_hash.rxplugin"
  DEPENDS library classlib compiler_exit_bin _hash
  COMMENT "Staging exact imports for the Level B Level G builder ...")

crexx_add_fixture_command(
  OUTPUT "${_stage_g_linked_builder}"
  BYPRODUCTS
    "${_stage_g_controller_copy}"
    "${_stage_g_worker_copy}"
    "${_stage_g_controller_rxas}"
    "${_stage_g_controller_rxbin}"
    "${_stage_g_worker_rxas}"
    "${_stage_g_worker_rxbin}"
    "${_stage_g_controller_import_root}/build_stage_g_worker.rxbin"
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${_stage_g_bootstrap_root}/controller-source"
          "${_stage_g_bootstrap_root}/worker-source"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_stage_g_worker_source}" "${_stage_g_worker_copy}"
  COMMAND $<TARGET_FILE:rxc>
          --no-exe-import
          -i "${_stage_g_bootstrap_import_root}"
          -o build_stage_g_worker
          "${_stage_g_worker_copy}"
  COMMAND $<TARGET_FILE:rxas>
          -o build_stage_g_worker build_stage_g_worker
  COMMAND ${CMAKE_COMMAND} -E rm -rf
          "${_stage_g_controller_import_root}"
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "${_stage_g_controller_import_root}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_stage_g_worker_rxbin}"
          "${_stage_g_controller_import_root}/build_stage_g_worker.rxbin"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${_stage_g_controller_source}" "${_stage_g_controller_copy}"
  COMMAND $<TARGET_FILE:rxc>
          --no-exe-import
          -i "${_stage_g_bootstrap_import_root}"
          -i "${_stage_g_controller_import_root}"
          -o build_stage_g
          "${_stage_g_controller_copy}"
  COMMAND $<TARGET_FILE:rxas>
          -o build_stage_g build_stage_g
  COMMAND $<TARGET_FILE:rxlink>
          -s
          -o "${_stage_g_bootstrap_root}/build_stage_g_linked"
          "${_stage_g_controller_rxbin}"
          "${_stage_g_worker_rxbin}"
          "${CMAKE_BINARY_DIR}/bin/library.rxbin"
          "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
  DEPENDS
    "${_stage_g_controller_source}"
    "${_stage_g_worker_source}"
    stage_g_builder_imports
    ${_stage_g_builder_import_files}
    rxc rxas rxlink
    "$<TARGET_FILE:rxc>"
    "$<TARGET_FILE:rxas>"
    "$<TARGET_FILE:rxlink>"
  WORKING_DIRECTORY "${_stage_g_bootstrap_root}"
  COMMENT "Bootstrapping the Level B Level G builder ..."
  VERBATIM)

set(_stage_g_product_sources)
set(_stage_g_byproducts)
foreach(_stage_g_module IN LISTS RXFNSG_REXX_MODULES)
  list(APPEND _stage_g_product_sources "${CMAKE_CURRENT_SOURCE_DIR}/${_stage_g_module}.crexx")
  list(APPEND _stage_g_byproducts
    "${_stage_g_members_root}/${_stage_g_module}/${_stage_g_module}.rxas"
    "${_stage_g_members_root}/${_stage_g_module}/${_stage_g_module}.rxbin")
endforeach()

crexx_add_fixture_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin"
  BYPRODUCTS ${_stage_g_byproducts}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_stage_g_work_root}"
  COMMAND $<TARGET_FILE:rxbvm>
          "${_stage_g_linked_builder}"
          -a
          "${CMAKE_CURRENT_SOURCE_DIR}"
          "${_stage_g_work_root}"
          "${CMAKE_BINARY_DIR}/bin"
          "$<TARGET_FILE:rxc>"
          "$<TARGET_FILE:rxas>"
          "$<TARGET_FILE:rxlink>"
          "$<TARGET_FILE:rxbvm>"
          "${CMAKE_COMMAND}"
          "$<CONFIG>"
          "${CREXX_LEVEL_G_BUILD_JOBS}"
          all
  # Record successful producer completion even when the controller retained
  # byte-identical output after a prerequisite was regenerated.
  COMMAND ${CMAKE_COMMAND} -E touch_nocreate
          "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin"
  DEPENDS
    "${_stage_g_linked_builder}"
    ${_stage_g_product_sources}
    rxunicode_core
    "${RXUNICODE_CORE_RXBIN}"
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
    "$<TARGET_FILE:fs>"
    rxc rxas rxlink rxbvm library classlib compiler_exit_bin fs
    "$<TARGET_FILE:rxc>"
    "$<TARGET_FILE:rxas>"
    "$<TARGET_FILE:rxlink>"
    "$<TARGET_FILE:rxbvm>"
  WORKING_DIRECTORY "${_stage_g_bootstrap_root}"
  COMMENT "Building Level G with the shared Level B Unicode core ..."
  VERBATIM)

add_custom_target(rxfnsg ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin")
