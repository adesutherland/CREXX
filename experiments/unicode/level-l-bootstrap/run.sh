#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_dir=$(CDPATH= cd -- "$script_dir/../../.." && pwd)
product_build_dir=${CREXX_BUILD_DIR:-"$repo_dir/cmake-build-unicode-debug"}
work_dir=${CREXX_LEVEL_L_BOOTSTRAP_BUILD_DIR:-"$product_build_dir/level-l-bootstrap"}
build_jobs=${CREXX_BUILD_JOBS:-10}
cxx_bin=${CXX:-c++}

mkdir -p "$work_dir/rxtvm" "$work_dir/rxbvm"
# rxc resolves imports from the output directory.  Reusing a noopt directory
# can therefore make an old copy of the module being rebuilt visible as an
# import and, at present, abort the compiler.  Give every validation run a
# clean, retained directory; CREXX_LEVEL_L_BOOTSTRAP_NOOPT_DIR remains useful
# for a caller that already provides a clean staging location.
if [ -n "${CREXX_LEVEL_L_BOOTSTRAP_NOOPT_DIR:-}" ]; then
    noopt_dir=$CREXX_LEVEL_L_BOOTSTRAP_NOOPT_DIR
    mkdir -p "$noopt_dir"
else
    noopt_dir=$(mktemp -d "$work_dir/noopt.XXXXXX")
fi

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
normalization_input="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd/NormalizationTest.txt"
unicode_data_input="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd/UnicodeData.txt"
normalization_properties_input="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd/DerivedNormalizationProps.txt"
casefold_input="$repo_dir/experiments/unicode/inputs/unicode-17.0.0/ucd/CaseFolding.txt"
poc_dir="$repo_dir/experiments/unicode/poc"

for unicode_signal_source in \
        "$script_dir/unicode_d.crexx" \
        "$script_dir/unicode_casefold.crexx" \
        "$script_dir/unicode_nfd_lexer_generate.crexx"; do
    if grep -E 'assembler[[:space:]]+signal|call[[:space:]]+raise' \
            "$unicode_signal_source" > /dev/null; then
        echo "Unicode source bypasses the Level B SIGNAL surface: $unicode_signal_source" >&2
        exit 1
    fi
done

if [ "$("$re2c_bin" --vernum)" != "040501" ]; then
    echo "expected vendored re2c 4.5.1 (040501)" >&2
    exit 1
fi

run_rxc() {
    use_exits=$1
    shift
    if [ "$use_exits" -eq 1 ]; then
        "$rxc_bin" "$@"
    else
        "$rxc_bin" -x "$@"
    fi
}

