# PERF3-11 M02 equivalent scalar-constant writes

Status: **complete — first Release verdict accepted 2026-08-03**

M02 replaces the legacy raw-register availability solver for repeated integer
and bitwise-equal float loads with the reusable per-procedure proof service.
The old accepted repeated-load case remains the minimum floor, while the new
consumer also admits equal-value phis, linked storage and ordered TRACE paths.

The evidence was produced on branch
`codex/perf3-rxas-flow-infrastructure` from committed M01 base `972ef2f41`,
with only the M02 production, tests, documentation and evidence changes dirty.
The final commit is intentionally represented by repository history rather
than self-referenced here.  No push was performed.

## Proof and ownership model

The consumer selects metadata-admitted exact scalar constant writes, applies a
non-authoritative demand filter, and asks
`rxas_flow_prove_redundant_constant_write()` for the decision.  A deletion
requires all of the following:

- the before/after `StorageId` is identical;
- every reachable scalar `ValueId` leaf is the same integer value or the same
  exact float bit pattern as the candidate write;
- the candidate's resulting constant is the expected write-once definition;
- reference and native-payload components are already proved absent on every
  reaching path; and
- the instruction metadata proves an exact total component-only write with no
  signal, context, cursor, implicit or control-flow effect.

The hidden cleanup condition is essential.  VM integer/float assignment
releases a reference payload and finalizes a host-owned native payload before
installing the scalar.  M02 therefore adds `NATIVE_PAYLOAD` as an eighth
component, makes integer/float loads explicitly clear both hidden components,
and makes `BCOPY` read/write binary plus native-payload state.  The focused
oracles retain both reference-cleanup and native-cleanup loads.

All migrated consumers in one fixed-point epoch now share a single immutable
proof session.  The cheap demand filter can suppress impossible first
occurrences but can never authorize deletion; same-register repetition, phi
repetition and explicitly connected link/swap candidates still reach the
proof service.

## Focused decision result

The old M01 assembler removes only the retained repeated-integer floor.  M02
removes five writes: that floor plus four stronger cases.

| Case | M01 | M02 | Result |
| --- | ---: | ---: | --- |
| repeated integer floor | 1 load | 1 load | old floor recovered |
| equal-constant phi | 3 loads | 2 loads | join write proved redundant |
| repeated exact float | 2 loads | 1 load | bitwise-identical write removed |
| linked constant storage | 2 loads | 1 load | actual `StorageId` followed |
| ordered TRACE path | 2 loads | 1 load | equal observed value preserved |

Different-constant phis, `-0.0` versus `+0.0`, reference cleanup and native
payload cleanup remain rejected.  The detailed counts and decisions are in
`focused-decisions.csv`.

## Representative output gate

The exact retained Richards, Towers and RexxCPS inputs are byte-identical under
the frozen M01 assembler and the M02 Release assembler:

| Workload | RXBIN SHA-256 before and after |
| --- | --- |
| Richards | `ca4efcf26edd849d09c67482feda6da82aecbb7d4dd76f3b6a1b37be00dfd8ca` |
| Towers | `0fdac42959023dd36ac5b5fbdf43335302dbfaad76bd59624b2dfb9e872fc011` |
| RexxCPS | `beeba2fefc0496e2f17dbb2ee48619c0f80ae69ed5f3aaa41f4ffe6c4fa40467` |

There is no ordinary runtime-image change to time.  Adrian accepted the
mandatory first Release verdict on 2026-08-03 on this checksum closure plus the
assembler scale and focused correctness evidence.

## Scale boundary

This is an assembler-processing diagnostic, not a formal runtime benchmark.
Three same-session serial ordinary Release samples used no warmup.  Median
RexxCPS assembly is **0.05 s** for both M01 and M02.  Median peak RSS rises from
**22,134,784** to **30,081,024 bytes**.

Refreshed lazy-SSA accounting attributes the peak to the 319-block
`__rxtrace_handler`: M02 materializes 45,983 storage versions and 9,637 value
versions, retaining 17,535,936 bytes for that procedure.  The analysis remains
procedure-local, budgeted and released before the next procedure.  Diagnostic
assembly has the same 0.39 s median before and after.  Adrian accepted this
bounded memory tradeoff within the previously selected seconds-scale assembler
budget.  Raw samples are retained in `scale-samples.csv`.

## Correctness

- strict GNU90 syntax checks pass with one pre-existing unused-parameter
  warning;
- opcode metadata and immutable-flow proof tests pass;
- the focused RXAS optimizer set passes **53/53**;
- current Release `rxvm` and `rxbvm` pass the optimized/unoptimized signal and
  hidden-cleanup matrix **4/4**;
- the three canonical images are byte-identical; and
- broad Debug CTest passes **1,991/1,991** in 186.89 seconds.
