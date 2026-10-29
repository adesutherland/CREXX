#!/bin/sh
# Build the cREXX walkthrough from already prepared Level L/Classic libraries.
set -eu
assets=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build=${1:-"$assets/../../../../../cmake-build-debug"}
build=$(CDPATH= cd -- "$build" && pwd)
imports="$build/research/level-l/compilers/levelc/opt/imports-llccompiler"
classic="$build/research/level-l/compilers/levelc/opt"
common="$build/research/level-l/compilers/common/opt"
cmake=$(command -v cmake)
work=$(mktemp -d "${TMPDIR:-/tmp}/crexx-level-l.XXXXXX")
cleanup() {
  status=$?
  trap - 0
  "$cmake" -E rm -rf "$work"
  exit "$status"
}
trap cleanup 0
trap 'exit 130' INT
trap 'exit 143' HUP TERM
mkdir "$work/imports"
for file in "$imports"/*.rxbin; do
  ln -s "$file" "$work/imports/$(basename -- "$file")"
done
"$build/bin/rxc" -x --no-exe-import -i "$work/imports" -o "$work/sqrtflow" "$assets/flowview.crexx"
"$build/bin/rxas" "$work/sqrtflow"
ln -s "$work/sqrtflow.rxbin" "$work/imports/sqrtflow.rxbin"
"$build/bin/rxc" -x --no-exe-import -i "$work/imports" -o "$work/journey" "$assets/journey.crexx"
"$build/bin/rxas" "$work/journey"
set -- "$work/journey.rxbin" "$work/sqrtflow.rxbin" \
  "$common/llscalar.rxbin" "$common/llcompilercontracts.rxbin" \
  "$build/langtools/common/llrxastext.rxbin" "$build/langtools/common/llbinmeta.rxbin" \
  "$build/research/level-l/compilers/frontends/llcgrammar.rxbin" \
  "$build/lib/rxfnsc/members/RexxClassicUnicodeProperties/RexxClassicUnicodeProperties.rxbin"
for part in ir language symbols bifs bindings srcmap diagnostics parseprogram procedureprotocol signalprotocol doprotocol clauseprotocol commandprotocol parse frontend graph; do
  set -- "$@" "$classic/llc$part.rxbin"
done
set -- "$@" "$build/bin/rxfnsl-analysis.rxbin" "$build/bin/rxfnsl.rxbin" "$build/bin/library.rxbin"
"$build/bin/rxlink" -s -o "$work/journey-linked" "$@"
"$build/bin/rxbvm" "$work/journey-linked.rxbin" -a "$assets" "$build/bin"
