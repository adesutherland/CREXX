#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_pin=bb1ae84701060462636c07c814286ac2875e0568
task_source=${SQUIRREL_SOURCE:-"$task_dir/build/squirrel-source"}
task_dart=${DART_BIN:-"$task_dir/build/dart-runtime/dart-sdk/bin/dart"}
task_log=$(mktemp /tmp/crexx-squirrel-reference.XXXXXX)
printf 'Reference log: %s\n' "$task_log"
if [[ ! -d "$task_source/.git" ]]; then
  git clone -q https://github.com/lukehutch/squirrelparser.git "$task_source" >> "$task_log" 2>&1
  git -C "$task_source" checkout -q --detach "$task_pin" >> "$task_log" 2>&1
fi
test "$(git -C "$task_source" rev-parse HEAD)" = "$task_pin"
git -C "$task_source" diff --exit-code >> "$task_log" 2>&1
mkdir -p "$task_source/dart/bin"
cp "$task_dir/cases/SquirrelProbe.dart" "$task_source/dart/bin/level_l_probe.dart"
mkdir -p "$task_dir/evidence"
(
  cd "$task_source/dart"
  "$task_dart" pub get >> "$task_log" 2>&1
  "$task_dart" run bin/level_l_probe.dart "$task_dir/evidence/squirrel-expected.txt" > "$task_dir/evidence/squirrel-reference.jsonl" 2>> "$task_log"
)
"$task_dart" --version > "$task_dir/evidence/squirrel-runtime.txt" 2>&1
printf 'Pinned reference: %s\n' "$task_pin" >> "$task_dir/evidence/squirrel-runtime.txt"
shasum -a 256 "$task_dart" "$task_dir/cases/SquirrelProbe.dart" "$task_source/dart/pubspec.lock" >> "$task_dir/evidence/squirrel-runtime.txt"
printf 'Reference output: %s\n' "$task_dir/evidence/squirrel-reference.jsonl"
