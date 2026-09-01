#!/bin/zsh
set -eu

diag_root=/private/tmp/crexx-perf2-02.kJ5sZT/q7-core/perf2-02-q7-diagnostics
diag_bin="$diag_root/diagnostic-src/cmake-build-perf2-q7-diag/bin"
product_root=/private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release
library="$product_root/bin/library.rxbin"
bounce="$product_root/tests/benchmarks/benchmark_awfy_bounce_opt.rxbin"
richards="$product_root/tests/benchmarks/benchmark_awfy_richards_opt.rxbin"
guard=/private/tmp/crexx-perf2-02.kJ5sZT/perf2_qref_guard.rxbin

for vm in rxvm rxbvm; do
  "$diag_bin/$vm" "$bounce" "$library" -a 1 \
    >"$diag_root/logs/bounce-$vm.out" \
    2>"$diag_root/logs/bounce-$vm.err"
  "$diag_bin/$vm" "$richards" "$library" -a 1 \
    >"$diag_root/logs/richards-$vm.out" \
    2>"$diag_root/logs/richards-$vm.err"
  "$diag_bin/$vm" "$guard" "$library" \
    >"$diag_root/logs/guard-$vm.out" \
    2>"$diag_root/logs/guard-$vm.err"
done

for log in "$diag_root"/logs/{bounce,richards,guard}-{rxvm,rxbvm}.{out,err}; do
  print -- "--- ${log:t}"
  tail -n 4 "$log"
done
