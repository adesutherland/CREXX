#!/usr/bin/env bash
# Isolated review matrix. No writes to a developer's primary build or install.
set -euo pipefail
rs_root=$(cd "$(dirname "$0")/../.." && pwd)
rs_bin=${CREXX_BIN_DIR:-"$rs_root/cmake-build-levell-product/bin"}
rs_mode=${1:-opt}
rs_format=${2:-rxas}
case "$rs_format" in rxas|crexx) ;; *) exit 2;; esac
case "$rs_mode" in opt|noopt) ;; *) exit 2;; esac
rs_build="$rs_root/research/level-l/build/rexxscript1-$rs_mode"
if [[ "$rs_mode" == noopt ]]; then
  rs_core=${CREXX_LEVEL_L_BUILD:-"$rs_root/research/level-l/build/product-noopt/bootstrap-build"}
  rs_runtime="$rs_core/bin/rxfnsl.rxbin"
  rs_generator="$rs_core/tools/bootstrap/L2/levell-linked.rxbin"
else
  rs_core=${CREXX_LEVEL_L_BUILD:-"$rs_root/cmake-build-levell-product"}
  rs_runtime="$rs_core/bin/rxfnsl.rxbin"
  rs_generator="$rs_core/lib/rxfnsl/tools/bootstrap/L2/levell-linked.rxbin"
fi
rs_imports="$rs_build/current-imports"
mkdir -p "$rs_imports"
# This directory is owned by the review script; retain no old linked prototypes.
rm -f "$rs_imports/"*.rxbin
rs_log=$(mktemp /tmp/crexx-rexxscript1-review.XXXXXX)
printf 'Build log: %s\n' "$rs_log"
trap 'rs_rc=$?; if [[ $rs_rc != 0 ]]; then tail -35 "$rs_log"; fi' EXIT
rs_flags=(-x --no-exe-import)
rs_as=()
if [[ "$rs_mode" == noopt ]]; then rs_flags+=(-n); rs_as+=(-n); fi
for lib in library classlib rxfnsc; do
  ln -sf "$rs_bin/$lib.rxbin" "$rs_imports/$lib.rxbin"
done
ln -sf "$rs_runtime" "$rs_imports/rxfnsl.rxbin"
rs_core_bins=("$rs_runtime")
rs_generate=()
if [[ $rs_format == rxas ]]; then rs_generate+=(--rxas); fi
"$rs_bin/rxtvm" "$rs_generator" -a "$rs_root/rexxscript/grammar/rexxscript.levell" ${rs_generate[@]+"${rs_generate[@]}"} >"$rs_build/rexxscriptgrammar.$rs_format.tmp" 2>>"$rs_log"
mv "$rs_build/rexxscriptgrammar.$rs_format.tmp" "$rs_build/rexxscriptgrammar.$rs_format"

compile() {
  local name=$1 source=$2
  "$rs_bin/rxc" "${rs_flags[@]}" -i "$rs_imports" -o "$rs_build/$name" "$source" >>"$rs_log" 2>&1
  "$rs_bin/rxas" ${rs_as[@]+"${rs_as[@]}"} "$rs_build/$name" >>"$rs_log" 2>&1
  ln -sf "../$name.rxbin" "$rs_imports/$name.rxbin"
}
if [[ $rs_format == rxas ]]; then
  "$rs_bin/rxas" ${rs_as[@]+"${rs_as[@]}"} "$rs_build/rexxscriptgrammar" >>"$rs_log" 2>&1
  ln -sf ../rexxscriptgrammar.rxbin "$rs_imports/rexxscriptgrammar.rxbin"
else
  compile rexxscriptgrammar "$rs_build/rexxscriptgrammar.crexx"
fi
rs_modules=("$rs_build/rexxscriptgrammar.rxbin")
for name in RexxScriptBifs RexxScriptOperators RexxScriptProgram RexxScriptExecution; do
  compile "$name" "$rs_root/rexxscript/$name.crexx"
  rs_modules+=("$rs_build/$name.rxbin")
done
compile RexxScript1Runner "$rs_root/rexxscript/RexxScript1Runner.crexx"
compile testRexxScript1 "$rs_root/rexxscript/tests_functional/testRexxScript1.crexx"
for name in RexxScript1Runner testRexxScript1; do
  "$rs_bin/rxlink" -o "$rs_build/$name-linked" "$rs_build/$name.rxbin" "${rs_modules[@]}" "${rs_core_bins[@]}" "$rs_bin/rxfnsc.rxbin" "$rs_bin/rxfnsg.rxbin" "$rs_bin/classlib.rxbin" "$rs_bin/library.rxbin" >>"$rs_log" 2>&1
done
cp "$rs_log" "$rs_build/build.log"
printf 'Built %s\n' "$rs_build/testRexxScript1-linked.rxbin"
