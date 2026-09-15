# S3-D01 — Native worker startup transition repair proposal

Status: Adrian approved implementation and accepted the first Release cost on
2026-09-14. Native model and broader normal Debug qualification pass locally.
Parent scope: [STEP-03](native-inference-step-03.md).
All original OUT/NI/AC/STEP obligations survive. The preceding instruction to
hold sanitizer builds/tests remains in force. Adrian subsequently approved
STEP-03 closure with SAN-009 and remaining S3-D01 sanitizer proof assigned to
STEP-06 native-inference release QA, owned by Codex under his direction.

## Vision and outcome

Native cREXX programs must be able to start worker pools while preserving the
existing protection for legacy process-shared plugins. This is required for
llama.rexx's multiple private worker sessions over shared immutable weights.
Dynamic cREXX workers already pass the corresponding ownership checks. The
native route must have equivalent observable behavior without weakening legacy
callback serialization or changing the language/plugin ABI.

## Reproduced mechanism

The provider-free `worker_transport_positive.crexx`, compiled with `crexx -native`,
times out at startup. `provider_worker_acceptance.crexx` does the same. Thus the
failure does not depend on llama.cpp, a model or the new provider.

The native package retains legacy provider registrations from the linked standard
library. The parent is counted as an active legacy-capable executor throughout
`run()`. `channel_local_open()` waits in `rxvm_executor_create_attached()` for its
worker's startup. The worker replays the static provider catalogue; registering
the second legacy-capable VM waits in `rxpa_compatibility_bind_legacy()` for the
parent to leave execution. Neither can proceed. The retained
[native stack sample](../qa/native-inference-step03/native-worker-startup-sample.txt)
shows both ends; the
[provider-free negative control](../qa/native-inference-step03/native-worker-positive-red.log)
records the independent timeout. Loading the dynamic controls from a complete
installed bytecode search directory succeeds; a missing `classlib.rxbin` is a
separate harness configuration error, not this deadlock.

A simple `execution_leave`/`execution_enter` around the wait is insufficient:
a nested VM can leave another active VM on the same waiting OS thread, and a
suspended legacy callback can still hold process-shared C state. Neither may be
mistaken for safe quiescence.

## Numbered checkable acceptance criteria

1. **S3-D01-AC-01:** The ordinary provider-free native worker positive control
   completes with its success marker; the unrepaired implementation times out.
2. **S3-D01-AC-02:** Native foreign-handle rejection and four actual cREXX workers
   sharing BGE/Smol weights with private preparation pass on local CPU/Metal.
3. **S3-D01-AC-03:** A deterministic coordinator regression proves that suspended
   execution at worker startup permits the cold transition, active execution
   still blocks it, and all resumed legacy invokers are correctly rebound.
4. **S3-D01-AC-04:** Nested same-thread VM execution is covered. Starting this
   blocking worker operation while a legacy callback is still on the C stack
   returns a bounded failure and cleans up, rather than declaring its shared
   state quiescent. This explicit behavior requires approval.
5. **S3-D01-AC-05:** Existing legacy serialization, recursive re-entry,
   process-reentrant and session-affine controls remain passing. Run matching
   Debug/maintained sanitizer checks and retain any required first ordinary
   Release verdict before broad closeout of a changed native-call path.

## Numbered implementation steps

1. **S3-D01-01:** Promote the native positive reproducer and deterministic
   transition/nesting controls into permanent ordinary failing regressions.
   Measure any nested aggregate in normal Debug and maintained ASan before
   assigning its serial scheduling and timeout.
2. **S3-D01-02:** Track the executing VM contexts on each OS thread at cold
   execution entry/leave boundaries. Track legacy C callback nesting using a
   separately load-bound legacy adapter; preserve direct process-reentrant and
   session-affine dispatch and the existing recursive serialization policy.
3. **S3-D01-03:** Add an internal suspend/resume boundary around synchronous
   attached-worker startup. Suspend all active contexts on that thread only
   when no legacy C callback is on the stack; otherwise return an explicit
   provider failure. Resume on every success/failure path after any cold
   transition has finished. No public ABI, opcode or syntax change is proposed.
4. **S3-D01-04:** Run focused coordinator, actual native-worker and RXPA policy
   checks. If the hot native-call path changes, follow the repository's first
   ordinary Release verdict rule before broad qualification or tuning.
5. **S3-D01-05:** Complete appropriate broader correctness/sanitizer checks,
   update the interpreter/library protocol documentation, and reconcile STEP-03
   and every still-open full-plan criterion. Keep this repair independently
   reviewable from the provider implementation.

## Selected implementation and first Release check

1. The status quo deadlocks on the retained native positive control. Omitting
   linked legacy providers would mask this case while leaving legitimate mixed
   legacy/worker applications broken. A single-context leave/enter does not
   account for nested same-thread VMs or live legacy callback state.
