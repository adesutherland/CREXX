#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
product_build_dir=${CREXX_BUILD_DIR:-"$repo_dir/cmake-build-unicode-debug"}
work_dir=${CREXX_LEVEL_L_BOOTSTRAP_BUILD_DIR:-"$product_build_dir/level-l-bootstrap"}
build_jobs=${CREXX_BUILD_JOBS:-10}
cxx_bin=${CXX:-c++}

mkdir -p "$work_dir/rxtvm" "$work_dir/rxbvm"

build_log="$work_dir/product-build.log"
if [ ! -f "$product_build_dir/CMakeCache.txt" ]; then
    if ! cmake -S "$repo_dir" -B "$product_build_dir" \
        -DCMAKE_BUILD_TYPE=Debug > "$build_log" 2>&1; then
        echo "CREXX configuration failed; tail of $build_log:" >&2
        tail -n 120 "$build_log" >&2
        exit 1
    fi
fi

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

rxc_bin="$product_build_dir/bin/rxc"
rxas_bin="$product_build_dir/bin/rxas"
rxlink_bin="$product_build_dir/bin/rxlink"
rxtvm_bin="$product_build_dir/bin/rxtvm"
rxbvm_bin="$product_build_dir/bin/rxbvm"
re2c_bin=${RE2C_BIN:-"$product_build_dir/re2c/re2c"}
adapter_dir="$repo_dir/experiments/unicode/level-l-emitter"
spec_dir="$script_dir/specs"
generated_dir="$script_dir/generated"
nfc_input="$repo_dir/experiments/unicode/inputs/icu-78.3/nfc.txt"
poc_dir="$repo_dir/experiments/unicode/poc"

if [ "$("$re2c_bin" --vernum)" != "040501" ]; then
    echo "expected vendored re2c 4.5.1 (040501)" >&2
    exit 1
fi

compile_source() {
    source_file=$1
    output_base=$2
    import_dir=$3
    log_file=$4
    if ! "$rxc_bin" -x -i "$product_build_dir/bin" -i "$import_dir" \
            -o "$output_base" "$source_file" > "$log_file" 2>&1 || \
       ! "$rxas_bin" "$output_base" >> "$log_file" 2>&1; then
        echo "Level L build failed; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
    if [ -s "$log_file" ]; then
        echo "Level L build emitted unexpected diagnostics; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
}

compile_source "$script_dir/levell_bootstrap.crexx" \
    "$work_dir/levell_bootstrap" "$product_build_dir/bin" \
    "$work_dir/levell-bootstrap-build.log"
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/levell_bootstrap.ctl")

compile_source "$script_dir/levell_generate.crexx" \
    "$work_dir/levell_generate" "$work_dir" \
    "$work_dir/levell-generate-build.log"
compile_source "$script_dir/test_frontend.crexx" \
    "$work_dir/test_frontend" "$work_dir" \
    "$work_dir/test-frontend-build.log"

generate_with_vm() {
    vm_bin=$1
    vm_name=$2
    spec_name=$3
    output_dir="$work_dir/$vm_name"
    (cd "$work_dir" && "$vm_bin" levell_generate \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        levell_bootstrap_linked \
        -a "$spec_dir/$spec_name.levell" \
        "$output_dir/$spec_name.re" \
        "$output_dir/$spec_name.tokens" \
        "$output_dir/$spec_name.ast") > "$output_dir/$spec_name-generate.txt"
}

