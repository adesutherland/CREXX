#!/bin/sh
set -eu
root=/Users/adrian/CLionProjects/crexx-scaling-20260906.1lT795
export CREXX_HOME="$root/install-qualified/"
mkdir -p "$root/offline-native-package"
cp "$root/rag-build-qualified/crexx-application/project/crexxrag-project.rxbin" "$root/offline-native-package/crexxrag.rxbin"
cd "$root/offline-native-package"
"$root/install-qualified/bin/crexx" -native -nocompile -noexec -nocolor -verbose2 --linkmap "$root/offline-native-package/crexxrag-native.map" crexxrag.crexx
./crexxrag --library "$root/offline-native-library" --config architecture-local --profile generic-profile --access admin --format json library init
./crexxrag --library "$root/offline-native-library" --config architecture-local --profile generic-profile --access read --format json library status
