# PERF3-02 copy/ownership candidate decision

Status: **panel complete; authoritative clean-host timing captured; Adrian
selection required before any production edit**

The original remote-terminal timing remains preserved but provisional. The
authoritative result is the clean-host serial rerun in `rerun/`: 376 recorded
samples plus 36 warmups, with standing absolute-noise and paired-interval
appends applied without removing a sample. Exact final paired statistics are in
[`rerun/paired-effects.csv`](rerun/paired-effects.csv).

## Outcome

Current generic copies are overwhelmingly recursive object work, but they are
not owned by one universal layer. Two separate compiler facts proved useful:

1. C1a-R2 can reuse a nested multi-return receiver only when the callee has no
   calls and neither reads nor writes receiver attributes. It removes 33,245
   Richards top-level copies, 4,910,249 total copy operations and 39,016,032
   logical bytes. Authoritative paired elapsed medians improve 9.18%/9.33% on
   `rxvm`/`rxbvm`; all 22/22 pairs are favorable and the mean 95% intervals are
   `[-9.74%,-8.69%]`/`[-9.94%,-8.53%]`.
2. C1c-R1 can directly use object-formal storage after ordinary argument
   binding has already created an isolated by-value local, while retaining the
   existing alias/generated/reference/array/flow-substitute exclusions. It
   removes 245,620 Towers top-level copies, 7,140,440 total copy operations,
   55,158,560 logical bytes and 202,314 attribute blocks. Paired elapsed
   medians improve 19.42%/19.65% on `rxvm`/`rxbvm`; all 22/22 and 12/12 pairs
   are favorable and the mean 95% intervals are
   `[-19.96%,-18.06%]`/`[-20.78%,-18.55%]`.

Both candidates pass optimized, no-opt and dual-VM focused correctness. Their
opposite workload images are byte-identical guards. All four guard intervals
still cross zero at the 36-pair cap and are therefore labelled
`noisy/inconclusive` for direction, but their upper bounds range from +0.22% to
+0.84%, safely below the +3% workload-regression boundary. No public
RXAS/RXBIN, ABI, language or runtime representation change is required.

## Recommendation for Adrian's selection

**C1c-R1** is the recommended first production candidate. It has the larger
deterministic work removal, the larger confirmed wall-clock effect, the
simplest ownership fact, and the tighter failure boundary: the
formal's private value already exists before the nested receiver decision. A
production change should encode that fact explicitly and add focused tests; it
should not merely copy the six-line experimental diff.

C1a-R2 is the viable independent alternative and remains fully reproducible
for a later separately gated slice. Do not combine C1a and C1c before the first
Release verdict, because doing so would lose independent attribution.

No production candidate is selected by this evidence update. If Adrian selects
C1c-R1, the next authorized slice is only: production-quality
proof/helper and targeted inline tests; minimum focused correctness; freeze;
ordinary profiling-off Release build; paired Towers target plus byte-identical
Richards guard in both VMs; report and stop. Broad CTest, sanitizer, packaging,
platform work and a combined candidate remain after Adrian accepts that
verdict.

## Rejected and deferred routes

- C1a-R1 proves that “receiver not written” is insufficient. Attribute reads
  create receiver-owned links; skipping the private receiver made Richards
  return queue/hold counts `0/1` instead of `23246/9297`.
- The two extra failed C1a-R1 sites expose a material but invalid ceiling:
  172,394 top-level copies, 25,341,738 recursive operations and 201,354,752
  bytes. C1b needs explicit per-exit/unwind link ownership and remains deferred.
- C2-E1 lets the current RXAS fact engine inspect generic copies but accepts
  zero. Richards' reached copy becomes tainted; Towers yields 28 tainted and
  one live-at-effect-barrier rejection. C2 is not a standalone owner without
  compiler semantic ownership/metadata.
- C3's current residual is nine scalar static sites, 32,557 dynamic operations
  and zero logical bytes. It is not material enough to justify a new typed form
  or ISA/ABI decision.
- C4's exact direct-operation ceiling is already met by C1a-R2 and C1c-R1 at
  their proved sites: zero receiver copy, destination allocation, recursive
  traversal or ownership search on the success path.

## Evidence map

- `summary/copy-sites.csv` and `summary/copy-site-payload.csv`: exact C0 static,
  dynamic, source/TRACE and recursive payload attribution;
- `summary/owner-eligibility.csv`: C1-C4 ownership ledger including CRI-13;
- `summary/mechanism-deltas.csv` and `summary/timing-effects.csv`: deterministic
  effects plus preserved provisional and authoritative Release observations;
- `summary/candidate-panel.csv`: proof, semantic risk, scope, maintenance,
  ceiling and disposition;
- `summary/correctness.csv` and `summary/c2-flow.csv`: focused proof and
  assembler negative evidence;
- `poc/`: isolated source diffs, correctness, count profiles and timing;
- `diagnostic/` and `raw/`: checksum replay, RXSEQ/disassembly and site payload
  records; and
- `artifacts.csv`, `provenance.md`, `checksums.sha256`: exact identities and
  recursive closure.
- `rerun/`: clean-host host gates, preserved option matrix, all raw samples,
  governed append manifests, final absolute summary and paired effects.