for spec_name in tinyexpr gennorm2; do
    generate_with_vm "$rxtvm_bin" rxtvm "$spec_name"
    generate_with_vm "$rxbvm_bin" rxbvm "$spec_name"
    cmp "$work_dir/rxtvm/$spec_name.re" "$work_dir/rxbvm/$spec_name.re"
    cmp "$work_dir/rxtvm/$spec_name.tokens" "$work_dir/rxbvm/$spec_name.tokens"
    cmp "$work_dir/rxtvm/$spec_name.ast" "$work_dir/rxbvm/$spec_name.ast"
    cmp "$generated_dir/$spec_name.re" "$work_dir/rxtvm/$spec_name.re"
    cmp "$generated_dir/$spec_name.tokens" "$work_dir/rxtvm/$spec_name.tokens"
    cmp "$generated_dir/$spec_name.ast" "$work_dir/rxtvm/$spec_name.ast"

    (cd "$adapter_dir" && "$re2c_bin" --lang python --syntax crexx.syntax \
        --input-encoding utf8 --no-debug-info --no-generation-date --no-version \
        -o "$work_dir/$spec_name.crexx" "$work_dir/rxtvm/$spec_name.re")
    # Generic re2c templates indent some otherwise blank lines. Normalize only
    # trailing horizontal whitespace so generated Git artifacts stay clean.
    perl -pi -e 's/[ \t]+$//' "$work_dir/$spec_name.crexx"
    cmp "$generated_dir/$spec_name.crexx" "$work_dir/$spec_name.crexx"

    compile_source "$work_dir/$spec_name.crexx" \
        "$work_dir/$spec_name" "$product_build_dir/bin" \
        "$work_dir/$spec_name-build.log"
    (cd "$work_dir" && "$rxlink_bin" -c "$script_dir/$spec_name.ctl")

    grep -F ".jtable " "$work_dir/$spec_name.rxas" > /dev/null
    grep -F "jumpi " "$work_dir/$spec_name.rxas" > /dev/null
    grep -F "bgetu8 " "$work_dir/$spec_name.rxas" > /dev/null
    grep -F "bsetu16 " "$work_dir/$spec_name.rxas" > /dev/null
    grep -F "bsetu32 " "$work_dir/$spec_name.rxas" > /dev/null
done

compile_source "$script_dir/unicode_gennorm2.crexx" \
    "$work_dir/unicode_gennorm2" "$work_dir" \
    "$work_dir/unicode-gennorm2-build.log"
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/unicode_gennorm2.ctl")
compile_source "$script_dir/unicode_gennorm2_generate.crexx" \
    "$work_dir/unicode_gennorm2_generate" "$work_dir" \
    "$work_dir/unicode-gennorm2-generate-build.log"
compile_source "$script_dir/test_unicode_gennorm2.crexx" \
    "$work_dir/test_unicode_gennorm2" "$work_dir" \
    "$work_dir/test-unicode-gennorm2-build.log"

reference_cpp="$work_dir/nfd_reference.cpp"
reference_bin="$work_dir/nfd_reference"
(cd "$poc_dir" && "$re2c_bin" --no-debug-info --no-generation-date --no-version \
    -o "$reference_cpp" nfd_reference.re)
cmp "$poc_dir/generated/nfd_reference.cpp" "$reference_cpp"
"$cxx_bin" -std=c++17 -O2 -Wall -Wextra -Werror \
    "$reference_cpp" -o "$reference_bin"

compile_source "$script_dir/test_tinyexpr.crexx" \
    "$work_dir/test_tinyexpr" "$work_dir" \
    "$work_dir/test-tinyexpr-build.log"
compile_source "$script_dir/test_gennorm2.crexx" \
    "$work_dir/test_gennorm2" "$work_dir" \
    "$work_dir/test-gennorm2-build.log"

run_frontend_test() {
    vm_bin=$1
    output_file=$2
    (cd "$work_dir" && "$vm_bin" test_frontend \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        levell_bootstrap_linked \
        -a "$spec_dir/tinyexpr.levell" "$spec_dir/gennorm2.levell") \
        > "$output_file"
}

run_tinyexpr_test() {
    vm_bin=$1
    output_file=$2
    (cd "$work_dir" && "$vm_bin" test_tinyexpr \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        tinyexpr_linked) > "$output_file"
}

run_gennorm2_test() {
    vm_bin=$1
    output_file=$2
    (cd "$work_dir" && "$vm_bin" test_gennorm2 \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        gennorm2_linked -a "$nfc_input") > "$output_file"
}

generate_unicode_records() {
    vm_bin=$1
    vm_name=$2
    output_dir="$work_dir/$vm_name"
    (cd "$work_dir" && "$vm_bin" unicode_gennorm2_generate \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        unicode_gennorm2_linked -a "$nfc_input" \
        "$output_dir/gennorm2.records" \
        "$output_dir/gennorm2.semantic") \
        > "$output_dir/unicode-gennorm2-generate.txt"
}

