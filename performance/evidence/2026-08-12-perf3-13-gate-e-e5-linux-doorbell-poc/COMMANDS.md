# PERF3-13 E5 Intel Linux command record

Commands ran from `/home/adrian/CLionProjects/CREXX` unless an explicit source
or build directory is shown. Long build/test output was redirected to the
corresponding retained log. All runtime and tool paths are build-tree paths;
no installed `~/.local` artifact was used.

## Branch and source

```sh
git status --short
git fetch origin mthread
git switch mthread
git switch --create mthread --track origin/mthread  # used because no local branch existed
git rev-parse HEAD origin/mthread
git status --short
```

The local branch was created directly at
`b6edf2556f2411fe5033049e32ee77ddd9a2e15f`; no rebase or merge was needed.
The E4 control is its parent:

```sh
git rev-parse HEAD^
git worktree add --detach /tmp/crexx-e4-linux-control-src \
  295a6d886b33b161e57d71bc641970e394f58f66
```

## Host inventory

The pre/post state files use combinations of these commands:

```sh
date -u +'%Y-%m-%dT%H:%M:%SZ'
hostnamectl
uname -a
sed -n '1,20p' /etc/os-release
lscpu
getconf GNU_LIBC_VERSION
cc --version
cmake --version
ninja --version
upower -i /org/freedesktop/UPower/devices/line_power_ACAD
upower -i /org/freedesktop/UPower/devices/battery_BAT1
powerprofilesctl get
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_driver
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
cat /sys/devices/system/cpu/cpu0/cpufreq/energy_performance_preference
cat /sys/devices/system/cpu/intel_pstate/status
cat /sys/devices/system/cpu/intel_pstate/no_turbo
cat /sys/class/thermal/thermal_zone0/temp
cat /sys/class/thermal/thermal_zone1/temp
uptime
cat /proc/loadavg
free -h
ps -eo pid,stat,comm,%cpu,%mem,args --sort=-%cpu
```

## Candidate Debug build and stress

```sh
cmake -S . -B /tmp/crexx-mthread-linux-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCREXX_VM_HANDLER_PANEL=profile-20 \
  -DCREXX_VM_PROFILING=OFF

cmake --build /tmp/crexx-mthread-linux-debug --parallel 6 --target \
  persistent_worker_executor-rxvml \
  persistent_worker_executor-rxbvml \
  persistent_worker_executor-rxtvml
```

The first build stopped when GCC rejected
`__atomic_always_lock_free(sizeof(uint32_t), 0)` as a file-scope array bound.
After replacing the Linux proof with `__GCC_ATOMIC_INT_LOCK_FREE == 2` plus the
size check, the same build command succeeded.

The first Release object inspection later found a handler stack canary and
reachable `__stack_chk_fail`. After adding the handler-only
`no_stack_protector` attribute, the same target build was repeated in Debug and
Release before the final tests below.

```sh
ctest --test-dir /tmp/crexx-mthread-linux-debug \
  -R '^persistent_worker_executor-' \
  --repeat until-fail:20 --output-on-failure --timeout 120
```

## Candidate ordinary Release build and focused tests

```sh
cmake -S . -B /tmp/crexx-mthread-linux-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_HANDLER_PANEL=profile-20 \
  -DCREXX_VM_PROFILING=OFF

cmake --build /tmp/crexx-mthread-linux-release --parallel 6 --target \
  rxbvm rxtvm crexx library classlib_native rx_treemap_static.a \
  rx_llist_static.a \
  persistent_worker_executor-rxvml \
  persistent_worker_executor-rxbvml \
  persistent_worker_executor-rxtvml \
  benchmark_awfy_sieve_opt_artifact \
  benchmark_awfy_permute_opt_artifact \
  benchmark_rexxcps_levelb_opt_artifact

ctest --test-dir /tmp/crexx-mthread-linux-release \
  -R '^persistent_worker_executor-' \
  --output-on-failure --timeout 120
```

The matrix tool's two generated self-tests were run before timing and both
printed `PASS: compact cross-runtime matrix tool self-test`.

## Cancellation latency

```sh
/tmp/crexx-mthread-linux-release/interpreter/persistent_worker_executor-rxbvml \
  --doorbell-latency \
  /tmp/crexx-mthread-linux-release/interpreter/test_persistent_worker_executor.rxbin \
  1000 > /tmp/crexx-mthread-linux-release-rxbvm-latency-final.log 2>&1

/tmp/crexx-mthread-linux-release/interpreter/persistent_worker_executor-rxtvml \
  --doorbell-latency \
  /tmp/crexx-mthread-linux-release/interpreter/test_persistent_worker_executor.rxbin \
  1000 > /tmp/crexx-mthread-linux-release-rxtvm-latency-final.log 2>&1
```

