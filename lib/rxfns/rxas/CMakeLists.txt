cmake_minimum_required(VERSION 3.18)

set(RXASBINS
              _elapsed.rxbin
              copies.rxbin
              lastpos.rxbin
              left.rxbin
              length.rxbin
              pos.rxbin
              reverse.rxbin
              right.rxbin
              substr.rxbin
              subword.rxbin
              word.rxbin
              wordindex.rxbin
              wordlen.rxbin
              wordlength.rxbin
              words.rxbin              
        )

foreach(_binfile ${RXASBINS})
  get_filename_component(_basename ${_binfile} NAME_WE)
  add_custom_command(
          #COMMAND ${CMAKE_COMMAND} -E chdir  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/compiler/rxas ${_basename}
          COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas -l ${CMAKE_CURRENT_SOURCE_DIR} ${_basename}
          COMMAND ${CMAKE_COMMAND} -E copy   ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxbin ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.rxbin
          COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}.rxbin

          DEPENDS ${_basename}.rxas rxas rxc rxvm rxbvm
          OUTPUT ${_binfile}
          COMMENT "cREXX assemble ${_basename} ..."
  )

endforeach()

add_custom_command(
        OUTPUT library-rxas.rxbin
        COMMAND ${CMAKE_COMMAND} -E cat ${RXASBINS} > library-rxas.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${RXASBINS} rxas rxc
)

add_custom_target(library-rxas ALL DEPENDS library-rxas.rxbin)