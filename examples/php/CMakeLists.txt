cmake_minimum_required(VERSION 3.24)

# The PHP converter is an example/toolchain project. Generated grammar,
# reader, and translated cREXX artifacts stay in the build directory.
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION examples/php
        FILES_MATCHING
        PATTERN "*.php"
        PATTERN "*.levell"
        PATTERN "*.crexx"
        PATTERN "*.md")
