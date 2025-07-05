cmake_minimum_required(VERSION 3.24)

set(VECS
		veclib
)

foreach (VEC ${VECS})
	set(VECBINS ${VECBINS} ${VEC}.rxbin)
	set(REXXSRC ${REXXSRC} ${VEC}.rexx)
endforeach ()

unset(_prev_target)
foreach(_binfile ${VECBINS})
	get_filename_component(_basename ${_binfile} NAME_WE)
	add_custom_command(
		  COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i ${CMAKE_BINARY_DIR}/lib/rxfnsb -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
		  COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas ${_basename}
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
