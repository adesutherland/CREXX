#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
bootstrap_dir="$repo_dir/experiments/unicode/level-l-bootstrap"
product_build_dir=${CREXX_BUILD_DIR:-"$repo_dir/cmake-build-unicode-release"}
work_root=${CREXX_UNICODE_COMPARE_BUILD_DIR:-"$product_build_dir/unicode-prepared-compare"}
mkdir -p "$work_root"
work_dir=$(mktemp -d "$work_root/run.XXXXXX")
evidence_dir=${CREXX_UNICODE_COMPARE_EVIDENCE_DIR:-"$work_dir/evidence"}
warmups=${CREXX_UNICODE_COMPARE_WARMUPS:-1}
samples=${CREXX_UNICODE_COMPARE_SAMPLES:-3}
buffer_repetitions=${CREXX_UNICODE_COMPARE_BUFFER_REPETITIONS:-"1"}
variants=${CREXX_UNICODE_COMPARE_VARIANTS:-"original-nfd generated-nfd"}
skip_product_build=${CREXX_UNICODE_COMPARE_SKIP_PRODUCT_BUILD:-0}
build_jobs=${CREXX_BUILD_JOBS:-10}
nfc_input="$repo_dir/experiments/unicode/inputs/icu-78.3/nfc.txt"
ucd_dir="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd"
unicode_data_input="$ucd_dir/UnicodeData.txt"
normalization_properties_input="$ucd_dir/DerivedNormalizationProps.txt"
normalization_test_input="$ucd_dir/NormalizationTest.txt"
re2c_bin="$product_build_dir/re2c/re2c"
adapter_dir="$repo_dir/experiments/unicode/level-l-emitter"

mkdir -p "$work_dir" "$evidence_dir"
build_log="$evidence_dir/product-build.txt"
if [ ! -f "$product_build_dir/CMakeCache.txt" ]; then
    if ! cmake -S "$repo_dir" -B "$product_build_dir" -DCMAKE_BUILD_TYPE=Release \
            > "$build_log" 2>&1; then
        tail -n 120 "$build_log" >&2
        exit 1
    fi
fi
if [ "$skip_product_build" -eq 0 ]; then
    if ! cmake --build "$product_build_dir" --target rxfnsl rxtvm rxbvm \
            --parallel "$build_jobs" >> "$build_log" 2>&1; then
        tail -n 120 "$build_log" >&2
        exit 1
    fi
fi

rxc_bin="$product_build_dir/bin/rxc"
rxas_bin="$product_build_dir/bin/rxas"
rxlink_bin="$product_build_dir/bin/rxlink"
rxtvm_bin="$product_build_dir/bin/rxtvm"
rxbvm_bin="$product_build_dir/bin/rxbvm"

if [ "$("$re2c_bin" --vernum)" != "040501" ]; then
    echo "expected vendored re2c 4.5.1 (040501)" >&2
    exit 1
fi

compile_source() {
    source_file=$1
    output_base=$2
    log_file=$3
    if ! "$rxc_bin" -x -i "$product_build_dir/bin" -i "$work_dir" \
            -o "$output_base" "$source_file" > "$log_file" 2>&1 || \
       ! "$rxas_bin" "$output_base" >> "$log_file" 2>&1; then
        echo "Unicode prepared comparison build failed; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
    if [ -s "$log_file" ]; then
        echo "Unicode prepared comparison build emitted diagnostics:" >&2
        cat "$log_file" >&2
        exit 1
    fi
}

compile_source "$bootstrap_dir/generated/gennorm2.crexx" \
    "$work_dir/gennorm2" "$evidence_dir/gennorm2-build.txt"
compile_source "$bootstrap_dir/unicode_gennorm2.crexx" \
    "$work_dir/unicode_gennorm2" "$evidence_dir/unicode-gennorm2-build.txt"
compile_source "$bootstrap_dir/unicode_nfd.crexx" \
    "$work_dir/unicode_nfd" "$evidence_dir/unicode-nfd-build.txt"
compile_source "$bootstrap_dir/unicode_data.crexx" \
    "$work_dir/unicode_data" "$evidence_dir/unicode-data-build.txt"
compile_source "$bootstrap_dir/unicode_normprops.crexx" \
    "$work_dir/unicode_normprops" "$evidence_dir/unicode-normprops-build.txt"
compile_source "$bootstrap_dir/unicode_d.crexx" \
    "$work_dir/unicode_d" "$evidence_dir/unicode-d-build.txt"
compile_source "$bootstrap_dir/unicode_nfd_lexer_generate.crexx" \
    "$work_dir/unicode_nfd_lexer_generate" "$evidence_dir/unicode-nfd-lexer-generate-build.txt"