run_unicode_parser_test() {
    vm_bin=$1
    output_file=$2
    (cd "$work_dir" && "$vm_bin" test_unicode_gennorm2 \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        unicode_gennorm2_linked) > "$output_file"
}

run_frontend_test "$rxtvm_bin" "$work_dir/rxtvm-frontend.txt"
run_frontend_test "$rxbvm_bin" "$work_dir/rxbvm-frontend.txt"
run_tinyexpr_test "$rxtvm_bin" "$work_dir/rxtvm-tinyexpr.txt"
run_tinyexpr_test "$rxbvm_bin" "$work_dir/rxbvm-tinyexpr.txt"
run_gennorm2_test "$rxtvm_bin" "$work_dir/rxtvm-gennorm2.txt"
run_gennorm2_test "$rxbvm_bin" "$work_dir/rxbvm-gennorm2.txt"
generate_unicode_records "$rxtvm_bin" rxtvm
generate_unicode_records "$rxbvm_bin" rxbvm
run_unicode_parser_test "$rxtvm_bin" "$work_dir/rxtvm-unicode-parser.txt"
run_unicode_parser_test "$rxbvm_bin" "$work_dir/rxbvm-unicode-parser.txt"

"$reference_bin" --dump-rules "$nfc_input" > "$work_dir/reference-gennorm2.semantic"

cmp "$work_dir/rxtvm-frontend.txt" "$work_dir/rxbvm-frontend.txt"
cmp "$work_dir/rxtvm-tinyexpr.txt" "$work_dir/rxbvm-tinyexpr.txt"
cmp "$work_dir/rxtvm-gennorm2.txt" "$work_dir/rxbvm-gennorm2.txt"
cmp "$work_dir/rxtvm/gennorm2.records" "$work_dir/rxbvm/gennorm2.records"
cmp "$work_dir/rxtvm/gennorm2.semantic" "$work_dir/rxbvm/gennorm2.semantic"
cmp "$generated_dir/gennorm2.records" "$work_dir/rxtvm/gennorm2.records"
cmp "$generated_dir/gennorm2.semantic" "$work_dir/rxtvm/gennorm2.semantic"
cmp "$work_dir/reference-gennorm2.semantic" "$work_dir/rxtvm/gennorm2.semantic"
cmp "$work_dir/rxtvm-unicode-parser.txt" "$work_dir/rxbvm-unicode-parser.txt"
grep -Fx "PASS: Level L bootstrap frontend" "$work_dir/rxtvm-frontend.txt" > /dev/null
grep -Fx "PASS: authored Level L TinyExpr matches rxfnsl" "$work_dir/rxtvm-tinyexpr.txt" > /dev/null
grep -Fx "PASS: authored Level L lexer accepts ICU gennorm2 nfc.txt" "$work_dir/rxtvm-gennorm2.txt" > /dev/null
grep -Fx "PASS: tokenized gennorm2 parsed into typed Unicode records" "$work_dir/rxtvm/unicode-gennorm2-generate.txt" > /dev/null
grep -Fx "PASS: deterministic typed Unicode rule parser and panic contract" "$work_dir/rxtvm-unicode-parser.txt" > /dev/null

summary_actual="$work_dir/result.txt"
{
    echo "Level B frontend: deterministic Token and AST dumps"
    echo "parser policy: single cursor, no backtracking, first-fault panic"
    echo "TinyExpr cREXX: byte-identical generation"
    echo "TinyExpr differential oracle: PASS"
    echo "ICU gennorm2 nfc.txt lexer: PASS (2500 lines)"
    echo "Unicode rule parser: PASS (2485 typed records with byte/token spans)"
    echo "C++/re2c semantic oracle: byte-identical (2485 records)"
    echo "generated dispatch: .jtable and jumpi"
    echo "binary scan/stores: bgetu8, bsetu16, bsetu32"
    echo "rxtvm: PASS"
    echo "rxbvm: PASS"
} > "$summary_actual"

cmp "$script_dir/evidence/result.txt" "$summary_actual"
cat "$summary_actual"
