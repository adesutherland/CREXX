@echo off
erase "rxdb.rxas"
erase "rxdb.rxbin"

..\cmake-build-debug\compiler\rxc rxdb
..\cmake-build-debug\assembler\rxas rxdb
..\cmake-build-debug\assembler\rxas globals
# ..\cmake-build-debug\disassembler\rxdas rxdb
..\cmake-build-debug\interpreter\rxvm rxdb globals
