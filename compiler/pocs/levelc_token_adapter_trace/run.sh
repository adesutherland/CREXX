#!/usr/bin/env sh
set -eu

POC_DIR=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$POC_DIR/build"

mkdir -p "$BUILD_DIR"

cc -std=c11 -Wall -Wextra -Werror \
  "$POC_DIR/adapter_trace.c" \
  -o "$BUILD_DIR/adapter_trace"

"$BUILD_DIR/adapter_trace"
