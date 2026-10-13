#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
task_ref=${PIKA_SOURCE_DIR:-"$task_dir/build/pika-source"}
task_commit=8050ceaf70c79a04416fdbdc374e92f9478de7b0
task_log=$(mktemp /tmp/crexx-levell-pika.XXXXXX)
printf 'Pika build log: %s\n' "$task_log"
if [[ ! -d "$task_ref/.git" ]]; then
  git clone --quiet https://github.com/lukehutch/pikaparser.git "$task_ref" >> "$task_log" 2>&1
  git -C "$task_ref" checkout --quiet "$task_commit" >> "$task_log" 2>&1
fi
if [[ $(git -C "$task_ref" rev-parse HEAD) != "$task_commit" ]]; then
  printf 'Reference checkout is not the pinned commit.\n' >&2; exit 1
fi
mkdir -p "$task_dir/build/java-classes"
rg --files "$task_ref/src/main/java" -g '*.java' > "$task_dir/build/java-sources.txt"
javac -d "$task_dir/build/java-classes" @"$task_dir/build/java-sources.txt" >> "$task_log" 2>&1
javac -cp "$task_dir/build/java-classes" -d "$task_dir/build/java-classes" "$task_dir/cases/PikaProbe.java" >> "$task_log" 2>&1
java -cp "$task_dir/build/java-classes" PikaProbe > "$task_dir/evidence/pika-reference.out" 2>> "$task_log"
cat "$task_dir/evidence/pika-reference.out"
