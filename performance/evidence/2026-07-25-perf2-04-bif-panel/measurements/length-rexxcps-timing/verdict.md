# PERF2-04 LENGTH LEN-H1 bounded wall verdict

## Scope

This is a scratch-only, profiling-off Release comparison at repository commit
`6567f0ba23f20623e01322f5a62323b2347ab09d`. It compares the current RexxCPS
image (`LEN-C0-current`) with the direct-result hand-equivalent ceiling
(`LEN-H1-direct-result`) on the exact B0-R `rxvm`, `rxbvm` and library. The
maintained Level B matrix driver used canonical RexxCPS defaults, serial rotated
sampling, two warmups and seven recorded samples per cell.

All 36 invocations exited 0 and passed `PASS: RexxCPS 2.2d cREXX port`; no
stderr rows were retained. The capture manifest reports `result: pass`. All
four recorded cells have `rerun_recommended=no`.

## Result

| VM | current median clauses/s | LEN-H1 median clauses/s | candidate/current | median change | paired favorable | paired median change | paired mean change | paired mean 95% t interval |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `rxvm` | 29,363,287 | 29,362,309 | 0.999966693102 | -0.003330690% | 3/7 | -0.003330690% | +0.151226842% | [-0.559483352%, +0.861937036%] |
| `rxbvm` | 27,533,292 | 27,739,431 | 1.007486899859 | +0.748689986% | 5/7 | +0.614624455% | +0.431619407% | [-1.000215330%, +1.863454144%] |

Absolute-cell variability was bounded: relative MAD ranged from 0.059348% to
0.773296%, and span ranged from 1.198959% to 4.150082%. The paired intervals
use Student's t with 6 degrees of freedom over the seven matched recorded
rounds. They are descriptive bounded evidence, not the formal 12-pair
production-decision protocol.

## Decision

The wall result is neutral and is not production-selecting alone. `rxvm` is
effectively flat. The sub-1% `rxbvm` median uplift is not cross-VM consistent,
and both paired mean intervals cross zero. Removing exactly 84 dynamic
instructions per top-level timed iteration (about 1.5967% of normalized
instructions) therefore did not establish a reliable end-to-end improvement
in the smallest deciding cell.

LEN-H1 remains valid machine-work evidence for a bounded PERF2-03-F03 reopen:
compiler inline cleanup can remove result initialization, return copy and block
exit around the existing Unicode-aware `STRLEN`. It should only accompany a
broader selected compiler-cleanup slice with independent materiality evidence.
There is no evidence here for a standalone LENGTH production slice, new RXAS
opcode, VM assist or native owner. No further timing is requested for LEN-H1.

The scratch rewrite also does not establish exact TRACE/source/signal identity,
so a future compiler-owned implementation must preserve or synthesize the
existing observation contract, or fail closed.
