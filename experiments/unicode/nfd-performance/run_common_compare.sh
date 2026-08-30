#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
default_build_dir="$repo_dir/cmake-build-unicode-release"
work_root=${CREXX_UNICODE_COMMON_COMPARE_BUILD_DIR:-"$default_build_dir/unicode-common-compare"}
prepared_dir=${CREXX_UNICODE_COMMON_COMPARE_PREPARED_DIR:-}
if [ -z "$prepared_dir" ]; then
    latest_file="$work_root/latest-prepared.txt"
    if [ ! -f "$latest_file" ]; then
        echo "no prepared comparison; run $script_dir/prepare_common_compare.sh first" >&2
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
linked_image="$prepared_dir/unicode_common_compare_linked.rxbin"
benchmark_module="$prepared_dir/benchmark_unicode_common_compare.rxbin"
apple_plugin="$prepared_dir/rxunicode_common_apple.rxplugin"
for required in "$rxtvm_bin" "$rxbvm_bin" "$linked_image" \
        "$benchmark_module" "$apple_plugin"; do
    if [ ! -f "$required" ]; then
        echo "prepared comparison artifact not found: $required" >&2
        exit 1
    fi
done

fingerprint() {
    cmake -E sha256sum "$@" | shasum -a 256 | awk '{print $1}'
}
expected_prepared_fingerprint=$(manifest_value prepared_fingerprint)
actual_prepared_fingerprint=$(fingerprint \
    "$benchmark_module" "$linked_image" "$apple_plugin")
if [ "$actual_prepared_fingerprint" != "$expected_prepared_fingerprint" ]; then
    echo "prepared comparison artifacts no longer match their manifest" >&2
    exit 1
fi

if [ "$(uname -s)" != "Darwin" ]; then
    echo "the Apple/CoreFoundation control requires macOS" >&2
    exit 1
fi
if [ "${CREXX_UNICODE_COMMON_COMPARE_REQUIRE_AC:-1}" = "1" ] && \
        ! pmset -g batt | grep -F "AC Power" > /dev/null; then
    echo "performance comparison requires AC power" >&2
    exit 1
fi

warmups=${CREXX_UNICODE_COMMON_COMPARE_WARMUPS:-2}
samples=${CREXX_UNICODE_COMMON_COMPARE_SAMPLES:-10}
normalization_buffer_repetitions=${CREXX_UNICODE_COMMON_COMPARE_NORMALIZATION_BUFFER_REPETITIONS:-8}
casefold_buffer_repetitions=${CREXX_UNICODE_COMMON_COMPARE_CASEFOLD_BUFFER_REPETITIONS:-128}
host_note=${CREXX_UNICODE_COMMON_COMPARE_HOST_NOTE:-"dedicated AC-powered host"}
operations=${CREXX_UNICODE_COMMON_COMPARE_OPERATIONS:-"nfd nfkd nfc nfkc is-nfd is-nfkd is-nfc is-nfkc simple full turkic-simple turkic-full"}
variant_mode=${CREXX_UNICODE_COMMON_COMPARE_VARIANT_MODE:-common}
case "$warmups:$samples:$normalization_buffer_repetitions:$casefold_buffer_repetitions" in
    *[!0-9:]*|'')
        echo "warmup, sample, and repetition counts must be integers" >&2
        exit 1
        ;;
esac
if [ "$samples" -lt 1 ] || [ "$normalization_buffer_repetitions" -lt 1 ] || \
        [ "$casefold_buffer_repetitions" -lt 1 ]; then
    echo "sample and repetition counts must be positive" >&2
    exit 1
fi
for operation in $operations; do
    case $operation in
        nfd|nfkd|nfc|nfkc|is-nfd|is-nfkd|is-nfc|is-nfkc|simple|full|turkic-simple|turkic-full) ;;
        *)
            echo "unknown operation in CREXX_UNICODE_COMMON_COMPARE_OPERATIONS: $operation" >&2
            exit 1
            ;;
    esac
    if [ "$variant_mode" = "certificates" ]; then
        case $operation in
            nfd|nfkd|nfc|nfkc|is-nfd|is-nfkd|is-nfc|is-nfkc) ;;
            *)
                echo "certificate variant mode accepts normalization operations only: $operation" >&2
                exit 1
                ;;
        esac
    fi
done
case $variant_mode in
    common|certificates) ;;
    *)
        echo "unknown CREXX_UNICODE_COMMON_COMPARE_VARIANT_MODE: $variant_mode" >&2
        exit 1
        ;;
esac

ucd_dir="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd"
unicode_data_input="$ucd_dir/UnicodeData.txt"
normalization_properties_input="$ucd_dir/DerivedNormalizationProps.txt"
normalization_test_input="$ucd_dir/NormalizationTest.txt"
casefold_input="$ucd_dir/CaseFolding.txt"
runs_root="$prepared_dir/runs"
mkdir -p "$runs_root"
run_dir=$(mktemp -d "$runs_root/run.XXXXXX")
evidence_dir=${CREXX_UNICODE_COMMON_COMPARE_EVIDENCE_DIR:-"$run_dir/evidence"}
mkdir -p "$evidence_dir"

