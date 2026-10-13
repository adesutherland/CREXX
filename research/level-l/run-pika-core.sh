#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/../CREXX/cmake-build-release/bin"}
task_mode=${1:-opt}
task_ref=${PIKA_SOURCE_DIR:-"$task_dir/build/pika-source"}
task_log=$(mktemp /tmp/crexx-levell-pikacore.XXXXXX)
printf 'Pika core log: %s\n' "$task_log"
if [[ "$task_mode" == reference ]]; then
  [[ $(git -C "$task_ref" rev-parse HEAD) == 8050ceaf70c79a04416fdbdc374e92f9478de7b0 ]]
  mkdir -p "$task_dir/build/java-core"
  rg --files "$task_ref/src/main/java" -g '*.java' > "$task_dir/build/java-core/sources.txt"
  javac -d "$task_dir/build/java-core" @"$task_dir/build/java-core/sources.txt" >> "$task_log" 2>&1
  javac -cp "$task_dir/build/java-core" -d "$task_dir/build/java-core" "$task_dir/cases/PikaCoreProbe.java" >> "$task_log" 2>&1
  java -cp "$task_dir/build/java-core" PikaCoreProbe "$task_dir/evidence/pika-core-fixtures.txt" "$task_dir/evidence/pika-core-expected.txt" >> "$task_log" 2>&1
  tail -3 "$task_log"
  exit 0
fi
case "$task_mode" in opt|noopt) ;; *) exit 2;; esac
task_build="$task_dir/build/pika-core-$task_mode"
mkdir -p "$task_build/imports"
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
task_flags=(--no-exe-import)
if [[ "$task_mode" == noopt ]]; then task_flags+=(-n); fi
task_inputs=()
for task_module in llcore llhistory llpika pika; do
  task_source="$task_dir/src/$task_module.crexx"
  if [[ "$task_module" == pika ]]; then task_source="$task_dir/cases/pika.crexx"; fi
  "$task_bin/rxc" "${task_flags[@]}" -x -i "$task_build/imports" -o "$task_build/$task_module" "$task_source" >> "$task_log" 2>&1
  if [[ "$task_mode" == noopt ]]; then
    "$task_bin/rxas" -n "$task_build/$task_module" >> "$task_log" 2>&1
  else
    "$task_bin/rxas" "$task_build/$task_module" >> "$task_log" 2>&1
  fi
  ln -sf "../$task_module.rxbin" "$task_build/imports/$task_module.rxbin"
  task_inputs+=("$task_build/$task_module.rxbin")
done
"$task_bin/rxlink" -o "$task_build/pika-linked" "$task_build/pika.rxbin" "${task_inputs[@]:0:3}" "$task_bin/library.rxbin" >> "$task_log" 2>&1
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/pika-linked.rxbin" -a "$task_dir/evidence/pika-core-fixtures.txt" "$task_dir/evidence/pika-core-expected.txt" > "$task_build/$task_vm.out" 2>&1
  tail -2 "$task_build/$task_vm.out"
  rg -q '^pika_failures=0$' "$task_build/$task_vm.out"
done
cp "$task_log" "$task_build/build.log"
