#!/bin/zsh

set -eu

audit_dir=${0:A:h}
repo=${audit_dir:h:h:h:h}
image_dir="$audit_dir/portfolio-images"
output="$audit_dir/portfolio-release-timing.csv"

zmodload zsh/datetime

capture() {
  local workload=$1
  local arg=$2
  local vm=$3
  local round=$4
  local position=$5
  local variant=$6
  local start elapsed log

  log="$audit_dir/portfolio-release-${workload}-${vm}-round-${round}-${variant}.log"
  start=$EPOCHREALTIME
  "$repo/cmake-build-release/bin/$vm" \
    "$image_dir/$workload-$variant-linked.rxbin" -a "$arg" > "$log" 2>&1
  elapsed=$(( EPOCHREALTIME - start ))
  printf '%s,%s,%d,%d,%s,%s,%.9f\n' \
    "$workload" "$vm" "$round" "$position" "$variant" "$arg" "$elapsed" >> "$output"
}

printf 'workload,vm,round,position,variant,argv,elapsed_seconds\n' > "$output"

for spec in awfy_list:100 awfy_richards:1 awfy_storage:10 base64_roundtrip:500; do
  workload=${spec%%:*}
  arg=${spec##*:}

  for vm in rxvm rxbvm; do
    # Warm both exact images outside the recorded set.
    "$repo/cmake-build-release/bin/$vm" \
      "$image_dir/$workload-baseline-linked.rxbin" -a "$arg" > /dev/null 2>&1
    "$repo/cmake-build-release/bin/$vm" \
      "$image_dir/$workload-candidate-linked.rxbin" -a "$arg" > /dev/null 2>&1

    # Four alternating rounds put each image first twice. Reverse the initial
    # order between VM modes.
    for round in {1..4}; do
      if { [[ "$vm" = rxvm ]] && (( round % 2 )); } || \
         { [[ "$vm" = rxbvm ]] && (( round % 2 == 0 )); }; then
        capture "$workload" "$arg" "$vm" "$round" 1 baseline
        capture "$workload" "$arg" "$vm" "$round" 2 candidate
      else
        capture "$workload" "$arg" "$vm" "$round" 1 candidate
        capture "$workload" "$arg" "$vm" "$round" 2 baseline
      fi
    done
  done
done

