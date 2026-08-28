cmake_minimum_required(VERSION 3.24)

set(VECS
		veclib
)

foreach (VEC ${VECS})
	set(VECBINS ${VECBINS} ${VEC}.rxbin)
	set(REXXSRC ${REXXSRC} ${VEC}.crexx)
endforeach ()

set(VECLIB_IMPORT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/imports")
crexx_add_import_root(veclib_imports
	DIRECTORY "${VECLIB_IMPORT_ROOT}"
	OUTPUT_VARIABLE VECLIB_IMPORT_FILES
	FILES
		"${CMAKE_BINARY_DIR}/bin/library.rxbin"
		"${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
	DEPENDS
		library
		compiler_exit_bin
	COMMENT "Staging deterministic vector-library imports ...")

unset(_prev_target)
foreach(_binfile ${VECBINS})
	get_filename_component(_basename ${_binfile} NAME_WE)
	add_custom_command(
		  COMMAND $<TARGET_FILE:rxc> --no-exe-import -i "${VECLIB_IMPORT_ROOT}" -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx
		  COMMAND $<TARGET_FILE:rxas> ${_basename}
		  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx rxc rxas veclib_imports ${VECLIB_IMPORT_FILES}
		  OUTPUT ${_binfile}
		  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		  COMMENT "cREXX compile and assemble ${_basename} ..."
	)
	set(_prev_target ${_binfile})
endforeach()

# this only when we are making a library
# add_custom_command(
#  		OUTPUT veclib.rxbin
# 		COMMAND ${CMAKE_COMMAND} -E cat ${VECBINS} > veclib.rxbin
# 		COMMENT "cREXX REXX Library Archive ..."
# 		DEPENDS ${VECBINS}
# )

add_custom_target(veclib ALL DEPENDS veclib.rxbin)
add_dependencies(veclib library)
