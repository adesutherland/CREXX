cmake_minimum_required(VERSION 3.24)

# Library implementation only; Level L tools, examples and tests are in langtools.
add_subdirectory(rexx)
add_subdirectory(analysis)
add_subdirectory(rxas)
add_subdirectory(execute)
crexx_install_bytecode(rxfnsl "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin")
