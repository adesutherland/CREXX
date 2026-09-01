cmake_minimum_required(VERSION 3.24)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxConcurrencyTests.cmake)

set(CLSS
  StringIterable
  StringIterator
  ObjectIterable
  Iterable
  ObjectPrintable
  Printable
  ObjectEquatable
  ObjectKeyStrategy
  ObjectIterator
  Iterator
  StringArrayList
  StringArrayListIterator
  ObjectArrayList
  ArrayList
  ObjectArrayListIterator
  ArrayListIterator
  ObjectObjectTreeMapIterator
  ObjectObjectHashMapIterator
  TreeMapIterator
  ObjectObjectTreeMap
  ObjectObjectHashMap
  Map
  Bag
  ArrayBag
  ArrayBagIterator
  TreeMap
  ObjectComparator
  Comparator
  RexxComparator
  StringHashMap
  StringHashMapIterator
  StringObjectHashMap
  StringObjectHashMapIterator
  StringHashSet
  StringHashSetIterator
  StringLinkedList
  ObjectLinkedList
  Rexx
  StringStack
  ObjectStack
  StringTreeMap
  StringOldTreeMap
  StringTreeMapIterator
  StringObjectTreeMap
  StringObjectTreeMapIterator
  StringTreeSet
  StringTreeSetIterator
  Qfind
  Scanlex
  Concurrency
  DateTime
  ISO8601
  OffsetDateTime
)

set(CLASSLIB_NATIVE_CLSS
  Id
  KeyDB
  Os
)

set(CLSBINS)
set(CLASSLIB_MEMBER_ROOT "${CMAKE_CURRENT_BINARY_DIR}/members")
foreach(CLS IN LISTS CLSS)
  list(APPEND CLSBINS
    "${CLASSLIB_MEMBER_ROOT}/${CLS}/${CLS}.rxbin")
endforeach()

set(CLASSLIB_MEMBER_BUILD_DEPS
  rxc
  rxas
  classlib_imports
)

