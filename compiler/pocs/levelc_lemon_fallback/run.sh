#!/usr/bin/env sh
set -eu

ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
POC_DIR="$ROOT/compiler/pocs/levelc_lemon_fallback"
BUILD_DIR="$POC_DIR/build"
LEMON="$ROOT/cmake-build-debug/lemon/lemon"
LEMPAR="$ROOT/lemon/lempar.c"

mkdir -p "$BUILD_DIR"

"$LEMON" -T"$LEMPAR" -d"$BUILD_DIR" "$POC_DIR/fallback_no_then.y"
"$LEMON" -T"$LEMPAR" -d"$BUILD_DIR" "$POC_DIR/fallback_with_then.y"

cc -std=c11 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable \
  -I"$POC_DIR" -c "$BUILD_DIR/fallback_no_then.c" \
  -o "$BUILD_DIR/fallback_no_then.o"
cc -std=c11 -Wall -Wextra -Werror -I"$POC_DIR" -I"$BUILD_DIR" \
  -DPOC_HEADER='"fallback_no_then.h"' \
  -DPARSE_PREFIX=LevelCFallbackNoThen \
  -DEXPECT_THEN_CONDITION_ACCEPT=0 \
  -c "$POC_DIR/run_cases.c" \
  -o "$BUILD_DIR/run_cases_no_then.o"
cc "$BUILD_DIR/run_cases_no_then.o" "$BUILD_DIR/fallback_no_then.o" \
  -o "$BUILD_DIR/no_then_poc"

cc -std=c11 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable \
  -I"$POC_DIR" -c "$BUILD_DIR/fallback_with_then.c" \
  -o "$BUILD_DIR/fallback_with_then.o"
cc -std=c11 -Wall -Wextra -Werror -I"$POC_DIR" -I"$BUILD_DIR" \
  -DPOC_HEADER='"fallback_with_then.h"' \
  -DPARSE_PREFIX=LevelCFallbackWithThen \
  -DEXPECT_THEN_CONDITION_ACCEPT=1 \
  -c "$POC_DIR/run_cases.c" \
  -o "$BUILD_DIR/run_cases_with_then.o"
cc "$BUILD_DIR/run_cases_with_then.o" "$BUILD_DIR/fallback_with_then.o" \
  -o "$BUILD_DIR/with_then_poc"

echo "== no THEN fallback =="
"$BUILD_DIR/no_then_poc"

echo
echo "== with THEN fallback =="
"$BUILD_DIR/with_then_poc"
