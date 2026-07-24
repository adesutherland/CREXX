#!/bin/zsh
set -eu

diag_root=/private/tmp/crexx-perf2-02.kJ5sZT/q7-core/perf2-02-q7-diagnostics
diag_src="$diag_root/diagnostic-src"
q7_root=/private/tmp/crexx-perf2-02.kJ5sZT/q7-core
product_root=/private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release
product_bin="$product_root/bin"
include_flags=(
  -I"$diag_root/raw"
  -I"$diag_src/cmake-build-perf2-q7-diag/generated"
  -I"$diag_src/interpreter"
  -I"$diag_src/interpreter/rxvmplugin"
  -I"$diag_src/assembler"
  -I"$diag_src/binutils/include"
  -I"$diag_src/rxpa"
  -I"$diag_src/platform"
  -I"$diag_src/avl_tree"
  -I"$diag_src/utf8"
  -I"$diag_src/inc"
  -I"$diag_src/interpreter/rxvmplugin/rxvmplugins/mc_decimal/decnumber"
)

git -C "$diag_src" show HEAD:interpreter/rxvmintp.h \
  >"$diag_root/raw/baseline-rxvmintp.h"

/usr/bin/cc "${include_flags[@]}" \
  -DPROBE_HEADER='"baseline-rxvmintp.h"' \
  "$diag_root/raw/size_probe.c" -o "$diag_root/raw/baseline-size-probe"
/usr/bin/cc "${include_flags[@]}" \
  -DPROBE_HEADER='"rxvmintp.h"' -DPERF2_Q7_SIZE \
  "$diag_root/raw/size_probe.c" -o "$diag_root/raw/q7-size-probe"

{
  print -- '== struct sizes =='
  print -- '-- baseline HEAD --'
  "$diag_root/raw/baseline-size-probe"
  print -- '-- Q7 --'
  "$diag_root/raw/q7-size-probe"
  print -- '== normal product image file sizes =='
  stat -f '%N|%z' \
    "$product_bin/rxvm" "$product_bin/rxbvm" \
    "$q7_root/cmake-build-perf2-release/bin/rxvm" \
    "$q7_root/cmake-build-perf2-release/bin/rxbvm"
  print -- '== normal product __text sizes =='
  for image in \
    "$product_bin/rxvm" "$product_bin/rxbvm" \
    "$q7_root/cmake-build-perf2-release/bin/rxvm" \
    "$q7_root/cmake-build-perf2-release/bin/rxbvm"; do
    print -- "-- $image"
    size -m "$image" | awk '/Section __text:/ { print $0 }'
  done
} >"$diag_root/raw/size-output.txt"

{
  print -- 'image|copy_reg_reg|mkref_reg_reg|records'
  "$product_bin/rxdas" \
    "$product_root/tests/benchmarks/benchmark_awfy_bounce_opt.rxbin" |
    awk -v image=bounce-opt '
      /^[[:space:]]*([^:]+:[[:space:]]*)?copy[[:space:]]/ { copy_count++ }
      /^[[:space:]]*([^:]+:[[:space:]]*)?mkref[[:space:]]/ { mkref_count++ }
      END { printf "%s|%d|%d|%d\n", image, copy_count, mkref_count,
                   copy_count + mkref_count }
    '
  "$product_bin/rxdas" \
    "$product_root/tests/benchmarks/benchmark_awfy_richards_opt.rxbin" |
    awk -v image=richards-opt '
      /^[[:space:]]*([^:]+:[[:space:]]*)?copy[[:space:]]/ { copy_count++ }
      /^[[:space:]]*([^:]+:[[:space:]]*)?mkref[[:space:]]/ { mkref_count++ }
      END { printf "%s|%d|%d|%d\n", image, copy_count, mkref_count,
                   copy_count + mkref_count }
    '
  "$product_bin/rxdas" "$product_bin/library.rxbin" |
    awk -v image=library '
      /^[[:space:]]*([^:]+:[[:space:]]*)?copy[[:space:]]/ { copy_count++ }
      /^[[:space:]]*([^:]+:[[:space:]]*)?mkref[[:space:]]/ { mkref_count++ }
      END { printf "%s|%d|%d|%d\n", image, copy_count, mkref_count,
                   copy_count + mkref_count }
    '
} >"$diag_root/raw/static-record-counts.txt"

{
  print -- 'case|records|record_bytes|modules|module_delta_bytes|requested_bytes'
  print -- "bounce|943|$((943 * 56))|144|$((144 * 24))|$((943 * 56 + 144 * 24))"
  print -- "richards|1055|$((1055 * 56))|144|$((144 * 24))|$((1055 * 56 + 144 * 24))"
  print -- 'image|file_delta|text_delta'
  print -- "rxvm|$((998776 - 982264))|$((812620 - 806300))"
  print -- "rxbvm|$((998952 - 982392))|$((807760 - 799132))"
} >"$diag_root/raw/requested-byte-arithmetic.txt"

shasum -a 256 \
  "$product_bin/rxvm" "$product_bin/rxbvm" \
  "$q7_root/cmake-build-perf2-release/bin/rxvm" \
  "$q7_root/cmake-build-perf2-release/bin/rxbvm" \
  >"$diag_root/raw/image-sha256.txt"

sed -n '1,200p' "$diag_root/raw/size-output.txt"
sed -n '1,20p' "$diag_root/raw/static-record-counts.txt"
sed -n '1,20p' "$diag_root/raw/requested-byte-arithmetic.txt"
