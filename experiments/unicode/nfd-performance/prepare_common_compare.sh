#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
bootstrap_dir="$repo_dir/experiments/unicode/level-l-bootstrap"
product_build_dir=${CREXX_BUILD_DIR:-"$repo_dir/cmake-build-unicode-release"}
work_root=${CREXX_UNICODE_COMMON_COMPARE_BUILD_DIR:-"$product_build_dir/unicode-common-compare"}
build_jobs=${CREXX_BUILD_JOBS:-10}
cxx_bin=${CXX:-c++}
ucd_dir="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd"

if [ "$(uname -s)" != "Darwin" ]; then
    echo "the Apple/CoreFoundation control requires macOS" >&2
    exit 1
fi

mkdir -p "$work_root"
work_dir=${CREXX_UNICODE_COMMON_COMPARE_PREPARED_DIR:-}
if [ -z "$work_dir" ]; then
    work_dir=$(mktemp -d "$work_root/prepared.XXXXXX")
fi
evidence_dir="$work_dir/preparation"
mkdir -p "$evidence_dir"

regression_log="$evidence_dir/release-regression.txt"
if ! grep -Fx "rxtvm: PASS" "$regression_log" > /dev/null 2>&1 || \
   ! grep -Fx "rxbvm: PASS" "$regression_log" > /dev/null 2>&1; then
    if ! CREXX_BUILD_DIR="$product_build_dir" \
            CREXX_LEVEL_L_BOOTSTRAP_BUILD_DIR="$work_dir" \
            CREXX_BUILD_JOBS="$build_jobs" \
            "$bootstrap_dir/run.sh" > "$regression_log" 2>&1; then
        echo "Release Unicode regression failed; tail of $regression_log:" >&2
        tail -n 160 "$regression_log" >&2
        exit 1
    fi
fi
grep -Fx "optimized/noopt linked images: byte-identical conformance summaries" \
    "$regression_log" > /dev/null
grep -Fx "rxtvm: PASS" "$regression_log" > /dev/null
grep -Fx "rxbvm: PASS" "$regression_log" > /dev/null

rxc_bin="$product_build_dir/bin/rxc"
rxas_bin="$product_build_dir/bin/rxas"
rxlink_bin="$product_build_dir/bin/rxlink"
rxtvm_bin="$product_build_dir/bin/rxtvm"
rxbvm_bin="$product_build_dir/bin/rxbvm"

plugin_log="$evidence_dir/apple-plugin-build.txt"
if ! "$cxx_bin" -std=c++17 -O3 -DNDEBUG -Wall -Wextra -Werror \
        -DBUILD_DLL -DPLUGIN_ID=unicode_common_apple \
        -I "$repo_dir/rxpa" -iquote "$product_build_dir/generated" \
        -bundle "$script_dir/unicode_common_apple_plugin.cpp" \
        -framework CoreFoundation \
        -o "$work_dir/rxunicode_common_apple.rxplugin" \
        > "$plugin_log" 2>&1; then
    echo "Apple/CoreFoundation plugin build failed; contents of $plugin_log:" >&2
    cat "$plugin_log" >&2
    exit 1
fi
if [ -s "$plugin_log" ]; then
    echo "Apple/CoreFoundation plugin build emitted diagnostics:" >&2
    cat "$plugin_log" >&2
    exit 1
fi

benchmark_log="$evidence_dir/benchmark-build.txt"
if ! "$rxc_bin" -x -i "$product_build_dir/bin" -i "$work_dir" \
        -o "$work_dir/benchmark_unicode_common_compare" \
        "$script_dir/benchmark_unicode_common_compare.crexx" \
        > "$benchmark_log" 2>&1 || \
   ! "$rxas_bin" "$work_dir/benchmark_unicode_common_compare" \
        >> "$benchmark_log" 2>&1; then
    echo "Unicode common-table benchmark build failed; contents of $benchmark_log:" >&2
    cat "$benchmark_log" >&2
    exit 1
fi
if [ -s "$benchmark_log" ]; then
    echo "Unicode common-table benchmark build emitted diagnostics:" >&2
    cat "$benchmark_log" >&2
    exit 1
fi

link_log="$evidence_dir/benchmark-link.txt"
if ! (cd "$work_dir" && "$rxlink_bin" -c \
        "$script_dir/benchmark_unicode_common_compare.ctl") \
        > "$link_log" 2>&1; then
    echo "Unicode common-table benchmark link failed; contents of $link_log:" >&2
    cat "$link_log" >&2
    exit 1
fi
if [ -s "$link_log" ]; then
    echo "Unicode common-table benchmark link emitted diagnostics:" >&2
    cat "$link_log" >&2
    exit 1
fi

fingerprint() {
    cmake -E sha256sum "$@" | shasum -a 256 | awk '{print $1}'
}

source_fingerprint=$(fingerprint \
    "$script_dir/prepare_common_compare.sh" \
    "$script_dir/run_common_compare.sh" \
    "$script_dir/benchmark_unicode_common_compare.crexx" \
    "$script_dir/benchmark_unicode_common_compare.ctl" \
    "$script_dir/unicode_common_apple_plugin.cpp" \
    "$bootstrap_dir/run.sh" \
    "$bootstrap_dir/unicode_data.crexx" \
    "$bootstrap_dir/unicode_normprops.crexx" \
    "$bootstrap_dir/unicode_d.crexx" \
    "$bootstrap_dir/unicode_casefold.crexx" \
    "$bootstrap_dir/unicode_normalization_generate.crexx" \
    "$ucd_dir/UnicodeData.txt" \
    "$ucd_dir/DerivedNormalizationProps.txt" \
    "$ucd_dir/NormalizationTest.txt" \
    "$ucd_dir/CaseFolding.txt")
product_fingerprint=$(fingerprint \
    "$rxc_bin" "$rxas_bin" "$rxlink_bin" "$rxtvm_bin" "$rxbvm_bin" \
    "$product_build_dir/bin/library.rxbin" \
    "$product_build_dir/bin/rxfnsl.rxbin")
prepared_fingerprint=$(fingerprint \
    "$work_dir/benchmark_unicode_common_compare.rxbin" \
    "$work_dir/unicode_common_compare_linked.rxbin" \
    "$work_dir/rxunicode_common_apple.rxplugin")

manifest="$work_dir/prepared.manifest"
{
    echo "format=1"
    echo "activity=UNICODE-COMMON-COMPARE"
    echo "branch=$(git -C "$repo_dir" branch --show-current)"
    echo "commit=$(git -C "$repo_dir" rev-parse HEAD)"
    echo "source_fingerprint=$source_fingerprint"
    echo "product_fingerprint=$product_fingerprint"
    echo "prepared_fingerprint=$prepared_fingerprint"
    echo "product_build_dir=$product_build_dir"
    echo "prepared_dir=$work_dir"
    echo "unicode_version=17.0.0"
    echo "apple_simple_fold=not_available"
    echo "apple_full_fold=CoreFoundation_CFStringFold_case_insensitive"
    echo "apple_normalization_predicate=CFStringNormalize_then_CFEqual_control"
} > "$manifest"

printf '%s\n' "$work_dir" > "$work_root/latest-prepared.txt"
echo "Prepared Unicode common-table comparison: $work_dir"
echo "Regression evidence: $regression_log"
echo "Run: CREXX_UNICODE_COMMON_COMPARE_PREPARED_DIR=$work_dir $script_dir/run_common_compare.sh"
