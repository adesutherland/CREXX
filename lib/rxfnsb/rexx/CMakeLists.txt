cmake_minimum_required(VERSION 3.24)

set(BIFS
		_address
		_rxsystem
		_jdn
		_datei
		_dateo
		_ftrunc
		_itrunc
		_typedformat
		abbrev
		abs
		intabs
		floatabs
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
		intformat
		floatformat
		fnv
#		global
		getenv
		insert
		length
		linesize
		loadmodule
		lower
		max
		intmax
		floatmax
		min
		intmin
		floatmin
		numeric
		overlay
		pos
		qscan
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
		intsign
		floatsign
		space
		strip
		substr
		substro
		symbol
		time
		translate
		trunc
		floattrunc
		upper
		value
		verify
		version
		sourceinfo
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
		objectarrayinsert
		objectarraydelete
		objectarrayappend
		objectarrayprepend
		objectarraydrop
		objectarraymove
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
		fmtmask
		compilemask
		runmask
		parseExec
		stem
		rxjson
		rxsocket
		rxhttp
		wordrep
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
	add_custom_command(
		  COMMAND ${CMAKE_COMMAND} -E rm -f ${_basename}.rxas
		  # A library self-build must not import the previous library image next
		  # to rxc. Current sibling sources and the explicit RXAS root are the
		  # complete compile-time interface set for every module.
		  COMMAND $<TARGET_FILE:rxc> -x --no-exe-import --import-rxas -o ${_basename} -i ${CMAKE_CURRENT_SOURCE_DIR}/../rxas ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx
		  COMMAND $<TARGET_FILE:rxas> ${_basename}
		  DEPENDS rxas rxc library-rxas ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.crexx ${REXX_IMPORT_RXAS} ${_prev_target}
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
