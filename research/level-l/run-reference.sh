#!/usr/bin/env bash
set -euo pipefail
task_dir=$(cd "$(dirname "$0")" && pwd)
if [[ -z "${LEVEL_L_RUN_ROOT:-}" ]]; then
  exec "$task_dir/run-clean.sh" "$0" "$@"
fi
task_ref=${PIKA_SOURCE_DIR:-"$LEVEL_L_RUN_ROOT/build/pika-source"}
task_commit=8050ceaf70c79a04416fdbdc374e92f9478de7b0
task_log=$(mktemp "$LEVEL_L_RUN_ROOT"/crexx-levell-pika.XXXXXX)
printf 'Pika build log: %s\n' "$task_log"
if [[ ! -d "$task_ref/.git" ]]; then
  git clone --quiet https://github.com/lukehutch/pikaparser.git "$task_ref" >> "$task_log" 2>&1
  git -C "$task_ref" checkout --quiet "$task_commit" >> "$task_log" 2>&1
fi
if [[ $(git -C "$task_ref" rev-parse HEAD) != "$task_commit" ]]; then
  printf 'Reference checkout is not the pinned commit.\n' >&2; exit 1
fi
mkdir -p "$LEVEL_L_RUN_ROOT/build/java-classes"
rg --files "$task_ref/src/main/java" -g '*.java' > "$LEVEL_L_RUN_ROOT/build/java-sources.txt"
javac -d "$LEVEL_L_RUN_ROOT/build/java-classes" @"$LEVEL_L_RUN_ROOT/build/java-sources.txt" >> "$task_log" 2>&1
javac -cp "$LEVEL_L_RUN_ROOT/build/java-classes" -d "$LEVEL_L_RUN_ROOT/build/java-classes" "$task_dir/cases/PikaProbe.java" >> "$task_log" 2>&1
java -cp "$LEVEL_L_RUN_ROOT/build/java-classes" PikaProbe > "$LL_RESEARCH_OUTPUT/pika-reference.out" 2>> "$task_log"
cat "$LL_RESEARCH_OUTPUT/pika-reference.out"
