# NR-26 first profiling-off Release verdict

Verdict: **NOISY/INCONCLUSIVE; REVISE recommended, with REVERT still live.**
The frozen implementation is provisional and uncommitted. No broad CTest,
sanitizer, install/package proof, documentation closeout, commit or push
followed this gate.

## Scope and provenance

- Branch: `develop`
- Starting/current HEAD and `origin/develop`:
  `4ab5f3d8da673c10b81af4249757763d052dda34`
- Starting tree: clean; the candidate is exactly the uncommitted NR-26 scope.
- Product: ordinary `Release`, `-O3 -DNDEBUG`,
  `CREXX_VM_PROFILING=OFF`.
- Baseline: retained accepted NR-14 product. Its linked RexxCPS image was
  reproduced exactly at the accepted SHA-256.
- Workload: canonical-default RexxCPS, `100 x 100` iterations of 1,000
  clauses, under both current `rxvm` and `rxbvm` so VM drift is removed.
- Sampling: serial, balanced/interleaved; one warmup per cell, followed by 36
  valid paired rounds per VM after the approved noise append rules.
- Interpretation: same-session first-verdict evidence, not a release-wide or
  cross-platform claim.

## Selected implementation

Design B adds a procedure-local CFG overlay over the final typed AST before
register assignment. It records reachability, use/def, definite assignment,
liveness, reaching definitions, constants/copies, ownership, alias/exposure,
exceptional edges and affected-value uncertainty. Unknown facts fail closed at
the affected transformation site. It emits no RXAS metadata, ISA, RXBIN or ABI
change.

The first production batch contains:

- F1: remove a scalar local's eager default `NULL` only after a safe
  must-write-before-first-read proof;
- F2: remove an NR-12 small scalar by-value entry copy under the same proof.

The focused fixture proves positive and negative straight-line, branch, loop,
nested-loop, early-return, signal-handler, reference, exposure and TRACE cases.
Optimized/no-opt assembly and both VMs pass. Adjacent NR-09, inline by-value
and reference regressions also pass. F1 removes seven fixture operations; F2
removes three entry copies without changing `.locals`.

## Static Release result

The bounded optimized portfolio found F1 changes in RexxCPS, its opaque form,
the frozen-PARSE benchmark and the runner. Other inspected optimized workloads
and the entire no-opt portfolio are unchanged. F2 has no current bounded
portfolio footprint.

Canonical optimized RexxCPS removes 16 `null` source-RXAS lines and shrinks by
190 bytes. The linked image shrinks by 72 bytes. Disassembly shows four fewer
packed initialization operations in `main` and two fewer at the timed
`cps_subroutine` entry. Exact hashes and sizes are in
`artifact-inventory.csv`.

## First Release result

All 148 valid executions—four warmups and 144 recorded cells—returned zero,
printed the expected PASS marker and recorded exact canonical-default
provenance. Percentages use `(candidate / baseline - 1) * 100`; positive CPS
and negative elapsed are favorable. The headline is the paired median; the
interval is the two-sided 95% Student-t interval around the mean paired change.

| Lane | VM | Pairs | Q1 | Median | Q3 | Mean 95% interval | Favorable |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| native CPS | `rxvm` | 36 | -0.261% | **+0.601%** | +1.279% | -0.294% to +1.310% | 24/36 |
| native CPS | `rxbvm` | 36 | -0.658% | **+0.317%** | +1.267% | -0.331% to +1.150% | 20/36 |
| process elapsed | `rxvm` | 36 | -1.258% | **-0.596%** | +0.296% | -1.256% to +0.352% | 23/36 |
| process elapsed | `rxbvm` | 36 | -1.226% | **-0.339%** | +0.588% | -1.086% to +0.345% | 20/36 |

All absolute CPS cells exceeded the 10% span rule. The required append block
left both paired intervals crossing zero, so a final 12-pair block reached the
36-pair cap. Both directions remain favorable, but both are formally
noisy/inconclusive. No favorable subset was selected.

One attempted final block was interrupted before retaining any sample because
its immediate pre-start host check showed CLion at 281% CPU. The invalid
directory contains only a `running` capture manifest. After CLion returned to
2.3%, the whole block was rerun in `timing-append-02-retry/`.

## Decision boundary

The current F1/F2 footprint is too small to accept as a completed performance
change. REVISE is recommended: keep this frozen worktree available for review
and select a stronger evidence-backed consumer of the flow layer. If no such
consumer is wanted, revert NR-26 rather than landing the foundation on this
inconclusive gain. Work stops here pending Adrian's direction.

## Evidence map

- `input-manifest.txt`: exact four-cell commands and expected output.
- `timing/`: initial warmup plus 12-pair block and raw stdout.
- `timing-append-01/`: first governed 12-pair append.
- `timing-append-02/`: invalid pre-sample attempt retained for disposition.
- `timing-append-02-retry/`: valid final 12-pair block.
- `timing-consolidated-36/summary.csv`: capped absolute-cell statistics.
- `paired-summary.csv`: capped paired CPS and process-elapsed statistics.
- `artifact-inventory.csv`: exact compiler, library, RXAS and linked-image
  hashes and sizes.
- `commands.md`: build, link, capture and reduction contract.
- `host-state.md`: pre/post environment and invalid-attempt evidence.
