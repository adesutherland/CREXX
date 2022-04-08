#!/bin/sh
rm "rxdb.rxas"
rm "rxdb.rxbin"

../cmake-build-debug/compiler/rxc rxdb
../cmake-build-debug/assembler/rxas rxdb
../cmake-build-debug/interpreter/rxvm rxdb
