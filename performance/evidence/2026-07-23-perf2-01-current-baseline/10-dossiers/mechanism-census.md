# PERF2-01 cross-workload mechanism census

This census combines ordinary-product gaps with schema-5 counts, bounded
profile timing, uninstrumented native samples and selected allocation stacks.
No single diagnostic column is a performance verdict.

| Mechanism | Measured portfolio footprint | Workloads most exposed | Ownership / disposition |
| --- | --- | --- | --- |
| Reference storage and construction | Bounce `MKREF` 263 ms; reference-tree storage 1,228/1,221 rxvm and 1,201 rxbvm samples | Bounce; guard Storage/List | value/frame work plus VM reference path; first bounded PoC family |
| Recursive/general value copy | Richards 96.1M operations / 762.8 MB and `COPY_REG_REG` 838/853 ms; Towers 61.7M / 481.2 MB; Storage 36.3M / 285.7 MB with 117 MB heap high-water subtree | Richards, Storage, Towers, Permute | value/frame work; prove scalar/reference specialization and lifetime safety before broader use |
| Call and reused-frame setup | List 4.396M calls; JSON 745k; Permute 433k; RexxCPS 348k/341k; Richards 119k | List, JSON, Permute, RexxCPS, Richards | compiler/inliner plus frame machinery; strong frequency, but selected Richards frame entry is only ~6.4 ms |
| String/binary position and copy | Base64 `SETSTRPOS` 207/180 ms, decoder 1.45/1.47 s, 3.08M string copies; JSON 3.53 GB transfer bytes | Base64, JSON, RexxCPS | RXAS/BIF plus value implementation; Base64 is the closest qualified deficit |
| Decimal/string conversion and materialization | RexxCPS 1.57M/1.54M conversions, ~48/47 ms bounded conversion time; native decimal parse/format helpers; Mandelbrot 501k conversions | RexxCPS, Mandelbrot | BIF/inliner and value representation; exact family controls exist |
| Attribute trimming/reset/teardown | Storage 86.1M reset-family ops; Towers 41.2M; trim is a repeated native stack in Permute/Richards/Storage | Storage, Towers, Permute, Richards | value/frame lifetime; high safety risk and must be guarded by retained correctness/RSS evidence |
| Arithmetic/branch dispatch | Mandelbrot 129.0M dynamic instructions; JSON 118.6M; Sieve already 5.4-7.2x ooRexx | Mandelbrot, JSON, Sieve control | RXAS/VM execution stream; cross-runtime publication remains capability-qualified for the first two |
| Loader/image footprint | shared library is 54,722 instructions / 858,073 bytes; module sets span 862,074-937,679 bytes; lifecycle load-first-result ~2.7 ms | all, especially short probes | link/load; keep separate from steady-state and do not choose a throughput optimization from this lane |
| Stable selector caches | zero selector attempts in all accepted optimized profiles | none | current selector quickening is not selected from this evidence; add a site family only when a real executed site is measured |

Cross-workload conclusion: the qualified geometric-mean deficit is not a
uniform dispatch problem. Sieve and Permute already exceed the cell target;
Richards and Bounce are dominated by two concrete value mechanisms, while
Base64 is a distinct string/binary path. A broad private-opcode batch would
mix owners and obscure causality.
