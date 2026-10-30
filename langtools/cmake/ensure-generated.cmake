# Make-style generators do not notice a missing byproduct of an intact stamp.
# Use the same output owner and arguments as the ordinary generation command.
if(NOT EXISTS "${OUTPUT}" OR IS_DIRECTORY "${OUTPUT}")
  include("${CMAKE_CURRENT_LIST_DIR}/generate.cmake")
endif()
