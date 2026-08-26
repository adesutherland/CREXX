#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
bootstrap_dir="$repo_dir/experiments/unicode/level-l-bootstrap"
product_build_dir=${CREXX_BUILD_DIR:-"$repo_dir/cmake-build-unicode-release"}
work_dir=${CREXX_NFD_PERFORMANCE_BUILD_DIR:-"$product_build_dir/level-l-bootstrap"}
evidence_dir=${CREXX_NFD_PERFORMANCE_EVIDENCE_DIR:-"$product_build_dir/nfd-performance-evidence"}
warmups=${CREXX_NFD_PERFORMANCE_WARMUPS:-2}
samples=${CREXX_NFD_PERFORMANCE_SAMPLES:-10}
buffer_repetitions=${CREXX_NFD_PERFORMANCE_BUFFER_REPETITIONS:-"1 8 64"}
apple_buffer_repetitions=${CREXX_NFD_PERFORMANCE_APPLE_BUFFER_REPETITIONS:-"1 8"}
host_note=${CREXX_NFD_PERFORMANCE_HOST_NOTE:-"unspecified"}
build_jobs=${CREXX_BUILD_JOBS:-10}
cxx_bin=${CXX:-c++}
nfc_input="$repo_dir/experiments/unicode/inputs/icu-78.3/nfc.txt"
normalization_input="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd/NormalizationTest.txt"

if [ "$(uname -s)" != "Darwin" ]; then
    echo "UNICODE-NFD-01 Apple control requires macOS CoreFoundation" >&2
    exit 1
fi
if [ "$warmups" -lt 0 ] || [ "$samples" -lt 1 ]; then
    echo "invalid warmup/sample count" >&2
    exit 1
fi
for repetitions in $buffer_repetitions; do
    case $repetitions in
        ''|*[!0-9]*)
            echo "invalid large-buffer repetition count: $repetitions" >&2
            exit 1
            ;;
    esac
    if [ "$repetitions" -lt 1 ]; then
        echo "large-buffer repetition count must be positive" >&2
        exit 1
    fi
done
for repetitions in $apple_buffer_repetitions; do
    case $repetitions in
        ''|*[!0-9]*)
            echo "invalid Apple large-buffer repetition count: $repetitions" >&2
            exit 1
            ;;
    esac
    if [ "$repetitions" -lt 1 ]; then
        echo "Apple large-buffer repetition count must be positive" >&2
        exit 1
    fi
done

mkdir -p "$evidence_dir"

if ! CREXX_BUILD_DIR="$product_build_dir" \
        CREXX_LEVEL_L_BOOTSTRAP_BUILD_DIR="$work_dir" \
        CREXX_BUILD_JOBS="$build_jobs" \
        "$bootstrap_dir/run.sh" > "$evidence_dir/portable-conformance.txt" 2>&1; then
    echo "portable NFD correctness gate failed; tail follows:" >&2
    tail -n 120 "$evidence_dir/portable-conformance.txt" >&2
    exit 1
fi

rxc_bin="$product_build_dir/bin/rxc"
rxas_bin="$product_build_dir/bin/rxas"
rxlink_bin="$product_build_dir/bin/rxlink"
rxtvm_bin="$product_build_dir/bin/rxtvm"
rxbvm_bin="$product_build_dir/bin/rxbvm"

compile_source() {
    source_file=$1
    output_base=$2
    log_file=$3
    if ! "$rxc_bin" -x -i "$product_build_dir/bin" -i "$work_dir" \
            -o "$output_base" "$source_file" > "$log_file" 2>&1 || \
       ! "$rxas_bin" "$output_base" >> "$log_file" 2>&1; then
        echo "UNICODE-NFD-01 build failed; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
    if [ -s "$log_file" ]; then
        echo "UNICODE-NFD-01 build emitted unexpected diagnostics; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
}

compile_source "$script_dir/unicode_nfd_packed.crexx" \
    "$work_dir/unicode_nfd_packed" "$evidence_dir/unicode-nfd-packed-build.txt"
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/unicode_nfd_packed.ctl")
compile_source "$script_dir/test_unicode_nfd_packed.crexx" \
    "$work_dir/test_unicode_nfd_packed" "$evidence_dir/test-unicode-nfd-packed-build.txt"

run_packed_conformance() {
    vm_bin=$1
    output_file=$2
    (cd "$work_dir" && "$vm_bin" test_unicode_nfd_packed \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        unicode_nfd_packed_linked -a "$nfc_input" "$normalization_input") \
        > "$output_file"
}

