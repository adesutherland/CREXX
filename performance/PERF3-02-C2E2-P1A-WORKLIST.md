# PERF3-02-C2E2-P1A assembler-cost recovery worklist

Status: **complete — A1 retained; A3 rejected and replayable**

Approved: 2026-08-01

Purpose: recover the bounded assembly-time cost introduced by the retained P1
typed-continuation and symbolic-storage infrastructure without changing RXAS,
RXBIN, runtime behaviour, diagnostic point identity or the meaning of any
existing rewrite.

## Authority and mandatory stop

Adrian approved P1A and the later bounded PERF3-05 and PERF3-03 panels on
2026-08-01. P1A may refactor internal RXAS flow/storage implementation and add
focused proof. It must not install a storage-aware rewrite, delete a tactical
rule, alter signal semantics, change a public format/ABI, or push.

After the first production edit, run the minimum focused correctness proof,
freeze implementation, build ordinary profiling-off Release `rxas`, run the
smallest decisive paired assembly comparison, report the verdict and stop for
acceptance before PERF3-05 begins.

## Exact starting point

- Branch/HEAD: local `develop` at
  `4a3940395980dc40ea45917d71d99caa080e89bb`, three commits ahead of
  `origin/develop`.
- Dirty scope: only five protected untracked generated lifecycle RXBIN files;
  do not stage, overwrite or delete them.
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs, macOS 26.5.2.
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2.
- Ordinary Release: `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`.
- Pre-P1 Release `rxas` oracle:
  `/tmp/crexx-c2e2-p1-baseline.VosQ4p/rxas-baseline`, SHA-256
  `af80b2c9f664a2ed1b7ec7a4cca5a419718f2f5384e3cfb880f86c7a06ad7422`.
- Locked P1 Release `rxas`: `cmake-build-release/bin/rxas`, SHA-256
  `09ea8e49c54dfc839833a1fa40dfa5809467aa53c730dc050ae0a2b95f73669e`.
- Richards input SHA-256
  `ac1737cc7d6d7ba59a1a3771d1c6d977b974f5d5c23fa5f10c032f0d00c259d9`;
  Towers input SHA-256
  `35ead6d2a0313e64986370c803bf960cc20a73bf5b1813d9c0e8d2ac2a7c3e4e`.
- Selected linked-image oracles remain
  `d046069d8de8d82c41bf994cd5188b0a78dfac00ebb740dd25254b0d5bf5a5b6`
  for Richards and
  `2a89a49830608e755fe2eddb678e4efe4e783587b7e1f4d6bb625c9183dfa267`
  for Towers.

The previously quoted P1 cost is Richards `+5.3%` median and Towers `+0.5%`
median (`+2.0%` mean) over 30 balanced pairs. Its raw evidence was not
published separately; P1A must retain a checksum-closed baseline/current/
candidate comparison and close that explicit P1 worklist obligation.

## Falsifiable hypothesis and target

The dominant ordinary-build cost is eager construction of the complete
point-by-register storage environment after all rewrites, even though the
current product has no storage-aware rewrite consumer and non-debug assembly
cannot observe the result. Demand-driven attachment should retain the exact
service and debug proof while avoiding dead ordinary work.

The primary success target is:

1. byte-identical ordinary Richards, Towers and focused fixture RXBIN;
2. identical debug storage/edge summaries and runtime behaviour;
3. candidate assembly median no more than 1% slower than the preserved pre-P1
   oracle on Richards and Towers, with all raw balanced observations retained;
4. no material candidate-vs-locked-P1 regression; and
5. no weakening of the future consumer entry: a selected consumer can demand
   the same complete graph-owned analysis before querying it.

The pre-P1 binary is a machine ceiling, not a semantic candidate: it lacks the
typed edge/storage infrastructure and exists only to quantify recovered cost.

## Compared designs

### A0 — retain unconditional eager storage attachment

Keep the final whole-procedure graph rebuild and full storage analysis on every
optimized assembly. This is the emitted-image and diagnostic oracle, but pays
for an analysis whose result is immediately freed in non-debug assembly.

