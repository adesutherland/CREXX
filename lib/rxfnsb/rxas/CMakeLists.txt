cmake_minimum_required(VERSION 3.24)

set(RXASBINS
        _elapsed.rxbin
        # copies.rxbin
        # lastpos.rxbin
        # left.rxbin
        # length.rxbin
        # pos.rxbin
        # reverse.rxbin
        # right.rxbin
        # substr.rxbin
        # subword.rxbin
        # word.rxbin
        # wordindex.rxbin
        # wordlen.rxbin
        # wordlength.rxbin
        # words.rxbin
)

set(RXXFNSB_RXAS_SELECTORS)
foreach(_rxas_bin ${RXASBINS})
  get_filename_component(_rxas_selector ${_rxas_bin} NAME_WE)
  list(APPEND RXXFNSB_RXAS_SELECTORS ${_rxas_selector})
endforeach()
set(RXXFNSB_RXAS_SELECTORS ${RXXFNSB_RXAS_SELECTORS} PARENT_SCOPE)
set(RXXFNSB_RXAS_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/library-rxas.rxbin")
set(RXXFNSB_RXAS_ARCHIVE "${RXXFNSB_RXAS_ARCHIVE}" PARENT_SCOPE)

unset(_prev_target)
foreach(_binfile ${RXASBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_command(
          COMMAND $<TARGET_FILE:rxas> -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
          DEPENDS rxas ${_basename}.rxas ${_prev_target}
          OUTPUT ${_binfile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "cREXX assemble ${_basename} ..."
  )
  set(_prev_target ${_binfile})
endforeach()

add_custom_command(
        OUTPUT "${RXXFNSB_RXAS_ARCHIVE}"
        COMMAND ${CMAKE_COMMAND} -E cat ${RXASBINS} > "${RXXFNSB_RXAS_ARCHIVE}"
        COMMENT "cREXX RXAS Library Archive ..."
        DEPENDS ${RXASBINS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(library-rxas ALL DEPENDS "${RXXFNSB_RXAS_ARCHIVE}")
