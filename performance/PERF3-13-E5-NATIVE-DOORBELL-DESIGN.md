# PERF3-13 E5 native thread-doorbell design

Date: 2026-08-11

Status: **POSIX physical hypothesis passes on macOS and Intel Linux in the
private E5 PoC; Windows native delivery passes its private PoC; the corrected
sparse targetable-worker fallback, compiler matrix, industrial mailbox
integration and portable backend selection remain open**

## Decision question

Can a foreign producer wake a CPU-running RXVM worker through the operating
system, allowing the target handler/callback to set the worker's existing
execution-local interrupt mask, without adding any poll or capability branch
to the E4 local dispatch path?

The hypothesis is **yes on supported native-thread platforms**:

- POSIX uses a process-private, thread-directed signal delivered with
  `pthread_kill()`;
- Windows 11 build 22000 / Server 2022 or later may use a special user-mode APC
  queued with `QueueUserAPC2(..., QUEUE_USER_APC_FLAGS_SPECIAL_USER_APC)`.

A targetable worker on a platform without a safe prompt target-thread callback
uses the separately selected sparse compatibility owner described below. The
sparse experiment was rejected as the primary carrier on macOS, Linux and
capable Windows 11 because native delivery leaves the E4 loop unchanged. It was
not rejected as the portability fallback where native delivery is unavailable.
The earlier experimental source and its exact opcode ledger were not retained
in a branch, reflog, stash, worktree, evidence bundle or recoverable source
blob, so its fallback design must be reconstructed from the recorded learning,
the current instruction set and new measurements.

This is a physical delivery hypothesis, not a new Rexx signal, public worker
API, message ABI or Gate F transport decision.

## Required hot-path invariant

Non-targetable/local execution and native-doorbell-capable targetable workers
execute the accepted E4 owner:

```text
if (pending_interrupts != 0) -> existing cold interrupt route
```

There is no worker-count read, targetability branch, external atomic load,
instruction counter or semantic-safepoint poll in the ordinary dispatch edge.
Starting a second thread does not change an unrelated VM and does not require
the first VM to switch loops. A non-targetable context always remains on E4.
A targetable worker also remains on E4 when POSIX thread signals or Windows
special APCs can inject the existing execution-local interrupt word.

## Notification and message separation

The native callback is a **doorbell only**. It never transports a request,
completion, copied register image, cancellation reason or shutdown envelope.
The producer first publishes a correlated event in stable worker-owned mailbox
storage and then rings the doorbell:

```text
producer                         target worker
--------                         -------------
publish event + generation
lease live worker handle
native thread-directed wake  ->  minimal handler/callback
                                  set one owner-local VM interrupt bit
                               -> next E4 dispatch enters cold route
                                  claim mailbox with acquire ordering
                                  validate request generation
                                  map CANCEL/KILL/shutdown/completion work
release handle lease
```

Signals and APCs may coalesce. Correctness is therefore level-triggered by the
mailbox/event word: one callback is sufficient to make the cold route drain all
currently published work. A wake which races with a claim either joins that
claim or leaves the mailbox armed for a later callback. A stale notification
must not cancel the next request; request generation and arm/disarm state are
validated in the cold route.

## POSIX backend

- Reserve one internal signal which is not exposed as a Rexx/host signal.
- Install one process-wide `sigaction` handler and direct it to a retained
  `pthread_t` with `pthread_kill()`.
- Publish the active execution-local mask before unblocking the signal on the
  owner thread. Block it while moving/clearing that publication and during
  final teardown.
- The handler performs no allocation, locking, logging, plugin/runtime call or
  graph/context traversal. It performs only the proven lock-free mailbox/local
  flag operation needed to enter the existing cold route.
- A compound handler OR must not race an interrupted cold read/modify/write on
  the same mask. Production either reserves a one-writer internal wake flag or
  blocks the private signal around cold mask claims, clears and slot migration.
  This protection is cold-only and does not alter E4 dispatch.
- Worker-handle generation and a bounded writer lease prevent `pthread_t`
  reuse and stack-slot destruction while a producer or handler is in flight.
