cmake_minimum_required(VERSION 3.24)

set(CLSS
  Stack
  TreeMap
)

foreach (CLS ${CLSS})
	set(CLSBINS ${CLSBINS} ${CLS}.rxbin)
	set(REXXSRC ${REXXSRC} ${CLS}.rexx)
endforeach ()

unset(_prev_target)
foreach(_binfile ${CLSBINS})
	get_filename_component(_basename ${_binfile} NAME_WE)
	add_custom_command(
		  COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
		  COMMAND $<TARGET_FILE:rxas> ${_basename}
		  OUTPUT ${_binfile}
		  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		  COMMENT "cREXX compile and assemble ${_basename} ..."
	)
	set(_prev_target ${_binfile})
endforeach()

add_custom_command(
 		OUTPUT classlib.rxbin
		COMMAND ${CMAKE_COMMAND} -E cat ${CLSBINS} > classlib.rxbin
		COMMENT "cREXX REXX Library Archive ..."
		DEPENDS ${CLSBINS}
)

add_custom_target(classlib ALL DEPENDS classlib.rxbin)
add_dependencies(classlib library)
