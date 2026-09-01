#!/bin/zsh

set -eu

evidence_dir=${0:A:h}
repo_root=${evidence_dir:h:h:h}
capture_tool="$repo_root/performance/tools/run_cross_runtime.crexx"
crexx="$repo_root/cmake-build-release/bin/crexx"

capture() {
  local vm=$1
  local round=$2
  local position=$3
  local variant=$4
  local warmups=0
  local cell="$evidence_dir/cells/$vm/round-${round}-position-${position}-${variant}"

  if [[ -f "$cell/manifest.json" ]]; then
    print -- "skip vm=$vm round=$round position=$position variant=$variant (complete)"
    return
  fi

  if [[ "$round" = "01" ]]; then
    warmups=1
  fi

  print -- "capture vm=$vm round=$round position=$position variant=$variant warmups=$warmups"
  "$crexx" "$capture_tool" --nokeep --args \
    --workload rexxcps \
    --runtime "$vm" \
    --variant "$variant" \
    --warmups "$warmups" \
    --runs 1 \
    --expect 'PASS: RexxCPS 2.2c cREXX port' \
    --metric-prefix 'Performance:' \
    --output-dir "$cell" -- \
    "$repo_root/cmake-build-release/bin/$vm" \
    "$evidence_dir/images/rexxcps-${variant}-linked.rxbin"
}

# Alternate order within each runtime and reverse the first order between VMs.
# Six rounds make each variant precede the other exactly three times per VM.
capture rxvm  01 1 baseline
capture rxvm  01 2 candidate
capture rxbvm 01 1 candidate
capture rxbvm 01 2 baseline

capture rxvm  02 1 candidate
capture rxvm  02 2 baseline
capture rxbvm 02 1 baseline
capture rxbvm 02 2 candidate

capture rxvm  03 1 baseline
capture rxvm  03 2 candidate
capture rxbvm 03 1 candidate
capture rxbvm 03 2 baseline

capture rxvm  04 1 candidate
capture rxvm  04 2 baseline
capture rxbvm 04 1 baseline
capture rxbvm 04 2 candidate

capture rxvm  05 1 baseline
capture rxvm  05 2 candidate
capture rxbvm 05 1 candidate
capture rxbvm 05 2 baseline

capture rxvm  06 1 candidate
capture rxvm  06 2 baseline
capture rxbvm 06 1 baseline
capture rxbvm 06 2 candidate
