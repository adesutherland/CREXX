# PERF3-13 Gate E E5 Intel Linux Clang native-doorbell PoC

Status: **accepted Intel Linux Clang physical-delivery PoC pass; focused
correctness and generated-code safety pass; performance result accepted as
overall noisy/inconclusive with two guards**

## Verdict

Clang 21.1.8 proves the same Linux POSIX mechanism as the accepted GCC run. A
producer targets a persistent RXVM worker with `pthread_kill(SIGURG)` and the
selected worker's bounded handler ORs `CANCEL` into the existing E4
execution-local interrupt word. Infinite loops and recursion stop on both
concrete engines; isolation, drain/join and teardown remain correct; all 2,000
retained latency cycles complete.

The Clang x86-64 handler is 114 bytes. It contains no call, allocator, lock,
log, TLS resolution, stack-canary edge or other runtime traversal. Its only
relocations are two PC-relative references to the fixed slot array. The
ordinary E4 dispatch source is unchanged and the normalized generated `run`
stream is identical to the Clang E4 control for both engines.

Adrian accepts the campaign as an **overall noisy/inconclusive performance
result and successful physical PoC**, not as a guard-clean production
qualification. Sieve/`rxtvm` and Permute/`rxtvm` hit the 3% guard at the
36-pair cap, but both confidence intervals cross zero on a hot, busy host. The
two individually clear favourable RexxCPS rows remain exactly recorded but are
not promoted into a general benefit claim. Recommendation: **retain** the
private Linux backend and proceed to Windows proof. Repeat runtime compiler
selection only after rebooting and thermally settling a quiet, reserved host.

For day-to-day Linux development of this PoC, Clang is the stronger compiler
choice from the evidence available: the controlled fresh two-VM Release build
was 3.240x faster, used 62.16% less peak RSS and produced binaries about 33.5%
smaller than GCC. Runtime results are workload-dependent and too noisy to name
an overall code-speed winner, so GCC should remain a required validation
compiler.

## Provenance and dirty state

- Branch: `mthread`.
- Tested commit and `origin/mthread`:
  `b6edf2556f2411fe5033049e32ee77ddd9a2e15f`.
- Frozen E4 control: `295a6d886b33b161e57d71bc641970e394f58f66`.
- The original Linux work started from a clean worktree. This Clang repeat
  intentionally tested the same accepted, uncommitted Linux source patch.
- Tracked diff: `interpreter/CMakeLists.txt`, `interpreter/interrupt.c` and
  `interpreter/rxvmexecutor.c`; 92 insertions, 28 deletions.
- Clang required no further product-source change. This evidence directory is
  the only Clang-specific addition.
- No commit, push, merge, rebase, install or package operation was performed
  during qualification; the status and patch records describe that capture.

`STATUS.txt` records the final branch/status and `SOURCE-DIFF.patch` is the
exact tracked Linux patch.

## Linux implementation used by Clang

The implementation is identical to the accepted GCC proof and preserves the
Apple path:

1. The private targets are enabled for Apple and Linux; CTest labels remain
   platform-specific.
2. Linux defines `_GNU_SOURCE` and discovers each worker's immutable stack
   bounds outside the handler with `pthread_getattr_np()`,
   `pthread_attr_getstack()` and `pthread_attr_destroy()`. The slot is
   published while SIGURG is blocked. Apple continues to use
   `pthread_get_stackaddr_np()` and `pthread_get_stacksize_np()`.
3. The Linux lock-free compile proof uses
   `__GCC_ATOMIC_INT_LOCK_FREE == 2` plus a size check. Clang defines and
   satisfies that GNU compatibility macro; the generated active load is
   inline and has no `libatomic` call.
4. `no_stack_protector` applies only to the handler. No public workers,
   channels, mailbox protocol, syntax, ABI, Gate F, APC or alternate
   interpreter loop is introduced.

## Host and builds

- Acer Aspire A515-56G; Intel Core i5-1135G7; 4 cores / 8 threads; x86-64.
- Ubuntu 26.04 LTS; kernel `7.0.0-29-generic`; glibc 2.43.
- Ubuntu Clang 21.1.8 (`6ubuntu1`), POSIX thread model; CMake 4.2.3; Ninja
  1.13.2. The controlled compiler comparison used GCC 15.2.0.
