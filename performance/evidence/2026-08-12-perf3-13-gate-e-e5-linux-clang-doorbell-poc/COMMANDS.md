# PERF3-13 E5 Linux Clang commands

Commands ran from `/home/adrian/CLionProjects/CREXX`. Explicit build-tree tools
were used throughout; no installed `~/.local` tool participated.

## Provenance and host

```sh
git rev-parse HEAD origin/mthread
git branch --show-current
git status --short
hostnamectl
uname -a
sed -n '1,20p' /etc/os-release
lscpu
getconf GNU_LIBC_VERSION
/usr/bin/clang --version
/usr/bin/clang++ --version
cmake --version
ninja --version
free -h
powerprofilesctl get
upower -i /org/freedesktop/UPower/devices/line_power_ACAD
upower -i /org/freedesktop/UPower/devices/battery_BAT1
```

CPU governor, EPP, turbo, thermal, load, process and artifact hashes were
captured before/after governed blocks in the retained `pre-*.txt` and
`post-*.txt` files.

## Fresh Clang Debug correctness/stress

```sh
cmake -S . -B /tmp/crexx-mthread-linux-clang-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
  -DCREXX_VM_HANDLER_PANEL=profile-20 \
  -DCREXX_VM_PROFILING=OFF

cmake --build /tmp/crexx-mthread-linux-clang-debug --parallel 6 --target \
  persistent_worker_executor-rxvml \
  persistent_worker_executor-rxbvml \
  persistent_worker_executor-rxtvml

ctest --test-dir /tmp/crexx-mthread-linux-clang-debug \
  -R '^persistent_worker_executor-' \
  --repeat until-fail:20 --output-on-failure --timeout 120
```

## Fresh Clang ordinary Release

```sh
/usr/bin/time -v cmake -S . \
  -B /tmp/crexx-mthread-linux-clang-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
  -DCREXX_VM_HANDLER_PANEL=profile-20 \
  -DCREXX_VM_PROFILING=OFF

/usr/bin/time -v cmake --build /tmp/crexx-mthread-linux-clang-release \
  --parallel 6 --target \
  rxbvm rxtvm crexx library classlib_native \
  rx_treemap_static.a rx_llist_static.a \
  persistent_worker_executor-rxvml \
  persistent_worker_executor-rxbvml \
  persistent_worker_executor-rxtvml \
  benchmark_awfy_sieve_opt_artifact \
  benchmark_awfy_permute_opt_artifact \
  benchmark_rexxcps_levelb_opt_artifact

cmake --build /tmp/crexx-mthread-linux-clang-release --parallel 6 --target \
  rx_treemap.rxplugin rx_llist.rxplugin

ctest --test-dir /tmp/crexx-mthread-linux-clang-release \
  -R '^persistent_worker_executor-' --output-on-failure --timeout 120
```

The dynamic plugin build followed a build-tree driver self-test that initially
reported a missing `rx_treemap` module. The same self-test passed after those
explicit build-tree plugins were built; this was build preparation, not a PoC
test failure.

## Latency

```sh
CREXX_VM_POC_DOORBELL=posix \
  /tmp/crexx-mthread-linux-clang-release/interpreter/persistent_worker_executor-rxbvml \
  --doorbell-latency \
  /tmp/crexx-mthread-linux-clang-release/interpreter/test_persistent_worker_executor.rxbin \
  1000

CREXX_VM_POC_DOORBELL=posix \
  /tmp/crexx-mthread-linux-clang-release/interpreter/persistent_worker_executor-rxtvml \
  --doorbell-latency \
  /tmp/crexx-mthread-linux-clang-release/interpreter/test_persistent_worker_executor.rxbin \
  1000
```

The first manual `rxbvml` command accidentally omitted the environment
assignment. It was terminated after 5m10s when native cancellation refusal and
the expected infinite-loop wait were diagnosed. No value from it was used.
`clang-rxbvm-latency-hang.txt`, the diagnostic re-ring, and the superseded log
retain the incident.

## Frozen E4 Clang control and governed artifacts

The detached source tree is `/tmp/crexx-e4-linux-control-src` at
`295a6d886b33b161e57d71bc641970e394f58f66`.

```sh
/usr/bin/time -v cmake -S /tmp/crexx-e4-linux-control-src \
  -B /tmp/crexx-e4-linux-clang-control-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/usr/bin/clang \
  -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
  -DCREXX_VM_HANDLER_PANEL=profile-20 \
  -DCREXX_VM_PROFILING=OFF

/usr/bin/time -v cmake --build /tmp/crexx-e4-linux-clang-control-release \
  --parallel 6 --target rxbvm rxtvm

sha256sum \
  /tmp/crexx-e4-linux-clang-control-release/bin/rxbvm \
  /tmp/crexx-e4-linux-clang-control-release/bin/rxtvm \
  /tmp/crexx-mthread-linux-clang-release/bin/rxbvm \
  /tmp/crexx-mthread-linux-clang-release/bin/rxtvm \
  /tmp/crexx-mthread-linux-clang-release/bin/crexx \
  /tmp/crexx-mthread-linux-clang-release/bin/library.rxbin \
  /tmp/crexx-mthread-linux-clang-release/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  /tmp/crexx-mthread-linux-clang-release/tests/benchmarks/benchmark_awfy_permute_opt.rxbin \
  /tmp/crexx-mthread-linux-clang-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin \
  performance/tools/run_cross_runtime_matrix.crexx
```

