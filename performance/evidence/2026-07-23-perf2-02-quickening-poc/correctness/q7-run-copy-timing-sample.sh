#!/bin/zsh
set -eu

diag_root=/private/tmp/crexx-perf2-02.kJ5sZT/q7-core/perf2-02-q7-diagnostics
diag_bin="$diag_root/diagnostic-src/cmake-build-perf2-q7-diag/bin"
product_root=/private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release
library="$product_root/bin/library.rxbin"
richards="$product_root/tests/benchmarks/benchmark_awfy_richards_opt.rxbin"
runs=12

: >"$diag_root/raw/copy-timing-richards-12x.txt"
for vm in rxvm rxbvm; do
  run=1
  while (( run <= runs )); do
    suffix=$(printf '%02d' "$run")
    "$diag_bin/$vm" "$richards" "$library" -a 1 \
      >"$diag_root/logs/timing-richards-$vm-$suffix.out" \
      2>"$diag_root/logs/timing-richards-$vm-$suffix.err"
    grep '^PERF2_Q7_DIAG ' \
      "$diag_root/logs/timing-richards-$vm-$suffix.err" |
      sed "s/^/vm=$vm run=$suffix /" \
      >>"$diag_root/raw/copy-timing-richards-12x.txt"
    (( run++ ))
  done
done

awk '
  {
    delete value
    vm = ""
    for (i = 1; i <= NF; i++) {
      split($i, pair, "=")
      value[pair[1]] = pair[2]
    }
    vm = value["vm"]
    runs[vm]++
    cold_n[vm] += value["copy_cold_executions"]
    cold_ticks[vm] += value["copy_cold_ticks"]
    cold_spec_n[vm] += value["copy_cold_specialized"]
    cold_spec_ticks[vm] += value["copy_cold_specialized_ticks"]
    cold_disable_n[vm] += value["copy_cold_disabled"]
    cold_disable_ticks[vm] += value["copy_cold_disabled_ticks"]
    specialized_n[vm] += value["copy_specialized_executions"]
    specialized_ticks[vm] += value["copy_specialized_ticks"]
    fast_n[vm] += value["copy_fast_hits"]
    fallback_n[vm] += value["copy_specialized_fallbacks"]
    calibration_n[vm] += value["timer_calibration_samples"]
    calibration_ticks[vm] += value["timer_pair_ticks"]
    numer[vm] = value["timer_numer"]
    denom[vm] = value["timer_denom"]
  }
  END {
    print "vm|runs|cold_n|cold_ticks|cold_gross_ns_per_event|cold_specialize_n|cold_specialize_ticks|cold_specialize_gross_ns_per_event|cold_disable_n|cold_disable_ticks|cold_disable_gross_ns_per_event|specialized_n|specialized_ticks|specialized_gross_ns_per_event|fast_n|fallback_n|timer_pairs|timer_pair_ticks|timer_pair_mean_ns"
    order[1] = "rxvm"
    order[2] = "rxbvm"
    for (j = 1; j <= 2; j++) {
      vm = order[j]
      factor = numer[vm] / denom[vm]
      printf "%s|%d|%d|%d|%.6f|%d|%d|%.6f|%d|%d|%.6f|%d|%d|%.6f|%d|%d|%d|%d|%.6f\n", \
        vm, runs[vm], cold_n[vm], cold_ticks[vm], \
        cold_ticks[vm] * factor / cold_n[vm], \
        cold_spec_n[vm], cold_spec_ticks[vm], \
        cold_spec_ticks[vm] * factor / cold_spec_n[vm], \
        cold_disable_n[vm], cold_disable_ticks[vm], \
        cold_disable_ticks[vm] * factor / cold_disable_n[vm], \
        specialized_n[vm], specialized_ticks[vm], \
        specialized_ticks[vm] * factor / specialized_n[vm], \
        fast_n[vm], fallback_n[vm], calibration_n[vm], \
        calibration_ticks[vm], \
        calibration_ticks[vm] * factor / calibration_n[vm]
    }
  }
' "$diag_root/raw/copy-timing-richards-12x.txt" \
  >"$diag_root/raw/copy-timing-richards-12x-summary.txt"

sed -n '1,8p' "$diag_root/raw/copy-timing-richards-12x.txt"
sed -n '1,10p' "$diag_root/raw/copy-timing-richards-12x-summary.txt"
