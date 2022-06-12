@ECHO OFF
erase "test.rxas"
erase "test.rxbin"

..\cmake-build-debug\compiler\rxc test
..\cmake-build-debug\assembler\rxas test
..\cmake-build-debug\interpreter\rxvm test
..\cmake-build-debug\disassembler\rxdas test