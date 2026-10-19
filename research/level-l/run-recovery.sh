#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
if [[ -z "${LEVEL_L_RUN_ROOT:-}" ]]; then
  exec "$task_dir/run-clean.sh" "$0" "$@"
fi
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/cmake-build-release/bin"}
task_mode=${1:-opt}
task_case=${2:-label}
case "$task_mode" in opt|noopt) ;; *) exit 2;; esac
case "$task_case" in label|recovery) ;; *) exit 2;; esac
task_build="$LEVEL_L_RUN_ROOT/build/recovery-$task_mode"
mkdir -p "$task_build/imports"
task_log=$(mktemp "$LEVEL_L_RUN_ROOT"/crexx-levell-recovery.XXXXXX)
printf 'Recovery build log: %s\n' "$task_log"
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
task_flags=(--no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags+=(-n); fi
source "$task_dir/bootstrap-front.sh"
task_inputs=()
level_l_reuse_products
for task_module in llcore llcheck llcap llhistory llpeg llcfg lllr llforest llearley llrepair llpika llpikaexplain llpikalower llc14cost llc14 lllrrepair llproject llstage llfront lllabel "$task_case"; do

  if level_l_is_product "$task_module"; then continue; fi
  task_source="$task_dir/src/$task_module.crexx"
  if [[ "$task_module" == "$task_case" ]]; then task_source="$task_dir/cases/$task_case.crexx"; fi
  "$task_bin/rxc" "${task_flags[@]}" -x -i "$task_build/imports" -o "$task_build/$task_module" "$task_source" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then "$task_bin/rxas" -n "$task_build/$task_module" >> "$task_log" 2>&1
  else "$task_bin/rxas" "$task_build/$task_module" >> "$task_log" 2>&1; fi
  ln -sf "../$task_module.rxbin" "$task_build/imports/$task_module.rxbin"
  if [[ "$task_module" != "$task_case" ]]; then task_inputs+=("$task_build/$task_module.rxbin"); fi
done
"$task_bin/rxlink" -o "$task_build/$task_case-linked" "$task_build/$task_case.rxbin" "${task_inputs[@]}" "$task_bin/library.rxbin" >> "$task_log" 2>&1
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/$task_case-linked.rxbin" > "$task_build/$task_case-$task_vm.out" 2>&1
  tail -4 "$task_build/$task_case-$task_vm.out"
  rg -q "^${task_case}_failures=0$" "$task_build/$task_case-$task_vm.out"
done
cp "$task_log" "$task_build/$task_case-build.log"