- 18 GiB RAM, 8 GiB swap; AC online; balanced profile; intel_pstate active;
  powersave governor; balance_performance preference; turbo enabled.
- Candidate and controls are ordinary profiling-off `Release`, handler panel
  `profile-20`. Debug and Release builds explicitly select `/usr/bin/clang`
  and `/usr/bin/clang++`.
- Every build and measurement uses explicit `/tmp/...` build-tree tools. No
  installed `~/.local` tool or library is used.

The host was not suitable for a clean performance verdict. Thermal readings
were 83-91 C; load averages were material; the Codex/ChatGPT renderer and GPU
commonly consumed roughly 90-115% combined CPU. Every initial absolute timing
cell exceeded the governance MAD or span rerun threshold. Balanced pairs and
all extensions are retained. Wide intervals and guards are not waived; they
are accepted as inconclusive within the limited physical-PoC decision. The
stressed host is useful functional robustness evidence but not a clean timing
baseline.

## Focused correctness and stress

- Fresh Clang Debug: 20 consecutive repetitions of each private `rxvml`,
  `rxbvml` and `rxtvml` fixture pass: 60/60, 42.97 seconds.
- Fresh ordinary Clang Release: the same panel passes 3/3: 0.29, 0.57 and
  0.47 seconds respectively; 1.63 seconds total.
- Each fixture execution covers two simultaneous workers, fixed affinity,
  infinite-loop and infinite-recursion cancellation, private globals, copied
  arguments, bounded-queue backpressure, queued cancellation, failure
  isolation/recovery, no cancellation spill, drain/join and zero live runtime
  allocations at teardown.
- The 1,000 consecutive cancellation cycles per concrete engine exercise
  repeated delivery. Coalescing is physically safe here because the handler's
  action is the idempotent level operation `pending |= CANCEL`; this does not
  provide an industrial event count or payload protocol.

No broad Clang CTest, sanitizer, install or package validation was run.

## Cancellation latency

Latency is request-to-terminal time for a newly submitted infinite-loop
request. All raw samples are retained.

