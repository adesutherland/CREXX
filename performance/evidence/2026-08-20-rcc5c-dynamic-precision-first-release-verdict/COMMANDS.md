# Commands

Commands ran from `/Users/adrian/CLionProjects/CREXX`.

## First Release verdict

The control was built from detached `298f412dc` under
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-rcc5c-baseline.XXXXXX.GMVnDUb2Ll`.
Both build trees were ordinary profiling-off Release builds. The benchmark was
compiled, assembled, and linked separately against each tree.

```sh
cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-20-rcc5c-dynamic-precision-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-20-rcc5c-dynamic-precision-first-release-verdict \
  --measurement timing --warmups 1 --runs 12
```

The governance-directed append used the same command with
`manifest-append-2.txt`/`append-2`; the final four-cold-cell append used
`manifest-append-3.txt`/`append-3`. The retained Level B reducer combined all
applicable sample files into `paired-summary.csv`.

## Correctness and packaging

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release --parallel 30 --output-on-failure

tools/asan-run.sh --phase full --build-leaks off --leaks off --test-jobs 8
tools/asan-run.sh --phase build --build-target ts_math_decimal_contract \
  --build-leaks off
tools/asan-run.sh --phase ctest \
  --regex '^(rcc5c_math_contract_surface_coverage|ts_math_decimal_contract_(rxbvm|rxtvm)_(noopt|opt))$' \
  --leaks off --test-jobs 1 \
  --fixture-exclude-setup linked_opt_runtime_artifacts

cmake --install cmake-build-release \
  --prefix /tmp/crexx-rcc5c-install.4ZRKqH
cd /tmp/crexx-rcc5c-smoke.SRuL3s
CREXX_HOME=/tmp/crexx-rcc5c-install.4ZRKqH \
  /tmp/crexx-rcc5c-install.4ZRKqH/bin/crexx \
  /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-08-20-rcc5c-dynamic-precision-first-release-verdict/decimal_install_smoke.crexx \
  -lrxfnsg --nocolour --nokeep
cd /tmp/crexx-rcc5c-native.17wDCf
CREXX_HOME=/tmp/crexx-rcc5c-install.4ZRKqH \
  /tmp/crexx-rcc5c-install.4ZRKqH/bin/crexx \
  /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-08-20-rcc5c-dynamic-precision-first-release-verdict/decimal_install_smoke.crexx \
  -lrxfnsg --native --nocolour --nokeep
mv /Users/adrian/CLionProjects/CREXX/performance/evidence/2026-08-20-rcc5c-dynamic-precision-first-release-verdict/decimal_install_smoke \
  /tmp/crexx-rcc5c-native.17wDCf/decimal_install_smoke
/tmp/crexx-rcc5c-native.17wDCf/decimal_install_smoke
```

Sanitizer logs are retained under:

- `cmake-build-debugasan/asan-logs/20260820-205831-full` (known RXAS failure);
- `cmake-build-debugasan/asan-logs/20260820-210349-build` (decimal build pass);
- `cmake-build-debugasan/asan-logs/20260820-210422-ctest` (decimal 5/5 pass).
