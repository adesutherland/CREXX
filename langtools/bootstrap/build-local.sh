#!/usr/bin/env bash
set -euo pipefail
task_root=$(git -C "$(dirname "$0")" rev-parse --show-toplevel)
task_bin=${CREXX_BIN_DIR:-"$task_root/cmake-build-release/bin"}
task_mode=${1:-opt}
case "$task_mode" in opt) task_noopt=OFF;; noopt) task_noopt=ON;; *) exit 2;; esac
task_work=$(mktemp -d "${TMPDIR:-/tmp}/crexx-bootstrap.XXXXXX")
trap 'rm -rf "$task_work"' EXIT
cmake -S "$task_root/langtools/bootstrap" -B "$task_work/build" \
  -DCREXX_HOST_BIN="$task_bin" -DCREXX_LEVEL_L_NOOPT="$task_noopt" -DCMAKE_BUILD_TYPE=Debug >"$task_work/build.log" 2>&1
if ! cmake --build "$task_work/build" --target rxlanglib levell_unicode_lexer --parallel 8 >>"$task_work/build.log" 2>&1; then
  tail -60 "$task_work/build.log"; exit 1
fi
printf 'Level L %s bootstrap converged.\n' "$task_mode"