set(CLASSLIB_IMPORT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/imports/main")
set(CLASSLIB_IMPORT_LIBRARY "${CLASSLIB_IMPORT_ROOT}/library.rxbin")
set(CLASSLIB_IMPORT_EXITS "${CLASSLIB_IMPORT_ROOT}/rxcexits.rxbin")
add_custom_command(
  OUTPUT
    "${CLASSLIB_IMPORT_LIBRARY}"
    "${CLASSLIB_IMPORT_EXITS}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CLASSLIB_IMPORT_ROOT}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CLASSLIB_IMPORT_LIBRARY}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
    "${CLASSLIB_IMPORT_EXITS}"
  DEPENDS
    library
    compiler_exit_bin
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
  COMMENT "Staging immutable class library compiler imports ..."
  VERBATIM
)
add_custom_target(classlib_imports
  DEPENDS
    "${CLASSLIB_IMPORT_LIBRARY}"
    "${CLASSLIB_IMPORT_EXITS}"
)

set(CLASSLIB_SELECTORS ${CLSS})
set(CLASS_DEPS_StringIterable StringIterator)
set(CLASS_DEPS_RexxComparator ObjectComparator Comparator)
set(CLASS_DEPS_ObjectIterable ObjectIterator)
set(CLASS_DEPS_Rexx ObjectPrintable)
set(CLASS_DEPS_StringArrayList StringIterable)
set(CLASS_DEPS_StringArrayListIterator StringArrayList StringIterator)
set(CLASS_DEPS_ObjectArrayList ObjectIterable)
set(CLASS_DEPS_ObjectArrayListIterator ObjectArrayList ObjectIterator)
set(CLASS_DEPS_ObjectArrayListIterator ObjectArrayList ObjectIterator)
set(CLASS_DEPS_ArrayList ObjectIterable)
set(CLASS_DEPS_ArrayListIterator ArrayList Iterator)
set(CLASS_DEPS_Iterable Iterator)
set(CLASS_DEPS_ArrayListIterator ArrayList Iterator)
set(CLASS_DEPS_StringHashMap StringTreeSet StringHashMapIterator StringIterable)
set(CLASS_DEPS_StringHashMapIterator StringIterator)
set(CLASS_DEPS_StringObjectHashMap StringTreeSet StringObjectHashMapIterator StringIterator StringIterable)
set(CLASS_DEPS_StringObjectHashMapIterator StringIterator)
set(CLASS_DEPS_StringHashSet StringHashSetIterator)
set(CLASS_DEPS_StringHashSetIterator StringIterator)
set(CLASS_DEPS_StringOldTreeMap StringIterable StringTreeMapIterator StringTreeSet)
set(CLASS_DEPS_StringTreeMap StringIterable StringTreeMapIterator StringTreeSet ObjectPrintable)
set(CLASS_DEPS_StringTreeMapIterator StringIterator)
set(CLASS_DEPS_StringObjectTreeMap StringTreeSet StringObjectTreeMapIterator StringIterator StringIterable)
set(CLASS_DEPS_StringObjectTreeMapIterator StringIterator ObjectPrintable)
set(CLASS_DEPS_StringTreeSet StringIterable StringIterator StringTreeSetIterator)
set(CLASS_DEPS_StringTreeSetIterator StringIterator)
set(CLASS_DEPS_ObjectObjectTreeMapIterator ObjectIterator)
set(CLASS_DEPS_ObjectObjectTreeMap ObjectObjectTreeMapIterator ObjectComparator ObjectPrintable)
set(CLASS_DEPS_ObjectObjectHashMapIterator ObjectIterator)
set(CLASS_DEPS_ObjectObjectHashMap ObjectObjectHashMapIterator ObjectKeyStrategy Map)
set(CLASS_DEPS_TreeMap Map TreeMapIterator Comparator Printable ObjectPrintable)
set(CLASS_DEPS_ArrayBag  ArrayBagIterator Bag Printable)
set(CLASS_DEPS_DateTime Printable)
set(CLASS_DEPS_ISO8601 DateTime Printable)

# Classlib currently has route-sensitive source/RXBIN metadata semantics. Keep
# the source side of cycles explicit and stage every other internal provider as
# a generated RXBIN until route parity is qualified in Phase 2.
set(CLASS_SOURCE_DEPS_StringArrayList StringArrayListIterator)
set(CLASS_SOURCE_DEPS_ObjectArrayList ObjectArrayListIterator)
set(CLASS_SOURCE_DEPS_ArrayList ArrayListIterator)
set(CLASS_SOURCE_DEPS_ArrayBagIterator ArrayBag)
set(CLASS_SOURCE_DEPS_RexxComparator Rexx)
set(CLASS_SOURCE_DEPS_ISO8601 OffsetDateTime)
set(CLASS_SOURCE_DEPS_OffsetDateTime DateTime)
foreach(_binfile IN LISTS CLSBINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)
  get_filename_component(_member_dir "${_binfile}" DIRECTORY)
  # Makefile generators enter WORKING_DIRECTORY before executing the first
  # custom command, whereas Ninja may materialize an output directory for us.
  # Own the directory at configure time so both generators have the same clean
  # build contract.
  file(MAKE_DIRECTORY "${_member_dir}")
  set(_class_source_modules ${CLASS_SOURCE_DEPS_${_basename}})
  list(REMOVE_ITEM _class_source_modules "${_basename}")
  list(REMOVE_DUPLICATES _class_source_modules)
  set(_class_binary_modules ${CLASS_DEPS_${_basename}})
  foreach(_class_source_module IN LISTS _class_source_modules)
    list(APPEND _class_binary_modules
      ${CLASS_DEPS_${_class_source_module}})
  endforeach()
  set(_class_dep_index 0)
  list(LENGTH _class_binary_modules _class_dep_count)
  while(_class_dep_index LESS _class_dep_count)
    list(GET _class_binary_modules ${_class_dep_index} _class_dep)
    list(APPEND _class_binary_modules ${CLASS_DEPS_${_class_dep}})
    list(REMOVE_DUPLICATES _class_binary_modules)
    math(EXPR _class_dep_index "${_class_dep_index} + 1")
    list(LENGTH _class_binary_modules _class_dep_count)
  endwhile()
  list(REMOVE_ITEM _class_binary_modules
    "${_basename}" ${_class_source_modules})
  list(REMOVE_DUPLICATES _class_binary_modules)

  set(_class_source_deps)
  set(_class_staged_sources)
  set(_class_stage_source_commands)
  set(_class_binary_deps)
  set(_class_import_roots "${CLASSLIB_IMPORT_ROOT}")
  foreach(_dep IN LISTS _class_binary_modules)
    list(APPEND _class_binary_deps
      "${CLASSLIB_MEMBER_ROOT}/${_dep}/${_dep}.rxbin")
    list(APPEND _class_import_roots
      "${CLASSLIB_MEMBER_ROOT}/${_dep}")
  endforeach()
  string(JOIN ";" _class_source_signature ${_class_source_modules})
  string(JOIN ";" _class_binary_signature ${_class_binary_modules})
  string(SHA256 _class_dep_signature
    "${_basename};source=${_class_source_signature};binary=${_class_binary_signature}")
  string(SUBSTRING "${_class_dep_signature}" 0 12 _class_dep_signature_short)
  set(_member_source_root
    "${_member_dir}/sources-${_class_dep_signature_short}")
  set(_member_source "${_member_source_root}/${_basename}.crexx")
  foreach(_dep IN LISTS _class_source_modules)
    set(_class_dep_source "${CMAKE_CURRENT_SOURCE_DIR}/${_dep}.crexx")
    set(_class_staged_source "${_member_source_root}/${_dep}.crexx")
    list(APPEND _class_source_deps "${_class_dep_source}")
    list(APPEND _class_staged_sources "${_class_staged_source}")
    list(APPEND _class_stage_source_commands
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${_class_dep_source}"
        "${_class_staged_source}")
  endforeach()
  add_custom_command(
    OUTPUT "${_binfile}"
    BYPRODUCTS
      "${_member_source}"
      "${_member_dir}/${_basename}.rxas"
      ${_class_staged_sources}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_member_source_root}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      "${_member_source}"
    ${_class_stage_source_commands}
    COMMAND $<TARGET_FILE:rxc>
    --no-exe-import
    -i "${_class_import_roots}"
    -o "${_basename}"
    "${_member_source}"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      classlib_imports
      "${CLASSLIB_IMPORT_LIBRARY}"
      "${CLASSLIB_IMPORT_EXITS}"
      ${_class_source_deps}
      ${_class_binary_deps}
      ${CLASSLIB_MEMBER_BUILD_DEPS}
    WORKING_DIRECTORY "${_member_dir}"
    COMMENT "cREXX compile and assemble ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_classlib_control_file ${CMAKE_CURRENT_BINARY_DIR}/classlib_link_$<CONFIG>.ctl)
set(_classlib_link_root "${CMAKE_CURRENT_BINARY_DIR}/linked/main")
set(_classlib_link_image "${_classlib_link_root}/classlib.rxbin")
set(_classlib_publish_image "${CMAKE_BINARY_DIR}/bin/classlib.rxbin")
set(_classlib_publish_temp "${CMAKE_CURRENT_BINARY_DIR}/publish/classlib.rxbin.tmp")
string(CONCAT _classlib_control_text
  "* Auto-generated control file for the consolidated cREXX class library\n")
foreach(_cls_bin IN LISTS CLSBINS)
  string(APPEND _classlib_control_text "INPUT ${_cls_bin}\n")
endforeach()
foreach(_selector IN LISTS CLASSLIB_SELECTORS)
  string(APPEND _classlib_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _classlib_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${_classlib_link_root}/classlib\n")

file(GENERATE OUTPUT ${_classlib_control_file} CONTENT "${_classlib_control_text}")

add_custom_command(
  OUTPUT "${_classlib_link_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_classlib_link_root}"
  COMMAND $<TARGET_FILE:rxlink> -c ${_classlib_control_file}
  DEPENDS rxlink ${CLSBINS} ${_classlib_control_file}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked Class Library Image ..."
  VERBATIM
)

add_custom_command(
  OUTPUT "${_classlib_publish_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/publish"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${_classlib_link_image}"
    "${_classlib_publish_temp}"
  COMMAND ${CMAKE_COMMAND} -E rename
    "${_classlib_publish_temp}"
    "${_classlib_publish_image}"
  DEPENDS "${_classlib_link_image}"
  COMMENT "Publishing consolidated class library image ..."
  VERBATIM
)

add_custom_target(classlib ALL
  DEPENDS "${_classlib_publish_image}"
)

add_dependencies(classlib library)

set(CLASSLIB_NATIVE_MEMBER_ROOT "${CREXX_CLASSLIB_NATIVE_MEMBER_ROOT}")
set(CLASSLIB_NATIVE_IMPORT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/imports/native")
set(CLASSLIB_NATIVE_IMPORT_LIBRARY "${CLASSLIB_NATIVE_IMPORT_ROOT}/library.rxbin")
set(CLASSLIB_NATIVE_IMPORT_EXITS "${CLASSLIB_NATIVE_IMPORT_ROOT}/rxcexits.rxbin")
set(CLASSLIB_NATIVE_BINS)
foreach(CLS IN LISTS CLASSLIB_NATIVE_CLSS)
  list(APPEND CLASSLIB_NATIVE_BINS
    "${CLASSLIB_NATIVE_MEMBER_ROOT}/${CLS}/${CLS}.rxbin")
endforeach()

add_custom_command(
  OUTPUT
    "${CLASSLIB_NATIVE_IMPORT_LIBRARY}"
    "${CLASSLIB_NATIVE_IMPORT_EXITS}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CLASSLIB_NATIVE_IMPORT_ROOT}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CLASSLIB_NATIVE_IMPORT_LIBRARY}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
    "${CLASSLIB_NATIVE_IMPORT_EXITS}"
  DEPENDS
    library
    compiler_exit_bin
    "${CMAKE_BINARY_DIR}/bin/library.rxbin"
    "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
  COMMENT "Staging immutable native-backed class library compiler imports ..."
  VERBATIM
)
add_custom_target(classlib_native_imports
  DEPENDS
    "${CLASSLIB_NATIVE_IMPORT_LIBRARY}"
    "${CLASSLIB_NATIVE_IMPORT_EXITS}"
)

# A native-backed adapter may see only the providers it declares here.  This
# keeps rxc's source/RXBIN/plugin search order deterministic and prevents a
# plugin built for another adapter from silently satisfying an import.
set(CLASSLIB_NATIVE_PROVIDER_TARGETS_Id id)
set(CLASSLIB_NATIVE_PROVIDER_FILES_Id rxid.rxplugin)
set(CLASSLIB_NATIVE_PROVIDER_TARGETS_KeyDB _keyaccess)
set(CLASSLIB_NATIVE_PROVIDER_FILES_KeyDB rx_keyaccess.rxplugin)
set(CLASSLIB_NATIVE_PROVIDER_TARGETS_Os fs _platform)
set(CLASSLIB_NATIVE_PROVIDER_FILES_Os rxfs.rxplugin rxplatform.rxplugin)

foreach(_binfile IN LISTS CLASSLIB_NATIVE_BINS)
  get_filename_component(_basename "${_binfile}" NAME_WE)
  get_filename_component(_member_dir "${_binfile}" DIRECTORY)
  file(MAKE_DIRECTORY "${_member_dir}")
  set(_provider_targets ${CLASSLIB_NATIVE_PROVIDER_TARGETS_${_basename}})
  set(_provider_files ${CLASSLIB_NATIVE_PROVIDER_FILES_${_basename}})
  list(LENGTH _provider_targets _provider_target_count)
  list(LENGTH _provider_files _provider_file_count)
  if(NOT _provider_target_count EQUAL _provider_file_count)
    message(FATAL_ERROR
      "Native classlib adapter ${_basename} has mismatched provider targets and files")
  endif()

  string(JOIN ";" _provider_signature ${_provider_files})
  string(SHA256 _native_dep_signature
    "${_basename};providers=${_provider_signature};base=library.rxbin,rxcexits.rxbin")
  string(SUBSTRING "${_native_dep_signature}" 0 12 _native_dep_signature_short)
  set(_member_source_root
    "${_member_dir}/sources-${_native_dep_signature_short}")
  set(_member_plugin_root
    "${_member_dir}/plugins-${_native_dep_signature_short}")
  set(_member_source "${_member_source_root}/${_basename}.crexx")
  set(_provider_stage_commands)
  set(_provider_staged_files)
  set(_provider_deps)
  if(_provider_target_count GREATER 0)
    math(EXPR _provider_last_index "${_provider_target_count} - 1")
    foreach(_provider_index RANGE ${_provider_last_index})
      list(GET _provider_targets ${_provider_index} _provider_target)
      list(GET _provider_files ${_provider_index} _provider_file)
      set(_provider_staged_file "${_member_plugin_root}/${_provider_file}")
      list(APPEND _provider_stage_commands
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE:${_provider_target}>"
          "${_provider_staged_file}")
      list(APPEND _provider_staged_files "${_provider_staged_file}")
      list(APPEND _provider_deps ${_provider_target})
    endforeach()
  endif()

  add_custom_command(
    OUTPUT "${_binfile}"
    BYPRODUCTS
      "${_member_source}"
      "${_member_dir}/${_basename}.rxas"
      ${_provider_staged_files}
    COMMAND ${CMAKE_COMMAND} -E make_directory
      "${_member_source_root}"
      "${_member_plugin_root}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      "${_member_source}"
    ${_provider_stage_commands}
    COMMAND $<TARGET_FILE:rxc>
      --no-exe-import
      -i "${CLASSLIB_NATIVE_IMPORT_ROOT};${_member_plugin_root}"
      -o "${_basename}"
      "${_member_source}"
    COMMAND $<TARGET_FILE:rxas> "${_basename}"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx"
      classlib_native_imports
      "${CLASSLIB_NATIVE_IMPORT_LIBRARY}"
      "${CLASSLIB_NATIVE_IMPORT_EXITS}"
      rxc
      rxas
      ${_provider_deps}
    WORKING_DIRECTORY "${_member_dir}"
    COMMENT "cREXX compile and assemble native-backed classlib adapter ${_basename} ..."
    VERBATIM
  )
endforeach()

set(_classlib_native_control_file ${CMAKE_CURRENT_BINARY_DIR}/classlib_native_link_$<CONFIG>.ctl)
set(_classlib_native_link_root "${CMAKE_CURRENT_BINARY_DIR}/linked/native")
set(_classlib_native_link_image "${_classlib_native_link_root}/classlib_native.rxbin")
set(_classlib_native_publish_image "${CMAKE_BINARY_DIR}/bin/classlib_native.rxbin")
set(_classlib_native_publish_temp
  "${CMAKE_CURRENT_BINARY_DIR}/publish/classlib_native.rxbin.tmp")
string(CONCAT _classlib_native_control_text
  "* Auto-generated control file for native-backed cREXX class library adapters\n")
foreach(_cls_bin IN LISTS CLASSLIB_NATIVE_BINS)
  string(APPEND _classlib_native_control_text "INPUT ${_cls_bin}\n")
endforeach()
foreach(_selector IN LISTS CLASSLIB_NATIVE_CLSS)
  string(APPEND _classlib_native_control_text "INCLUDE ${_selector}\n")
endforeach()
string(APPEND _classlib_native_control_text
  "PRESERVE INLINE\n"
  "$<$<NOT:$<CONFIG:Debug>>:STRIP SOURCE\n>"
  "OUTPUT ${_classlib_native_link_root}/classlib_native\n")

file(GENERATE OUTPUT ${_classlib_native_control_file} CONTENT "${_classlib_native_control_text}")

add_custom_command(
  OUTPUT "${_classlib_native_link_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_classlib_native_link_root}"
  COMMAND $<TARGET_FILE:rxlink> -c ${_classlib_native_control_file}
  DEPENDS rxlink ${CLASSLIB_NATIVE_BINS} ${_classlib_native_control_file}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "cREXX Linked native-backed Class Library Adapter Image ..."
  VERBATIM
)

add_custom_command(
  OUTPUT "${_classlib_native_publish_image}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/publish"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${_classlib_native_link_image}"
    "${_classlib_native_publish_temp}"
  COMMAND ${CMAKE_COMMAND} -E rename
    "${_classlib_native_publish_temp}"
    "${_classlib_native_publish_image}"
  DEPENDS "${_classlib_native_link_image}"
  COMMENT "Publishing native-backed class library image ..."
  VERBATIM
)

add_custom_target(classlib_native ALL
  DEPENDS "${_classlib_native_publish_image}"
)

add_test(
  NAME classlib_rxdas_inline_preserve_smoke
  COMMAND ${CMAKE_COMMAND}
    -DRXDAS=$<TARGET_FILE:rxdas>
    -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
    -DINPUT_RXBIN=${CMAKE_BINARY_DIR}/bin/classlib.rxbin
    -DOUTPUT_RXAS=${CMAKE_CURRENT_BINARY_DIR}/classlib_inline_check.rxas
    "-DEXPECT_TEXT_1=.meta \"data.stringarraylist.init\"=\".inline\""
    "-DEXPECT_TEXT_2=.meta \"data.stringhashmap.size\"=\".inline\""
    "-DEXPECT_ABSENT_1=.meta \"data_stringlist.stringarraylist.init\"=\".inline\""
    "-DEXPECT_ABSENT_2=.meta \"data_stringhashmap.stringhashmap.size\"=\".inline\""
    -P ${CMAKE_SOURCE_DIR}/linker/tests/check_rxdas_contains.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(classlib_rxdas_inline_preserve_smoke PROPERTIES LABELS "inline;library")

set(_classlib_rexxdoc_check_source "${CMAKE_SOURCE_DIR}/tools/rexxDoc/check_rexxdoc_coverage.crexx")
set(_classlib_rexxdoc_check_script "${CMAKE_CURRENT_BINARY_DIR}/run_classlib_rexxdoc_coverage.cmake")
set(_classlib_rexxdoc_check_content "")
string(APPEND _classlib_rexxdoc_check_content
  "execute_process(COMMAND \"$<TARGET_FILE:rxc>\" -i \"${CMAKE_BINARY_DIR}/bin\" -o \"check_rexxdoc_coverage\" \"${_classlib_rexxdoc_check_source}\" WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND _classlib_rexxdoc_check_content
  "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxc failed for RexxDoc coverage checker: \${out}\${err}\")\nendif()\n")
string(APPEND _classlib_rexxdoc_check_content
  "execute_process(COMMAND \"$<TARGET_FILE:rxas>\" \"check_rexxdoc_coverage\" WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND _classlib_rexxdoc_check_content
  "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"rxas failed for RexxDoc coverage checker: \${out}\${err}\")\nendif()\n")
string(APPEND _classlib_rexxdoc_check_content
  "execute_process(COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${CMAKE_BINARY_DIR}/bin/library.rxbin\" \"${CMAKE_CURRENT_BINARY_DIR}/check_rexxdoc_coverage.rxbin\" -a \"${CMAKE_SOURCE_DIR}/tools/rexxDoc/classlib_rexxdoc_baseline.txt\" \"${CMAKE_SOURCE_DIR}/lib\" WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}\" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)\n")
string(APPEND _classlib_rexxdoc_check_content
  "message(STATUS \"\${out}\${err}\")\nif(NOT res EQUAL 0)\n  message(FATAL_ERROR \"RexxDoc coverage check failed\")\nendif()\n")
file(GENERATE OUTPUT "${_classlib_rexxdoc_check_script}" CONTENT "${_classlib_rexxdoc_check_content}")

add_test(
  NAME classlib_rexxdoc_coverage
  COMMAND ${CMAKE_COMMAND} -P "${_classlib_rexxdoc_check_script}"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(classlib_rexxdoc_coverage PROPERTIES LABELS "docs;classlib")
crexx_label_concurrency_tests(SP09 classlib_rexxdoc_coverage)

add_test(
  NAME rexx_api_doc_exposed_regression
  COMMAND ${CMAKE_COMMAND}
    -DRXC=$<TARGET_FILE:rxc>
    -DRXAS=$<TARGET_FILE:rxas>
    -DRXVM=${CREXX_RXVM_EXECUTABLE}
    -DLIBRARY=${CMAKE_BINARY_DIR}/bin
    -DSOURCE=${CMAKE_SOURCE_DIR}/tools/rexxDoc/rexxApiDoc.crexx
    -DFIXTURE=${CMAKE_SOURCE_DIR}/tools/rexxDoc/tests/rexx_api_doc_exposed_fixture.crexx
    -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/rexx_api_doc_test
    -P ${CMAKE_SOURCE_DIR}/tools/rexxDoc/tests/check_rexx_api_doc.cmake
)
set_tests_properties(rexx_api_doc_exposed_regression PROPERTIES LABELS "docs;classlib")
crexx_label_concurrency_tests(SP09 rexx_api_doc_exposed_regression)

add_test(
  NAME gate_f_levelb_bridge_inspection
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/Concurrency.crexx
    -P ${CMAKE_SOURCE_DIR}/cmake/CheckGateFLevelBBridge.cmake
)
set_tests_properties(gate_f_levelb_bridge_inspection
  PROPERTIES LABELS "gate-f;classlib;inspection")
crexx_label_concurrency_tests(SP05 gate_f_levelb_bridge_inspection)
crexx_label_concurrency_tests(SP09 gate_f_levelb_bridge_inspection)

add_subdirectory(tests_functional)

crexx_finalize_directory_qa_tiers()
