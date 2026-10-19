#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
if [[ -z "${LEVEL_L_RUN_ROOT:-}" ]]; then
  exec "$task_dir/run-clean.sh" "$0" "$@"
fi
task_ref=${PIKA_SOURCE_DIR:-"$LEVEL_L_RUN_ROOT/build/pika-source"}
task_output=${1:-"$LEVEL_L_RUN_ROOT/build/toolkit-opt/pikalower-rxtvm.out"}
task_log=$(mktemp "$LEVEL_L_RUN_ROOT"/crexx-levell-ranked-reference.XXXXXX)
printf 'Pika lowering reference log: %s\n' "$task_log"
[[ $(git -C "$task_ref" rev-parse HEAD) == 8050ceaf70c79a04416fdbdc374e92f9478de7b0 ]]
mkdir -p "$LEVEL_L_RUN_ROOT/build/java-core"
rg --files "$task_ref/src/main/java" -g '*.java' > "$LEVEL_L_RUN_ROOT/build/java-core/sources.txt"
javac -d "$LEVEL_L_RUN_ROOT/build/java-core" @"$LEVEL_L_RUN_ROOT/build/java-core/sources.txt" >> "$task_log" 2>&1
javac -cp "$LEVEL_L_RUN_ROOT/build/java-core" -d "$LEVEL_L_RUN_ROOT/build/java-core" "$task_dir/cases/PikaRankedProbe.java" >> "$task_log" 2>&1
java -cp "$LEVEL_L_RUN_ROOT/build/java-core" PikaRankedProbe "$task_output" >> "$task_log" 2>&1
tail -2 "$task_log"
