#!/usr/bin/env bash
# Sourced by historical research runners: use the maintained generated reader
# and its Level B dependencies rather than a second handwritten frontend.
level_l_reuse_front() {
  local product="$task_root/research/level-l/build/product-$task_mode"
  CREXX_BIN_DIR="$task_bin" "$task_root/lib/rxfnsl/tools/build-local.sh" "$task_mode" >>"$task_log" 2>&1
  local module path existing present
  for module in llcharset llmunch llsource lllanguage lltree llsyntax llfront; do
    path="$task_build/$module.rxbin"
    cp "$product/$module.rxbin" "$path"
    ln -sf "../$module.rxbin" "$task_build/imports/$module.rxbin"
    if [[ $module == llfront ]]; then continue; fi
    present=0
    for existing in ${task_inputs[@]+"${task_inputs[@]}"}; do
      if [[ $existing == "$path" ]]; then present=1; fi
    done
    if [[ $present == 0 ]]; then task_inputs+=("$path"); fi
  done
}
