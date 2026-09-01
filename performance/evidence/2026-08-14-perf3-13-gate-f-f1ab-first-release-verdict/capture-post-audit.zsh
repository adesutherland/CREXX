#!/bin/zsh

set -euo pipefail

gf_root=$(cd "$(dirname "$0")/../../.." && pwd)
gf_build_dir=${GF_BUILD_DIR:-"$gf_root/cmake-build-release"}
gf_control_dir=${GF_CONTROL_DIR:-"/private/tmp/crexx-e6-c0-closeout-release/interpreter"}
gf_output_dir=${1:?"usage: capture-post-audit.zsh OUTPUT_DIR"}
gf_candidate_dir="$gf_build_dir/interpreter"
gf_rxbin="$gf_candidate_dir/test_persistent_worker_executor.rxbin"
gf_channel_rxbin="$gf_candidate_dir/test_gate_f_channel_benchmark.rxbin"
gf_raw="$gf_output_dir/raw.log"
gf_samples="$gf_output_dir/samples.csv"
gf_position=0

mkdir -p "$gf_output_dir"
: > "$gf_raw"
print -r -- 'phase,round,position,vm,cell,variant,rate_jobs_per_second,elapsed_us' > "$gf_samples"

for gf_required in \
    "$gf_control_dir/persistent_worker_executor-rxbvml" \
    "$gf_control_dir/persistent_worker_executor-rxtvml" \
    "$gf_candidate_dir/persistent_worker_executor-rxbvml" \
    "$gf_candidate_dir/persistent_worker_executor-rxtvml" \
    "$gf_build_dir/bin/rxbvm" \
    "$gf_build_dir/bin/rxtvm" \
    "$gf_rxbin" \
    "$gf_channel_rxbin"; do
    [[ -f "$gf_required" ]] || {
        print -u2 -r -- "missing timing artifact: $gf_required"
        exit 1
    }
done

gf_run_executor() {
    local gf_phase=$1
    local gf_round=$2
    local gf_vm=$3
    local gf_cell=$4
    local gf_variant=$5
    local gf_executable
    local gf_mode
    local gf_jobs
    local gf_iterations
    local gf_workload
    local gf_output
    local gf_rate
    local gf_elapsed_ns
    local gf_elapsed_us

    if [[ "$gf_variant" == accepted ]]; then
        gf_executable="$gf_control_dir/persistent_worker_executor-$gf_vm"
    else
        gf_executable="$gf_candidate_dir/persistent_worker_executor-$gf_vm"
    fi
    case "$gf_cell" in
        spin-w1)
            gf_mode=executor1
            gf_jobs=256
            gf_iterations=65536
            gf_workload=spin
            ;;
        spin-w4)
            gf_mode=executor4
            gf_jobs=256
            gf_iterations=65536
            gf_workload=spin
            ;;
        identity)
            gf_mode=executor1
            gf_jobs=1024
            gf_iterations=1
            gf_workload=identity
            ;;
        *)
            print -u2 -r -- "unknown executor cell: $gf_cell"
            exit 1
            ;;
    esac

    gf_position=$((gf_position + 1))
    print -r -- \
        "executor phase=$gf_phase round=$gf_round position=$gf_position vm=$gf_vm cell=$gf_cell variant=$gf_variant" \
        >> "$gf_raw"
    gf_output=$(CREXX_VM_E5_DOORBELL=posix \
        "$gf_executable" --benchmark "$gf_mode" "$gf_rxbin" \
        "$gf_jobs" "$gf_iterations" "$gf_workload" 2>&1)
    print -r -- "$gf_output" >> "$gf_raw"
    print -r -- "$gf_output" | grep -q 'E5_BENCHMARK result=PASS'
    if [[ "$gf_phase" == recorded ]]; then
        gf_rate=$(print -r -- "$gf_output" | awk '/^E5_RATE:/ {print $2}')
        gf_elapsed_ns=$(print -r -- "$gf_output" |
            sed -n 's/.*elapsed_ns=\([0-9][0-9]*\).*/\1/p')
        gf_elapsed_us=$(awk -v value="$gf_elapsed_ns" 'BEGIN {printf "%.3f", value / 1000.0}')
        print -r -- \
            "$gf_phase,$gf_round,$gf_position,$gf_vm,$gf_cell,$gf_variant,$gf_rate,$gf_elapsed_us" \
            >> "$gf_samples"
    fi
}

