# PERF3-13 Gate E E5 clean macOS native-doorbell PoC

Status: **macOS physical-delivery hypothesis passes; industrial E5 and
portable backend selection remain open**

This is the retained evidence for the clean `mthread` branch. The branch is E4
plus one Apple-only private proof: a producer targets a persistent RXVM worker
with `pthread_kill(SIGURG)`, and a bounded handler ORs `CANCEL` into the
worker's existing execution-local interrupt word. The ordinary E4 dispatch
edge is unchanged; there is no external-event poll, hot flag, worker-count
test, targetability branch, instruction counter or dual-loop selector.

The PoC executor and its header are private test sources. Its CMake targets are
created only on Apple when testing is enabled. It changes no public RXVML API,
plugin ABI, RXAS/RXBIN format or product worker surface. The product changes
are limited to the private `CANCEL` signal mapping and Apple doorbell support in
the existing signal owner.

## Physical mechanism result

- The focused normal Debug executor panel passes all three library forms for
  20 consecutive repetitions each. It covers two simultaneous workers, fixed
  affinity, infinite-loop and infinite-recursion cancellation, worker-private
  globals, copied arguments, bounded-queue backpressure, failure isolation,
  no cancellation spill, drain/join and zero runtime leaks.
- Ordinary profiling-off Release passes the same 3/3 panel.
- The complete normal Debug build passes all 2,039 CTests in 263.93 seconds.
- A fresh supported Apple AddressSanitizer build passes 3/3 with
  `detect_leaks=0`; Apple LeakSanitizer is unavailable, while the executor's
  runtime teardown checks retain the zero-live-allocation assertion.
- The complete profiling-off Release build succeeds and the combined E4/E5
  focused panel passes 6/6.
- `rxbvm` completes 1,000 consecutive infinite-loop cancellations at 6 us
  median, 7 us p95 and 12 us maximum request-to-terminal latency.
- `rxtvm` completes 1,000 at 6 us median, 6 us p95 and 9 us maximum.
- Apple Clang's TLS form was rejected because the handler called a Mach-O TLV
  resolver. The retained fixed 64-slot stack-range scan contains no handler
  call, allocation, lock, log or TLS resolver.

## E4 local-owner Release verdict

The ordinary profiling-off Release products are compared with the frozen E4
controls over Sieve, Permute and canonical RexxCPS on both concrete engines.
Every cell has one warmup and 12 pairwise-balanced recorded rounds. All 156
processes pass and no 3% workload guard fires. Percentages are paired means;
positive elapsed and negative RexxCPS rate are adverse.

| Workload | `rxbvm` | `rxtvm` |
|---|---:|---:|
| Sieve elapsed | +1.115666%, clear bounded adverse | -0.723607%, inconclusive |
| Permute elapsed | -2.102904%, clear favourable | +0.000219%, inconclusive |
| RexxCPS rate | -1.193320%, clear bounded adverse | +0.048081%, inconclusive |

The bounded layout movement is well inside the programme guard and smaller
than the discarded combined carrier experiments. This selects the native
doorbell for portable proof while preserving the E4 hot-loop design.

## Interpretation boundary

Direct `CANCEL` is a physical stand-in, not the industrial protocol. Production
E5 must publish a correlated mailbox event and generation before ringing one
internal doorbell bit, then validate and drain it in the cold route. It must
also complete copied logical register-image requests and typed completions,
prioritise `CANCEL`/`KILL`/shutdown, enforce deadlines and quarantine, and join
deterministically. Linux must repeat the POSIX ABI and generated-code proof;
Windows must prove special user-mode APC delivery and runtime fallback. Gate F
may reuse a host-local doorbell beneath its channels, but its transport and
open wire protocol remain separate.

The selected design is recorded in
[`PERF3-13-E5-NATIVE-DOORBELL-DESIGN.md`](../../PERF3-13-E5-NATIVE-DOORBELL-DESIGN.md).

## Evidence map

- `timing/`, `manifest.txt` and `paired-summary.csv`: retained 156-process
  E4-versus-PoC verdict;
- `rxbvm-latency.log` and `rxtvm-latency.log`: 2,000 raw cancellation samples;
- `debug-focused-stress.log` and `release-focused.log`: private executor
  correctness;
- `debug-full-build.log`, `debug-full-ctest.log`, `release-full-build.log`,
  `release-e4-e5-focused.log`, `asan-build.log` and `asan-focused.log`: broad
  and sanitizer closeout;
- `interrupt-disassembly.txt` and `interrupt-symbols.txt`: generated Apple
  arm64 handler objects and symbol inventories;
- `SOURCE-SHA256SUMS` and `ARTIFACT-SHA256SUMS`: exact source/product owners;
  and
- `SHA256SUMS`: checksum closure for this evidence bundle.
