#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_ref=${PIKA_SOURCE_DIR:-"$task_dir/build/pika-source"}
task_output=${1:-"$task_dir/build/toolkit-opt/pikalower-rxtvm.out"}
task_log=$(mktemp /tmp/crexx-levell-ranked-reference.XXXXXX)
printf 'Pika lowering reference log: %s\n' "$task_log"
[[ $(git -C "$task_ref" rev-parse HEAD) == 8050ceaf70c79a04416fdbdc374e92f9478de7b0 ]]
mkdir -p "$task_dir/build/java-core"
rg --files "$task_ref/src/main/java" -g '*.java' > "$task_dir/build/java-core/sources.txt"
javac -d "$task_dir/build/java-core" @"$task_dir/build/java-core/sources.txt" >> "$task_log" 2>&1
javac -cp "$task_dir/build/java-core" -d "$task_dir/build/java-core" "$task_dir/cases/PikaRankedProbe.java" >> "$task_log" 2>&1
java -cp "$task_dir/build/java-core" PikaRankedProbe "$task_output" >> "$task_log" 2>&1
tail -2 "$task_log"
