#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_bison=${BISON_BIN:-"$task_dir/build/bison-runtime/installed/bin/bison"}
task_build="$task_dir/build/cfg-reference"
task_evidence="$task_dir/evidence/cfg-reference"
mkdir -p "$task_build" "$task_evidence"
task_log=$(mktemp /tmp/crexx-cfg-reference.XXXXXX)
printf 'CFG reference log: %s\n' "$task_log"
"$task_bison" --version > "$task_evidence/runtime.txt"
shasum -a 256 "$task_bison" "$task_dir/cases/lr-reference.y" "$task_dir/cases/glr-reference.y" >> "$task_evidence/runtime.txt"
for task_mode in canonical-lr ielr lalr; do
  "$task_bison" -Dlr.type="$task_mode" --report=all -Wcounterexamples --report-file="$task_evidence/$task_mode.output" -o "$task_build/$task_mode.c" "$task_dir/cases/lr-reference.y" 2> "$task_evidence/$task_mode.diagnostics"
  cc -O2 "$task_build/$task_mode.c" -o "$task_build/$task_mode" >> "$task_log" 2>&1
  : > "$task_evidence/$task_mode.out"
  for task_input in acd bcd ace bce acc ''; do
    "$task_build/$task_mode" "$task_input" >> "$task_evidence/$task_mode.out"
  done
done
"$task_bison" -o "$task_build/glr.c" "$task_dir/cases/glr-reference.y" >> "$task_log" 2>&1
cc -O2 "$task_build/glr.c" -o "$task_build/glr" >> "$task_log" 2>&1
: > "$task_evidence/glr.out"
for task_input in a aa aaa aaaa aaaaa aab; do
  "$task_build/glr" "$task_input" >> "$task_evidence/glr.out"
done
rg -q 'reduce/reduce conflicts' "$task_evidence/lalr.diagnostics"
! rg -q 'conflict' "$task_evidence/canonical-lr.diagnostics"
! rg -q 'conflict' "$task_evidence/ielr.diagnostics"
cmp "$task_evidence/canonical-lr.out" "$task_evidence/ielr.out"
rg -q '^input=aaa accepted=1 derivations=2$' "$task_evidence/glr.out"
rg -q '^input=aaaaa accepted=1 derivations=14$' "$task_evidence/glr.out"
cat "$task_evidence/canonical-lr.out" "$task_evidence/glr.out"
printf 'PASS: independent LR family and packed-ambiguity controls\n'
