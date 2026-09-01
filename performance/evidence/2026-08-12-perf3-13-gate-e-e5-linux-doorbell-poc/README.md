# PERF3-13 Gate E E5 Intel Linux native-doorbell PoC

Status: **accepted Intel Linux GCC physical-delivery PoC pass; focused
correctness and generated-code safety pass; performance result accepted as
overall noisy/inconclusive, not guard-clean**

## Verdict

The Linux POSIX mechanism works: a producer targets a persistent RXVM worker
with `pthread_kill(SIGURG)`, and the selected worker's bounded handler ORs
`CANCEL` into the existing E4 execution-local interrupt word. Infinite loops
and infinite recursion stop on both concrete engines, worker isolation and
teardown remain correct, and 2,000 retained cancellation samples complete.

The generated Linux x86-64 handler also satisfies the PoC's strict signal
contract. It is 95 bytes in both Release objects, calls nothing, has no TLS
access or stack-protector failure edge, and reaches only the fixed slot array
and selected pending word. The E4 dispatch macro source is byte-identical to
the frozen control, and the normalized generated `run` stream is identical on
both engines.

The performance campaign is accepted as an **overall noisy/inconclusive result
and a physical-PoC pass**, not as a guard-clean production qualification. At
the 36-pair cap, Sieve on `rxtvm` has an adverse +8.097073% paired mean and
trips the 3% workload guard, but its wide 95% interval crosses zero
(-1.532315% to +17.726461%). Permute/`rxbvm` is individually clear favourable;
that exact classification remains in the table but is not promoted into a
general benefit claim. The visibly hot and loaded host prevents clean causal
performance attribution.

Adrian accepts the combination of functional stress, unchanged ordinary
dispatch and retained paired evidence as showing no demonstrated performance
harm at PoC precision. Recommendation: **retain** this private Linux physical
backend and proceed to Windows proof. Repeat any production or compiler-speed
selection only after rebooting and thermally settling a quiet, reserved host.

## Provenance and dirty state

- Tested branch: `mthread`.
- Tested commit and `origin/mthread` at final capture:
  `b6edf2556f2411fe5033049e32ee77ddd9a2e15f`.
- Frozen E4 control: the branch parent
  `295a6d886b33b161e57d71bc641970e394f58f66` in a detached worktree.
- The worktree was clean before fetch/switch and before editing.
- Final tracked diff: three files, 92 insertions and 28 deletions:
  `interpreter/CMakeLists.txt`, `interpreter/interrupt.c`, and
  `interpreter/rxvmexecutor.c`.
- At measurement capture this Linux evidence directory was untracked. No
  commit, push, merge, rebase, install or package operation was performed
  during qualification.

`STATUS.txt` records the final branch/status, and `SOURCE-DIFF.patch` is the
exact tracked dirty patch.

## Linux implementation differences

The existing Apple behavior is preserved behind the same private PoC:

1. Linux enables the private executor targets and labels them `linux`; Apple
   still uses its `macos` label.
2. `_GNU_SOURCE` exposes `pthread_getattr_np()`. Each worker resolves its stack
   range before it publishes its slot and before SIGURG is unblocked, using
   `pthread_getattr_np()`, `pthread_attr_getstack()` and
   `pthread_attr_destroy()`. The base, size and overflow are validated. Apple
   continues to use `pthread_get_stackaddr_np()` and
   `pthread_get_stacksize_np()` through the same outside-handler helper.
3. GCC's compile-time lock-free proof uses
   `__GCC_ATOMIC_INT_LOCK_FREE == 2` with a size check; Apple retains
   `__atomic_always_lock_free()`.
4. Linux running cancellation uses the existing `pthread_kill(worker->thread,
   SIGURG)` path. No public API, syntax, ABI, RXAS/RXBIN format or product
   worker/channel surface changes.
5. `no_stack_protector` is attached only to the handler. Ubuntu's Release
   hardening otherwise inserted a canary and reachable `__stack_chk_fail`,
   which violated the async-handler closure. Final objects contain neither in
   the handler.

The first GCC build also rejected `__atomic_always_lock_free()` as a file-scope
constant expression under this GNU99 configuration. The retained GCC macro
proof fixes that build-only issue. Both discoveries and their final rebuilds
are retained in `logs/`.

## Host and build

