# SWAP cost diagnostic

This retained RXAS diagnostic separates the cost of one VM `SWAP` dispatch
from the register-affinity implementation and from a source benchmark. The two
images execute the same 100,000,000-iteration integer loop. `swap.rxbin` adds
exactly one non-cancellable `SWAP` per iteration; `control.rxbin` does not.
Disassembly confirms that the optimized images otherwise have identical
instruction histograms.

The ordinary profiling-off Release product was warmed once, then measured for
16 alternating, order-balanced rounds per image and VM. The median same-round
increment was 43.4405 ms in `rxvm` and 70.6431 ms in `rxbvm`, giving:

| VM | Control median | SWAP median | Paired median increment | Product cost per SWAP |
|---|---:|---:|---:|---:|
| `rxvm` | 203.184 ms | 245.252 ms | 43.441 ms | **0.434 ns** |
| `rxbvm` | 210.196 ms | 279.917 ms | 70.643 ms | **0.706 ns** |

The profiling build records exactly 100,000,000 `SWAP_REG_REG` executions.
Its timed handler average is 13 ns in `rxvm` and 14 ns in `rxbvm` (1.385 s and
1.405 s aggregate respectively). Those values include per-instruction
profiling overhead and are diagnostic attribution values, not ordinary-product
costs. The paired profiling-off Release delta above is the applicable product
estimate.

The VM handler only exchanges two `value *` register pointers and dispatches.
This sub-nanosecond product cost explains why substantial reductions in dynamic
SWAP count can remain immaterial end to end.

