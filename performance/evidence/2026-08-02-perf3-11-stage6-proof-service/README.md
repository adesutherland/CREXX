# PERF3-11 Stage 6 proof service and first authority migration

Status: **complete — Gate 6 and the first consumer verdict accepted**

This bundle closes the reusable proof-query layer and the migration of the
accepted redundant `ITOS` consumer.  The new proof service is the sole
authority for that consumer; the old per-generator solver has been deleted.

The migration standard is deliberately not decision parity.  The legacy
solver supplies a minimum safe-capability baseline and replayable comparison,
while the new write-once SSA proof may establish additional cases.  Every
additional rewrite must be justified by the new proof, covered by focused and
broad correctness, and subjected to the normal output-changing Release gate.

## Provenance

- Branch: `codex/perf3-rxas-flow-infrastructure`.
- Stage 5 base: `d31674f11` (`perf: add sparse RXAS component SSA`).
- Stage 6 source: that base plus the Stage 6 code, tests, documentation and
  this evidence; the resulting local commit is authoritative in Git history.
- No push is authorized or performed.
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs, 24 GiB RAM.
- Power: AC; low-power mode off.  No thermal, performance or CPU-power warning
  was recorded.
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2.
- Product: ordinary profiling-off CMake/Ninja Release.

## Proof infrastructure result

`assembler/rxas_flow_proof.c` owns a per-procedure, per-epoch proof service
over the immutable graph, structural analysis, signal/effect state and sparse
component SSA.  The first public queries cover:

- dominated successful repetition with cached accept/reject reasons;
- instruction speculatability;
- loop must-execute and component-invariance checks; and
- stable proof keys based on symbolic storage rather than register numbers.

Value and effect phis are compared by conservative reduction and equivalent
leaf sets, so a join can preserve a real proof without treating source order
or a numeric ID as semantic equality.  Successful completion and the exact
signal/effect dependencies are part of repetition proof.  Unsupported state,
stale epochs, invalid graphs, allocation failure and work-budget exhaustion
fail closed.

The proof work also closes metadata gaps encountered by the first queries.
Alias-topology changes now have a separate effect identity from mutation that
can be observed through an existing reference.  The VM-backed contracts mark
`CONCAT`/`SCONCAT` non-signalling, `STEMSET`/`STEMSET2` failure-atomic before
writes, and decimal comparisons as plugin-signalling after their integer result
write and success-stable.  An unknown exceptional signal set no longer invents
a normal numeric-context write for an otherwise classified opcode.  Focused
metadata and edge-state tests lock each correction.

The Stage 5 ITOS solver and its private storage-availability walk were removed.
The migrated consumer now asks the generic service whether a prior `ITOS`
dominates the candidate with equivalent storage, integer source, string result,
numeric context and reference-visible effects.  The service, not the deleted
solver, decides the rewrite.

## Correctness discovery and resolution

The initial broad Debug run passed 1,986/1,987 tests and failed only
`inline_summary_version_fallback_binary_opt`.  That was a useful new-proof
failure, not a reason to restore the old solver.  The proof had allowed a
repeated `ITOS` across a range call whose callee mutated caller-owned argument
storage.  The old solver rejected every call globally; the new proof needed the
more precise model.

The sparse SSA state now records range-call windows and explicit `CALL1` to
`CALL4` argument storages.  Explicit arguments receive unknown component
definitions on normal and failure continuations.  A range call uses the
constant count component when it is statically available, otherwise it
conservatively covers all later locals.  Storage outside the argument window
remains provably unchanged.  This is the intended advanced result: precise
caller-owned mutation rather than a global call barrier.

A permanent runtime fixture proves both sides:

- a no-argument call leaves an unrelated local unchanged and permits the
  repeated `ITOS` deletion;
- a range call whose callee increments its actual argument retains the
  post-call `ITOS` and produces `42` in optimized and no-opt images under both
  VMs.

After the correction, the focused matrix passes **10/10** and the complete
Debug suite passes **1,987/1,987**.  Strict GNU90 syntax checks and the ordinary
Release `rxas` build pass.  The canonical RexxCPS candidate hash remains
`beeba2fe...`; therefore the correction did not invalidate the accepted runtime
comparison.

## First ordinary Release verdict

The retained Stage 5 image contains 21 executable `ITOS` instructions.  The
new proof image contains 19: five dominated r51 repetitions are proved in the
primary procedure, while two later candidates fail closed with
`generator-source-unknown`.  The exact static delta is two additional
deletions beyond the old solver's output.

One warmup and 12 balanced/interleaved recorded rounds per cell used the same
current Release `rxvm`, `rxbvm` and linked library for both images.  All 48
recorded executions and all four warmups pass.  No sample was removed and no
rerun was recommended.

| VM | Stage 5 median CPS | Stage 6 median CPS | Change | Favourable pairs |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 43,042,540.5 | 46,257,365.5 | **+7.469%** | 12/12 |
| `rxbvm` | 40,840,816.5 | 43,644,889.5 | **+6.866%** | 12/12 |

Adrian accepted this first Release verdict.  It proves the runtime value of
the stronger proof for this slice; it does not authorize unreviewed expansion
to other operations.

## Assembler cost

The final current Release `rxas` produced the exact accepted RexxCPS image in
five ordinary samples of 382.569 ms (cold anomaly), 53.033 ms, 58.177 ms,
51.633 ms and 57.140 ms; median 57.140 ms.  The retained Stage 5 median was
54.526 ms.  The roughly 2.6 ms median increase remains in the agreed
seconds-scale proof-analysis budget.

The final diagnostic completed in 0.39 s at 20,185,088 B maximum RSS.  Its
primary proof service used 3,913 work units of a 93,696 budget and retained
603,960 B.  It answered seven repetition queries: five proved and two rejected.

## Validation boundary

This accepted proportional closeout includes strict syntax, focused tests,
the complete Debug suite, ordinary Release build, exact image replay, retained
proof decisions and the accepted dual-VM Release verdict.  Sanitizer,
install/package and cross-platform validation were not added: the repository
closeout policy explicitly avoids those unrelated expansions unless a failure
or Adrian requires them.

The next PERF3-11 activity is a separate inventory and baseline of every
remaining legacy proof.  Each will migrate one at a time, preserving the old
safe domain while admitting separately evidenced stronger results.