run_packed_conformance "$rxtvm_bin" "$evidence_dir/rxtvm-packed-conformance.txt"
run_packed_conformance "$rxbvm_bin" "$evidence_dir/rxbvm-packed-conformance.txt"
cmp "$evidence_dir/rxtvm-packed-conformance.txt" "$evidence_dir/rxbvm-packed-conformance.txt"
grep -Fx "Result: PASS" "$evidence_dir/rxtvm-packed-conformance.txt" > /dev/null

if ! "$cxx_bin" -std=c++17 -O3 -DNDEBUG -Wall -Wextra -Werror \
        -DBUILD_DLL -DPLUGIN_ID=rxunicode_nfd_apple \
        -I "$repo_dir/rxpa" -iquote "$product_build_dir/generated" \
        -bundle "$script_dir/unicode_nfd_apple_plugin.cpp" \
        -framework CoreFoundation \
        -o "$work_dir/rxunicode_nfd_apple.rxplugin" \
        > "$evidence_dir/apple-rxpa-build.txt" 2>&1; then
    echo "Apple CoreFoundation RXPA control build failed; contents follow:" >&2
    cat "$evidence_dir/apple-rxpa-build.txt" >&2
    exit 1
fi

compile_source "$script_dir/benchmark_unicode_nfd.crexx" \
    "$work_dir/benchmark_unicode_nfd" "$evidence_dir/benchmark-unicode-nfd-build.txt"
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/benchmark_unicode_nfd.ctl")

if ! "$cxx_bin" -std=c++17 -O3 -DNDEBUG -Wall -Wextra -Werror \
        "$script_dir/apple_corefoundation_nfd.cpp" -framework CoreFoundation \
        -o "$work_dir/apple_corefoundation_nfd" \
        > "$evidence_dir/apple-build.txt" 2>&1; then
    echo "Apple CoreFoundation control build failed; contents follow:" >&2
    cat "$evidence_dir/apple-build.txt" >&2
    exit 1
fi

{
    echo "activity=UNICODE-NFD-01"
    echo "branch=$(git -C "$repo_dir" branch --show-current)"
    echo "commit=$(git -C "$repo_dir" rev-parse HEAD)"
    echo "warmups=$warmups"
    echo "samples=$samples"
    echo "buffer_repetitions=$buffer_repetitions"
    echo "apple_buffer_repetitions=$apple_buffer_repetitions"
    echo "host_note=$host_note"
    echo "serial=true"
    echo "timed_lifecycle=preloaded-corpus rows-or-prebuilt-large-buffer normalize-materialize-utf8-and-observe only"
    echo "crexx_output=optimized standard metadata retained"
    echo "apple_rxpa_output=RXPA UTF-8 copy-out, CoreFoundation normalize, RXPA UTF-8 return string"
    echo "apple_standalone_output=CFMutableString copy normalize materialize UTF-8 observe release"
    uname -a
    sw_vers
    sysctl -n hw.model
    sysctl -n hw.logicalcpu
    "$cxx_bin" --version
    "$product_build_dir/re2c/re2c" --version
    cmake --version
    pmset -g batt
    pmset -g therm || true
    echo "dirty_scope_begin"
    git -C "$repo_dir" status --short
    echo "dirty_scope_end"
    date -u
    uptime
    sysctl -n vm.loadavg
} > "$evidence_dir/host.txt" 2>&1

run_crexx_cell() {
    vm_bin=$1
    vm_name=$2
    variant=$3
    workload=$4
    repetitions=$5
    suffix=$workload
    if [ "$workload" = "buffer" ]; then suffix="buffer-$repetitions"; fi
    output_file="$evidence_dir/$vm_name-$variant-$suffix.txt"
    rss_file="$evidence_dir/$vm_name-$variant-$suffix-rss.txt"
    if ! (cd "$work_dir" && /usr/bin/time -l \
        "$vm_bin" benchmark_unicode_nfd \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        rxunicode_nfd_apple \
        unicode_nfd_performance_linked -a \
        "$variant" "$vm_name" "$nfc_input" "$normalization_input" \
        "$warmups" "$samples" "$workload" "$repetitions") > "$output_file" 2> "$rss_file"; then
        echo "CREXX benchmark cell failed: $vm_name $variant $workload $repetitions" >&2
        tail -n 100 "$output_file" >&2
        tail -n 100 "$rss_file" >&2
        exit 1
    fi
    grep -F "phase=summary variant=$variant vm=$vm_name" "$output_file" > /dev/null
    grep -F "performance cell" "$output_file" > /dev/null
}