- A signal delivered while no execution slot is armed leaves mailbox work
  pending for the next execution entry; it must not be redirected to another
  VM.

The retained macOS PoC uses private `SIGURG` and the direct `CANCEL` bit as a
physical stand-in. It installs the handler only while a private test executor
is live, registers at most 64 worker stack ranges, blocks `SIGURG` while a
worker is idle or changes its active execution pointer, and drains a late
pending doorbell before request retirement. Apple Clang's TLS access was
rejected because it emitted a Mach-O TLV resolver call in the handler; the
retained bounded stack-range scan emits no call, allocation, lock or TLS
resolver. Production qualification still requires the correlated mailbox
bridge, generation check and cold-path mask exclusion described above.

## Windows backend

`QueueUserAPC2` special user-mode APCs can interrupt a native Windows thread
which is executing user code without an alertable wait. If the target is in a
system call or non-alertable wait, the callback runs after that operation
finishes. The API must be discovered at runtime; older Windows and WoW require
the separately selected compatibility backend.

Special APCs cannot be blocked and may nest. The callback must therefore be
strictly re-entrant and restricted to stable worker storage plus compatible
`Interlocked` operations. The executor already retains the thread `HANDLE`
returned by `_beginthreadex`; lifetime and generation validation must extend
that ownership until no callback can execute.

References:

- <https://learn.microsoft.com/windows/win32/api/processthreadsapi/nf-processthreadsapi-queueuserapc2>
- <https://learn.microsoft.com/windows/win32/api/processthreadsapi/ne-processthreadsapi-queue_user_apc_flags>

## Blocking and process boundaries

A native callback is prompt for CPU-running bytecode but does not make an
uncooperative native/plugin call safely cancellable. The existing deadline,
quarantine and eventual join rules remain authoritative. A process worker
continues to use a framed pipe/socket for envelopes and an OS signal/event only
as the prompt notification. Cross-host Gate F likewise retains an open framed
transport; native doorbells are host-local optimisations.

## macOS and Intel Linux proof result

The clean `mthread` PoC establishes:

1. a `pthread_kill()` callback executes on the selected running worker and
   stops an otherwise infinite bytecode loop through the E4 local mask;
2. recursion, simultaneous workers, repeated/coalesced cancellation, no spill
   into the next request and join/teardown remain correct;
3. callback and cancellation latency are retained with raw samples;
4. the ordinary local VM dispatch edge contains no new operation, and local
   Sieve, Permute and RexxCPS remain within the retained E4 workload guard; and
5. the handler's generated code and all functions it reaches satisfy the
   bounded async-handler contract on Apple Clang.

The retained clean-branch evidence records 3/3 focused tests in both normal
Debug and ordinary profiling-off Release, 1,000 consecutive cancellations per
concrete engine, 6 us median cancellation latency on both engines, and a
156-process E4-versus-PoC Release comparison with no 3% workload guard hit.
The complete Debug suite and focused Apple AddressSanitizer results are part of
the branch closeout record, not prerequisites for interpreting the physical
delivery result.

Intel Linux repeats the proof with GCC 15.2.0 and Clang 21.1.8. Linux resolves
stack bounds outside the handler with `pthread_getattr_np()` and
`pthread_attr_getstack()`. Both compilers pass focused Debug stress, focused
profiling-off Release, 1,000 cancellations per concrete engine and the
generated-code/async-signal-safety audit. Their handler objects call nothing,
have no TLS/runtime traversal, and ring the same E4 word without changing
ordinary dispatch.

The Linux E4-versus-PoC timings are accepted as an overall
**noisy/inconclusive performance result and physical-PoC pass**. Individual
clear favourable rows remain recorded but do not support a general benefit
claim; the observed guard hits remain inconclusive because their intervals
cross zero on a visibly hot and loaded host. Adrian accepts this as no
demonstrated harm at PoC precision and as useful stressed-host functional
evidence. A future compiler or production-performance selection must start
after a reboot on a thermally settled, quiet, reserved host and pass a short
drift/noise pilot.

