#!/bin/sh
# Build the cREXX auditor, then analyse the selected Classic source files.
set -eu
assets=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build=${1:-"$assets/../../../../../cmake-build-debug"}
build=$(CDPATH= cd -- "$build" && pwd)
if [ "$#" -gt 0 ]; then shift; fi
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
"$build/bin/rxc" -x --no-exe-import -i "$work/imports" -o "$work/audit" "$assets/audit.crexx"
"$build/bin/rxas" "$work/audit"
# Keep the caller's source arguments while building the library argument list.
link_auditor() {
set -- "$work/audit.rxbin" \
  "$common/llscalar.rxbin" "$common/llcompilercontracts.rxbin" \
  "$build/langtools/common/llrxastext.rxbin" "$build/langtools/common/llbinmeta.rxbin" \
  "$build/research/level-l/compilers/frontends/llcgrammar.rxbin" \
  "$build/lib/rxfnsc/members/RexxClassicUnicodeProperties/RexxClassicUnicodeProperties.rxbin"
for part in ir language symbols bifs bindings srcmap diagnostics parseprogram procedureprotocol signalprotocol doprotocol clauseprotocol commandprotocol parse frontend graph; do
  set -- "$@" "$classic/llc$part.rxbin"
done
set -- "$@" "$build/bin/rxfnsl-analysis.rxbin" "$build/bin/rxfnsl.rxbin" "$build/bin/library.rxbin"
"$build/bin/rxlink" -s -o "$work/audit-linked" "$@"
}
link_auditor
if [ "${1:-}" = '--check' ]; then
  sh "$assets/check.sh" "$build/bin/rxbvm" "$work/audit-linked.rxbin" "$build/bin" "$assets"
  exit "$?"
fi
if [ "$#" -eq 0 ]; then
  set -- "$assets/average.rexx" "$assets/average-changed.rexx" \
    "$assets/average-late.rexx" "$assets/average-loop.rexx" "$assets/countdown.rexx" \
    "$assets/countdown-skip.rexx" "$assets/readings.rexx" "$assets/readings-broken.rexx"
fi
for source in "$@"; do
  "$build/bin/rxbvm" "$work/audit-linked.rxbin" -a "$source" "$build/bin"
done
