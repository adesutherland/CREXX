#!/bin/sh
echo "Running $1 $2 $3 $4"

echo "cleanup"
rm $1".rxas"
rm $1".rxbin"

echo "rxc -n $1"
../cmake-build-debug/bin/rxc -n $1

echo "rxas-n $1"
../cmake-build-debug/bin/rxas -n $1

echo "rxdas $1"
../cmake-build-debug/bin/rxdas $1

echo "rxvm $1"
../cmake-build-debug/bin/rxvm $1 library -a $2 $3 $4

echo rc=$?
