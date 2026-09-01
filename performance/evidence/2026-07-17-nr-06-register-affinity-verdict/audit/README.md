# NR-06 implementation and timing audit

This audit was added after the first Release verdict because the direction of
the wall-clock results did not match the expectation that removing instructions
must reduce work.

## Binary and RXAS identity

The retained linked inputs are distinct and were run by the current profiling
VMs:

- baseline SHA-256: `d8eb5549d5061d029439b6e64195995b3cb160c81464649a57a6e979bcc89740`;
- candidate SHA-256: `a486c2bc4e90d23d9cb49075a342b7c18b70790384e340eea95d263c5eb9940b`.

Each profile's procedure rows identify the corresponding baseline or candidate
module path. All runs returned zero, all benchmark correctness markers passed,
counter overflow is zero and call-mechanics attribution is complete.

Standalone optimized assembly changes RexxCPS from 120 to 76 `SWAP`s. After
removing `SWAP` from both disassembled opcode streams, the remaining 1,534
opcodes are identical and ordered identically. Linking with the same library
preserves exactly the 44-instruction difference (908 to 864 linked `SWAP`s).
The assembler therefore neither loses nor fabricates the compiler result.

## Why RexxCPS did not improve

All 44 removed RexxCPS instructions are setup/restoration pairs in
`rexxcps_levelb.__rxtrace_handler()`. Normal benchmark execution has tracing
off, the handler is not entered, and both exact images execute 484,376 swaps in
both VM modes. RexxCPS therefore has **zero executed SWAP change**. Its earlier
ordinary-Release timing split measured host noise between different images
whose executed SWAP work was identical.

The same reachability check across all 58 portfolio removals finds 44 cold
trace-handler swaps and two swaps in `Permute.run()`, whose standalone body is
not entered because the caller uses the optimized inline body. The remaining
12 static sites are executed.

## Other workloads

Schema-4 profiles of exact assembled/linked baseline and candidate modules
give the same dynamic counts in both VM modes:

| Workload and argv | Static sites | Executed SWAP reduction | Other dynamic opcode change |
|---|---:|---:|---|
| List `100` | 4 | 142,200 | none |
| Richards `1` | 4 | 23,242 | none |
| Storage `10` | 2 | 81,920 | 40 `BCTP` become 40 `INC1` + 40 `BR` |
| Base64 `500` | 2 | 1,000 | none |

The Storage side effect is caused by register numbering moving the loop counter
to `r1`. RXAS's fixed-register `INC1` rule runs before its more valuable
`INC`+`BR` to `BCTP` rule, adding 40 dispatches while the affinity placement
removes 81,920 swaps. Correctness is unchanged, but this proves that register
renumbering can affect later optimizer choices and is not mechanically isolated
to call swaps.

## Product cost and timing verdict

The retained direct RXAS diagnostic measures one ordinary profiling-off Release
swap at 0.434 ns (`rxvm`) and 0.706 ns (`rxbvm`). Applying those observed costs
to the exact dynamic reductions predicts only 0.000108% to 0.040474% end-to-end
change across the four workloads.

A compact four-round, order-balanced ordinary Release comparison retained all
64 passing executions. Its paired changes range from -17.618% to +35.352% and
split direction by workload/VM. This is many orders of magnitude noisier than
the causal cost estimate and cannot establish an additional product effect.

The implementation is working where eligible direct named locals occur, but
the product-speed opportunity is immaterial. RexxCPS was a cold-site selection
error, not evidence that an executed `SWAP` has negative cost. The broader
profiles and direct microbenchmark show why this optimization should not be
used to justify more complex specialist call mechanisms whose principal
benefit is merely avoiding these pointer-swap dispatches. Adrian nevertheless
selected this bounded implementation for retention because it removes verified
static and executed work without adding runtime instructions. Constant/by-value
flag and branch work remains a separate backlog topic.

## Accepted closeout verification

The final production boundary retains NR-06 affinity placement and its
dedicated tests; rejected NR-07 production edits and fixtures remain removed.
Target-only Debug and ordinary profiling-off Release `rxc` rebuilds passed,
followed by 26/26 focused selections, including the linked-runtime fixture, in
each build tree. The first full Debug CTest
identified seven intentional optimized register-layout golden changes. Their
matching runnable fixtures passed, and the new output removes another 86 static
swaps. After updating those goldens through the documented `--update-gold`
path, the focused compiler/runtime set passed 16/16, the complete Debug build
passed, and final Debug CTest passed 1,849/1,849 at parallelism 30 in 331.21
seconds. The initial failed and final clean logs are both retained here.
The production-code snapshot is `nr06-audited-implementation.patch`; combined
and superseded PoC fixtures are retained under `archived-fixtures/`.
