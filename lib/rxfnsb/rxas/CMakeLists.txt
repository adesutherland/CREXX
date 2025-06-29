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
        subword.rxbin
        word.rxbin
        wordindex.rxbin
        # wordlen.rxbin
        # wordlength.rxbin
        words.rxbin
)

unset(_prev_target)
foreach(_binfile ${RXASBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_command(
          COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -o ${_basename} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}
          DEPENDS rxas ${_basename}.rxas ${_prev_target}
          OUTPUT ${_binfile}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "cREXX assemble ${_basename} ..."
  )
  set(_prev_target ${_binfile})
endforeach()

add_custom_command(
        OUTPUT library-rxas.rxbin
        COMMAND ${CMAKE_COMMAND} -E cat ${RXASBINS} > library-rxas.rxbin
        COMMENT "cREXX RXAS Library Archive ..."
        DEPENDS ${RXASBINS}
)

add_custom_target(library-rxas ALL DEPENDS library-rxas.rxbin)
