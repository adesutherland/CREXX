#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
bootstrap_dir="$repo_dir/experiments/unicode/level-l-bootstrap"
adapter_dir="$repo_dir/experiments/unicode/level-l-emitter"
product_build_dir=${CREXX_BUILD_DIR:-"$repo_dir/cmake-build-unicode-release"}
work_root=${CREXX_UNICODE_COMPARE_BUILD_DIR:-"$product_build_dir/unicode-prepared-compare"}
build_jobs=${CREXX_BUILD_JOBS:-10}
ucd_dir="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd"
unicode_data_input="$ucd_dir/UnicodeData.txt"
normalization_properties_input="$ucd_dir/DerivedNormalizationProps.txt"
normalization_test_input="$ucd_dir/NormalizationTest.txt"
nfc_input="$repo_dir/experiments/unicode/inputs/icu-78.3/nfc.txt"

mkdir -p "$work_root"
work_dir=$(mktemp -d "$work_root/prepared.XXXXXX")
evidence_dir="$work_dir/preparation"
mkdir -p "$work_dir/rxtvm" "$work_dir/rxbvm" "$evidence_dir"

build_log="$evidence_dir/product-build.txt"
if [ ! -f "$product_build_dir/CMakeCache.txt" ]; then
    if ! cmake -S "$repo_dir" -B "$product_build_dir" -DCMAKE_BUILD_TYPE=Release \
            > "$build_log" 2>&1; then
        tail -n 120 "$build_log" >&2
        exit 1
    fi
fi
if ! cmake --build "$product_build_dir" --target rxfnsl rxtvm rxbvm \
        --parallel "$build_jobs" >> "$build_log" 2>&1; then
    tail -n 120 "$build_log" >&2
    exit 1
fi

rxc_bin="$product_build_dir/bin/rxc"
rxas_bin="$product_build_dir/bin/rxas"
rxlink_bin="$product_build_dir/bin/rxlink"
rxtvm_bin="$product_build_dir/bin/rxtvm"
rxbvm_bin="$product_build_dir/bin/rxbvm"
re2c_bin="$product_build_dir/re2c/re2c"

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

fingerprint() {
    cmake -E sha256sum "$@" | shasum -a 256 | awk '{print $1}'
}

source_fingerprint=$(fingerprint \
    "$script_dir/prepare_prepared_compare.sh" \
    "$script_dir/benchmark_unicode_prepared_compare.crexx" \
    "$script_dir/benchmark_unicode_prepared_compare.ctl" \
    "$bootstrap_dir/generated/gennorm2.crexx" \
    "$bootstrap_dir/unicode_gennorm2.crexx" \
    "$bootstrap_dir/unicode_nfd.crexx" \
    "$bootstrap_dir/unicode_data.crexx" \
    "$bootstrap_dir/unicode_normprops.crexx" \
    "$bootstrap_dir/unicode_d.crexx" \
    "$bootstrap_dir/unicode_nfd_lexer_generate.crexx" \
    "$bootstrap_dir/unicode_nfd_lexer_generate.ctl" \
    "$adapter_dir/crexx_transducer.syntax" \
    "$nfc_input" "$unicode_data_input" "$normalization_properties_input" \
    "$normalization_test_input")
product_fingerprint=$(fingerprint \
    "$rxc_bin" "$rxas_bin" "$rxlink_bin" "$rxtvm_bin" "$rxbvm_bin" \
    "$re2c_bin" "$product_build_dir/bin/library.rxbin" \
    "$product_build_dir/bin/rxfnsl.rxbin")
prepared_fingerprint=$(fingerprint \
    "$work_dir/benchmark_unicode_prepared_compare.rxbin" \
    "$work_dir/unicode_prepared_compare_linked.rxbin")

manifest="$work_dir/prepared.manifest"
{
    echo "format=1"
    echo "branch=$(git -C "$repo_dir" branch --show-current)"
    echo "commit=$(git -C "$repo_dir" rev-parse HEAD)"
    echo "source_fingerprint=$source_fingerprint"
    echo "product_fingerprint=$product_fingerprint"
    echo "prepared_fingerprint=$prepared_fingerprint"
    echo "product_build_dir=$product_build_dir"
    echo "prepared_dir=$work_dir"
} > "$manifest"

printf '%s\n' "$work_dir" > "$work_root/latest-prepared.txt"
echo "Prepared Unicode comparison: $work_dir"
echo "Run: CREXX_UNICODE_COMPARE_PREPARED_DIR=$work_dir $script_dir/run_prepared_compare.sh"