### A1 — demand-driven graph-owned attachment

Retain typed edges, every transfer/query helper and the exact bounded storage
service. Attach it only when debug diagnostics or a separately selected
rewrite consumer requests it. The current non-debug path has no consumer, so
it avoids only dead analysis work. This is the leading minimal candidate.

### A2 — eager sparse or compressed point state

Keep unconditional attachment but compress the `node x register` environment
or track a proved relevant subset. This may retain eager availability, but it
adds indexing/meet complexity and risks losing the all-register point-identity
contract. Prototype only if A1 does not recover the target or if evidence shows
the ordinary eager result has an independent consumer.

### A3 — feature-gated typed CFG construction

Build signal-skip/retry edges only when a storage consumer is active. This may
recover allocation/traversal cost across every rewrite iteration, but changes
ordinary graph availability and couples legacy consumers to a graph mode. It
is a fallback design, not the first implementation.

Adrian authorized A3 rework on 2026-08-01 after reviewing the neutral A1
verdict. The bounded implementation keeps A1 demand-driven storage attachment,
builds normal-only graphs for the existing fixed-point rewrite consumers, and
requests typed signal continuations explicitly for the final debug/storage
graph. Graph mode is recorded and `flow_storage_attach()` fails closed when
typed continuations were not requested. No existing rewrite is permitted to
consume signal edges in this slice.

## Proof obligations

- Debug storage summaries for the focused fixture, Richards and Towers are
  exact before/after, including normal/skip/retry edge counts and point state.
- Optimized and no-opt focused runtime tests pass on both VMs.
- Every focused and representative ordinary output is byte-identical.
- Demand state is explicit and fail-closed: a future consumer cannot query an
  unattached analysis accidentally.
- Allocation failure/bounds and graph lifetime remain unchanged.
- No optimizer rule, public operation, serialized format, TRACE/source record,
  signal handler or VM path changes.

## Work stages

### Stage A — baseline and attribution

- [x] Freeze commit, dirty scope, host, tools and artifact hashes.
- [x] Record A0/A1/A2/A3 and the hard Release stop.
- [x] Preserve the locked P1 `rxas` outside the build tree.
- [x] Capture a balanced pre-P1/P1 diagnostic comparison and exact outputs.
- [x] Attribute storage analysis, typed-edge construction and final graph
      rebuild separately enough to select A1, A2 or A3.

### Stage B — bounded implementation

- [x] Implement only the selected recovery.
- [x] Add or refine focused tests for demand/fail-closed semantics if needed.
      Existing debug-only identity and optimized/no-opt dual-VM tests exercise
      the complete contract; no new public or test-only switch was needed.
- [x] Run focused Debug RXAS/storage/runtime correctness: 24/24 pass.
- [x] Prove debug diagnostic and emitted-image equivalence for the focused
      fixture, Richards and Towers.

### Stage C — mandatory first Release verdict

- [x] Freeze implementation and build ordinary profiling-off Release `rxas`.
- [x] Run at least one warmup and 12 balanced/interleaved recorded pairs for
      pre-P1, locked-P1 and candidate on Richards and Towers; extend only under
      the governed noise rule.
- [x] Retain raw elapsed samples, outputs, identities and environment state in
      one checksum-closed P1A evidence bundle.
- [x] Report the verdict and stop for Adrian's rework-or-accept decision.

### Stage C2 — authorized A3 rework and repeated verdict

- [x] Preserve separate pre-P1, locked-P1 and A1 Release binaries.
- [x] Demand-gate typed signal-edge construction without changing normal-edge
      rewrite behaviour or the final debug/storage graph.
- [x] Prove the same 24 focused tests, exact debug diagnostics and byte-identical
      fixture/Richards/Towers RXBIN outputs.
- [x] Run a balanced same-session Release comparison of pre-P1, locked-P1, A1
      and A3 on Richards and Towers, retain raw samples and report the verdict.
- [x] Stop for Adrian's acceptance before accepted closeout or PERF3-05.

### Stage D — accepted closeout only