compile_source() {
    source_file=$1
    output_base=$2
    import_dir=$3
    log_file=$4
    use_exits=${5:-0}
    if ! run_rxc "$use_exits" -i "$product_build_dir/bin" -i "$import_dir" \
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

compile_source_noopt() {
    source_file=$1
    output_base=$2
    import_dir=$3
    log_file=$4
    use_exits=${5:-0}
    if ! run_rxc "$use_exits" -n -i "$product_build_dir/bin" -i "$import_dir" \
            -o "$output_base" "$source_file" > "$log_file" 2>&1 || \
       ! "$rxas_bin" -n -o "$output_base" "$output_base" >> "$log_file" 2>&1; then
        echo "Noopt Level L build failed; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
    if [ -s "$log_file" ]; then
        echo "Noopt Level L build emitted unexpected diagnostics; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
}

compile_source_to_rxas() {
    source_file=$1
    output_base=$2
    import_dir=$3
    log_file=$4
    use_exits=${5:-0}
    if ! run_rxc "$use_exits" -i "$product_build_dir/bin" -i "$import_dir" \
            -o "$output_base" "$source_file" > "$log_file" 2>&1; then
        echo "Level L RXAS generation failed; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
    if [ -s "$log_file" ]; then
        echo "Level L RXAS generation emitted unexpected diagnostics; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
}

compile_source_to_rxas_noopt() {
    source_file=$1
    output_base=$2
    import_dir=$3
    log_file=$4
    use_exits=${5:-0}
    if ! run_rxc "$use_exits" -n -i "$product_build_dir/bin" -i "$import_dir" \
            -o "$output_base" "$source_file" > "$log_file" 2>&1; then
        echo "Noopt Level L RXAS generation failed; contents of $log_file:" >&2
        cat "$log_file" >&2
        exit 1
    fi
    if [ -s "$log_file" ]; then
        echo "Noopt Level L RXAS generation emitted unexpected diagnostics; contents of $log_file:" >&2
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
compile_source "$script_dir/unicode_nfd.crexx" \
    "$work_dir/unicode_nfd" "$work_dir" \
    "$work_dir/unicode-nfd-build.log"
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/unicode_nfd.ctl")
compile_source "$script_dir/test_unicode_nfd.crexx" \
    "$work_dir/test_unicode_nfd" "$work_dir" \
    "$work_dir/test-unicode-nfd-build.log"
compile_source "$script_dir/unicode_data.crexx" \
    "$work_dir/unicode_data" "$work_dir" \
    "$work_dir/unicode-data-build.log"
compile_source "$script_dir/unicode_normprops.crexx" \
    "$work_dir/unicode_normprops" "$work_dir" \
    "$work_dir/unicode-normprops-build.log"
compile_source "$script_dir/unicode_d.crexx" \
    "$work_dir/unicode_d" "$work_dir" \
    "$work_dir/unicode-d-build.log" 1
compile_source "$script_dir/unicode_casefold.crexx" \
    "$work_dir/unicode_casefold" "$work_dir" \
    "$work_dir/unicode-casefold-build.log"
compile_source "$script_dir/unicode_normalization_generate.crexx" \
    "$work_dir/unicode_normalization_generate" "$work_dir" \
    "$work_dir/unicode-normalization-generate-build.log"
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/unicode_normalization_generate.ctl")

generate_normalization_runtime() {
    vm_bin=$1
    vm_name=$2
    output_dir="$work_dir/$vm_name"
    (cd "$work_dir" && "$vm_bin" unicode_normalization_generate \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        unicode_normalization_generate_linked \
        -a "$unicode_data_input" "$normalization_properties_input" \
        "$casefold_input" "$script_dir/unicode_d.crexx" \
        "$script_dir/unicode_casefold.crexx" \
        "$output_dir/unicode_normalization.crexx") \
        > "$output_dir/unicode-normalization-generate.txt"
}

generate_normalization_runtime "$rxtvm_bin" rxtvm
generate_normalization_runtime "$rxbvm_bin" rxbvm
cmp "$work_dir/rxtvm/unicode_normalization.crexx" "$work_dir/rxbvm/unicode_normalization.crexx"
cmp "$work_dir/rxtvm/unicode-normalization-generate.txt" "$work_dir/rxbvm/unicode-normalization-generate.txt"
if grep -E 'assembler[[:space:]]+signal|call[[:space:]]+raise' \
        "$work_dir/rxtvm/unicode_normalization.crexx" > /dev/null; then
    echo "generated normalization source bypasses the Level B SIGNAL surface" >&2
    exit 1
fi
grep -F 'signal unicode_error' \
    "$work_dir/rxtvm/unicode_normalization.crexx" > /dev/null

compile_source_to_rxas "$work_dir/rxtvm/unicode_normalization.crexx" \
    "$work_dir/rxtvm/unicode_normalization" "$work_dir" \
    "$work_dir/unicode-normalization-runtime-build.log" 1

if [ "$(grep -c '^\.const §rxc\.const\.[0-9][0-9]*\.unicode_data binary 0x' \
        "$work_dir/rxtvm/unicode_normalization.rxas")" -ne 1 ]; then
    echo "compiler-generated normalization RXAS does not contain exactly one binary constant alias" >&2
    exit 1
fi
normalization_alias=$(sed -n 's/^\.const \(§rxc\.const\.[0-9][0-9]*\.unicode_data\) binary 0x.*/\1/p' \
    "$work_dir/rxtvm/unicode_normalization.rxas")
if [ -z "$normalization_alias" ]; then
    echo "compiler-generated normalization RXAS constant alias is missing" >&2
    exit 1
fi
grep -E "^[[:space:]]*bgetu32 [^,]+,$normalization_alias," \
    "$work_dir/rxtvm/unicode_normalization.rxas" > /dev/null
if grep -F '.unicodednormalizer._image' \
        "$work_dir/rxtvm/unicode_normalization.rxas" > /dev/null; then
    echo "constant-backed normalizer unexpectedly owns an image attribute" >&2
    exit 1
fi
if ! "$rxas_bin" -o "$work_dir/unicode_normalization" \
        "$work_dir/rxtvm/unicode_normalization" \
        > "$work_dir/unicode-normalization-runtime-assemble.log" 2>&1; then
    echo "constant-backed normalization assembly failed:" >&2
    cat "$work_dir/unicode-normalization-runtime-assemble.log" >&2
    exit 1
fi
if [ -s "$work_dir/unicode-normalization-runtime-assemble.log" ]; then
    echo "constant-backed normalization assembly emitted unexpected diagnostics:" >&2
    cat "$work_dir/unicode-normalization-runtime-assemble.log" >&2
    exit 1
fi
compile_source "$script_dir/unicode_nfd_lexer_generate.crexx" \
    "$work_dir/unicode_nfd_lexer_generate" "$work_dir" \
    "$work_dir/unicode-nfd-lexer-generate-build.log"
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/unicode_nfd_lexer_generate.ctl")

generate_nfd_lexer() {
    vm_bin=$1
    vm_name=$2
    output_dir="$work_dir/$vm_name"
    (cd "$work_dir" && "$vm_bin" unicode_nfd_lexer_generate \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        unicode_nfd_lexer_generate_linked \
        -a "$unicode_data_input" "$normalization_properties_input" \
        "$output_dir/level_l_unicode_nfd.re") \
        > "$output_dir/unicode-nfd-lexer-generate.txt"
}

generate_nfd_lexer "$rxtvm_bin" rxtvm
generate_nfd_lexer "$rxbvm_bin" rxbvm
cmp "$work_dir/rxtvm/level_l_unicode_nfd.re" "$work_dir/rxbvm/level_l_unicode_nfd.re"
cmp "$work_dir/rxtvm/unicode-nfd-lexer-generate.txt" "$work_dir/rxbvm/unicode-nfd-lexer-generate.txt"
(cd "$adapter_dir" && "$re2c_bin" --lang python \
    --syntax crexx_transducer.syntax --input-encoding utf8 \
    --no-debug-info --no-generation-date --no-version \
    -o "$work_dir/level_l_unicode_nfd.crexx" \
    "$work_dir/rxtvm/level_l_unicode_nfd.re")
perl -pi -e 's/[ \t]+$//' "$work_dir/level_l_unicode_nfd.crexx"
if grep -E 'assembler[[:space:]]+signal|call[[:space:]]+raise' \
        "$work_dir/level_l_unicode_nfd.crexx" > /dev/null; then
    echo "generated NFD wrapper bypasses the Level B SIGNAL surface" >&2
    exit 1
fi
grep -F 'signal invalid_arguments construction_error' \
    "$work_dir/level_l_unicode_nfd.crexx" > /dev/null
compile_source "$work_dir/level_l_unicode_nfd.crexx" \
    "$work_dir/level_l_unicode_nfd" "$work_dir" \
    "$work_dir/level-l-unicode-nfd-build.log" 1
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/unicode_d.ctl")

normalize_rxas_audit="$work_dir/unicode-d-normalize.rxas"
normalize_binary_rxas_audit="$work_dir/unicode-d-normalize-binary.rxas"
is_normalized_rxas_audit="$work_dir/unicode-d-is-normalized.rxas"
lookup_string_rxas_audit="$work_dir/unicode-d-lookup-string.rxas"
mapping_string_rxas_audit="$work_dir/unicode-d-mapping-string.rxas"
check_composed_rxas_audit="$work_dir/unicode-d-check-composed.rxas"
check_emit_rxas_audit="$work_dir/unicode-d-check-emit.rxas"
compose_pair_rxas_audit="$work_dir/unicode-normalization-compose-pair.rxas"
awk '/^§unicode_d\.unicodednormalizer\.normalize\(\)/ { active = 1 }
     /^§unicode_d\.unicodednormalizer\.normalize_binary\(\)/ { if (active) exit }
     active { print }' "$work_dir/unicode_d.rxas" > "$normalize_rxas_audit"
awk '/^§unicode_d\.unicodednormalizer\.normalize_binary\(\)/ { active = 1 }
     /^§unicode_d\.unicodednormalizer\.is_normalized\(\)/ { if (active) exit }
     active { print }' "$work_dir/unicode_d.rxas" > "$normalize_binary_rxas_audit"
awk '/^§unicode_d\.unicodednormalizer\.is_normalized\(\)/ { active = 1 }
     /^§unicode_d\.unicodednormalizer\.is_normalized_binary\(\)/ { if (active) exit }
     active { print }' "$work_dir/unicode_d.rxas" > "$is_normalized_rxas_audit"
awk '/^§unicode_d\.unicodednormalizer\._process_lookup_scalar_string\(\)/ { active = 1 }
     active && seen && /^§/ { exit }
     active { print; seen = 1 }' "$work_dir/unicode_d.rxas" > "$lookup_string_rxas_audit"
awk '/^§unicode_d\.unicodednormalizer\._process_mapping_string\(\)/ { active = 1 }
     active && seen && /^§/ { exit }
     active { print; seen = 1 }' "$work_dir/unicode_d.rxas" > "$mapping_string_rxas_audit"
awk '/^§unicode_d\.unicodednormalizer\._check_composed_string\(\)/ { active = 1 }
     active && seen && /^§/ { exit }
     active { print; seen = 1 }' "$work_dir/unicode_d.rxas" > "$check_composed_rxas_audit"
awk '/^§unicode_d\.unicodednormalizer\._check_emit\(\)/ { active = 1 }
     active && seen && /^§/ { exit }
     active { print; seen = 1 }' "$work_dir/unicode_d.rxas" > "$check_emit_rxas_audit"
awk '/^§unicode_d\.unicodednormalizer\._compose_pair\(\)/ { active = 1 }
     active && seen && /^§/ { exit }
     active { print; seen = 1 }' "$work_dir/unicode_d.rxas" > "$compose_pair_rxas_audit"
normalize_locals=$(sed -n '1s/.*\.locals=\([0-9][0-9]*\).*/\1/p' "$normalize_rxas_audit")
if [ -z "$normalize_locals" ] || [ "$normalize_locals" -gt 64 ]; then
    echo "optimized prepared NFD normalizer has an unexpected local count: $normalize_locals" >&2
    exit 1
fi
grep -E '^[[:space:]]*strchar ' "$normalize_rxas_audit" > /dev/null
grep -E '^[[:space:]]*bgetu32 ' "$normalize_rxas_audit" > /dev/null
grep -F ".jtable " "$normalize_rxas_audit" > /dev/null
grep -E '^[[:space:]]*jumpi ' "$normalize_rxas_audit" > /dev/null
grep -E '^[[:space:]]*appendchar ' "$normalize_rxas_audit" > /dev/null
grep -E '^[[:space:]]*getandtp ' "$normalize_rxas_audit" > /dev/null
grep -E '^[[:space:]]*setortp ' "$normalize_rxas_audit" > /dev/null
grep -F 'unicodednormalizer._process_lookup_scalar_string()' "$normalize_rxas_audit" > /dev/null
if grep -E '^[[:space:]]*(bcopy|stobin|bintos|bresize) ' "$normalize_rxas_audit" > /dev/null; then
    echo "optimized string normalizer unexpectedly copies, converts, or resizes binary data" >&2
    exit 1
fi
is_normalized_locals=$(sed -n '1s/.*\.locals=\([0-9][0-9]*\).*/\1/p' "$is_normalized_rxas_audit")
if [ -z "$is_normalized_locals" ] || [ "$is_normalized_locals" -gt 64 ]; then
    echo "optimized normalization predicate has an unexpected local count: $is_normalized_locals" >&2
    exit 1
fi
grep -E '^[[:space:]]*strchar ' "$is_normalized_rxas_audit" > /dev/null
grep -E '^[[:space:]]*bgetu32 ' "$is_normalized_rxas_audit" > /dev/null
grep -E '^[[:space:]]*bgetu8 ' "$is_normalized_rxas_audit" > /dev/null
grep -E '^[[:space:]]*getandtp ' "$is_normalized_rxas_audit" > /dev/null
grep -E '^[[:space:]]*setortp ' "$is_normalized_rxas_audit" > /dev/null
grep -F 'unicodednormalizer._check_composed_string()' "$is_normalized_rxas_audit" > /dev/null
if grep -E '^[[:space:]]*(appendchar|bcopy|stobin|bintos|bresize) ' "$is_normalized_rxas_audit" > /dev/null || \
   grep -F 'unicodednormalizer.normalize()' "$is_normalized_rxas_audit" > /dev/null || \
   grep -F 'unicodednormalizer._quick_check()' "$is_normalized_rxas_audit" > /dev/null; then
    echo "optimized normalization predicate constructs output or enters a byte path" >&2
    exit 1
fi
grep -E '^[[:space:]]*bgetu8 ' "$lookup_string_rxas_audit" > /dev/null
grep -F 'unicodednormalizer._find_mapping()' "$lookup_string_rxas_audit" > /dev/null
grep -F 'unicodednormalizer._process_mapping_string()' "$lookup_string_rxas_audit" > /dev/null
grep -E '^[[:space:]]*bgetu32 ' "$mapping_string_rxas_audit" > /dev/null
grep -E '^[[:space:]]*bgetu8 ' "$mapping_string_rxas_audit" > /dev/null
grep -E '^[[:space:]]*appendchar ' "$mapping_string_rxas_audit" > /dev/null
grep -E '^[[:space:]]*strchar ' "$check_composed_rxas_audit" > /dev/null
grep -E '^[[:space:]]*bgetu32 ' "$check_composed_rxas_audit" > /dev/null
grep -F 'unicodednormalizer._check_lookup_scalar()' "$check_composed_rxas_audit" > /dev/null
grep -E '^[[:space:]]*strchar ' "$check_emit_rxas_audit" > /dev/null
if grep -E '^[[:space:]]*(appendchar|bcopy|stobin|bintos|bresize) ' \
        "$check_composed_rxas_audit" "$check_emit_rxas_audit" > /dev/null; then
    echo "optimized C-form predicate unexpectedly constructs or converts output" >&2
    exit 1
fi
if grep -E '^[[:space:]]*(scopy|bcopy|stobin|bintos) ' "$normalize_binary_rxas_audit" > /dev/null; then
    echo "optimized binary normalizer unexpectedly copies or converts its input/result" >&2
    exit 1
fi
grep -E '^[[:space:]]*bgetu32 ' "$compose_pair_rxas_audit" > /dev/null
grep -E '^[[:space:]]*bgetu16 ' "$compose_pair_rxas_audit" > /dev/null
grep -E '^[[:space:]]*imod ' "$compose_pair_rxas_audit" > /dev/null
if grep -E '^[[:space:]]*(scopy|bcopy|stobin|bintos|bresize) ' "$compose_pair_rxas_audit" > /dev/null; then
    echo "optimized canonical pair lookup copies, converts, or resizes data" >&2
    exit 1
fi
compile_source "$script_dir/test_unicode_d.crexx" \
    "$work_dir/test_unicode_d" "$work_dir" \
    "$work_dir/test-unicode-d-build.log"
compile_source "$script_dir/test_unicode_casefold.crexx" \
    "$work_dir/test_unicode_casefold" "$work_dir" \
    "$work_dir/test-unicode-casefold-build.log"
(cd "$work_dir" && "$rxlink_bin" -c "$script_dir/unicode_casefold.ctl")

compile_source_noopt "$work_dir/gennorm2.crexx" \
    "$noopt_dir/gennorm2" "$product_build_dir/bin" \
    "$noopt_dir/gennorm2-build.log"
for module in unicode_gennorm2 unicode_data unicode_normprops unicode_d unicode_casefold; do
    module_uses_exits=0
    if [ "$module" = "unicode_d" ]; then module_uses_exits=1; fi
    compile_source_noopt "$script_dir/$module.crexx" \
        "$noopt_dir/$module" "$noopt_dir" \
        "$noopt_dir/$module-build.log" "$module_uses_exits"
done
compile_source_to_rxas_noopt "$work_dir/rxtvm/unicode_normalization.crexx" \
    "$noopt_dir/unicode_normalization" "$noopt_dir" \
    "$noopt_dir/unicode-normalization-build.log" 1
if [ "$(grep -c '^\.const §rxc\.const\.[0-9][0-9]*\.unicode_data binary 0x' \
        "$noopt_dir/unicode_normalization.rxas")" -ne 1 ]; then
    echo "noopt compiler-generated normalization RXAS does not contain exactly one binary constant alias" >&2
    exit 1
fi
if ! "$rxas_bin" -n -o "$noopt_dir/unicode_normalization" \
        "$noopt_dir/unicode_normalization" \
        > "$noopt_dir/unicode-normalization-assemble.log" 2>&1; then
    echo "noopt constant-backed normalization assembly failed:" >&2
    cat "$noopt_dir/unicode-normalization-assemble.log" >&2
    exit 1
fi
if [ -s "$noopt_dir/unicode-normalization-assemble.log" ]; then
    echo "noopt constant-backed normalization assembly emitted unexpected diagnostics:" >&2
    cat "$noopt_dir/unicode-normalization-assemble.log" >&2
    exit 1
fi
compile_source_noopt "$work_dir/level_l_unicode_nfd.crexx" \
    "$noopt_dir/level_l_unicode_nfd" "$noopt_dir" \
    "$noopt_dir/level-l-unicode-nfd-build.log" 1
compile_source_noopt "$script_dir/test_unicode_d.crexx" \
    "$noopt_dir/test_unicode_d" "$noopt_dir" \
    "$noopt_dir/test_unicode_d-build.log"
(cd "$noopt_dir" && "$rxlink_bin" -c "$script_dir/unicode_d.ctl")
compile_source_noopt "$script_dir/test_unicode_casefold.crexx" \
    "$noopt_dir/test_unicode_casefold" "$noopt_dir" \
    "$noopt_dir/test_unicode_casefold-build.log"
(cd "$noopt_dir" && "$rxlink_bin" -c "$script_dir/unicode_casefold.ctl")

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

run_unicode_nfd_test() {
    vm_bin=$1
    output_file=$2
    (cd "$work_dir" && CREXX_HOME="$product_build_dir" \
        "$vm_bin" test_unicode_nfd \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        unicode_nfd_linked -a "$nfc_input" "$normalization_input") \
        > "$output_file"
}

run_unicode_normalization_test() {
    vm_bin=$1
    execution_dir=$2
    output_file=$3
    (cd "$execution_dir" && "$vm_bin" test_unicode_d \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        unicode_d_linked unicode_normalization \
        -a "$unicode_data_input" "$normalization_properties_input" "$normalization_input") \
        > "$output_file"
}

run_unicode_casefold_test() {
    vm_bin=$1
    execution_dir=$2
    output_file=$3
    (cd "$execution_dir" && "$vm_bin" test_unicode_casefold \
        "$product_build_dir/bin/library" \
        "$product_build_dir/bin/rxfnsl" \
        unicode_casefold_linked unicode_normalization \
        -a "$casefold_input") > "$output_file"
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
run_unicode_nfd_test "$rxtvm_bin" "$work_dir/rxtvm-unicode-nfd.txt"
run_unicode_nfd_test "$rxbvm_bin" "$work_dir/rxbvm-unicode-nfd.txt"
run_unicode_normalization_test "$rxtvm_bin" "$work_dir" "$work_dir/rxtvm-unicode-normalization.txt"
run_unicode_normalization_test "$rxbvm_bin" "$work_dir" "$work_dir/rxbvm-unicode-normalization.txt"
run_unicode_normalization_test "$rxtvm_bin" "$noopt_dir" "$work_dir/rxtvm-unicode-normalization-noopt.txt"
run_unicode_normalization_test "$rxbvm_bin" "$noopt_dir" "$work_dir/rxbvm-unicode-normalization-noopt.txt"
run_unicode_casefold_test "$rxtvm_bin" "$work_dir" "$work_dir/rxtvm-unicode-casefold.txt"
run_unicode_casefold_test "$rxbvm_bin" "$work_dir" "$work_dir/rxbvm-unicode-casefold.txt"
run_unicode_casefold_test "$rxtvm_bin" "$noopt_dir" "$work_dir/rxtvm-unicode-casefold-noopt.txt"
run_unicode_casefold_test "$rxbvm_bin" "$noopt_dir" "$work_dir/rxbvm-unicode-casefold-noopt.txt"

"$reference_bin" --dump-rules "$nfc_input" > "$work_dir/reference-gennorm2.semantic"
"$reference_bin" "$nfc_input" "$normalization_input" > "$work_dir/reference-nfd.txt"

cmp "$work_dir/rxtvm-frontend.txt" "$work_dir/rxbvm-frontend.txt"
cmp "$work_dir/rxtvm-tinyexpr.txt" "$work_dir/rxbvm-tinyexpr.txt"
cmp "$work_dir/rxtvm-gennorm2.txt" "$work_dir/rxbvm-gennorm2.txt"
cmp "$work_dir/rxtvm/gennorm2.records" "$work_dir/rxbvm/gennorm2.records"
cmp "$work_dir/rxtvm/gennorm2.semantic" "$work_dir/rxbvm/gennorm2.semantic"
cmp "$generated_dir/gennorm2.records" "$work_dir/rxtvm/gennorm2.records"
cmp "$generated_dir/gennorm2.semantic" "$work_dir/rxtvm/gennorm2.semantic"
cmp "$work_dir/reference-gennorm2.semantic" "$work_dir/rxtvm/gennorm2.semantic"
cmp "$work_dir/rxtvm-unicode-parser.txt" "$work_dir/rxbvm-unicode-parser.txt"
cmp "$work_dir/rxtvm-unicode-nfd.txt" "$work_dir/rxbvm-unicode-nfd.txt"
cmp "$script_dir/evidence/nfd-result.txt" "$work_dir/rxtvm-unicode-nfd.txt"
cmp "$work_dir/rxtvm-unicode-normalization.txt" "$work_dir/rxbvm-unicode-normalization.txt"
cmp "$work_dir/rxtvm-unicode-normalization.txt" "$work_dir/rxtvm-unicode-normalization-noopt.txt"
cmp "$work_dir/rxtvm-unicode-normalization.txt" "$work_dir/rxbvm-unicode-normalization-noopt.txt"
cmp "$script_dir/evidence/unicode-normalization-result.txt" "$work_dir/rxtvm-unicode-normalization.txt"
cmp "$work_dir/rxtvm-unicode-casefold.txt" "$work_dir/rxbvm-unicode-casefold.txt"
cmp "$work_dir/rxtvm-unicode-casefold.txt" "$work_dir/rxtvm-unicode-casefold-noopt.txt"
cmp "$work_dir/rxtvm-unicode-casefold.txt" "$work_dir/rxbvm-unicode-casefold-noopt.txt"
cmp "$script_dir/evidence/unicode-casefold-result.txt" "$work_dir/rxtvm-unicode-casefold.txt"
cmp "$poc_dir/evidence/nfd-conformance.txt" "$work_dir/reference-nfd.txt"
grep -Fx "PASS: Level L bootstrap frontend" "$work_dir/rxtvm-frontend.txt" > /dev/null
grep -Fx "PASS: authored Level L TinyExpr matches rxfnsl" "$work_dir/rxtvm-tinyexpr.txt" > /dev/null
grep -Fx "PASS: authored Level L lexer accepts ICU gennorm2 nfc.txt" "$work_dir/rxtvm-gennorm2.txt" > /dev/null
grep -Fx "PASS: tokenized gennorm2 parsed into typed Unicode records" "$work_dir/rxtvm/unicode-gennorm2-generate.txt" > /dev/null
grep -Fx "PASS: deterministic typed Unicode rule parser and panic contract" "$work_dir/rxtvm-unicode-parser.txt" > /dev/null
grep -Fx "Focused NFD fixtures: PASS" "$work_dir/rxtvm-unicode-nfd.txt" > /dev/null
grep -Fx "Result: PASS" "$work_dir/rxtvm-unicode-nfd.txt" > /dev/null
grep -Fx "Focused prepared normalization fixtures: PASS" "$work_dir/rxtvm-unicode-normalization.txt" > /dev/null
grep -Fx "Prepared normalization tables: PASS" "$work_dir/rxtvm-unicode-normalization.txt" > /dev/null
grep -Fx "Generated prepared NFD wrapper: PASS" "$work_dir/rxtvm-unicode-normalization.txt" > /dev/null
grep -Fx "NFD/NFKD/NFC/NFKC normalization predicate checks: 400680" \
    "$work_dir/rxtvm-unicode-normalization.txt" > /dev/null
grep -Fx "Result: PASS" "$work_dir/rxtvm-unicode-normalization.txt" > /dev/null
grep -Fx "Prepared simple/full/default/Turkic case folding: PASS" "$work_dir/rxtvm-unicode-casefold.txt" > /dev/null
grep -Fx "Result: PASS" "$work_dir/rxtvm-unicode-casefold.txt" > /dev/null

summary_actual="$work_dir/result.txt"
{
    echo "Level B frontend: deterministic Token and AST dumps"
    echo "parser policy: single cursor, no backtracking, first-fault panic"
    echo "TinyExpr cREXX: byte-identical generation"
    echo "TinyExpr differential oracle: PASS"
    echo "ICU gennorm2 nfc.txt lexer: PASS (2500 lines)"
    echo "Unicode rule parser: PASS (2485 typed records with byte/token spans)"
    echo "C++/re2c semantic oracle: byte-identical (2485 records)"
    echo "portable Level B NFD table: PASS (1186472 bytes)"
    echo "Unicode 17.0.0 NFD conformance: PASS (100170 corpus relations)"
    echo "unlisted scalar NFD identity: PASS (1094978 scalars)"
    echo "C++/re2c NFD oracle: retained conformance evidence reproduced"
    echo "portable prepared four-form image: PASS (format 4; 4456448-byte NFD prepared-symbol section; packed NFC/NFKC Quick_Check)"
    echo "constant-backed Unicode runtime: PASS (one shared 11237664-byte portable image)"
    echo "normalizer instances: PASS (no owning image attribute and no per-instance image copy)"
    echo "RXAS Unicode payload: PASS (one named constant alias with direct indexed operands)"
    echo "Unicode 17.0.0 NFD/NFKD/NFC/NFKC conformance: PASS (400680 corpus relations)"
    echo "Unicode 17.0.0 normalization predicates: PASS (400680 exact codepoint checks)"
    echo "generated prepared NFD wrapper: PASS (2081 mapping symbols, 964 identity-mark symbols)"
    echo "generated prepared NFD conformance: PASS (100170 corpus relations)"
    echo "unlisted scalar four-form identity: PASS (1094978 scalars per form)"
    echo "primary composition: PASS (961 pairs, 391 starters, 1120 exclusions)"
    echo "ordinary case folding: PASS (simple/full/default/Turkic; 1585 records)"
    echo "unlisted scalar case-fold identity: PASS (1110479 scalars per mode)"
    echo "optimized/noopt linked images: byte-identical conformance summaries"
    echo "four-form string path: strchar codepoint scan; no string/binary conversion"
    echo "binary path: strict UTF-8 decode"
    echo "prepared dispatch: dense at-u32 symbol lookup, .jtable and jumpi"
    echo "binary scan/stores: bgetu8, bsetu16, bsetu32"
    echo "rxtvm: PASS"
    echo "rxbvm: PASS"
} > "$summary_actual"

cmp "$script_dir/evidence/result.txt" "$summary_actual"
cat "$summary_actual"