## Frozen E4 control

```sh
cmake -S /tmp/crexx-e4-linux-control-src \
  -B /tmp/crexx-e4-linux-control-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_HANDLER_PANEL=profile-20 \
  -DCREXX_VM_PROFILING=OFF

cmake --build /tmp/crexx-e4-linux-control-release --parallel 6 \
  --target rxbvm rxtvm
```

The control commit, both control VMs, both candidate VMs, the candidate matrix
driver, library and three RXBIN images were hash-bound before timing.

## Formal paired timing

The first block used `manifest.txt`, one warmup and 12 recorded balanced pairs:

```sh
systemd-inhibit --what=idle:sleep \
  --who='CREXX PERF3-13 E5 Linux qualification' \
  --why='Retained paired Release timing' \
  /tmp/crexx-mthread-linux-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest \
    performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/manifest.txt \
  --output-dir \
    performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/timing \
  --measurement timing --warmups 1 --runs 12
```

The retained `summarize_paired_12.crexx` is the Level B macOS reducer with only
its comment and expected variant label mechanically changed to
`linux-doorbell-poc`. Its output is `paired-summary.csv`.

Five groups crossed zero or a guard, so `manifest-append-1.txt` appended 12
unchanged pairs with no warmup:

```sh
systemd-inhibit --what=idle:sleep \
  --who='CREXX PERF3-13 E5 Linux qualification' \
  --why='Retained paired Release timing append' \
  /tmp/crexx-mthread-linux-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest \
    performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/manifest-append-1.txt \
  --output-dir \
    performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/timing-append-1 \
  --measurement timing --warmups 0 --runs 12
```

The five groups still crossed zero or a guard. The same command with
`manifest-append-2.txt` and `timing-append-2` provided the final 12 pairs to the
36-pair cap.

The 24- and 36-pair Level B reducers are mechanical derivatives of the
retained reducer. Changes are limited to the expected Linux variant, pair
count, loop bound and the two-sided 95% Student-t multiplier: 2.068658 for 23
degrees of freedom and 2.030108 for 35 degrees of freedom.

```sh
/tmp/crexx-mthread-linux-release/bin/crexx \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/summarize_paired_24.crexx \
  --nokeep --args \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/paired-summary-24.csv \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/combined-24-samples.csv

/tmp/crexx-mthread-linux-release/bin/crexx \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/summarize_paired_36.crexx \
  --nokeep --args \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/paired-summary-36.csv \
  performance/evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/combined-36-samples.csv
```

`paired-summary-final.csv` takes the five 36-pair rows plus the already-clear
12-pair Permute/`rxbvm` row. No sample was removed.

## Handler and dispatch audit

The two Release object paths are:

```text
/tmp/crexx-mthread-linux-release/interpreter/CMakeFiles/rxbvm_core_objects.dir/interrupt.c.o
/tmp/crexx-mthread-linux-release/interpreter/CMakeFiles/rxtvm_core_objects.dir/interrupt.c.o
```

Commands used for both objects and linked VMs include:

```sh
nm -aS --size-sort OBJECT
readelf -sW OBJECT
readelf -rW OBJECT
objdump -drwC OBJECT
objdump -dwC --disassemble=rxvm_thread_doorbell_poc_handler BINARY

git diff 295a6d886b33b161e57d71bc641970e394f58f66 HEAD -- \
  interpreter/rxvmintp.h interpreter/rxvmintp.c
git rev-parse \
  295a6d886b33b161e57d71bc641970e394f58f66:interpreter/rxvmintp.h \
  HEAD:interpreter/rxvmintp.h
nm -S --size-sort BINARY | rg ' run$| rxvm_run_owned_core$'
objdump --no-show-raw-insn -dwC --disassemble=run BINARY
```

For the normalized `run` comparison, only load addresses, branch/call target
addresses and RIP-relative layout addresses/comments were removed. Mnemonics,
operands and symbolic call targets remain. Both control/candidate pairs have
the same normalized SHA-256 and compare byte-for-byte equal.

## Final status and closure

```sh
git diff --check
git status --short
git diff --binary -- \
  interpreter/CMakeLists.txt interpreter/interrupt.c \
  interpreter/rxvmexecutor.c > SOURCE-DIFF.patch
sha256sum ... > SOURCE-SHA256SUMS
sha256sum ... > ARTIFACT-SHA256SUMS
find . -type f ! -name SHA256SUMS -print0 | sort -z | \
  xargs -0 sha256sum > SHA256SUMS
```