- [x] Reconcile the P1 evidence checkbox and P1A roadmap status.
- [x] Run proportional broad validation only after verdict acceptance.
- [x] Review the final diff; commit and push remain separately authorized.

## Current decision record

The initial 30-pair diagnostic reproduced the retained direction on the exact
current inputs: locked P1 versus pre-P1 was `+3.393%` paired median on Richards
and `+0.832%` on Towers; outputs were byte-identical. Code attribution shows
that the final graph rebuild already existed before P1, typed edges are needed
by the retained service, and the newly unconditional `flow_storage_attach()`
is the complete point-environment work whose non-debug result is immediately
freed. A1 is selected for the bounded candidate. A2 and A3 remain fallbacks if
A1 misses the recovery target; neither is installed speculatively.

The mandatory three-way Release verdict used two warmups and 30 recorded,
six-order-counterbalanced rounds per variant/workload. A1 is correct but misses
the recovery target:

- Richards candidate versus locked P1: `-0.141%` paired median, `+0.180%` mean,
  95% mean interval `[-4.021%, +4.381%]`;
- Richards candidate versus pre-P1: `+3.679%` paired median, `+3.603%` mean,
  95% interval `[-1.779%, +8.986%]`;
- Towers candidate versus locked P1: `-0.945%` paired median, `-2.791%` mean,
  95% interval `[-6.048%, +0.466%]`.

No outliers were removed. The candidate does not demonstrate recovered
Richards cost, so it remains provisional and no PERF3-05 work starts. A2/A3
rework or acceptance of the retained P1 cost requires Adrian's direction.

Adrian selected A3 rework on 2026-08-01. A1 remains locked as a separate
correctness and performance oracle; its checksum-closed evidence is immutable.
A2 remains rejected for this rework because compressing an eager environment
cannot remove the typed-edge construction repeated by the current fixed-point
graphs and adds state/indexing risk without an ordinary consumer.

The A3 repeated verdict used two warmups, all 24 four-variant permutations and
the governed 12-pair balanced extension, for 36 rounds per workload/variant.
A3 is correct and byte-identical, and the focused matrix passes 24/24, but it
does not demonstrate recovered cost:

- Richards A3 versus pre-P1: `+2.556%` paired median, `+1.401%` mean,
  95% mean interval `[-1.615%, +4.417%]`;
- Richards A3 versus A1: `+1.460%` median, `+0.509%` mean,
  95% interval `[-2.287%, +3.305%]`;
- Towers A3 versus pre-P1: `+0.509%` median, `+0.140%` mean,
  95% interval `[-2.942%, +3.222%]`; and
- Towers A3 versus A1: `+0.250%` median, `-0.144%` mean,
  95% interval `[-2.710%, +2.422%]`.

No outlier was removed. Every interval remains ambiguous at the governed
maximum, so A3 is `noisy/inconclusive`, not a selectable improvement. The
same-session A1 control is only `+0.154%` median versus pre-P1 on Richards and
`+0.523%` on Towers, which weakens the earlier cost attribution. The evidence
supports reverting A3 and retaining A1 if Adrian accepts the bounded P1 cost;
the provisional A3 source remains frozen pending that decision. Evidence:
[`2026-08-01 A3 first Release verdict`](evidence/2026-08-01-perf3-02-c2e2-p1a-a3-first-release-verdict/).

Adrian accepted the recommended disposition on 2026-08-01. A3 is rejected and
its evidence/design remains replayable; the production source is restored
exactly to A1. P1A proceeds through the shortest accepted closeout before
PERF3-05 opens.

Accepted closeout reproduces the retained A1 source diff and Release binary
exactly (`7cec2f1245da24cf24ffd37ec94faba0309dfc41f088d5b94c382522c0e9258d`),
passes the focused matrix 24/24 and passes broad Debug CTest 1,972/1,972. The
checksum-closed closeout is
[`2026-08-01 P1A closeout`](evidence/2026-08-01-perf3-02-c2e2-p1a-closeout/).
The evidence closeout itself contains no commit or push. Adrian subsequently
authorized the combined local programme commit; push remains unauthorized.
