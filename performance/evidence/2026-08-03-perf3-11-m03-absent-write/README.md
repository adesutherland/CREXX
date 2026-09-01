# PERF3-11 M03 repeated all-component absence

Status: **complete — first Release verdict accepted 2026-08-03**

M03 replaces the legacy raw-register repeated-`NULL` availability solver with
the per-procedure proof service.  The old accepted join case remains the
minimum floor.  The new authority also proves equal-absence phis, linked
storage and an ordered TRACE path.

The evidence was produced on branch
`codex/perf3-rxas-flow-infrastructure` from committed M02 base `966c1396d`,
with only the M03 production, tests, documentation and evidence changes dirty.
The final commit is represented by repository history.  No push was performed.

## Proof boundary

`rxas_flow_prove_redundant_absent_write()` admits only an exact, classified,
non-signalling `NULL` with no context, cursor, implicit, branch or optimizer
barrier effect.  The target must resolve to a known pre-instruction
`StorageId`, and every reachable leaf of all eight tracked components must be
write-once `ABSENT`: integer, float, string, decimal, binary, attributes,
reference and native payload.

The query intentionally uses the pre-instruction storage identity.  VM `NULL`
clears the value object reached by a register; it does not relink that
register.  The SSA's post-`NULL` dynamic-map clobber is a conservative loss of
knowledge about other dynamic aliases, not a VM storage transition.  Requiring
all components absent proves that removing `NULL` skips no scalar/payload
destruction, reference release or host-native finalization.

A linear, path-insensitive repeat filter suppresses impossible first
occurrences and propagates demand across touched mapping registers, but never
authorizes deletion.  The proof service is the sole M03 authority; the old
`redundant-null` consumer and its TRACE/raw-register guards are deleted.

## Focused decisions

The frozen M02 assembler removes only the retained old floor.  M03 removes
that floor plus three stronger writes:

| Case | M02 `NULL`s | M03 `NULL`s | Result |
| --- | ---: | ---: | --- |
| old join floor | 1 | 1 | old floor recovered |
| equal-absence phi | 3 | 2 | join write removed |
| ordered TRACE | 2 | 1 | write removed; both events retained |
| linked storage | 2 | 1 | alias target followed by `StorageId` |

Different-absence phis, an intervening scalar write, reference cleanup and
native-payload cleanup remain rejected as `component-present`.  Exact counts
are retained in `focused-decisions.csv`.

## Release verdict

The exact M02 Release assembler was frozen before rebuilding M03.  Richards,
Towers and RexxCPS produce byte-identical M02/M03 images, so M03 changes no
runtime instructions in the current representative set and has no runtime
timing claim.  RXBIN embeds source provenance, so the retained hashes in this
bundle describe the exact absolute-path comparison; equality is established
by direct `cmp` for each old/new pair.

Adrian accepted this output-neutral verdict on 2026-08-03.

## Correctness

- strict GNU90 syntax passes with one pre-existing unused-parameter warning;
- the immutable graph/proof contract passes in Debug and Release;
- focused M03 plus adjacent M02/old-floor replay passes 7/7;
- both TRACE records survive the accepted TRACE-separated deletion;
- Richards, Towers and RexxCPS are byte-identical under M02 and M03; and
- broad Debug CTest passes **1,993/1,993** in 188.43 seconds.

The evidence is an output/correctness verdict, not a formal performance panel.
