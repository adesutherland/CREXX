#!/bin/sh
# build_with_prolog.sh -- compile a CREXX program that imports prolog.crexx
# and link it into one runnable image.
#
# Why this script exists: this CREXX toolchain build has a bug in its
# dynamic cross-module import resolution -- a program that does
# `import prolog` and calls `prolog..SomeFunction(...)` compiles fine
# but PANICS at runtime with FUNCTION_NOT_FOUND, as soon as the called
# function internally calls any other function. Pre-linking every
# module into a single combined .rxbin with rxlink and running THAT
# with rxvm reliably avoids the bug (this is a real toolchain issue,
# not something wrong with prolog.crexx -- see PORT_NOTES.md).
#
# prolog.crexx also has an unconditional `import crexxcallback` (the
# Prolog-calls-CREXX direction -- crexx_call/2,3 in Prolog code), so a
# crexxcallback.crexx MUST be compiled and linked in too, every time --
# the default one shipped in this package is a safe, working stub if
# you don't need that direction; point this script at your own version
# (second argument) once you do.
#
# Usage:
#   ./build_with_prolog.sh mycaller.crexx
#       (uses ./crexxcallback.crexx, the default stub, if you don't
#        need Prolog to call back into CREXX)
#   ./build_with_prolog.sh mycaller.crexx mycrexxcallback.crexx
#       (uses your own crexxcallback.crexx instead)
#   ./rxvm mycaller_linked.rxbin
# (or just run the last line this script prints for you)
#
# Requires prolog.crexx (and a crexxcallback.crexx -- see above) to be
# in the same directory as this script (or edit PROLOG_SRC /
# DEFAULT_CALLBACK_SRC below), and CREXX_BIN to point at the directory
# containing rxc, rxas, and rxlink.

set -e

CREXX_BIN="${CREXX_BIN:-/home/claude/CREXX/cmake-build-debug/bin}"
HERE="$(cd "$(dirname "$0")" && pwd)"
PROLOG_SRC="$HERE/prolog.crexx"
DEFAULT_CALLBACK_SRC="$HERE/crexxcallback.crexx"

if [ -z "$1" ]; then
  echo "usage: $0 yourprogram.crexx [yourcrexxcallback.crexx]" >&2
  exit 1
fi
CALLER_SRC="$1"
CALLER_STEM="$(basename "$CALLER_SRC" .crexx)"
CALLBACK_SRC="${2:-$DEFAULT_CALLBACK_SRC}"
CALLBACK_STEM="$(basename "$CALLBACK_SRC" .crexx)"
CALLBACK_DIR="$(cd "$(dirname "$CALLBACK_SRC")" && pwd)"
OUT="${CALLER_STEM}_linked"

if [ ! -f "$CALLBACK_SRC" ]; then
  echo "error: crexxcallback source not found: $CALLBACK_SRC" >&2
  echo "  (prolog.crexx always needs one to compile -- the default stub" >&2
  echo "   shipped in this package is fine if you don't use crexx_call/2,3)" >&2
  exit 1
fi

echo "== compiling prolog.crexx =="
"$CREXX_BIN/rxc" -s "$HERE" -s "$CALLBACK_DIR" "$PROLOG_SRC"
"$CREXX_BIN/rxas" "$HERE/prolog.rxas"

echo "== compiling $CALLBACK_SRC =="
"$CREXX_BIN/rxc" -s "$HERE" "$CALLBACK_SRC"
"$CREXX_BIN/rxas" "$CALLBACK_DIR/${CALLBACK_STEM}.rxas"

echo "== compiling $CALLER_SRC =="
"$CREXX_BIN/rxc" -s "$HERE" "$CALLER_SRC"
"$CREXX_BIN/rxas" "${CALLER_STEM}.rxas"

echo "== linking into $OUT.rxbin =="
"$CREXX_BIN/rxlink" -l "$HERE" -l "$CALLBACK_DIR" -o "$OUT" \
  "${CALLER_STEM}.rxbin" "$HERE/prolog.rxbin" "$CALLBACK_DIR/${CALLBACK_STEM}.rxbin"

echo "== done =="
echo "Run it with:"
echo "  $CREXX_BIN/rxvm $OUT.rxbin"
