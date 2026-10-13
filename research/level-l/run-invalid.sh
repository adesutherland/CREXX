#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/../CREXX/cmake-build-release/bin"}
task_build="$task_dir/build/opt"
mkdir -p "$task_build/invalid"
printf '1 a*\n' > "$task_build/invalid/nullable.lex"
printf '1 [z-a]\n' > "$task_build/invalid/range.lex"
printf "Start <- Missing\n" > "$task_build/invalid/undefined.peg"
printf "Start <- ('')*\n" > "$task_build/invalid/nullable.peg"
printf "Start <- ! 'a'\n" > "$task_build/invalid/predicate.peg"
for task_vm in rxtvm rxbvm; do
  task_output="$task_dir/evidence/invalid-$task_vm.out"
  : > "$task_output"
  for task_file in nullable.lex range.lex undefined.peg nullable.peg predicate.peg; do
    task_generator=parsegen
    task_providers=("$task_bin/library.rxbin")
    if [[ "$task_file" == *.lex ]]; then
      task_generator=lexgen
      task_providers+=("$task_build/lldfabuild.rxbin")
    fi
    if "$task_bin/$task_vm" "$task_build/$task_generator.rxbin" "${task_providers[@]}" -a "$task_build/invalid/$task_file" rejected >> "$task_output" 2>&1; then
      printf 'FAIL: invalid specification accepted: %s\n' "$task_file" >> "$task_output"; exit 1
    else
      task_status=$?
      if [[ "$task_status" != 1 ]]; then
        printf 'FAIL: expected controlled rejection, got %s\n' "$task_status" >> "$task_output"; exit 1
      fi
      printf 'PASS: controlled rejection of %s\n' "$task_file" >> "$task_output"
    fi
  done
  cat "$task_output"
done
