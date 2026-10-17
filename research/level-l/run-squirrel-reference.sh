#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
if [[ -z "${LEVEL_L_RUN_ROOT:-}" ]]; then
  exec "$task_dir/run-clean.sh" "$0" "$@"
fi
task_pin=bb1ae84701060462636c07c814286ac2875e0568
task_source=${SQUIRREL_SOURCE:-"$LEVEL_L_RUN_ROOT/build/squirrel-source"}
task_dart=${DART_BIN:-"$LEVEL_L_RUN_ROOT/build/dart-runtime/dart-sdk/bin/dart"}
task_log=$(mktemp "$LEVEL_L_RUN_ROOT"/crexx-squirrel-reference.XXXXXX)
printf 'Reference log: %s\n' "$task_log"
if [[ ! -d "$task_source/.git" ]]; then
  git clone -q https://github.com/lukehutch/squirrelparser.git "$task_source" >> "$task_log" 2>&1
  git -C "$task_source" checkout -q --detach "$task_pin" >> "$task_log" 2>&1
fi
test "$(git -C "$task_source" rev-parse HEAD)" = "$task_pin"
git -C "$task_source" diff --exit-code >> "$task_log" 2>&1
mkdir -p "$task_source/dart/bin"
cp "$task_dir/cases/SquirrelProbe.dart" "$task_source/dart/bin/level_l_probe.dart"
mkdir -p "$LL_RESEARCH_OUTPUT"
(
  cd "$task_source/dart"
  "$task_dart" pub get >> "$task_log" 2>&1
  "$task_dart" run bin/level_l_probe.dart "$task_dir/cases/fixtures/squirrel-expected.txt" > "$LL_RESEARCH_OUTPUT/squirrel-reference.jsonl" 2>> "$task_log"
)
"$task_dart" --version > "$LL_RESEARCH_OUTPUT/squirrel-runtime.txt" 2>&1
printf 'Pinned reference: %s\n' "$task_pin" >> "$LL_RESEARCH_OUTPUT/squirrel-runtime.txt"
shasum -a 256 "$task_dart" "$task_dir/cases/SquirrelProbe.dart" "$task_source/dart/pubspec.lock" >> "$LL_RESEARCH_OUTPUT/squirrel-runtime.txt"
printf 'Reference output: %s\n' "$LL_RESEARCH_OUTPUT/squirrel-reference.jsonl"
