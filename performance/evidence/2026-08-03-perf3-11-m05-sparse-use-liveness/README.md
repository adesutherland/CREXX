# PERF3-11 M05 sparse use/liveness and typed-copy redirection

Status: **complete — first Release verdict accepted 2026-08-03**

M05 replaces the legacy typed-copy availability/may-reach solver with one
cached sparse use/dependency analysis per procedure epoch.  The new service
indexes component and storage observations against write-once SSA identities,
propagates liveness through reverse phi dependencies, and lets the proof
service return an immutable all-or-nothing operand-rewrite plan.

The evidence was produced on branch
`codex/perf3-rxas-flow-infrastructure` from committed M04 base
`e9e9ccd6dbeabe90eea3f2206bd72d5ae01214d4`, with only the M05 production,
tests, documentation, worklists and evidence changes dirty. No push was
performed.

## Proof boundary

`rxas_flow_use.c` distinguishes explicit, read/write, implicit, metadata,
TRACE, cursor, call-window and opaque observations. Direct uses are indexed by
`ValueId`, cursor/lifetime observations by `StorageId`, and reverse phi edges
carry sparse liveness to reaching definitions. Range calls remain one sparse
window observation instead of being expanded across every local component.

`rxas_flow_prove_typed_copy_redirect()` accepts only exact `ICOPY`, `FCOPY` or
strict `SCOPY` with one local, non-external destination storage. The copied
component must be the instruction's write-once result and its source must remain
equivalent at every exact component-only consumer. Metadata, TRACE, read/write,
opaque, cursor and live call-window observations reject the plan. The optimizer
validates every expected operand before applying any rewrite and deleting the
copy. Stale epochs, allocation or budget failure, incomplete values and partial
plans fail closed.

The old per-candidate available/may-reach arrays, repeated whole-procedure
operand scans and blanket barrier policy are deleted. M06 producer forwarding
remains a separate legacy authority.

## Focused decisions

The frozen M04 assembler's ten valid M05 accepts are all recovered with reason
`all-uses-redirected-ssa`: integer compare, join and independent-region cases;
float and strict-string compares; dominated loop; independent region after
ENDLIFE; unrelated metadata; unobserved throw; and complete indirect control
flow.

`copy_before_endlife` is one separately evidenced stronger acceptance. M04's
M05 solver rejected at the exceptional ENDLIFE and M06 later retargeted the
producer. M05 now proves that ENDLIFE names unrelated storage and redirects the
consumer to the original source. The final instruction count is unchanged; the
only disassembly difference is `load r40`/`ieq ... r40` becoming
`load r39`/`ieq ... r39`.

Other-component, TRACE, mixed-entry, relevant metadata, live call-window,
handler-observed and SCOPY-cursor controls remain closed. Direct proof-contract
tests also cover an exact rewrite, atomic metadata rejection and stale-plan
failure.

## Release verdict

The frozen M04 assembler is the retained binary with SHA-256 `7646894c...`.
Using the exact same absolute RXAS input paths, Richards, Towers and RexxCPS
produce byte-identical M04/M05 images and recover the already-retained M04
hashes.

Three paired, serial `/usr/bin/time -lp` scale-screen rounds show:

| Workload | M04 elapsed | M05 elapsed | M04 peak RSS | M05 peak RSS |
| --- | --- | --- | --- | --- |
| Richards | 0.05 s | 0.05-0.06 s | 12.4-12.5 MB | 23.8-24.1 MB |
| Towers | 0.01-0.02 s | 0.02 s | 6.7-6.9 MB | 8.7-8.8 MB |
| RexxCPS | 0.05-0.06 s | 0.16-0.17 s | 29.8-29.9 MB | 102.8 MB |

The RexxCPS cost is material but bounded and remains inside Adrian's accepted
seconds-scale assembler budget. Because every canonical RXBIN is byte-identical,
M05 changes no representative runtime instruction and has no runtime-throughput
claim. Adrian accepted this first Release verdict on 2026-08-03.

## Correctness

- strict GNU90 syntax passes with one pre-existing unused-parameter warning;
- focused graph, metadata, optimized/no-opt flow and panel checks pass 6/6 in
  both Debug and Release;
- direct proof contracts cover sparse use/liveness, phi dependencies, budgets,
  cursor observations and atomic typed-copy plans;
- all ten old safe accepts and the retained negative floor are covered;
- exact canonical hashes are unchanged; and
- the complete Debug build and broad CTest pass **1,995/1,995** in 231.94
  seconds.

This is an output/correctness and assembler-scale verdict, not a formal runtime
performance panel.