{
    echo "activity=UNICODE-COMMON-COMPARE"
    echo "scope=experimental Release comparison; not a production performance verdict"
    echo "branch=$(manifest_value branch)"
    echo "commit=$(manifest_value commit)"
    echo "source_fingerprint=$(manifest_value source_fingerprint)"
    echo "product_fingerprint=$(manifest_value product_fingerprint)"
    echo "prepared_fingerprint=$actual_prepared_fingerprint"
    echo "prepared_dir=$prepared_dir"
    echo "run_dir=$run_dir"
    echo "host_note=$host_note"
    echo "warmups=$warmups"
    echo "samples=$samples"
    echo "normalization_buffer_repetitions=$normalization_buffer_repetitions"
    echo "casefold_buffer_repetitions=$casefold_buffer_repetitions"
    echo "serial=true"
    echo "operations=$operations"
    echo "variant_mode=$variant_mode"
    echo "canonical=per-instance prepared unicode_d or unicode_casefold object"
    echo "shared=single rxc-emitted binary constant used by unicode_normalization"
    echo "apple_normalization=CoreFoundation CFStringNormalize"
    echo "apple_predicate=CoreFoundation CFStringNormalize then CFEqual control"
    echo "predicate_buffer=known-normalized true path; complete input scan required"
    echo "apple_full_fold=CoreFoundation CFStringFold with case-insensitive option"
    echo "apple_simple_fold=not available through a public exact CoreFoundation API"
    echo "timed_lifecycle=preloaded row array or prebuilt buffer; normalize cells materialize output; predicate cells scan/compare and observe a weighted truth checksum"
    uname -a
    sw_vers
    sysctl -n hw.model
    sysctl -n hw.logicalcpu
    pmset -g batt
    pmset -g therm || true
    date -u
    uptime
    sysctl -n vm.loadavg
    echo "competing_processes_begin"
    ps -Ao pid,ppid,%cpu,%mem,comm | rg 'rxc|rxas|rxlink|rxtvm|rxbvm|cmake|ninja|ctest' || true
    echo "competing_processes_end"
    echo "dirty_scope_begin"
    git -C "$repo_dir" status --short
    echo "dirty_scope_end"
} > "$evidence_dir/host-before.txt" 2>&1

results_file="$evidence_dir/results.txt"
: > "$results_file"

run_cell() {
    vm_bin=$1
    vm_name=$2
    variant=$3
    operation=$4
    workload=$5
    repetitions=$6
    output_file="$evidence_dir/$vm_name-$variant-$operation-$workload-$repetitions.txt"
    rss_file="$evidence_dir/$vm_name-$variant-$operation-$workload-$repetitions-rss.txt"
    echo "running vm=$vm_name variant=$variant operation=$operation workload=$workload repetitions=$repetitions"
    if ! (cd "$prepared_dir" && /usr/bin/time -l \
            "$vm_bin" benchmark_unicode_common_compare \
            "$product_build_dir/bin/library" \
            "$product_build_dir/bin/rxfnsl" \
            rxunicode_common_apple unicode_common_compare_linked -a \
            "$variant" "$operation" "$vm_name" \
            "$unicode_data_input" "$normalization_properties_input" \
            "$normalization_test_input" "$casefold_input" \
            "$warmups" "$samples" "$workload" "$repetitions") \
            > "$output_file" 2> "$rss_file"; then
        echo "benchmark cell failed: $vm_name $variant $operation $workload $repetitions" >&2
        tail -n 120 "$output_file" >&2
        tail -n 120 "$rss_file" >&2
        exit 1
    fi
    summary=$(grep -F "phase=summary variant=$variant operation=$operation vm=$vm_name workload=$workload" "$output_file")
    max_rss=$(awk '/maximum resident set size/ {print $1; exit}' "$rss_file")
    printf '%s max_rss_bytes=%s\n' "$summary" "${max_rss:-unknown}" >> "$results_file"
}

apple_supported() {
    case $1 in
        simple|turkic-simple) return 1 ;;
        *) return 0 ;;
    esac
}

operation_index=0
for workload in rows buffer; do
    for operation in $operations; do
        operation_index=$((operation_index + 1))
        case $operation in
            nfd|nfkd|nfc|nfkc|is-nfd|is-nfkd|is-nfc|is-nfkc) repetitions=$normalization_buffer_repetitions ;;
            *) repetitions=$casefold_buffer_repetitions ;;
        esac
        if [ "$workload" = rows ]; then repetitions=0; fi
        for vm_name in rxtvm rxbvm; do
            if [ "$vm_name" = rxtvm ]; then
                vm_bin=$rxtvm_bin
                vm_offset=0
            else
                vm_bin=$rxbvm_bin
                vm_offset=1
            fi
            if [ "$variant_mode" = "certificates" ]; then
                if [ $(((operation_index + vm_offset) % 2)) -eq 0 ]; then
                    variants="shared-hit shared-cold"
                else
                    variants="shared-cold shared-hit"
                fi
            elif apple_supported "$operation"; then
                if [ $(((operation_index + vm_offset) % 2)) -eq 0 ]; then
                    variants="apple shared canonical"
                else
                    variants="canonical shared apple"
                fi
            elif [ $((operation_index % 2)) -eq 0 ]; then
                variants="shared canonical"
            else
                variants="canonical shared"
            fi
            for variant in $variants; do
                run_cell "$vm_bin" "$vm_name" "$variant" "$operation" \
                    "$workload" "$repetitions"
            done
        done
    done
done

{
    echo "benchmark=unicode_common_compare phase=availability variant=apple operation=simple status=not_available reason=no_exact_public_CoreFoundation_API"
    echo "benchmark=unicode_common_compare phase=availability variant=apple operation=turkic-simple status=not_available reason=no_exact_public_CoreFoundation_API"
} >> "$results_file"

{
    date -u
    uptime
    sysctl -n vm.loadavg
    pmset -g batt
    pmset -g therm || true
} > "$evidence_dir/host-after.txt" 2>&1

cat "$results_file"
echo "Evidence: $evidence_dir"
