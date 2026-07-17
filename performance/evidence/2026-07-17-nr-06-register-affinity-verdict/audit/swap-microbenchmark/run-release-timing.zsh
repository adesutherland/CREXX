#!/bin/zsh

set -eu

dir=${0:A:h}
repo=${dir:h:h:h:h:h}
output="$dir/release-timing.csv"

zmodload zsh/datetime

capture() {
  local vm=$1
  local round=$2
  local position=$3
  local variant=$4
  local start elapsed

  start=$EPOCHREALTIME
  "$repo/cmake-build-release/bin/$vm" "$dir/$variant.rxbin" \
    > "$dir/release-$vm-round-${round}-${variant}.log" 2>&1
  elapsed=$(( EPOCHREALTIME - start ))
  printf '%s,%d,%d,%s,%.9f\n' \
    "$vm" "$round" "$position" "$variant" "$elapsed" >> "$output"
}

printf 'vm,round,position,variant,elapsed_seconds\n' > "$output"

# Warm each VM and image once outside the recorded set.
for vm in rxvm rxbvm; do
  "$repo/cmake-build-release/bin/$vm" "$dir/control.rxbin" > /dev/null 2>&1
  "$repo/cmake-build-release/bin/$vm" "$dir/swap.rxbin" > /dev/null 2>&1
done

# Sixteen alternating rounds give each image each position eight times.
for vm in rxvm rxbvm; do
  for round in {1..16}; do
    if (( round % 2 )); then
      capture "$vm" "$round" 1 control
      capture "$vm" "$round" 2 swap
    else
      capture "$vm" "$round" 1 swap
      capture "$vm" "$round" 2 control
    fi
  done
done

