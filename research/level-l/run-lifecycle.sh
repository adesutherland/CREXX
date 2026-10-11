#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_dir/build/frozen-tools-20261010"}
task_mode=${1:-opt}
case "$task_mode" in opt|noopt) ;; *) exit 2;; esac
task_build="$task_dir/build/lifecycle-$task_mode"
mkdir -p "$task_build/imports"
task_log=$(mktemp /tmp/crexx-levell-lifecycle.XXXXXX)
printf 'Build log: %s\n' "$task_log"
task_flags=(-x --no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags+=(-n); fi
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
compile() {
  "$task_bin/rxc" "${task_flags[@]}" -i "$task_build/imports" -o "$task_build/$1" "$2" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then "$task_bin/rxas" -n "$task_build/$1" >> "$task_log" 2>&1
  else "$task_bin/rxas" "$task_build/$1" >> "$task_log" 2>&1; fi
  ln -sf "../$1.rxbin" "$task_build/imports/$1.rxbin"
}
task_inputs=()
for task_module in llcore llcheck llcap llhistory llpeg llcfg lllr llforest llearley llrepair llpika llpikalower llpikatree llc14cost llc14 lllrrepair llproject llstage lltabpeg llsquirrel llmunch lldfa lllex lldfabuild llpratt llregional; do
  compile "$task_module" "$task_dir/src/$task_module.crexx"
  task_inputs+=("$task_build/$task_module.rxbin")
done
compile lifecycle "$task_dir/cases/lifecycle.crexx"
compile lifecycle-summary "$task_dir/cases/lifecycle-summary.crexx"
"$task_bin/rxlink" -o "$task_build/lifecycle-linked" "$task_build/lifecycle.rxbin" "${task_inputs[@]}" "$task_bin/library.rxbin" >> "$task_log" 2>&1
cd "$task_root"
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/lifecycle-linked.rxbin" -a check > "$task_build/$task_vm.out" 2>&1
  tail -1 "$task_build/$task_vm.out"
  rg -q '^lifecycle_failures=0$' "$task_build/$task_vm.out"
done
cp "$task_log" "$task_build/build.log"
