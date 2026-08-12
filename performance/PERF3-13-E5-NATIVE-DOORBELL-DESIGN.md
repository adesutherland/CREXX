# PERF3-13 E5 native thread-doorbell design

Date: 2026-08-11

Status: **macOS physical hypothesis passes in the private E5 PoC; industrial
mailbox integration and portable backend selection remain open**

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

A platform without a safe prompt target-thread callback will need a separately
selected compatibility backend. The rejected hot flag, atomic mask, sparse
safepoint and dual-loop experiments are not carried by the `mthread` PoC
branch and are not a selected fallback.

This is a physical delivery hypothesis, not a new Rexx signal, public worker
API, message ABI or Gate F transport decision.

## Required hot-path invariant

`LOCAL` and native-doorbell-capable workers execute the accepted E4 owner:

```text
if (pending_interrupts != 0) -> existing cold interrupt route
```

There is no worker-count read, targetability branch, external atomic load,
instruction counter or semantic-safepoint poll in the ordinary dispatch edge.
Starting a second thread does not change an unrelated VM and does not require
the first VM to switch loops. The macOS PoC therefore uses the ordinary E4 VM
owner for both single-thread and persistent-worker execution.

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

## macOS proof result and remaining selection gates

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

Linux must repeat the POSIX proof on its signal ABI and generated handler code.
Windows must separately prove `QueueUserAPC2`, nested callback safety, runtime
API fallback and both concrete VM forms. Passing macOS does not authorize a
portable production backend, public executor API, worker/channel syntax or
Gate F transport.
