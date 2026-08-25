#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
input_dir="$repo_dir/experiments/unicode/inputs"
build_dir=${CREXX_UNICODE_BUILD_DIR:-"$repo_dir/cmake-build-unicode-poc"}
poc_build_dir="$build_dir/unicode-poc"
re2c_build_dir="$build_dir/re2c-build"
re2c_bin=${RE2C_BIN:-"$re2c_build_dir/re2c"}
cxx_bin=${CXX:-c++}

if command -v shasum > /dev/null 2>&1; then
    (cd "$input_dir" && shasum -a 256 -c SHA256SUMS)
elif command -v sha256sum > /dev/null 2>&1; then
    (cd "$input_dir" && sha256sum -c SHA256SUMS)
else
    echo "no SHA-256 verifier found (need shasum or sha256sum)" >&2
    exit 1
fi

mkdir -p "$poc_build_dir"

if [ ! -x "$re2c_bin" ]; then
    re2c_build_log="$poc_build_dir/re2c-build.log"
    if ! cmake -S "$repo_dir/re2c" -B "$re2c_build_dir" \
        -DCMAKE_BUILD_TYPE=Release > "$re2c_build_log" 2>&1 || \
       ! cmake --build "$re2c_build_dir" --target re2c --parallel \
        >> "$re2c_build_log" 2>&1; then
        echo "vendored re2c build failed; tail of $re2c_build_log:" >&2
        tail -n 100 "$re2c_build_log" >&2
        exit 1
    fi
fi

re2c_version=$("$re2c_bin" --vernum)
if [ "$re2c_version" != "040501" ]; then
    echo "expected vendored re2c 4.5.1 (040501), found $re2c_version" >&2
    exit 1
fi

generated_actual="$poc_build_dir/nfd_reference.cpp"
binary="$poc_build_dir/nfd_reference"
result_actual="$poc_build_dir/nfd-conformance.txt"

(cd "$script_dir" && "$re2c_bin" --no-debug-info --no-generation-date --no-version \
    -o "$generated_actual" nfd_reference.re)

cmp "$script_dir/generated/nfd_reference.cpp" "$generated_actual"

"$cxx_bin" -std=c++17 -O2 -Wall -Wextra -Werror \
    "$generated_actual" -o "$binary"

if "$binary" "$script_dir/fixtures/gennorm2-invalid-version.txt" \
    "$input_dir/unicode-17.0.0/ucd/NormalizationTest.txt" \
    > "$poc_build_dir/invalid-version.stdout" \
    2> "$poc_build_dir/invalid-version.stderr"; then
    echo "invalid gennorm2 version was unexpectedly accepted" >&2
    exit 1
fi
grep -F ":1: expected Unicode version 17.0.0" \
    "$poc_build_dir/invalid-version.stderr" > /dev/null

if "$binary" "$script_dir/fixtures/gennorm2-malformed-rule.txt" \
    "$input_dir/unicode-17.0.0/ucd/NormalizationTest.txt" \
    > "$poc_build_dir/malformed-rule.stdout" \
    2> "$poc_build_dir/malformed-rule.stderr"; then
    echo "malformed gennorm2 rule was unexpectedly accepted" >&2
    exit 1
fi
grep -F ":2: unrecognized gennorm2 input" \
    "$poc_build_dir/malformed-rule.stderr" > /dev/null

"$binary" \
    "$input_dir/icu-78.3/nfc.txt" \
    "$input_dir/unicode-17.0.0/ucd/NormalizationTest.txt" \
    > "$result_actual"

cmp "$script_dir/evidence/nfd-conformance.txt" "$result_actual"
cat "$result_actual"
