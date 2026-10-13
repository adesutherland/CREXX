#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/../CREXX/cmake-build-release/bin"}
task_mode=${1:-opt}
task_log=$(mktemp /tmp/crexx-levell-c14.XXXXXX)
printf 'c14 log: %s\n' "$task_log"
if [[ "$task_mode" == reference ]]; then
  task_ref=${SQUIRREL_SOURCE:-"$task_dir/build/squirrel-source"}
  task_dart=${DART_BIN:-"$task_dir/build/dart-runtime/dart-sdk/bin/dart"}
  [[ $(git -C "$task_ref" rev-parse HEAD) == bb1ae84701060462636c07c814286ac2875e0568 ]]
  mkdir -p "$task_ref/dart/tool"
  cp "$task_dir/cases/C14Probe.dart" "$task_ref/dart/tool/level_l_c14_probe.dart"
  "$task_dart" run "$task_ref/dart/tool/level_l_c14_probe.dart" "$task_dir/evidence/c14-fixtures.txt" "$task_dir/evidence/c14-expected.txt" >> "$task_log" 2>&1
  tail -3 "$task_log"
  exit 0
fi
case "$task_mode" in opt|noopt) ;; *) exit 2;; esac
task_build=${C14_BUILD_DIR:-"$task_dir/build/c14-$task_mode"}
mkdir -p "$task_build/imports"
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
task_flags=(--no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags+=(-n); fi
task_inputs=()
for task_module in llcore llhistory llc14cost llc14 c14; do
  task_source="$task_dir/src/$task_module.crexx"
  if [[ "$task_module" == llc14 && -n "${C14_SOURCE:-}" ]]; then task_source="$C14_SOURCE"; fi
  if [[ "$task_module" == c14 ]]; then task_source="${C14_CASE_SOURCE:-$task_dir/cases/c14.crexx}"; fi
  "$task_bin/rxc" "${task_flags[@]}" -x -i "$task_build/imports" -o "$task_build/$task_module" "$task_source" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then
    "$task_bin/rxas" -n "$task_build/$task_module" >> "$task_log" 2>&1
  else
    "$task_bin/rxas" "$task_build/$task_module" >> "$task_log" 2>&1
  fi
  ln -sf "../$task_module.rxbin" "$task_build/imports/$task_module.rxbin"
  task_inputs+=("$task_build/$task_module.rxbin")
done
"$task_bin/rxlink" -o "$task_build/c14-linked" "$task_build/c14.rxbin" "${task_inputs[@]:0:4}" "$task_bin/library.rxbin" >> "$task_log" 2>&1
task_args=("${C14_FIXTURES:-$task_dir/evidence/c14-fixtures.txt}" "${C14_EXPECTED:-$task_dir/evidence/c14-expected.txt}")
if [[ "${C14_SURVEY:-0}" == 1 ]]; then task_args+=(survey); fi
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/c14-linked.rxbin" -a "${task_args[@]}" > "$task_build/$task_vm.out" 2>&1
  tail -2 "$task_build/$task_vm.out"
  rg -q '^c14_failures=0$' "$task_build/$task_vm.out"
done
cp "$task_log" "$task_build/build.log"