Windows must separately prove `QueueUserAPC2`, nested callback safety, runtime
API fallback and both concrete VM forms. Passing both POSIX hosts does not
authorize a portable production backend, public executor API, worker/channel
syntax or Gate F transport.

## Windows compatibility owner selection

Adrian approved the Windows compatibility-owner investigation on 2026-08-13
and corrected the owner model after its first physical PoC. Targetability and
native-delivery capability are separate properties. Runtime selection is
immutable for a worker context and occurs before `rxvm_prepare()`:

| Execution kind | Native prompt delivery | Selected owner |
| --- | --- | --- |
| non-targetable/local context | irrelevant | accepted E4 owner |
| targetable persistent worker | POSIX thread signal or Windows special APC | accepted E4 owner |
| targetable persistent worker | unavailable | sparse compatibility owner |

A prepared or executing worker never changes owners. The sparse owner must be
selected before the direct-threaded execution image is prepared so its handler
addresses, or an equivalent compiler-specific duplicate owner, are fixed for
that worker. It must not jump between E4 and compatibility loops while running
and must not reinterpret every direct-threaded instruction through a generic
numeric switch.

The alternatives remain bounded as follows:

- ordinary APCs are rejected because CPU-running bytecode does not enter an
  alertable wait;
- adding an external atomic read or loop selector to the accepted E4 dispatch
  edge is rejected because it taxes non-targetable and native-capable
  execution; and
- an acquire load at every compatibility dispatch is rejected by the Windows
  W6 physical PoC. It was functionally correct and fast to observe cancellation
  but slowed the forced fallback by about 64% in `rxbvm` and 102% in `rxtvm`
  and grew product executables by about 11%. Those figures characterize only
  that superseded every-instruction owner, not the sparse design.

## Sparse fallback learning and instruction coverage

The earlier macOS experiment established that sparse observation was
functionally effective. The retained source and exact measured instruction
list are unavailable. Adrian's retained design recollection confirms checks at
procedure returns and taken backward branches. Review of the current RXBIN 007
instruction set adds the call boundary needed to cover unbounded recursion and
resolved indirect backedges needed to cover jump-table loops. This distinction
is important: returns and backedges are recovered design knowledge; the call
and indirect-edge requirements are the current functional-completeness review.

The compatibility owner therefore observes its stable worker-owned external
event word only at these semantic progress points:

1. once on entry to an armed bytecode request;
2. before a taken control-flow edge whose resolved target is at or before the
   current instruction, including unconditional, conditional, counted, fused
   comparison/status and private cleanup branches;
3. before transfer at a bytecode call boundary, including direct, dynamic and
   fixed-arity/fused call forms, so recursion cannot avoid observation;
4. at bytecode return boundaries (`RET`, `RET_REG`, `RET_INT`, `RET_FLOAT` and
   `RET_STRING`); and
5. after a native/plugin call returns to bytecode. No cooperative design can
   interrupt an uncooperative native/plugin call while that call is executing.

The backedge rule is semantic rather than a stale opcode allow-list. It covers
the current branch families `BR*`, `BCT*`, `BCF*`, integer/float/decimal fused
branches, `UNLINKBR`, status-flag branches and the `JUMPS`/`JUMPB`/`JUMPBS`/
`JUMPI`/`JUMPR`/`JUMPN` table forms whenever their selected target is backward.
Forward-only branches do not poll: in the absence of recursion they make finite
progress to a call, return, exit or backedge. Terminal `EXIT*` forms need no
poll because request completion and cancellation are serialized by the request
arm/disarm authority.

The current RXBIN 007 audit snapshot is:

