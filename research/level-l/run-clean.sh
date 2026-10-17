#!/usr/bin/env bash
# One disposable workspace shared by nested research commands; no run archives.
set -euo pipefail
task_workspace=$(mktemp -d "${TMPDIR:-/tmp}/crexx-level-l.XXXXXX")
trap 'rm -rf "$task_workspace"' EXIT
export LEVEL_L_RUN_ROOT="$task_workspace"
export LL_RESEARCH_OUTPUT="$task_workspace/output"
mkdir -p "$LL_RESEARCH_OUTPUT" "$task_workspace/build"
if "$@"; then exit 0; else
  task_status=$?
  find "$task_workspace" -type f \( -name "*.out" -o -name "*.log" -o -name "crexx-*" \) -exec tail -60 {} \; >&2
  exit "$task_status"
fi
