# PERF2-04 SUBSTR bounded wall verdict

## Scope

This is a scratch-only, profiling-off Release comparison at repository commit
`6567f0ba23f20623e01322f5a62323b2347ab09d`. It compares the current inlined
RexxCPS image (`SLC-C0-current`), the exact-site direct composition control
(`SLC-H1-direct-composition`) and the exact-site constant-fold ceiling
(`SLC-CF1-constant-fold`) on the exact B0-R `rxvm`, `rxbvm` and library.

The maintained Level B matrix driver used canonical RexxCPS defaults, serial
rotated sampling, two warmups and seven recorded samples per cell. All 54
invocations exited 0, used `argv_count=0`, passed
`PASS: RexxCPS 2.2d cREXX port`, and emitted no stderr. The adaptive benchmark
selected effective count 300 for 30 invocations and 400 for 24. Every cell has
`rerun_recommended=no` under the maintained runner's variability rule.

## Result

| VM | current median clauses/s | SLC-H1 median | H1/current | SLC-CF1 median | CF1/current | CF1/H1 |
|---|---:|---:|---:|---:|---:|---:|
| `rxvm` | 28,818,112 | 31,318,043 | +8.674860% | 32,145,168 | +11.545017% | +2.641049% |
| `rxbvm` | 27,540,875 | 29,437,133 | +6.885250% | 29,765,360 | +8.077031% | +1.115010% |

Absolute-cell relative MAD ranges from 0.331937% to 1.682097%; span ranges
from 1.270087% to 4.034880%. SLC-CF1 has the highest median in both VMs. Its
`rxbvm` distribution overlaps SLC-H1 and its +1.115010% median advantage is
small, so that incremental wall magnitude is directional rather than a formal
production verdict. The independent exact instruction proof is not ambiguous:
CF1 removes 140 timed instructions per outer iteration beyond H1, including
all 42 selected runtime slices.

## Decision

For these two exact constant sites, SLC-CF1 is the most efficient correct
machine ceiling and compiler ownership is the recommended placement. The
compiler should evaluate through the canonical Unicode/argument semantics and
replace only a fully proved constant call, while the complete Level B body
remains the fallback and behavioral documentation. SLC-H1 remains the dynamic
or proven-range composition control using existing `SETSTRPOS`/`SUBSTRING`
primitives; it shows no public opcode, VM assist or native owner is needed for
the selected cases.

This recommendation is conditional on production-equivalent source/TRACE,
signal and evaluation-order behavior. The source-rewrite controls cannot
reproduce the imported inlined body's exact `.srcstep`/TRACE identities. A
production constant folder must synthesize that observation contract or fail
closed. A general direct composition must also avoid observable mutation of a
caller-owned string cursor. No production implementation, broad portfolio or
formal production-selection timing was performed.