# Keep the two layouts adjacent within each concrete VM while avoiding one
# universal first/last ordering across the complete serial session.
run_crexx_cell "$rxtvm_bin" rxtvm portable rows 0
run_crexx_cell "$rxtvm_bin" rxtvm packed rows 0
run_crexx_cell "$rxtvm_bin" rxtvm apple-rxpa rows 0
run_crexx_cell "$rxbvm_bin" rxbvm apple-rxpa rows 0
run_crexx_cell "$rxbvm_bin" rxbvm packed rows 0
run_crexx_cell "$rxbvm_bin" rxbvm portable rows 0

for repetitions in $buffer_repetitions; do
    run_crexx_cell "$rxtvm_bin" rxtvm portable buffer "$repetitions"
    run_crexx_cell "$rxtvm_bin" rxtvm packed buffer "$repetitions"
    run_crexx_cell "$rxbvm_bin" rxbvm packed buffer "$repetitions"
    run_crexx_cell "$rxbvm_bin" rxbvm portable buffer "$repetitions"
done

for repetitions in $apple_buffer_repetitions; do
    run_crexx_cell "$rxtvm_bin" rxtvm apple-rxpa buffer "$repetitions"
    run_crexx_cell "$rxbvm_bin" rxbvm apple-rxpa buffer "$repetitions"
done

if ! /usr/bin/time -l "$work_dir/apple_corefoundation_nfd" \
        "$normalization_input" "$warmups" "$samples" \
        > "$evidence_dir/apple-corefoundation.txt" \
        2> "$evidence_dir/apple-corefoundation-rss.txt"; then
    echo "Apple CoreFoundation benchmark cell failed" >&2
    tail -n 100 "$evidence_dir/apple-corefoundation.txt" >&2
    tail -n 100 "$evidence_dir/apple-corefoundation-rss.txt" >&2
    exit 1
fi
grep -F "phase=summary variant=corefoundation-utf8" \
    "$evidence_dir/apple-corefoundation.txt" > /dev/null
grep -F "PASS: Apple CoreFoundation" \
    "$evidence_dir/apple-corefoundation.txt" > /dev/null

{
    echo "activity=UNICODE-NFD-01"
    echo "phase=post"
    echo "host_note=$host_note"
    date -u
    uptime
    sysctl -n vm.loadavg
    pmset -g batt
    pmset -g therm || true
} > "$evidence_dir/host-post.txt" 2>&1

summary_file="$evidence_dir/result.txt"
{
    grep -F "phase=summary" "$evidence_dir/rxtvm-portable-rows.txt"
    grep -F "phase=summary" "$evidence_dir/rxtvm-packed-rows.txt"
    grep -F "phase=correctness" "$evidence_dir/rxtvm-apple-rxpa-rows.txt"
    grep -F "phase=summary" "$evidence_dir/rxtvm-apple-rxpa-rows.txt"
    grep -F "phase=summary" "$evidence_dir/rxbvm-portable-rows.txt"
    grep -F "phase=summary" "$evidence_dir/rxbvm-packed-rows.txt"
    grep -F "phase=correctness" "$evidence_dir/rxbvm-apple-rxpa-rows.txt"
    grep -F "phase=summary" "$evidence_dir/rxbvm-apple-rxpa-rows.txt"
    for repetitions in $buffer_repetitions; do
        for vm_name in rxtvm rxbvm; do
            for variant in portable packed; do
                output_file="$evidence_dir/$vm_name-$variant-buffer-$repetitions.txt"
                grep -F "phase=summary" "$output_file"
            done
        done
    done
    for repetitions in $apple_buffer_repetitions; do
        for vm_name in rxtvm rxbvm; do
            output_file="$evidence_dir/$vm_name-apple-rxpa-buffer-$repetitions.txt"
            grep -F "phase=correctness" "$output_file"
            grep -F "phase=summary" "$output_file"
        done
    done
    grep -F "phase=correctness" "$evidence_dir/apple-corefoundation.txt"
    grep -F "phase=summary" "$evidence_dir/apple-corefoundation.txt"
} > "$summary_file"

cat "$summary_file"
echo "Evidence: $evidence_dir"