- branch forms eligible when the resolved edge is backward or a self-edge:
  `BR_ID`, `BRT_ID_REG`, `BRF_ID_REG`, `BRTF_ID_ID_REG`, `BEQ_ID_REG_REG`,
  `BEQ_ID_REG_INT`, `BNE_ID_REG_REG`, `BNE_ID_REG_INT`, `UNLINKBR_REG_ID`,
  `FGTBR_ID_REG_REG`, `FLTBR_ID_REG_REG`, `IGTBR_ID_REG_REG`,
  `ILTBR_ID_REG_REG`, `BCT_ID_REG`, `BCT_ID_REG_REG`, `BCTNM_ID_REG`,
  `BCTNM_ID_REG_REG`, `BCTP_ID_REG`, `BCF_ID_REG`, `BCF_ID_REG_REG`,
  `BGT_ID_REG_REG`, `BGT_ID_REG_INT`, `BGE_ID_REG_REG`, `BGE_ID_REG_INT`,
  `BLT_ID_REG_REG`, `BLT_ID_REG_INT`, `BLE_ID_REG_REG`, `BLE_ID_REG_INT`,
  `DGTBR_ID_REG_REG`, `DLTBR_ID_REG_REG`, `DEQBR_ID_REG_REG`,
  `BRTPT_ID_REG`, `BRTPANDT_ID_REG_INT`, `JUMPS_REG_BINARY`,
  `JUMPB_REG_BINARY`, `JUMPBS_REG_REG_BINARY`, `JUMPI_REG_BINARY`,
  `JUMPR_REG_BINARY` and `JUMPN_REG_BINARY`;
- actual bytecode call forms: `CALL_FUNC`, `CALL_REG_FUNC`,
  `CALL_REG_FUNC_REG`, `DCALL_REG_REG_REG`,
  `SWAPCALL_REG_FUNC_REG_REG_REG`,
  `SETTPSWAPCALL_REG_FUNC_REG_REG_INT_REG`,
  `SETTPCALL_REG_FUNC_REG_REG_INT`, `CALL1_REG_FUNC_REG`,
  `CALL2_REG_FUNC_REG_REG`, `CALL3_REG_FUNC_REG_REG_REG` and
  `CALL4_REG_FUNC_REG_REG_REG_REG`; and
- return forms: `RET`, `RET_REG`, `RET_INT`, `RET_FLOAT` and `RET_STRING`.

`SIGCALL*` and `SIGBR*` configure signal policy and are not themselves call or
branch transfers, so they are not sparse observation points. An external event
already observed by the local interrupt machinery continues through the
existing cold signal route rather than creating a second polling policy.

This set must be checked mechanically against `binutils/include/rxops.h`, the
actual `VM_SELECT_*` sites and private execution-image rewrites. New call,
return or branch opcodes must join the semantic classification instead of
silently escaping cancellation. Focused tests must include unconditional,
conditional, counted and indirect infinite loops, infinite recursion, nested
returns, native-call return, simultaneous workers and cancellation followed by
worker reuse.

The compatibility carrier remains only a direct `CANCEL` stand-in. The
producer publishes the stable event with release ordering; the sparse owner
observes it with acquire ordering and enters the existing local interrupt
route. Request completion clears the event while holding the same request
arm/disarm mutex used by cancellation, so a late event cannot spill into the
next request. The cancellation latency claim is bounded by time to the next
semantic progress point, not by an instruction count.

The corrected first verdict must report ordinary E4 generated-code and
performance invariance for non-targetable and native-capable execution, sparse
fallback correctness and latency, all progress-point families, both engine
forms, GCC/Clang/MSVC coverage, and the compatibility owner's throughput,
code-size and build-cost penalty. No product/public backend selection follows
until that verdict is accepted.

Evidence:
[`macOS`](evidence/2026-08-12-perf3-13-gate-e-e5-macos-doorbell-poc/),
[`Linux GCC`](evidence/2026-08-12-perf3-13-gate-e-e5-linux-doorbell-poc/) and
[`Clang plus compiler comparison`](evidence/2026-08-12-perf3-13-gate-e-e5-linux-clang-doorbell-poc/).
The combined-source and Windows continuation handoff is retained in
[`Windows 11`](evidence/2026-08-13-perf3-13-gate-e-e5-windows-doorbell-poc/).

## Windows 11 sparse-fallback PoC verdict — 2026-08-13

