# PERF3-11 M04 exact same-storage copy

Status: **complete — first Release verdict accepted 2026-08-03**

M04 replaces the legacy raw-register identity branch and its blanket forward
TRACE scan with the per-procedure proof service. The old accepted
COPY/ICOPY/FCOPY/SCOPY raw-self floor is preserved. The new authority also
proves same-storage DCOPY, ACOPY and BCOPY, LINK-established aliases, an
agreeing storage phi and a no-op copy before a retained TRACE event.

The evidence was produced on branch
'codex/perf3-rxas-flow-infrastructure' from committed M03 base 'c716e2279',
with only the M04 production, tests, documentation, worklists and evidence
changes dirty.
No push was performed.

## Proof boundary

'rxop_same_storage_copy_is_noop()' is the canonical conditional VM contract
for COPY, ICOPY, FCOPY, SCOPY, DCOPY, ACOPY and BCOPY: if both register
operands denote the same physical storage, the instruction performs no write,
allocation, signal or observable cursor/effect update. The conditional
contract is intentionally more exact than the opcode's general signal
contract; in particular, different-storage BCOPY remains conservative.

'rxas_flow_prove_redundant_self_copy()' admits identical physical register
operands directly, preserving the old floor even if entry storage is unknown.
Otherwise it requires equal pre-instruction StorageId, or two storage phis
whose unique write-once leaf is the same StorageId. Different, divergent or
unknown storage fails closed. A linear mapping-touch pass is only a demand
filter and never authorizes deletion.

The proof service is the sole M04 authority. The old identity branch is
deleted. TRACE is not a veto because deleting an instruction that performs no
event or state change does not remove or reorder the explicit trace event.

## Focused decisions

The frozen M03 assembler removes the four raw self-copies covered by its old
solver. M04 preserves that floor and removes seven stronger cases:

| Case | M03 | M04 |
| --- | --- | --- |
| raw COPY/ICOPY/FCOPY/SCOPY | removed | removed |
| raw DCOPY/ACOPY/BCOPY | retained | removed |
| LINK-established full/binary aliases | retained | removed |
| raw self-copy before TRACE | retained | removed; TRACE retained |
| agreeing storage phi | retained | removed |
| divergent storage phi | retained | retained |
| different physical storage | retained | retained |

Exact rows are retained in 'focused-decisions.csv'.

## Release verdict

The existing Release assembler was first verified as the exact retained M03
binary ('8c7ad38a...') and frozen before rebuilding. Richards, Towers and
RexxCPS produce byte-identical M03/M04 images. Their exact accepted hashes are
unchanged from M03.

The canonical inputs generate 129, 52 and 46 demand-filtered self-copy queries
respectively, all rejected as different or unknown storage. Three paired
scale-screen rounds at '/usr/bin/time -lp' clock granularity show Richards at
0.05 s for both tools, Towers at 0.01 s for both, and RexxCPS at
0.05/0.06/0.06 s for M03 versus 0.05/0.06/0.07 s for M04. Peak-RSS deltas
for Richards and Towers remain below 1 MiB; RexxCPS is unchanged within sample
variation. This is a seconds-scale guard, not a formal timing claim.

Because all representative RXBIN images are byte-identical, M04 changes no
representative runtime instructions and has no runtime-performance claim.
Adrian accepted this output-neutral first Release verdict on 2026-08-03.

## Correctness

- strict GNU90 syntax passes with one pre-existing unused-parameter warning;
- focused graph, metadata, optimized/no-opt and whole-panel checks pass 6/6 in
  both Debug and Release;
- the old four-copy safe floor is preserved;
- all seven conditional copy families, LINK identity, agreeing phi, TRACE,
  divergent phi and different-storage controls are covered;
- the first broad run passed 1,994/1,995 and exposed one stale dense-diagnostic
  expectation after the new proof deleted its input copy;
- the assertion was moved to the authoritative proof diagnostic; and
- the final broad Debug CTest passes **1,995/1,995** in 185.18 seconds.

The evidence is an output/correctness and assembler-scale verdict, not a
formal runtime-performance panel.
