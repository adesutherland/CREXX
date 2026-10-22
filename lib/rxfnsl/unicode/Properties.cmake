set(_ll_property_dir "${CMAKE_CURRENT_SOURCE_DIR}/../unicode")
get_filename_component(_ll_property_data "${_ll_property_dir}/../../rxfnsb/unicode/data/unicode-17.0.0" ABSOLUTE)
set(_ll_property_work "${CMAKE_CURRENT_BINARY_DIR}/properties")
file(MAKE_DIRECTORY "${_ll_property_work}")
if(TARGET rxtvm)
  set(_ll_property_vm rxtvm)
else()
  set(_ll_property_vm rxbvm)
endif()
crexx_add_fixture_command(OUTPUT "${_ll_property_work}/generator-linked.rxbin"
  COMMAND $<TARGET_FILE:rxc> ${_ll_flags} -x --no-exe-import
    -i "${_rxfnsl_import_root}" -o "${_ll_property_work}/generator"
    "${_ll_property_dir}/generate-properties.crexx"
  COMMAND $<TARGET_FILE:rxas> ${_ll_flags} "${_ll_property_work}/generator"
  COMMAND $<TARGET_FILE:rxlink> -s -o "${_ll_property_work}/generator-linked"
    "${_ll_property_work}/generator" "${CMAKE_BINARY_DIR}/bin/library"
  DEPENDS rxc rxas rxlink rxfnsl_imports ${_rxfnsl_import_files}
    "${_ll_property_dir}/generate-properties.crexx" VERBATIM)
set(_ll_data_source "${_ll_property_work}/lludata.crexx")
crexx_add_fixture_command(OUTPUT "${_ll_data_source}"
  COMMAND ${CMAKE_COMMAND} -DVM=$<TARGET_FILE:${_ll_property_vm}>
    -DGENERATOR=${_ll_property_work}/generator-linked.rxbin
    -DDATA=${_ll_property_data} -DOUTPUT=${_ll_data_source}
    -P "${_ll_property_dir}/generate-properties.cmake"
  DEPENDS ${_ll_property_vm} "${_ll_property_work}/generator-linked.rxbin"
    "${_ll_property_dir}/generate-properties.cmake"
    "${_ll_property_data}/SHA256SUMS" "${_ll_property_data}/DerivedCoreProperties.txt"
    "${_ll_property_data}/UnicodeData.txt" "${_ll_property_data}/PropList.txt"
  VERBATIM)
