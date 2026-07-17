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

# Each runtime sees a rotating order. The rxbvm order reverses the rxvm order
# in each round, balancing pair precedence across the complete experiment.
capture rxvm  01 1 original
capture rxvm  01 2 direct-only
capture rxvm  01 3 full
capture rxbvm 01 1 full
capture rxbvm 01 2 direct-only
capture rxbvm 01 3 original

capture rxvm  02 1 full
capture rxvm  02 2 original
capture rxvm  02 3 direct-only
capture rxbvm 02 1 direct-only
capture rxbvm 02 2 original
capture rxbvm 02 3 full

capture rxvm  03 1 direct-only
capture rxvm  03 2 full
capture rxvm  03 3 original
capture rxbvm 03 1 original
capture rxbvm 03 2 full
capture rxbvm 03 3 direct-only

capture rxvm  04 1 original
capture rxvm  04 2 full
capture rxvm  04 3 direct-only
capture rxbvm 04 1 direct-only
capture rxbvm 04 2 full
capture rxbvm 04 3 original

capture rxvm  05 1 full
capture rxvm  05 2 direct-only
capture rxvm  05 3 original
capture rxbvm 05 1 original
capture rxbvm 05 2 direct-only
capture rxbvm 05 3 full
