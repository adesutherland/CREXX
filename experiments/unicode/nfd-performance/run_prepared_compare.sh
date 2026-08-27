#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
default_build_dir="$repo_dir/cmake-build-unicode-release"
work_root=${CREXX_UNICODE_COMPARE_BUILD_DIR:-"$default_build_dir/unicode-prepared-compare"}
prepared_dir=${CREXX_UNICODE_COMPARE_PREPARED_DIR:-}
if [ -z "$prepared_dir" ]; then
    latest_file="$work_root/latest-prepared.txt"
    if [ ! -f "$latest_file" ]; then
        echo "no prepared comparison; run $script_dir/prepare_prepared_compare.sh first" >&2
        exit 1
    fi
    prepared_dir=$(sed -n '1p' "$latest_file")
fi

manifest="$prepared_dir/prepared.manifest"
if [ ! -f "$manifest" ]; then
    echo "prepared comparison manifest not found: $manifest" >&2
    exit 1
fi
manifest_value() {
    key=$1
    sed -n "s/^$key=//p" "$manifest"
}

product_build_dir=$(manifest_value product_build_dir)
manifest_prepared_dir=$(manifest_value prepared_dir)
if [ "$manifest_prepared_dir" != "$prepared_dir" ]; then
    echo "prepared comparison directory does not match its manifest" >&2
    exit 1
fi

rxtvm_bin="$product_build_dir/bin/rxtvm"
rxbvm_bin="$product_build_dir/bin/rxbvm"
linked_image="$prepared_dir/unicode_prepared_compare_linked.rxbin"
benchmark_module="$prepared_dir/benchmark_unicode_prepared_compare.rxbin"
for required in "$rxtvm_bin" "$rxbvm_bin" "$linked_image" "$benchmark_module"; do
    if [ ! -f "$required" ]; then
        echo "prepared comparison artifact not found: $required" >&2
        exit 1
    fi
done

fingerprint() {
    cmake -E sha256sum "$@" | shasum -a 256 | awk '{print $1}'
}
expected_prepared_fingerprint=$(manifest_value prepared_fingerprint)
actual_prepared_fingerprint=$(fingerprint "$benchmark_module" "$linked_image")
if [ "$actual_prepared_fingerprint" != "$expected_prepared_fingerprint" ]; then
    echo "prepared comparison artifacts no longer match their manifest" >&2
    exit 1
fi

warmups=${CREXX_UNICODE_COMPARE_WARMUPS:-1}
samples=${CREXX_UNICODE_COMPARE_SAMPLES:-3}
rounds=${CREXX_UNICODE_COMPARE_ROUNDS:-3}
buffer_repetitions=${CREXX_UNICODE_COMPARE_BUFFER_REPETITIONS:-1}
variants=${CREXX_UNICODE_COMPARE_VARIANTS:-"original-nfd generated-nfd"}
workloads=${CREXX_UNICODE_COMPARE_WORKLOADS:-"rows buffer"}
nfc_input="$repo_dir/experiments/unicode/inputs/icu-78.3/nfc.txt"
ucd_dir="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd"
unicode_data_input="$ucd_dir/UnicodeData.txt"
normalization_properties_input="$ucd_dir/DerivedNormalizationProps.txt"
normalization_test_input="$ucd_dir/NormalizationTest.txt"
runs_root="$prepared_dir/runs"
mkdir -p "$runs_root"
run_dir=$(mktemp -d "$runs_root/run.XXXXXX")
evidence_dir=${CREXX_UNICODE_COMPARE_EVIDENCE_DIR:-"$run_dir/evidence"}
mkdir -p "$evidence_dir"

{
    echo "activity=UNICODE-NORM-02"
    echo "scope=rough algorithm screen on prebuilt artifacts; not a formal performance verdict"
    echo "branch=$(manifest_value branch)"
    echo "commit=$(manifest_value commit)"
    echo "source_fingerprint=$(manifest_value source_fingerprint)"
    echo "product_fingerprint=$(manifest_value product_fingerprint)"
    echo "prepared_fingerprint=$actual_prepared_fingerprint"
    echo "prepared_dir=$prepared_dir"
    echo "run_dir=$run_dir"
    echo "warmups=$warmups"
    echo "samples=$samples"
    echo "rounds=$rounds"
    echo "buffer_repetitions=$buffer_repetitions"
    echo "variants=$variants"
    echo "workloads=$workloads"
    echo "timed_lifecycle=preloaded rows or prebuilt string buffer; normalize through the same string surface; materialize and observe result"
    echo "generated_route=prepared Unicode virtual alphabet to shared Level B exact-kind SELECT; generated wrapper built before this run"
    uname -a
    sw_vers 2>/dev/null || true
    sysctl -n hw.model 2>/dev/null || true
    sysctl -n hw.logicalcpu 2>/dev/null || true
    pmset -g batt 2>/dev/null || true
    pmset -g therm 2>/dev/null || true
    date -u
    uptime
    sysctl -n vm.loadavg 2>/dev/null || true
    echo "dirty_scope_begin"
    git -C "$repo_dir" status --short
    echo "dirty_scope_end"
} > "$evidence_dir/host-before.txt" 2>&1

run_cell() {
    vm_bin=$1
    vm_label=$2
    variant=$3
    workload=$4
    repetitions=$5
    round=$6
    output_file=$7
    (cd "$prepared_dir" && "$vm_bin" benchmark_unicode_prepared_compare \
        "$product_build_dir/bin/library" "$product_build_dir/bin/rxfnsl" \
        unicode_prepared_compare_linked -a "$variant" "$vm_label" \
        "$nfc_input" "$unicode_data_input" "$normalization_properties_input" \
        "$normalization_test_input" "$warmups" "$samples" \
        "$workload" "$repetitions") > "$output_file"
    summary=$(grep -F "phase=summary" "$output_file")
    printf '%s round=%s\n' "$summary" "$round"
}

results_file="$evidence_dir/results.txt"
: > "$results_file"
for vm_label in rxtvm rxbvm; do
    if [ "$vm_label" = rxtvm ]; then vm_bin=$rxtvm_bin; else vm_bin=$rxbvm_bin; fi
    for workload in $workloads; do
        if [ "$workload" = rows ]; then repetitions_list=0; else repetitions_list=$buffer_repetitions; fi
        for repetitions in $repetitions_list; do
            round=1
            while [ "$round" -le "$rounds" ]; do
                cell_variants=$variants
                if [ $((round % 2)) -eq 0 ] && [ "$variants" = "original-nfd generated-nfd" ]; then
                    cell_variants="generated-nfd original-nfd"
                fi
                for variant in $cell_variants; do
                    cell_file="$evidence_dir/$vm_label-$workload-$repetitions-round-$round-$variant.txt"
                    run_cell "$vm_bin" "$vm_label" "$variant" "$workload" \
                        "$repetitions" "$round" "$cell_file" >> "$results_file"
                done
                round=$((round + 1))
            done
        done
    done
done

{
    date -u
    uptime
    sysctl -n vm.loadavg 2>/dev/null || true
    pmset -g batt 2>/dev/null || true
    pmset -g therm 2>/dev/null || true
} > "$evidence_dir/host-after.txt" 2>&1

cat "$results_file"
echo "Evidence: $evidence_dir"