| Engine | Samples | Minimum | Median | p95 | Maximum |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxbvml` | 1,000 | 14.601 us | 26.016 us | 70.305 us | 132.770 us |
| `rxtvml` | 1,000 | 21.504 us | 53.167 us | 86.875 us | 300.131 us |

An initial manual `rxbvml` latency invocation accidentally omitted
`CREXX_VM_POC_DOORBELL=posix`. Native cancellation was therefore refused and
the intentional infinite loop could not terminate. It was stopped after
diagnosis and excluded; the command error and re-ring diagnostic are retained.

Against the accepted GCC session, Clang's `rxbvml` median is informally 32.98%
lower while its `rxtvml` median is 79.72% higher. Those opposing, unmatched
host sessions are not compiler guarantees.

## Generated-code and hot-edge findings

- The two Clang Release `interrupt.c.o` files are byte-identical.
- The 114-byte handler scans the fixed 64-slot table two slots at a time and
  finishes with the direct `orl $0x20000,(%rax)` E4 ring.
- Complete object and linked disassembly show no call, PLT transfer,
  allocation, lock, logging, TLS resolver/relocation, stack canary or
  unexpected runtime access. Everything reachable from the handler is data.
- `rxvmintp.h` has the same Git blob at E4 and the tested commit. The only
  E4-to-tested `rxvmintp.c` edits are cold CANCEL string/parse/default mappings;
  the Linux dirty patch changes neither interpreter-loop file.
- `run` remains 0x114 bytes in all four Clang control/candidate binaries. The
  normalized instruction/control-flow streams compare byte-for-byte equal.
  No poll, atomic read, worker-count test, targetability branch, instruction
  counter or alternate loop was added to ordinary dispatch.

See `audit/handler-static-checks.txt`, `audit/interrupt-disassembly.txt`,
`audit/interrupt-symbols-relocations.txt` and
`audit/e4-dispatch-structural-proof.txt`.

## E4 versus PoC accepted Release observation

The retained protocol used one warmup and 12 balanced pairs. Only groups whose
interval crossed zero or a guard were extended to 24 and then to the 36-pair
cap. RexxCPS/`rxbvm`, already clear at 12, was not oversampled. All 396 timed
processes passed; no sample was removed.

Candidate-versus-E4 percentages follow. Positive elapsed is adverse; positive
RexxCPS rate is favourable.

| Workload | Engine | Pairs | Paired median | Paired mean (95% CI) | Result | 3% guard |
| --- | --- | ---: | ---: | ---: | --- | --- |
| Sieve elapsed | `rxbvm` | 36 | -10.107010% | -7.908859% (-16.587004%, +0.769287%) | noisy/inconclusive | no |
| Sieve elapsed | `rxtvm` | 36 | +3.398039% | +7.046621% (-1.196086%, +15.289327%) | noisy/inconclusive | **yes** |
| Permute elapsed | `rxbvm` | 36 | -2.380586% | -0.244517% (-5.715827%, +5.226793%) | noisy/inconclusive | no |
| Permute elapsed | `rxtvm` | 36 | +3.111048% | +4.794044% (-1.605351%, +11.193440%) | noisy/inconclusive | **yes** |
| RexxCPS rate | `rxbvm` | 12 | +5.240705% | +23.181818% (+0.082115%, +46.281521%) | clear favourable | no |
| RexxCPS rate | `rxtvm` | 36 | +10.877128% | +14.522069% (+1.257936%, +27.786202%) | clear favourable | no |

`paired-summary-final.csv` is the decision table. The initial, 24-pair and
36-pair reducers/summaries and every raw timing file are retained separately.

Overall performance disposition: **noisy/inconclusive, accepted for the
physical PoC**. The favourable RexxCPS rows, adverse guard point estimates and
cross-zero intervals are all retained without selecting a preferred subset.

## GCC versus Clang: informal view

The build comparison is the reliable part: fresh Clang and GCC E4 trees used
the same frozen source, Release flags, profile-20 panel, profiling-off setting,
targets and `-j6` concurrency.

| Measure | Clang 21.1.8 | GCC 15.2.0 | Clang difference |
| --- | ---: | ---: | ---: |
| Configure wall | 20.16 s | 16.10 s | 25.22% slower |
| Build `rxbvm` + `rxtvm` wall | 108.39 s | 351.13 s | **69.13% less; 3.240x faster** |
| Build peak RSS | 258,744 KB | 683,840 KB | **62.16% less** |
| E4 `rxbvm` file | 1,345,728 B | 2,031,744 B | 33.76% smaller |
| E4 `rxtvm` file | 1,371,464 B | 2,061,536 B | 33.47% smaller |

The build remains a host observation and compiler versions differ, but its
magnitude and same-source design make Clang the practical developer-build
preference on this machine. Both compilers emitted only the same pre-existing
`rxspawn.c` discarded-const warning.

Runtime is mixed. Comparing the two initial 12-pair sessions informally,
Clang is much slower on Sieve, faster on Permute, and produces higher RexxCPS
rates. Every absolute cell in both sessions was noisy, the sessions occurred at
different times, and Debug/Release focused fixture timings also contradict one
another. `compiler-runtime-informal.csv` retains the exact medians and ratios.
This evidence supports no general runtime winner.

Before repeating compiler/runtime selection, reboot the host, allow thermal
and load state to settle, keep it on AC with low-power mode off, close or move
heavy GUI/rendering and background-update activity, reserve the machine, and
run a short drift/noise pilot before the governed campaign.

## Interpretation boundary and retained files

This remains a physical-delivery PoC. It does not prove the industrial mailbox
protocol, generation/correlation, priorities, deadlines, quarantine, public
workers/channels, transport, Windows delivery, syntax or ABI.

- `pre-*.txt` / `post-*.txt`: host state and bound hashes.
- `logs/`: Clang builds, controlled GCC build comparison and focused tests.
- `timing*` / `combined-*`: all raw and combined benchmark samples.
- `rxbvm-latency.log` / `rxtvm-latency.log`: all 2,000 latency samples.
- `audit/`: object, disassembly, dispatch and compiler-build evidence.
- `COMMANDS.md`: exact build/test/measurement/audit commands.
- `SHA256SUMS`: checksum closure for this bundle.