`manifest.txt` binds all cells to the explicit control/candidate paths.

## Formal paired timing

Initial one-warmup, 12-pair block:

```sh
systemd-inhibit --what=idle:sleep \
  --who='CREXX PERF3-13 E5 Linux Clang qualification' \
  --why='Retained paired Clang Release timing' \
  /tmp/crexx-mthread-linux-clang-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/manifest.txt \
  --output-dir performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/timing \
  --measurement timing --warmups 1 --runs 12
```

Five ambiguous/guard groups were appended twice, with the unchanged retained
manifest, to 24 and then the 36-pair cap:

```sh
systemd-inhibit --what=idle:sleep \
  --who='CREXX PERF3-13 E5 Linux Clang qualification' \
  --why='Retained paired Clang Release timing append' \
  /tmp/crexx-mthread-linux-clang-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/manifest-append-1.txt \
  --output-dir performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/timing-append-1 \
  --measurement timing --warmups 0 --runs 12

systemd-inhibit --what=idle:sleep \
  --who='CREXX PERF3-13 E5 Linux Clang qualification' \
  --why='Retained paired Clang Release timing final append' \
  /tmp/crexx-mthread-linux-clang-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/manifest-append-2.txt \
  --output-dir performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/timing-append-2 \
  --measurement timing --warmups 0 --runs 12
```

The 12/24/36 reducers use two-sided Student-t multipliers 2.200985,
2.068658 and 2.030108. They ran with the candidate build-tree `crexx`. The
combined CSVs contain recorded rows only; no sample was removed.

```sh
/tmp/crexx-mthread-linux-clang-release/bin/crexx \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/summarize_paired_24.crexx \
  --nokeep --args \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/paired-summary-24.csv \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/combined-24-samples.csv

/tmp/crexx-mthread-linux-clang-release/bin/crexx \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/summarize_paired_36.crexx \
  --nokeep --args \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/paired-summary-36.csv \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/combined-36-samples.csv
```

## Controlled GCC-versus-Clang build comparison

```sh
/usr/bin/time -v cmake -S /tmp/crexx-e4-linux-control-src \
  -B /tmp/crexx-e4-linux-gcc-control-comparison-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=/usr/bin/gcc \
  -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
  -DCREXX_VM_HANDLER_PANEL=profile-20 \
  -DCREXX_VM_PROFILING=OFF

/usr/bin/time -v cmake --build \
  /tmp/crexx-e4-linux-gcc-control-comparison-release \
  --parallel 6 --target rxbvm rxtvm

stat -c '%n %s' \
  /tmp/crexx-e4-linux-clang-control-release/bin/{rxbvm,rxtvm} \
  /tmp/crexx-e4-linux-gcc-control-comparison-release/bin/{rxbvm,rxtvm} \
  /tmp/crexx-mthread-linux-clang-release/bin/{rxbvm,rxtvm} \
  /tmp/crexx-mthread-linux-release/bin/{rxbvm,rxtvm}

size \
  /tmp/crexx-e4-linux-clang-control-release/bin/{rxbvm,rxtvm} \
  /tmp/crexx-e4-linux-gcc-control-comparison-release/bin/{rxbvm,rxtvm} \
  /tmp/crexx-mthread-linux-clang-release/bin/{rxbvm,rxtvm} \
  /tmp/crexx-mthread-linux-release/bin/{rxbvm,rxtvm}
```

## Handler and ordinary-dispatch audit

```sh
nm -aS --size-sort OBJECT
readelf -sW OBJECT
readelf -rW OBJECT
objdump -drwC --disassemble=rxvm_thread_doorbell_poc_handler OBJECT
objdump -dwC --disassemble=rxvm_thread_doorbell_poc_handler BINARY

git diff 295a6d886b33b161e57d71bc641970e394f58f66 \
  b6edf2556f2411fe5033049e32ee77ddd9a2e15f -- \
  interpreter/rxvmintp.c interpreter/rxvmintp.h
git diff -- interpreter/rxvmintp.c interpreter/rxvmintp.h
git rev-parse \
  295a6d886b33b161e57d71bc641970e394f58f66:interpreter/rxvmintp.h \
  b6edf2556f2411fe5033049e32ee77ddd9a2e15f:interpreter/rxvmintp.h
nm -S --size-sort BINARY | rg ' run$| rxvm_run_owned_core$'
objdump --no-show-raw-insn -dwC --disassemble=run BINARY
```

The normalized `run` comparison strips only load addresses, branch/call target
addresses, and RIP-relative layout addresses/comments. Mnemonics, operands and
symbolic call targets remain; both engine pairs compare equal.

## Closure

```sh
git diff --check
git status --short
git diff --binary -- \
  interpreter/CMakeLists.txt interpreter/interrupt.c \
  interpreter/rxvmexecutor.c > SOURCE-DIFF.patch
sha256sum interpreter/CMakeLists.txt interpreter/interrupt.c \
  interpreter/rxvmexecutor.c > SOURCE-SHA256SUMS
find . -type f ! -name SHA256SUMS -print0 | sort -z | \
  xargs -0 sha256sum > SHA256SUMS
sha256sum -c SHA256SUMS
```
