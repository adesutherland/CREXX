#!/usr/bin/env bash
# Research imports canonical products; no copied SDK implementations or aliases.
level_l_is_product() {
  [[ $1 == llfront || -f "$task_root/lib/rxfnsl/rexx/$1.crexx" ]]
}
level_l_reuse_products() {
  local module
  for module in rxfnsl rxlanglib; do
    test -f "$task_bin/$module.rxbin"
    ln -sf "$task_bin/$module.rxbin" "$task_build/imports/$module.rxbin"
    task_inputs+=("$task_bin/$module.rxbin")
  done
}
