#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/../CREXX/cmake-build-release/bin"}
task_build="$task_dir/build/opt"
task_log=$(mktemp /tmp/crexx-levell-summary.XXXXXX)
printf 'Summary build log: %s\n' "$task_log"
"$task_bin/rxc" -x --no-exe-import -i "$task_build/imports" -o "$task_build/summarize" "$task_dir/cases/summarize.crexx" >> "$task_log" 2>&1
"$task_bin/rxas" "$task_build/summarize" >> "$task_log" 2>&1
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/measure-linked.rxbin" -a "$task_vm" > "$task_dir/evidence/measure-$task_vm.csv" 2>&1
  if rg -q '^FAIL|PANIC' "$task_dir/evidence/measure-$task_vm.csv"; then exit 1; fi
  "$task_bin/rxtvm" "$task_build/summarize.rxbin" "$task_bin/library.rxbin" -a "$task_dir/evidence/measure-$task_vm.csv" > "$task_dir/evidence/summary-$task_vm.md" 2>> "$task_log"
  cat "$task_dir/evidence/summary-$task_vm.md"
done
