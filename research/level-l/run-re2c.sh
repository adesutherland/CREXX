#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
if [[ -z "${LEVEL_L_RUN_ROOT:-}" ]]; then
  exec "$task_dir/run-clean.sh" "$0" "$@"
fi
task_root=$(git -C "$task_dir" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/cmake-build-release/bin"}
task_re2c=${CREXX_RE2C:-"$task_bin/../third_party/re2c/re2c"}
task_build="$LEVEL_L_RUN_ROOT/build/opt"
task_log=$(mktemp "$LEVEL_L_RUN_ROOT"/crexx-levell-re2c.XXXXXX)
printf 're2c build log: %s\n' "$task_log"
"$task_bin/rxc" -x --no-exe-import -i "$task_build/imports" -o "$task_build/re2cwrap" "$task_dir/tools/re2cwrap.crexx" >> "$task_log" 2>&1
"$task_bin/rxas" "$task_build/re2cwrap" >> "$task_log" 2>&1
"$task_bin/rxtvm" "$task_build/re2cwrap.rxbin" "$task_bin/library.rxbin" -a "$task_dir/specs/common.lex" "$task_dir/tools/re2c-scanner.re.in" > "$task_build/re2c-control.re" 2>> "$task_log"
"$task_re2c" -W -o "$task_build/re2c-control.c" "$task_build/re2c-control.re" >> "$task_log" 2>&1
"${CC:-cc}" -O2 -Wall -Wextra -o "$task_build/re2c-scanner" "$task_build/re2c-control.c" >> "$task_log" 2>&1
"$task_bin/rxc" -i "$task_build/imports;$task_bin" -o "$task_build/re2ctest" "$task_dir/cases/re2c-control.crexx" >> "$task_log" 2>&1
"$task_bin/rxas" "$task_build/re2ctest" >> "$task_log" 2>&1
"$task_bin/rxlink" -o "$task_build/re2ctest-linked" "$task_build/re2ctest.rxbin" "$task_build/glex.rxbin" "$task_build/lllex.rxbin" "$task_bin/library.rxbin" "$task_bin/classlib.rxbin" >> "$task_log" 2>&1
for task_vm in rxtvm rxbvm; do
  "$task_bin/$task_vm" "$task_build/re2ctest-linked.rxbin" -a "$task_build/re2c-scanner" > "$LL_RESEARCH_OUTPUT/re2c-$task_vm.out" 2>&1
  cat "$LL_RESEARCH_OUTPUT/re2c-$task_vm.out"
done
cp "$task_log" "$task_build/re2c-build.log"
