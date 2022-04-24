#!/bin/sh
echo "Test Scripts"

echo "rxas main"
../cmake-build-debug-mingw/assembler/rxas main

echo "rxas func1"
../cmake-build-debug-mingw/assembler/rxas func1

#echo "rxas func2"
#../cmake-build-debug-mingw/assembler/rxas func2

echo "rxdas main"
../cmake-build-debug-mingw/disassembler/rxdas main

echo "rxdas func1"
../cmake-build-debug-mingw/disassembler/rxdas func1

#echo "rxdas func2"
#../cmake-build-debug-mingw/disassembler/rxdas func2

echo "rxvm debug main func1"
../cmake-build-debug-mingw/interpreter/rxvm main func1

echo "rxbvm debug main func1"
../cmake-build-debug-mingw/interpreter/rxbvm main func1

echo "release rxvm main func1"
../cmake-build-release-mingw/interpreter/rxvm main func1

echo "release rxbvm main func1"
../cmake-build-release-mingw/interpreter/rxbvm main func1
