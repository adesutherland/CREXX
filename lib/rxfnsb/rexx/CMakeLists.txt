cmake_minimum_required(VERSION 3.24)

set(BIFS
		_address
		_rxsystem
		_datei
		_dateo
		_ftrunc
		_itrunc
		_jdn
		abbrev
		abs
		c2x
		c2d
		center
		centre
		changestr
		compare
		countstr
		d2b
		d2c
		d2x
		date
		delstr
		delword
		fileio
		filter
		format
		fnv
#		global
		getenv
		insert
		length
		linesize
		lower
		max
		min
		overlay
		pos
		lastpos
		raise
		sign
		space
		strip
		substr
		symbol
		time
		translate
		trunc
		upper
		value
		verify
		version
		wordlength
		wordpos
		x2b
		x2c
		x2d
		reradix
		sequence
		find
		index
		xrange
		parsecompile
		parsetext
		parse
		datatype
)

foreach (BIF ${BIFS})
	set(REXXBINS ${REXXBINS} ${BIF}.rxbin)
	set(REXXSRC ${REXXSRC} ${BIF}.rexx)
endforeach ()

unset(_prev_target)
foreach(_binfile ${REXXBINS})
	get_filename_component(_basename ${_binfile} NAME_WE)
	add_custom_command(
		  COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -o ${_basename} -i ${CMAKE_CURRENT_SOURCE_DIR}/../rxas ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
		  COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas ${_basename}
		  DEPENDS rxas rxc ../rxas/library-rxas.rxbin ${REXXSRC} ${_prev_target}
		  OUTPUT ${_binfile}
		  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		  COMMENT "cREXX compile and assemble ${_basename} ..."
	)
	set(_prev_target ${_binfile})
endforeach()

add_custom_command(
 		OUTPUT library-rexx.rxbin
		COMMAND ${CMAKE_COMMAND} -E cat ${REXXBINS} > library-rexx.rxbin
		COMMENT "cREXX REXX Library Archive ..."
		DEPENDS ${REXXBINS}
)

add_custom_target(library-rexx ALL DEPENDS library-rexx.rxbin)