gf_run_channel() {
    local gf_phase=$1
    local gf_round=$2
    local gf_vm=$3
    local gf_product
    local gf_output
    local gf_latency_us
    local gf_throughput_us
    local gf_latency_rate
    local gf_throughput_rate

    if [[ "$gf_vm" == rxbvml ]]; then
        gf_product="$gf_build_dir/bin/rxbvm"
    else
        gf_product="$gf_build_dir/bin/rxtvm"
    fi
    gf_position=$((gf_position + 1))
    print -r -- \
        "channel phase=$gf_phase round=$gf_round position=$gf_position vm=$gf_vm" \
        >> "$gf_raw"
    gf_output=$(cd "$gf_candidate_dir" &&
        "$gf_product" test_gate_f_channel_benchmark 2>&1)
    print -r -- "$gf_output" >> "$gf_raw"
    print -r -- "$gf_output" | grep -q 'PASS: Gate F F1b channel benchmark'
    if [[ "$gf_phase" == recorded ]]; then
        gf_latency_us=$(print -r -- "$gf_output" |
            awk '/^GF_CHANNEL_LATENCY_US:/ {print $2}')
        gf_throughput_us=$(print -r -- "$gf_output" |
            awk '/^GF_CHANNEL_THROUGHPUT_US:/ {print $2}')
        gf_latency_rate=$(awk -v value="$gf_latency_us" \
            'BEGIN {printf "%.12f", 1024.0 * 1000000.0 / value}')
        gf_throughput_rate=$(awk -v value="$gf_throughput_us" \
            'BEGIN {printf "%.12f", 256.0 * 1000000.0 / value}')
        print -r -- \
            "$gf_phase,$gf_round,$gf_position,$gf_vm,identity,channel,$gf_latency_rate,$gf_latency_us" \
            >> "$gf_samples"
        print -r -- \
            "$gf_phase,$gf_round,$gf_position,$gf_vm,spin-w4,channel,$gf_throughput_rate,$gf_throughput_us" \
            >> "$gf_samples"
    fi
}

gf_forward_round() {
    local gf_phase=$1
    local gf_round=$2
    gf_run_executor "$gf_phase" "$gf_round" rxbvml spin-w1 accepted
    gf_run_executor "$gf_phase" "$gf_round" rxbvml spin-w1 candidate
    gf_run_executor "$gf_phase" "$gf_round" rxbvml spin-w4 accepted
    gf_run_executor "$gf_phase" "$gf_round" rxbvml spin-w4 candidate
    gf_run_executor "$gf_phase" "$gf_round" rxbvml identity candidate
    gf_run_channel "$gf_phase" "$gf_round" rxbvml
    gf_run_executor "$gf_phase" "$gf_round" rxtvml spin-w1 accepted
    gf_run_executor "$gf_phase" "$gf_round" rxtvml spin-w1 candidate
    gf_run_executor "$gf_phase" "$gf_round" rxtvml spin-w4 accepted
    gf_run_executor "$gf_phase" "$gf_round" rxtvml spin-w4 candidate
    gf_run_executor "$gf_phase" "$gf_round" rxtvml identity candidate
    gf_run_channel "$gf_phase" "$gf_round" rxtvml
}

gf_reverse_round() {
    local gf_phase=$1
    local gf_round=$2
    gf_run_channel "$gf_phase" "$gf_round" rxtvml
    gf_run_executor "$gf_phase" "$gf_round" rxtvml identity candidate
    gf_run_executor "$gf_phase" "$gf_round" rxtvml spin-w4 candidate
    gf_run_executor "$gf_phase" "$gf_round" rxtvml spin-w4 accepted
    gf_run_executor "$gf_phase" "$gf_round" rxtvml spin-w1 candidate
    gf_run_executor "$gf_phase" "$gf_round" rxtvml spin-w1 accepted
    gf_run_channel "$gf_phase" "$gf_round" rxbvml
    gf_run_executor "$gf_phase" "$gf_round" rxbvml identity candidate
    gf_run_executor "$gf_phase" "$gf_round" rxbvml spin-w4 candidate
    gf_run_executor "$gf_phase" "$gf_round" rxbvml spin-w4 accepted
    gf_run_executor "$gf_phase" "$gf_round" rxbvml spin-w1 candidate
    gf_run_executor "$gf_phase" "$gf_round" rxbvml spin-w1 accepted
}

gf_forward_round warmup 0
for gf_round in {1..12}; do
    if (( gf_round % 2 )); then
        gf_forward_round recorded "$gf_round"
    else
        gf_reverse_round recorded "$gf_round"
    fi
done

print -r -- "$gf_output_dir"
