#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
product_build_dir=${CREXX_BUILD_DIR:-"$repo_dir/cmake-build-unicode-debug"}
work_dir=${CREXX_LEVEL_L_EMITTER_BUILD_DIR:-"$product_build_dir/level-l-emitter"}
build_jobs=${CREXX_BUILD_JOBS:-10}

mkdir -p "$work_dir"

build_log="$work_dir/product-build.log"
if [ ! -f "$product_build_dir/CMakeCache.txt" ]; then
    if ! cmake -S "$repo_dir" -B "$product_build_dir" \
        -DCMAKE_BUILD_TYPE=Debug > "$build_log" 2>&1; then
        echo "CREXX configuration failed; tail of $build_log:" >&2
        tail -n 120 "$build_log" >&2
        exit 1
    fi
fi

# Build umbrella targets in dependency order. Asking Make for all of these as
# peer targets can race generated compiler-exit artifacts on a clean tree.
if ! cmake --build "$product_build_dir" --target rxfnsl \
        --parallel "$build_jobs" >> "$build_log" 2>&1 || \
   ! cmake --build "$product_build_dir" --target rxtvm \
        --parallel "$build_jobs" >> "$build_log" 2>&1 || \
   ! cmake --build "$product_build_dir" --target rxbvm \
        --parallel "$build_jobs" >> "$build_log" 2>&1; then
    echo "CREXX build failed; tail of $build_log:" >&2
    tail -n 120 "$build_log" >&2
    exit 1
fi

re2c_bin=${RE2C_BIN:-"$product_build_dir/re2c/re2c"}
rxc_bin="$product_build_dir/bin/rxc"
rxas_bin="$product_build_dir/bin/rxas"
rxlink_bin="$product_build_dir/bin/rxlink"
rxtvm_bin="$product_build_dir/bin/rxtvm"
rxbvm_bin="$product_build_dir/bin/rxbvm"

re2c_version=$("$re2c_bin" --vernum)
if [ "$re2c_version" != "040501" ]; then
    echo "expected vendored re2c 4.5.1 (040501), found $re2c_version" >&2
    exit 1
fi

generated_actual="$work_dir/tinyexpr.crexx"
(cd "$script_dir" && \
    "$re2c_bin" --lang python --syntax crexx.syntax \
        --no-debug-info --no-generation-date --no-version \
        -o "$generated_actual" tinyexpr.re)
cmp "$script_dir/generated/tinyexpr.crexx" "$generated_actual"

module_log="$work_dir/module-build.log"
if ! "$rxc_bin" -x -i "$product_build_dir/bin" \
        -o "$work_dir/level_l_re2c" "$generated_actual" \
        > "$module_log" 2>&1 || \
   ! "$rxas_bin" "$work_dir/level_l_re2c" >> "$module_log" 2>&1; then
    echo "generated Level L module build failed; contents of $module_log:" >&2
    cat "$module_log" >&2
    exit 1
fi
if [ -s "$module_log" ]; then
    echo "generated Level L module build emitted unexpected diagnostics:" >&2
    cat "$module_log" >&2
    exit 1
fi

(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/level_l_re2c.ctl")
test -f "$work_dir/level_l_re2c_linked.rxbin"

test_log="$work_dir/test-build.log"
if ! "$rxc_bin" -x -i "$product_build_dir/bin" -i "$work_dir" \
        -o "$work_dir/test_tinyexpr" "$script_dir/test_tinyexpr.crexx" \
        > "$test_log" 2>&1 || \
   ! "$rxas_bin" "$work_dir/test_tinyexpr" >> "$test_log" 2>&1; then
    echo "differential test build failed; contents of $test_log:" >&2
    cat "$test_log" >&2
    exit 1
fi
if [ -s "$test_log" ]; then
    echo "differential test build emitted unexpected diagnostics:" >&2
    cat "$test_log" >&2
    exit 1
fi

grep -F ".jtable " "$work_dir/level_l_re2c.rxas" > /dev/null
grep -F "jumpi " "$work_dir/level_l_re2c.rxas" > /dev/null
grep -F "bgetu8 " "$work_dir/level_l_re2c.rxas" > /dev/null
grep -F "bsetu16 " "$work_dir/level_l_re2c.rxas" > /dev/null
grep -F "bsetu32 " "$work_dir/level_l_re2c.rxas" > /dev/null
grep -F "bseti64 " "$work_dir/level_l_re2c.rxas" > /dev/null

(cd "$work_dir" && "$rxtvm_bin" test_tinyexpr \
    "$product_build_dir/bin/library" \
    "$product_build_dir/bin/rxfnsl" \
    "$work_dir/level_l_re2c_linked") > "$work_dir/rxtvm.txt"
(cd "$work_dir" && "$rxbvm_bin" test_tinyexpr \
    "$product_build_dir/bin/library" \
    "$product_build_dir/bin/rxfnsl" \
    "$work_dir/level_l_re2c_linked") > "$work_dir/rxbvm.txt"

grep -Fx "PASS: Level L re2c emitter matches TinyExpr" \
    "$work_dir/rxtvm.txt" > /dev/null
grep -Fx "PASS: Level L re2c emitter matches TinyExpr" \
    "$work_dir/rxbvm.txt" > /dev/null
cmp "$work_dir/rxtvm.txt" "$work_dir/rxbvm.txt"

summary_actual="$work_dir/result.txt"
{
    echo "generated source: byte-identical"
    echo "state dispatch: .jtable and jumpi"
    echo "binary scan: bgetu8"
    echo "token stores: bsetu16, bsetu32, bseti64"
    echo "linked image: level_l_re2c_linked.rxbin"
    echo "rxtvm: PASS"
    echo "rxbvm: PASS"
} > "$summary_actual"

cmp "$script_dir/evidence/result.txt" "$summary_actual"
cat "$summary_actual"
