#!/usr/bin/env bash
# Build the same three-stage graph from an already-built native B toolchain.
set -euo pipefail
task_root=$(cd "$(dirname "$0")/../../.." && pwd)
task_bin=${CREXX_BIN_DIR:-"$task_root/research/level-l/build/frozen-tools-20261010"}
task_mode=${1:-opt}
case "$task_mode" in opt) task_noopt=OFF;; noopt) task_noopt=ON;; *) exit 2;; esac
task_build="$task_root/research/level-l/build/product-$task_mode"
mkdir -p "$task_build/imports"
task_log=$(mktemp /tmp/crexx-levell-product.XXXXXX)
printf 'Build log: %s\n' "$task_log"
trap 'task_rc=$?; if [[ $task_rc != 0 ]]; then tail -45 "$task_log"; fi' EXIT
cmake -S "$task_root/lib/rxfnsl/bootstrap" -B "$task_build/bootstrap-build" \
  -DCREXX_HOST_BIN="$task_bin" -DCREXX_LEVEL_L_NOOPT="$task_noopt" -DCMAKE_BUILD_TYPE=Debug >>"$task_log" 2>&1
cmake --build "$task_build/bootstrap-build" --target rxfnsl levell_unicode_lexer --parallel 8 >>"$task_log" 2>&1
for task_module in lllimits llcore llcheck llcap llcfg lllr llmunch llproject llstage llcharset llregular llsource llpeg llforest llearley llpratt llregional lllanguage llrecover lltree; do
  ln -sf "bootstrap-build/rexx/$task_module.rxbin" "$task_build/$task_module.rxbin"
  ln -sf "../$task_module.rxbin" "$task_build/imports/$task_module.rxbin"
done
for task_module in llfront llsyntax; do
  ln -sf "bootstrap-build/tools/bootstrap/L2/$task_module.rxbin" "$task_build/$task_module.rxbin"
  ln -sf "../$task_module.rxbin" "$task_build/imports/$task_module.rxbin"
done
ln -sf "$task_bin/library.rxbin" "$task_build/imports/library.rxbin"
ln -sf bootstrap-build/tools/bootstrap/L2/levell-linked.rxbin "$task_build/levell.rxbin"
cp "$task_log" "$task_build/build.log"
printf 'Built %s\n' "$task_build/levell.rxbin"
