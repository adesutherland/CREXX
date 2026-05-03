#!/bin/sh
rm "test.rxas"
rm "test.rxbin"

../cmake-build-debug/compiler/rxc test
../cmake-build-debug/assembler/rxas test
../cmake-build-debug/interpreter/rxvm test ../cmake-build-debug/lib/rxfnsb/library
../cmake-build-debug/cpacker/rxcpack test ../cmake-build-debug/lib/rxfnsb/library
socket_libs=
case "$(uname -s 2>/dev/null)" in
    MINGW*|MSYS*|CYGWIN*) socket_libs=-lws2_32 ;;
esac
gcc -o test -lrxvml -lmachine -lavl_tree -lplatform -lm ${socket_libs} -L../cmake-build-debug/interpreter -L../cmake-build-debug/machine -L../cmake-build-debug/avl_tree -L../cmake-build-debug/platform test.c
./test
