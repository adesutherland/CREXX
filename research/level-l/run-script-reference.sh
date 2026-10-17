#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
if [[ -z "${LEVEL_L_RUN_ROOT:-}" ]]; then
  exec "$task_dir/run-clean.sh" "$0" "$@"
fi
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/cmake-build-release/bin"}
task_mode=${1:-opt}
case "$task_mode" in opt|noopt) ;; *) exit 2;; esac
task_build="$LEVEL_L_RUN_ROOT/build/script-reference-$task_mode"
task_deps="$LEVEL_L_RUN_ROOT/build/rexxscript-imports"
mkdir -p "$task_build/imports"
python3 "$task_dir/cases/prepare-script-reference.py" --bin "$task_bin" --output "$task_deps"
task_log=$(mktemp "$LEVEL_L_RUN_ROOT"/crexx-levell-script-reference.XXXXXX)
printf 'Build log: %s\n' "$task_log"
task_flags=(-x --no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags+=(-n); fi
for task_lib in rxfnsc classlib; do ln -sf "$task_deps/$task_lib.rxbin" "$task_build/imports/$task_lib.rxbin"; done
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
compile() {
  "$task_bin/rxc" "${task_flags[@]}" -i "$task_build/imports" -o "$task_build/$1" "$2" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then "$task_bin/rxas" -n "$task_build/$1" >> "$task_log" 2>&1
  else "$task_bin/rxas" "$task_build/$1" >> "$task_log" 2>&1; fi
  ln -sf "../$1.rxbin" "$task_build/imports/$1.rxbin"
}
compile RexxScriptProbeCore "$task_deps/RexxScriptProbeCore.crexx"
compile script-reference "$task_dir/cases/script-reference.crexx"
"$task_bin/rxlink" -o "$task_build/script-reference-linked" "$task_build/script-reference.rxbin" "$task_build/RexxScriptProbeCore.rxbin" "$task_deps/rxfnsc.rxbin" "$task_deps/classlib.rxbin" "$task_bin/library.rxbin" >> "$task_log" 2>&1
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/script-reference-linked.rxbin" -a "$task_dir/cases/fixtures/script-input.txt" > "$task_build/$task_vm.out" 2>&1
  tail -3 "$task_build/$task_vm.out"
done
cp "$task_log" "$task_build/build.log"