(cd "$work_dir" && "$rxlink_bin" -c "$bootstrap_dir/unicode_nfd_lexer_generate.ctl") \
    > "$evidence_dir/unicode-nfd-lexer-generate-link.txt" 2>&1

mkdir -p "$work_dir/rxtvm" "$work_dir/rxbvm"
generate_nfd_lexer() {
    vm_bin=$1
    vm_label=$2
    (cd "$work_dir" && "$vm_bin" unicode_nfd_lexer_generate \
        "$product_build_dir/bin/library" "$product_build_dir/bin/rxfnsl" \
        unicode_nfd_lexer_generate_linked -a \
        "$unicode_data_input" "$normalization_properties_input" \
        "$work_dir/$vm_label/level_l_unicode_nfd.re") \
        > "$evidence_dir/$vm_label-unicode-nfd-lexer-generate.txt"
}
generate_nfd_lexer "$rxtvm_bin" rxtvm
generate_nfd_lexer "$rxbvm_bin" rxbvm
cmp "$work_dir/rxtvm/level_l_unicode_nfd.re" "$work_dir/rxbvm/level_l_unicode_nfd.re"
cmp "$evidence_dir/rxtvm-unicode-nfd-lexer-generate.txt" \
    "$evidence_dir/rxbvm-unicode-nfd-lexer-generate.txt"
if ! (cd "$adapter_dir" && "$re2c_bin" --lang python \
        --syntax crexx_transducer.syntax --input-encoding utf8 \
        --no-debug-info --no-generation-date --no-version \
        -o "$work_dir/level_l_unicode_nfd.crexx" \
        "$work_dir/rxtvm/level_l_unicode_nfd.re") \
        > "$evidence_dir/re2c-level-l-unicode-nfd.txt" 2>&1; then
    cat "$evidence_dir/re2c-level-l-unicode-nfd.txt" >&2
    exit 1
fi
if [ -s "$evidence_dir/re2c-level-l-unicode-nfd.txt" ]; then
    echo "generated NFD re2c backend emitted diagnostics:" >&2
    cat "$evidence_dir/re2c-level-l-unicode-nfd.txt" >&2
    exit 1
fi
perl -pi -e 's/[ \t]+$//' "$work_dir/level_l_unicode_nfd.crexx"
compile_source "$work_dir/level_l_unicode_nfd.crexx" \
    "$work_dir/level_l_unicode_nfd" "$evidence_dir/level-l-unicode-nfd-build.txt"
compile_source "$script_dir/benchmark_unicode_prepared_compare.crexx" \
    "$work_dir/benchmark_unicode_prepared_compare" "$evidence_dir/benchmark-build.txt"
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/benchmark_unicode_prepared_compare.ctl") \
    > "$evidence_dir/link.txt" 2>&1

{
    echo "activity=UNICODE-NORM-02"
    echo "scope=rough algorithm screen; not a formal performance verdict"
    echo "branch=$(git -C "$repo_dir" branch --show-current)"
    echo "commit=$(git -C "$repo_dir" rev-parse HEAD)"
    echo "warmups=$warmups"
    echo "samples=$samples"
    echo "buffer_repetitions=$buffer_repetitions"
    echo "variants=$variants"
    echo "work_dir=$work_dir"
    echo "skip_product_build=$skip_product_build"
    echo "timed_lifecycle=preloaded rows or prebuilt UTF-8 buffer; normalize through same string surface; materialize and observe result"
    echo "generated_route=prepared Unicode IR to re2c UTF-8 DFA to Level B; generated outside timed region"
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
    output_file=$6
    (cd "$work_dir" && "$vm_bin" benchmark_unicode_prepared_compare \
        "$product_build_dir/bin/library" "$product_build_dir/bin/rxfnsl" \
        unicode_prepared_compare_linked -a "$variant" "$vm_label" \
        "$nfc_input" "$unicode_data_input" "$normalization_properties_input" \
        "$normalization_test_input" "$warmups" "$samples" \
        "$workload" "$repetitions") > "$output_file"
}

results_file="$evidence_dir/results.txt"
: > "$results_file"
for vm_label in rxtvm rxbvm; do
    if [ "$vm_label" = rxtvm ]; then vm_bin=$rxtvm_bin; else vm_bin=$rxbvm_bin; fi
    for workload in rows buffer; do
        if [ "$workload" = rows ]; then repetitions_list=0; else repetitions_list=$buffer_repetitions; fi
        for repetitions in $repetitions_list; do
            for variant in $variants; do
                cell_file="$evidence_dir/$vm_label-$workload-$repetitions-$variant.txt"
                run_cell "$vm_bin" "$vm_label" "$variant" "$workload" "$repetitions" "$cell_file"
                grep -F "phase=summary" "$cell_file" >> "$results_file"
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
