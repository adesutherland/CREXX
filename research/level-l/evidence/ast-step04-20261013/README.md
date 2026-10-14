# AST-STEP04: RexxScript, acyclic facts and checked constant folding

This increment implements the scope approved by Adrian on 13 October 2026 in
`CREXX-level-l-inline-storage`, on develop based on
`50926edb8b6b36e98ba496b568c35bd6b544f5b8`. It is local macOS ARM64 Debug
qualification. Adrian authorized commit/push to `develop` and light CI monitoring
on 14 October 2026. The enclosing commit is the publication payload. Normal
automatic publication workflows are expected to pass; their actual results
are reported in the conversation and GitHub run records.
The authoritative acceptance criteria are
[AST4-AC01–05 in PLAN.md](../../PLAN.md#ast-step04-first-increment--locally-qualified).

## Publication reconciliation

The publication checkout is based on develop
`60f58c0a98243a2a70973dc656956609d2e972a2`. Before preparing publication,
all 20 source, test and documentation files in `inputs.json` matched their
retained hashes. The intervening commits repair the source-byte test fixture,
the CRI-17 fixture's compiler-specific TLS declaration and transient Apple
installer stapling; they do not change the qualified Level L or RexxScript
implementation. Their own normal Build and CodeQL workflows succeeded.

The retained qualification is reused rather than repeating unchanged tests.
Only this receipt and the plan's publication wording changed for this commit.
`publication.json` records the reconciliation and the current plan/receipt
hashes; `inputs.json` preserves the original qualification identities. Hosted
results for the new publication remain a separate check. No new release tag or
completion of the wider AST programme is implied.

## Delivered behavior

RexxScript uses attached `then tree` mappings for named statement and expression
fields. The surrounding sequences, blocks and loops carry these nodes without
changing their language algorithms. The ordinary parse view remains available.
The interpreter still selects branches, short-circuits logical operations and
owns variable/BIF effects.

The general Level B analysis service visits children before parents, stores one
application object per node, and permits dependencies only on completed results.
Checked results belong to an owning immutable snapshot. Expired references,
foreign trees and replaced contents are rejected. Node, walk and child-cursor
accessors check the same lifecycle; obtaining the current cursor node cannot
revive an old snapshot before the next advance.

RexxScript reports known constants, direct variable names and possible effects.
Its folding uses the same numeric operations and literal decoder as execution.
Only exact typed round trips replace pure expressions; native float bits,
including signed zero, must agree. Failed arithmetic remains conditional on
reaching the original expression. Replacements preserve source ranges and the
last-evaluated-operand diagnostic anchor. No BIF executes during analysis.

Migrating the IF grammar exposed duplicate labelled LR reductions for identical
captures in shared prefixes. Interning a capture by target and field name fixes
that artificial conflict without changing choice semantics. The minimal
`ast-shared-prefix` fixture covers LR(1), packrat and Earley through both emitted
source and RXAS.

## Qualification

| Gate | Result | Evidence |
|---|---|---|
| Core product/bootstrap | Initial and final product builds pass without compiler warnings | `product-build.log`, `final-product-build.log` |
| Normal correctness | 30 CTests pass: AST/runtime cases, 40 new RexxScript cases and the existing 60-case suite, both VMs | `runtime-tests.log` |
| Final focused correctness | 28 CTests pass after the accessor guards, covering both VMs and opt/noopt consumers | `final-runtime-tests.log` |
| Pre-migration comparison | 28 scripts have exactly equal status, output and diagnostics | `differential.json`, `final-cli.json` |
| Full AST surface | 267 observations, 448 commands; four general backends, Pratt expressions, Unicode/composite settings, both emitters and both VMs | `surface.json` |
| Unoptimised Level L and changed RexxScript modules | 33 commands pass, including eight AST/analysis consumer/VM cells and the 40-case fold panel on both VMs | `noopt-runtime.json`, `check-noopt-runtime.py` |
| Final ownership accessor guard | Focused current-node/reassignment regressions pass | `lifecycle-final.log` |
| Unoptimised capture/diagnostics | 51 observations: 24 shared-prefix parses and 27 located L0/L1/L2 refusals | `noopt-surface.json` |
| Noopt bootstrap | Passes; all four optimized/unoptimized L1/L2 self-descriptions agree | `noopt-build.log`, `inspection.json` |
| Installed use | Eight commands pass: analysis example and calculator on both VMs, no-op rebuild and native script execution | `installed.json`, `check-installed.py`, `install.log` |
| Manual/API documentation | 91 local links/anchors resolve; existing RexxDoc coverage preserved | `inspection.json` |

The initial normal/differential/full-surface results precede the final three
accessor guards for a replaced or expired input. Those guards change rejection
of invalid borrowed handles, not successful execution. Final lifecycle/noopt
checks include the guards. The final 28-test integration run and final native
CLI comparisons also include them. The large surface/Unicode matrix and 60-case
legacy suite were not repeated for guards confined to invalid handles. `pre-guard-runtime.json` retains the earlier tool identities.

The new CTests each run one prebuilt VM image. They do not launch nested builds
or aggregate matrices. No new aggregate was added. Larger explicit panels remain
separate qualification commands. `inputs.json` identifies the source/test inputs.
Temporary compiler warnings and two implementation errors found during development
were repaired before the retained successful runs: capture interning and use of
native packed integer bits to compare floats.

## Contract and scope

[AST-CONTRACT.md](../../AST-CONTRACT.md) records the finite schedule, ownership,
atomic reconstruction and storage bounds. Analysis records are indexed by arena
ID; unreachable rewrite intermediates can make allocated arena size larger than
reachable node count. Node allowance bounds visits/filled records, and the
existing tree allowance bounds arena capacity. Application result sizes and
calculations are separate costs.

The folding argument assumes the shared scalar operations and exact typed
round trip, with sufficient resource allowances. Fuel measures interpreter work;
fewer visits can make a folded program fit a smaller allowance. The regression
corpus checks current behavior; it does not prove arbitrary callback semantics.

The approved acyclic slice is locally qualified. The
wider plan retains restricted cyclic analyses, grouping/lowering, synthetic and
recovered origins, and production cREXX compiler migration as open work.
