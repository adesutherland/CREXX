#!/bin/sh
rm "rxdb.rxas"
rm "rxdb.rxbin"

../cmake-build-debug-mingw/compiler/rxc rxdb
../cmake-build-debug-mingw/assembler/rxas rxdb
../cmake-build-debug-mingw/interpreter/rxvm rxdb
