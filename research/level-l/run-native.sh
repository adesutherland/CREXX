#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/cmake-build-release/bin"}
if [[ ! -x "$task_bin/rxc" ]]; then
  task_bin=${CREXX_BIN_DIR:-"$task_root/../CREXX/cmake-build-release/bin"}
fi
task_mode=${1:-opt}
task_build="$task_dir/build/$task_mode"
mkdir -p "$task_build/imports"
task_log=$(mktemp /tmp/crexx-levell-native.XXXXXX)
printf 'Build log: %s\n' "$task_log"
task_flags=(--no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags=(--no-exe-import -n); fi
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
compile() {
  "$task_bin/rxc" "${task_flags[@]}" -x -i "$task_build/imports" -o "$task_build/$1" "$2" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then
    "$task_bin/rxas" -n "$task_build/$1" >> "$task_log" 2>&1
  else
    "$task_bin/rxas" "$task_build/$1" >> "$task_log" 2>&1
  fi
  ln -sf "../$1.rxbin" "$task_build/imports/$1.rxbin"
}
compile lllex "$task_dir/src/lllex.crexx"
compile lldfa "$task_dir/src/lldfa.crexx"
compile lldfabuild "$task_dir/src/lldfabuild.crexx"
compile lldp "$task_dir/src/lldp.crexx"
compile lexgen "$task_dir/tools/lexgen.crexx"
compile parsegen "$task_dir/tools/parsegen.crexx"
"$task_bin/rxtvm" "$task_build/lexgen.rxbin" "$task_build/lldfabuild.rxbin" "$task_bin/library.rxbin" -a "$task_dir/specs/common.lex" glex > "$task_build/glex.crexx" 2>> "$task_log"
compile glex "$task_build/glex.crexx"
"$task_bin/rxtvm" "$task_build/lexgen.rxbin" "$task_build/lldfabuild.rxbin" "$task_bin/library.rxbin" -a "$task_dir/specs/common.lex" gdlex dfa > "$task_build/gdlex.crexx" 2>> "$task_log"
compile gdlex "$task_build/gdlex.crexx"
for task_spec in priority rollback; do
  "$task_bin/rxtvm" "$task_build/lexgen.rxbin" "$task_build/lldfabuild.rxbin" "$task_bin/library.rxbin" -a "$task_dir/specs/$task_spec.lex" "g$task_spec" dfa > "$task_build/g$task_spec.crexx" 2>> "$task_log"
  compile "g$task_spec" "$task_build/g$task_spec.crexx"
done
while read -r task_spec task_namespace; do
  "$task_bin/rxtvm" "$task_build/parsegen.rxbin" "$task_bin/library.rxbin" -a "$task_dir/specs/$task_spec.peg" "$task_namespace" > "$task_build/$task_namespace.crexx" 2>> "$task_log"
  compile "$task_namespace" "$task_build/$task_namespace.crexx"
done <<'SPECS'
expression gexpr
statements gstmt
array garray
indirect gindirect
chars gchars
statement-chars gschar
choice gchoice
mixed gmixed
mixed-factored gfactored
SPECS
compile native "$task_dir/cases/native.crexx"
task_inputs=("$task_build/native.rxbin" "$task_build/lllex.rxbin" "$task_build/lldfa.rxbin" "$task_build/lldp.rxbin" "$task_build/glex.rxbin" "$task_build/gdlex.rxbin" "$task_build/gpriority.rxbin" "$task_build/grollback.rxbin" "$task_build/gchoice.rxbin" "$task_build/gmixed.rxbin" "$task_build/gfactored.rxbin" "$task_build/gexpr.rxbin" "$task_build/gstmt.rxbin" "$task_build/garray.rxbin" "$task_build/gindirect.rxbin" "$task_build/gchars.rxbin" "$task_bin/library.rxbin")
"$task_bin/rxlink" -o "$task_build/native-linked" "${task_inputs[@]}" >> "$task_log" 2>&1
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/native-linked.rxbin" > "$task_build/$task_vm.out" 2>&1
  cat "$task_build/$task_vm.out"
  rg -q '^native_failures=0$' "$task_build/$task_vm.out"
done
if [[ "$task_mode" == opt ]]; then
  compile measure "$task_dir/cases/measure.crexx"
  "$task_bin/rxlink" -o "$task_build/measure-linked" "$task_build/measure.rxbin" "$task_build/lldfa.rxbin" "$task_build/lldp.rxbin" "$task_build/gdlex.rxbin" "$task_build/gstmt.rxbin" "$task_build/gschar.rxbin" "$task_bin/library.rxbin" >> "$task_log" 2>&1
fi
cp "$task_log" "$task_build/build.log"
