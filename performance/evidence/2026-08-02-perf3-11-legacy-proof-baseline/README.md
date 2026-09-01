# PERF3-11 remaining legacy-proof baseline

Status: **complete — migration floor locked**

This bundle inventories and baselines the semantic proofs that remain after
the accepted Stage 6 `ITOS` migration.  It is not a performance verdict and it
does not require the new proof service to make identical decisions.  The old
accepted cases are the minimum safe-capability floor; stronger new decisions
must be identified and validated separately.

## Provenance

- Branch: `codex/perf3-rxas-flow-infrastructure`.
- Clean foundation commit:
  `6cdc3733597e1e2a719b1a425fcd0d6127455114`.
- Frozen Stage 6 Release `rxas`:
  `bb79eb375bdc4e95fe52b604d76b45f469d0154b5f97f3aea441eeca230ff9ab`.
- Baseline source: that commit plus the output-neutral stable legacy-rule
  diagnostic, worklist and this bundle; the resulting local commit is
  authoritative in Git history.
- Host/toolchain are unchanged from the Stage 6 accepted bundle.
- No push is authorized or performed.

## Inventory result

Five old whole-procedure value consumers remain:

1. typed-copy availability/may-reach and atomic operand redirection;
2. adjacent producer-destination forwarding;
3. repeated identical integer/bitwise-float loads;
4. repeated `NULL`; and
5. repeated one-register `ITOF`.

Exact self-copy deletion and complete-control reachability cleanup are also
current transformations.  The raw-register liveness/availability substrate and
dense storage must-analysis remain implementation/oracle infrastructure rather
than separate output consumers.

The keyhole engine contains six proof-bearing families mixed with mechanical
fusions: swap cancellation, duplicate linked reads, duplicate attribute reads,
compare/branch fusion, branch threading, and full-copy/redundant-attribute-copy
cleanup.  Fixed-register specializations and adjacent superinstruction
collection are classified as mechanical and are not automatically rewritten
as graph proofs.

## Representative baseline

The current Richards, Towers and RexxCPS generated inputs exercise no remaining
old whole-procedure value acceptance.  They do exercise:

- five/ seven reachability deletions in Richards/Towers;
- nine keyhole compare/branch proofs in RexxCPS; and
- four/seven keyhole branch-threading rewrites in Richards/Towers.

The current new `ITOS` service separately proves five RexxCPS repetitions; it
is already migrated and is not counted as remaining legacy work.

The focused fixtures retain the actual safety floor:

| Old proof | Accepted focused decisions |
| --- | ---: |
| Typed-copy redirection | 10 |
| Producer-destination forwarding | 12 |
| Repeated load | 1 |
| Repeated null | 1 |
| Repeated `ITOF` | 1 |
| Exact full self-copy | 1 |
| Reachability instruction cleanup | 4 |

The fixtures also cover negative cases for other-view reads, TRACE/register
metadata, source steps, calls, caller-owned argument ranges, async and local
signal handlers, indirect control, hidden lifetime state, signed float zero,
different attribute slots and optimizer barriers.

## Stable keyhole diagnostic

`rxas -d` now reports every applied old keyhole rule as:

```text
PERF3 legacy-rule procedure=... signature=........ first=... records=...
```

The signature is a deterministic 32-bit FNV-1a hash of the complete rule-set
input/output shape.  It is diagnostic only and remains stable when an unrelated
rule is deleted or reordered.  The focused `copy_acopy` test locks signature
`d132e98f`.

Three proof-heavy focused images assembled before and after diagnostic
instrumentation are byte-identical:

- whole-procedure flow:
  `22c6b134e467c425d75bd1fd896baa9e080731cf1a31fd2931371a58d02ae5ab`;
- whole-procedure panel:
  `f7bbe09b625f66500c9afd3560cb756feb757bb71ab54db27e532c1c4649c0b1`;
- producer-forward harvest:
  `3e92b60364f91b3dc8ace5d89ecb0c988a7986cabfc1c0e9abfe4753936383df`.

## Correctness result

The focused optimizer matrix passes **49/49**, including optimized/no-opt
oracles, metadata/barrier cases, graph/storage/proof fixtures and the stable
diagnostic identity.  Strict GNU90 syntax and Debug/Release `rxas` builds pass.

## Migration interpretation

M01 starts with repeated `ITOF` because the accepted dominated-success
repetition service should already recover that old safety floor.  The actual
consumer is then generalized to metadata-admitted one-register `XTOY`
derivations, beginning with `ITOD` as a newly unlocked numeric/plugin-dependent
case.  Every further opcode requires complete source/result, context/plugin,
failure-phase and success-stability metadata and a separately recorded gate.
Loads, null and self-copy then establish a generic redundant-write query.
Typed copy and producer forwarding require a new sparse use/liveness service.
Compare/branch is the first keyhole migration with current representative hits.
Dense old storage/liveness/graph code is removed only after its final consumer
and oracle migrate.

Any newly accepted ordinary output triggers the mandatory profiling-off
Release stop.  Old negative cases remain replayable even when a more precise
proof legitimately accepts a case that the old blanket guard rejected.
