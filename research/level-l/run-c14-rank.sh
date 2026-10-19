#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
if [[ -z "${LEVEL_L_RUN_ROOT:-}" ]]; then
  exec "$task_dir/run-clean.sh" "$0" "$@"
fi
task_ref=${SQUIRREL_SOURCE:-"$LEVEL_L_RUN_ROOT/build/squirrel-source"}
task_dart=${DART_BIN:-"$LEVEL_L_RUN_ROOT/build/dart-runtime/dart-sdk/bin/dart"}
[[ $(git -C "$task_ref" rev-parse HEAD) == bb1ae84701060462636c07c814286ac2875e0568 ]]
for task_kind in rank pruning; do
  task_probe="$task_ref/dart/experiments/recovery/level_l_c14_${task_kind}_probe.dart"
  cat "$task_ref/dart/experiments/recovery/c14.dart" "$task_dir/cases/c14-$task_kind-probe.dart" > "$task_probe"
  "$task_dart" run "$task_probe" > "$LL_RESEARCH_OUTPUT/c14-$task_kind-reference.out" 2>&1
  cat "$LL_RESEARCH_OUTPUT/c14-$task_kind-reference.out"
done