- Acer Aspire A515-56G, Intel Core i5-1135G7, 4 cores / 8 threads, x86-64.
- Ubuntu 26.04 LTS, kernel `7.0.0-29-generic`, glibc 2.43.
- GCC 15.2.0 (`Ubuntu 15.2.0-16ubuntu1`), CMake 4.2.3, Ninja 1.13.2.
- 18 GiB RAM and 8 GiB swap.
- AC online, battery full, `balanced` profile, `intel_pstate` active,
  `powersave` governor, `balance_performance` preference, turbo enabled.
- Candidate and control use ordinary `Release`, `-O3 -DNDEBUG`, handler panel
  `profile-20`, and `CREXX_VM_PROFILING=OFF`.
- All builds and measurements use explicit `/tmp/...` build-tree tools. No
  `~/.local` binary or library is used.

The formal timing session is qualified by substantial host noise. Pre/post
loads were roughly 1.4-3.7, package thermal readings were 85-90 C, and the
Codex/ChatGPT renderer and GPU processes consumed about 90-105% combined CPU.
Every absolute timing cell exceeded the governance MAD or span rerun threshold.
The balanced same-session pairs are retained. The remaining wide intervals and
Sieve/`rxtvm` guard are not waived; they are accepted as inconclusive within
the deliberately limited physical-PoC decision.

## Focused correctness and stress

- Fresh normal Debug: the three `rxvml`, `rxbvml` and `rxtvml` private tests
  pass 20 consecutive repetitions each: 60/60 executions, 187.71 seconds.
- Fresh ordinary profiling-off Release: the same focused panel passes 3/3.
- Each fixture execution covers two simultaneous workers, fixed affinity,
  infinite-loop and infinite-recursion cancellation, worker-private globals,
  copied arguments, bounded-queue backpressure, queued cancellation, failure
  isolation and recovery, no cancellation spill into the next accepted
  request, drain/join, and zero live runtime allocations at teardown.
- Repeated delivery is additionally exercised by 1,000 consecutive
  infinite-loop cancellation cycles per concrete engine. Duplicate/coalesced
  SIGURG delivery is safe for this physical proof because the handler action
  is the idempotent level operation `pending |= CANCEL`; a signal does not
  carry a count or payload. The kernel's actual coalescence count is not
  observable in this fixture. An industrial mailbox/generation protocol is
  still required before this property can carry multiple logical events.

No broad CTest, sanitizer, install or package validation was run before the
first Linux verdict, as required. After Adrian accepted the PoC, the affected
Debug products and all three private executor fixtures rebuilt successfully.
A broad 2,039-test Debug closeout was then attempted, but the stressed host did
not produce a valid suite verdict: `linked_opt_runtime_artifacts_build` hit its
1,500-second timeout after generating 279/619 artifacts, dependent tests became
Not Run, and `parse_levelg_contract` separately hit its 120-second timeout
under concurrent load. The run was stopped at 37/2,039 rather than retuning or
rebooting this host. No E5 assertion or executor failure had appeared. This is
retained as an incomplete host-capacity result, not claimed as a broad pass or
as a product regression; broad closeout remains for a stabilized Linux host.

## Cancellation latency

Latency is request-to-terminal time for a newly submitted infinite-loop
request. All 2,000 samples pass; raw values are retained.