2. The approved choice tracks each executing context once in an allocation-free
   thread-local list at outer execution entry/leave. Startup temporarily parks
   that entire list without changing execution depths. A load-bound legacy
   adapter tracks callback depth; compatibility-lock ownership also prevents
   suspension inside protected provider initialization/payload callbacks.
   Reentrant and session-affine procedure adapters retain their direct route.
3. The synchronous attached-executor entry owns suspend/resume, including its
   allocation, worker-start and load-failure exits. Unsafe callback nesting
   returns the existing worker-start failure, mapped to channel provider failure.
   This adds no public status, ABI, instruction or language syntax.
4. The first Release comparison is a non-representative single-mechanism check:
   unchanged RXPA legacy call workload plus reentrant/session controls using the
   compiler-selected ordinary profiling-off `rxvm`, one warmup and 12 balanced
   paired rounds. Historical August binaries/samples do not establish a current
   matched baseline; retain a fresh pre-edit product and exact inputs before
   editing core code. Do not run a broad portfolio or claim RexxCPS coverage.
   Report any legacy call overhead separately from the repaired startup.
5. New nested native qualification stays an explicit target until both Debug
   and maintained sanitizer isolation measurements are allowed and complete.
   Permanent coordinator tests are direct C tests, not nested build aggregates.

The original approval boundary came from [AGENTS.md](../../AGENTS.md): "Pause for user
approval before making language-design decisions, syntax changes, or
architectural shifts." Treating a new safe-quiescence boundary and nested legacy
callback failure behavior as an architectural change is the implementation
agent's interpretation of that rule; the original provider approval explicitly
did not imply a VM architecture expansion.

## First Release verdict and current handoff — 2026-09-14

The implementation is frozen and uncommitted. The new ordinary attached-startup
and callback tests both timed out at 30 seconds before the repair. After it,
all 14 focused Debug RXPA/coordinator tests pass in 0.69 seconds, and the
original provider-free native cREXX worker transport control completes with its
success marker. There were no sanitizer runs or sanitizer builds.

The first ordinary Release comparison has 12 paired rounds per path, on Apple
M5/AC power with low-power mode off, and no reported thermal warning. The legacy
20-million-call workload has a **+2.846764% mean paired elapsed-time change**
(95% interval +1.454974% to +4.238554%). Its difference of elapsed medians is
35.3535 ms, equivalent to about 1.77 ns/call for this workload. The point estimate
is below the 3% guard but its interval crosses it; this is a measured cost,
not a claim of zero overhead. Reentrant and session-affine controls have no
statistically clear change (-0.529833% and -0.400618% mean paired changes).
No sample was excluded and no follow-on tuning or broad qualification occurred
before the first Release decision.

**Accepted, 2026-09-14:** after clarification that the measured cost is per
legacy call, Adrian accepted/approved the result and continuation of the
remaining native-model and broader Debug qualification. The session-affine llama.rexx adapter
does not use the newly tracked direct legacy route. This is a non-representative
call-mechanism result, not an inference throughput or overall product verdict.
The first Release decision gate is satisfied; no timing rerun or tuning is
needed. The [evidence bundle](../qa/native-inference-s3d01/README.md) retains
raw samples, commands, source/build identities and the pre-repair failures.

| Criterion | Disposition |
| --- | --- |
| S3-D01-AC-01 | Passed locally: original native cREXX control and direct attached-worker regression complete; pre-repair timeout retained. |
| S3-D01-AC-02 | Passed locally in the expanded Debug scratch-install matrix: native foreign-handle rejection and four-worker BGE/Smol CPU/Metal sharing/private preparation. The complete matrix took 114.687 seconds; matching sanitizer proof remains on hold under AC-05. |
| S3-D01-AC-03 | Passed locally: deterministic active-execution/transition and suspended-startup controls; resumed legacy slots are locked. |
| S3-D01-AC-04 | Passed locally: A -> B -> A execution, direct/locked callback and compatibility-lock rejection, worker-start failure and subsequent successful startup/cleanup. |
| S3-D01-AC-05 | Partial: 14 focused Debug controls, the expanded native/package matrix and all 2,314 non-measurement Debug CTests pass; the first Release result is accepted. Maintained sanitizer and required platform qualification remain open. |

S3-D01-01 through -04 are implemented and locally verified, including the
accepted Release decision and native model controls. Broader normal Debug passes
2,314/2,314 in 900.23 seconds after `qa-prep`, excluding `performance-measurement`
tests. Interpreter/library protocol documentation is updated. S3-D01-05 remains
assigned to STEP-06 for maintained sanitizer and required platform qualification.
Bounded implementation phase closure is approved. At takeover, preserve all
parent OUT/NI/AC/STEP requirements, SAN-009's open status and the sanitizer hold.
The authorized native/package and Debug checks are complete; reuse the retained
checks and unchanged timing evidence. Do not run the sanitizer or
repeat unchanged local qualification. STEP-03 is closed with its approved
STEP-06 handoff; SAN-009 remains open. No push or commit
is authorized by this record.
