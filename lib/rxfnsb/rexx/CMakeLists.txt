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
		b2x
		b2d
		binary
		c2x
		c2d
		center
		centre
		changestr
		compare
		copies
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
		loadmodule
		lower
		max
		min
		numeric
		overlay
		pos
		qpos
		qsplit
		qsplitsafe
		qextractall
		qextractpair
		qstripcomment
		qremoveall
		fsayfmt
		lastpos
		left
		right
		raise
		reverse
		signal
		trace
		sign
		space
		strip
		substr
		substro
		symbol
		time
		translate
		trunc
		upper
		value
		verify
		version
		word
		words
		wordindex
		subword
		wordlength
		wordpos
		qword
		qwordlength
		qwords
		qwordindex
		qwordpos
		qsubword
		regex
		x2b
		x2c
		x2d
		random
		reradix
		sequence
		find
		index
		xrange
		parsecompile
		parsestring
		parse
		datatype
		arrayfind
		splice
		arrayinsert
		arraydelete
		arrayappend
		arrayprepend
		arraypop
		arrayshift
		arrayget
		arrayset
		arraycontains
		arrayindexof
		arrayreverse
		arrayjoin
		arraysort
		arraycopy
		arraydrop
		arrayhi
		arraymove
		arraydump
		arrayformat
		fmtmask
		compilemask
		runmask
		parseExec
		stem
		rxjson
		rxsocket
		rxhttp
		wordrep
		evaluate
)

foreach (BIF ${BIFS})
	set(REXXBINS ${REXXBINS} ${BIF}.rxbin)
	set(REXXSRC ${REXXSRC} ${BIF}.crexx)
endforeach ()

set(RXXFNSB_REXX_SELECTORS ${BIFS} PARENT_SCOPE)
set(RXXFNSB_REXX_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/library-rexx.rxbin")
set(RXXFNSB_REXX_ARCHIVE "${RXXFNSB_REXX_ARCHIVE}" PARENT_SCOPE)
set(REXX_IMPORT_RXAS
		${CMAKE_CURRENT_SOURCE_DIR}/../rxas/_rxvml_address_native.rxas
)

unset(_prev_target)
foreach(_binfile ${REXXBINS})
	get_filename_component(_basename ${_binfile} NAME_WE)
	set(_rxc_import_args -i ${CMAKE_CURRENT_SOURCE_DIR}/../rxas)
	add_custom_command(
		  COMMAND ${CMAKE_COMMAND} -E rm -f ${_basename}.rxas
		  COMMAND $<TARGET_FILE:rxc> -x --import-rxas -o ${_basename} ${_rxc_import_args} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx
		  COMMAND $<TARGET_FILE:rxas> ${_basename}
		  DEPENDS rxas rxc library-rxas ${REXXSRC} ${REXX_IMPORT_RXAS} ${_prev_target}
		  OUTPUT ${_binfile}
		  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		  COMMENT "cREXX compile and assemble ${_basename} ..."
	)
	set(_prev_target ${_binfile})
endforeach()

add_custom_command(
		OUTPUT "${RXXFNSB_REXX_ARCHIVE}"
		COMMAND ${CMAKE_COMMAND} -E cat ${REXXBINS} > "${RXXFNSB_REXX_ARCHIVE}"
		COMMENT "cREXX REXX Library Archive ..."
		DEPENDS ${REXXBINS}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(library-rexx ALL DEPENDS "${RXXFNSB_REXX_ARCHIVE}")