| Engine | Samples | Minimum | Median | p95 | Maximum |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxbvml` | 1,000 | 22.365 us | 38.817 us | 80.924 us | 160.883 us |
| `rxtvml` | 1,000 | 16.123 us | 29.583 us | 69.270 us | 227.732 us |

These are host-specific observations, not portable latency guarantees.

## Generated-code and hot-edge findings

Both Release `interrupt.c.o` files are byte-identical. The handler is a fixed
64-slot stack-range scan followed by an inline load/OR/store of `0x20000`.
There is no `call`, allocation, lock, log, TLS segment access, PLT reference,
stack canary or unexpected runtime access. Its only range relocation is one
PC-relative `.bss` reference to the fixed slot array; because it calls no
function, its reachable-function closure is the handler itself.

The ordinary E4 edge gains no poll, atomic read, worker-count test,
targetability branch, instruction counter or alternate interpreter loop:

- `interpreter/rxvmintp.h`, which owns `RXVM_DISPATCH_PREPARE()` and
  `RXVM_OWNER_DISPATCH()`, has the same Git blob in E4 and the candidate.
- The only E4-to-`mthread` `rxvmintp.c` differences are CANCEL string mapping
  and cold frame-default initialization.
- No Linux dirty edit touches either interpreter-loop file.
- `run` remains `0x151` bytes in every control/candidate engine and its
  normalized generated instruction/control-flow stream is identical.
- `rxvm_run_owned_core` moves by 0x40 bytes on `rxbvm` and 0x80 bytes on
  `rxtvm`, consistent with cold mapping/layout movement; the paired verdict
  therefore remains necessary.

See `audit/handler-static-checks.txt`, `audit/interrupt-disassembly.txt`, and
`audit/e4-dispatch-structural-proof.txt`.

## E4 versus PoC accepted Release observation

There was one warmup and 12 balanced/interleaved recorded pairs initially.
Only groups whose interval crossed zero or a guard were extended by 12 pairs,
then by a final 12 to the 36-pair cap. The already-clear Permute/`rxbvm` group
remains at 12. All 396 benchmark processes pass. No sample was removed; this
includes the retained +137.441946% Permute/`rxtvm` pair.

Percentages below are candidate versus E4 paired changes. Positive elapsed is
adverse; positive RexxCPS rate is favourable. The interval is the two-sided
95% Student-t interval around the paired mean.

| Workload | Engine | Pairs | Paired median | Paired mean (95% CI) | Result | 3% guard |
| --- | --- | ---: | ---: | ---: | --- | --- |
| Sieve elapsed | `rxbvm` | 36 | +0.070784% | -0.610879% (-7.295202%, +6.073444%) | noisy/inconclusive | no |
| Sieve elapsed | `rxtvm` | 36 | +1.194171% | +8.097073% (-1.532315%, +17.726461%) | noisy/inconclusive | **yes** |
| Permute elapsed | `rxbvm` | 12 | -3.985344% | -9.353255% (-17.753068%, -0.953441%) | clear favourable | no |
| Permute elapsed | `rxtvm` | 36 | -2.731573% | +1.151642% (-8.487004%, +10.790289%) | noisy/inconclusive | no |
| RexxCPS rate | `rxbvm` | 36 | -1.409184% | +0.384760% (-5.085653%, +5.855172%) | noisy/inconclusive | no |
| RexxCPS rate | `rxtvm` | 36 | +0.272753% | -0.223597% (-4.692424%, +4.245229%) | noisy/inconclusive | no |

Overall performance disposition: **noisy/inconclusive, accepted for the
physical PoC**. The one clear favourable row is retained honestly, while the
guard hit and wide intervals prevent either a general benefit or regression
claim.

## Interpretation boundary and remaining risks

This proves physical Linux delivery only. Direct CANCEL is not an industrial
mailbox, and the PoC does not add public workers/channels, a correlation or
generation protocol, priorities, deadlines, quarantine, Gate F transport,
Windows APC support, language syntax or ABI changes.

Remaining risks are the unresolved Sieve/`rxtvm` guard on this noisy host,
untested Linux/libc/compiler/architecture variants, the private 64-worker slot
limit, and the fact that kernel signal coalescence is handled structurally but
not counted directly. The Apple source path is preserved but could not be
rebuilt on this Linux host. Production must use a stable mailbox/generation
and block or otherwise serialize the private signal around cold mask claims so
a logical event cannot be lost or applied to a later request.

For the next formal timing campaign, reboot first; wait for thermal and load
state to settle; use AC with low-power mode off; close or move heavy GUI,
renderer, update and indexing activity; reserve the host against other work;
and run a short drift/noise pilot before beginning governed samples. This run
remains useful stressed-host robustness evidence, not a clean performance
baseline.

## Evidence map

- `pre-run.txt`, `post-run.txt`, `pre-append-*.txt` and `post-append-*.txt`:
  host, power, thermal, load, process and artifact state.
- `debug-focused-stress.log`, `release-focused.log`, `rxbvm-latency.log` and
  `rxtvm-latency.log`: focused correctness and 2,000 raw latencies.
- `timing/`, `timing-append-1/`, `timing-append-2/`, manifests, combined sample
  files and `paired-summary-final.csv`: all 396 Release processes and capped
  paired interpretation.
- `audit/`: object/link disassembly, symbols, relocations, handler closure
  checks and E4 dispatch/generated-code comparisons.
- `logs/`: configure/build/control/self-test records, including the two Linux
  portability findings, successful focused rebuilds, and the incomplete
  post-acceptance broad-Debug attempt.
- `SOURCE-DIFF.patch`, `SOURCE-SHA256SUMS`, `ARTIFACT-SHA256SUMS` and
  `SHA256SUMS`: exact dirty patch and checksum closure.
- `COMMANDS.md`: reproducible command record.