The corrected implementation selects the owner at runtime from targetability
and native-delivery capability. Non-targetable contexts and targetable workers
with working special-APC delivery use the unchanged E4 owner. A targetable
worker for which the special-APC install/probe fails selects the duplicate
sparse owner before `rxvm_prepare()`. There is no executing-loop transition or
rethreading when a request or OS thread changes state.

The sparse owner retains the ordinary inline/outline placement and observes
the external cancellation word only at the semantic points above. Both the
switch and computed-goto owners implement the policy. The exact RXAS fixture
`interpreter/tests/test_persistent_worker_sparse.rxas` proves cancellation on
a conditional self-edge, counted self-edge and resolved indirect self/backedge,
then proves worker reuse through `RET`, `RET_FLOAT`, `RET_STRING`, `RET_REG` and
`RET_INT`. The normal persistent-worker fixture separately proves unconditional
loop and recursive-call cancellation, simultaneous workers, stress, no spill,
drain/join and teardown. Native/plugin-return observation remains a code-path
audit at each `RXVM_EXTERNAL_SAFEPOINT()`; an executing native call is still
outside the cooperative guarantee.

Final same-machine compiler qualification is green:

- MinGW GCC ordinary profiling-off Release: 19/19 focused tests pass across
  `rxvml`, `rxbvml` and `rxtvml`;
- MSVC 19.44 Debug: 13/13 focused tests pass across the two switch products;
- Clang 22.1.8, MSVC ABI, Debug: 19/19 focused tests pass across switch and
  computed-goto products; and
- the four core products (`rxc`, `rxas`, `rxlink` and the configured `rxvm`)
  build with all three compilers. The real `rxc.exe` also compiles the former
  access-violation fixture in optimized and no-opt modes.

The MSVC and Clang/MSVC-ABI product builds use `ENABLE_PARSER_MODE=OFF` because
the optional sibling `DSL-Syntax-Highlighter` dependency still directly
includes POSIX `unistd.h`. MinGW GCC qualifies the default parser-enabled
configuration. Making that external parser integration native-MSVC portable is
separate Windows dependency work, not an E5 or sparse-owner defect.

The GCC Release latency repeat passes 1,000 forced-fallback cancellations per
engine. `rxbvml` records 2.9 us median and 3.8 us p95; `rxtvml` records 3.0 us
median and 3.2 us p95. The retained 12-pair 25-million-iteration comparison
measures sparse fallback versus native delivery at +16.09% paired mean /
+14.89% median for `rxbvml`, and +5.69% mean / +5.07% median for `rxtvml`.
A post-portability six-pair confirmation records +21.13% mean / +19.42%
median and +4.62% mean / +5.86% median respectively. These are targetable
fallback costs only; they do not characterize non-targetable or native-capable
threads, which continue to execute the E4 owner. Product growth versus W5 is
about 19.4% for `rxbvm` and 20.0% for `rxtvm`, accepted as the expected cost of
the duplicate owner.

The Windows access violation was not Control Flow Guard or another Windows 11
security feature. The new `rxvm_active_state.compatibility_interrupts` member
was omitted from `rxinimod_common()`, so stack and malloc-backed embedded RXVML
contexts could enter the sparse selector through an indeterminate pointer. The
field is now initialized explicitly and `test_rxvmactive` poisons context
storage before initialization to prevent recurrence. Current `origin/develop`
has neither the field nor the split handler owner, so this initializer repair
must travel with the mthread change rather than be pushed independently.

Clean MSVC/Clang builds also exposed branch-independent Windows portability
work in the compiler: portable unused annotations, `_stricmp` mappings in
place of POSIX `strings.h`, byte-pointer rather than GNU `void *` arithmetic,
and MSVC-ABI Clang recognition when omitting `libm`. Those source defects are
present on current `origin/develop` and are suitable for a separate reviewed
develop commit. The handler-wrapper preprocessing and label-namespace repairs
depend on the mthread split-handler refactor, which is not yet on develop.

This remains a private physical/cancellation-owner PoC. It does not publish an
executor API, complete the industrial event mailbox, authorize Gate F, or
select a public worker/channel design.
