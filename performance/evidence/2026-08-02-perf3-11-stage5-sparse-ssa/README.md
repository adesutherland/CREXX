# PERF3-11 Stage 5 sparse storage/component SSA

Status: **complete — Gate 5 passes**

This bundle closes the output-neutral storage/component analysis layer.  It is
an assembler correctness, equivalence and scaling gate, not a VM runtime
benchmark or a new optimizer-consumer verdict.

## Provenance

- Branch: `codex/perf3-rxas-flow-infrastructure`
- Stage 4 base: `54ebd11f23a840092bbd6b463c3d64bddfd5656f`
  (`perf: add sparse RXAS signal policy analysis`).
- Stage 5 source: that base plus the Stage 5 code, tests, documentation and
  this bundle; the resulting local commit is authoritative in Git history.
- Frozen comparator: Gate 0 profiling-off Release `rxas` at
  `/tmp/crexx-perf3-11-stage0.35IBzf/base-binaries/rxas`.
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs, 24 GiB RAM.
- Power: AC; low-power mode off.  No thermal, performance or CPU-power warning
  was recorded.
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2.
- Build: CMake/Ninja Release, `CREXX_VM_PROFILING=OFF`.

## Semantic result

`assembler/rxas_flow_ssa.c` is a third demand-driven cache owned by the
immutable procedure epoch.  It uses sparse persistent states rather than a
point-by-register matrix.  Local, argument and global register names begin at
base `StorageId`s; link, linkarg, attribute/reference links, swap, unlink and
fused operations update symbolic mappings.  `linkarg` maps to caller-owned
argument storage.  Calls preserve structurally unchanged mappings while the
Stage 4 call/reference/effect identities advance.

Component writes create write-once `ValueId`s.  Entry, direct write, constant,
copy, derived, known absent, phi and unknown values are distinct.  `null` is
not an unavailable proof, and `dcopy` preserves the source's absent decimal.
Unknown storage has a separate identity and cannot collide with valid
zero-based `ValueId 0` at a join.  Integer-coded local copy/target operations
and `inc0`/`dec0`-family fixed-register writes are explicit definitions.

Derived ITOS, FTOS and DTOS values name the correct integer, float or decimal
source and their numeric/plugin/effect identities.  Canonical source-operand
metadata makes two-register `itof rTarget,rSource` use `rSource`.  Normal and
failure states consume the Stage 1 write phase and Stage 4 edge identities.
Where a fused opcode both remaps and writes values but lacks canonical
intra-instruction ordering metadata, mapping remains exact and component
values fail closed rather than guessing pre- versus post-remap storage.

## Correctness and image result

- Strict GNU90 syntax checking passes with `-Wall -Wextra -Wconversion
  -Wsign-conversion`.
- The focused Debug matrix passes **113/113**, including optimized/no-opt
  dual-VM signal fixtures, both decimal plugins, old storage/conversion
  behavior and the new sparse SSA contract tests.
- Ordinary profiling-off Release `rxas` builds.
- Canonical Richards, Towers and RexxCPS RXBIN hashes exactly match Gate 0.
- Every canonical diagnostic procedure returned an available analysis; no
  budget, allocation or graph disablement was observed.

## Scaling result

`rxas -d` materializes only actual derivation sites and values requested by
their sparse chains.  It does not force every possible point/component query.
The times below include deterministic diagnostic printing as well as analysis.

| Workload | Procedures | Elapsed | Peak RSS | Work / budget | Retained bytes | Storage versions / phis | Value versions / phis | Derived |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards | 24 | 0.21 s | 10,911,744 B | 32,835 / 27,373,568 | 3,394,256 | 1,451 / 375 | 7,464 / 159 | 5 |
| Towers | 13 | 0.07 s | 6,406,144 B | 11,707 / 8,814,592 | 1,349,416 | 527 / 279 | 3,115 / 22 | 3 |
| RexxCPS | 5 | 0.28 s | 18,710,528 B | 320,100 / 19,679,232 | 13,538,512 | 22,872 / 22,636 | 17,389 / 2,379 | 29 |

Adrian explicitly accepted a seconds-scale proof-analysis budget rather than
requiring the roughly 50 ms ordinary-assembly baseline.  The accepted result
is well within that budget and memory remains bounded per procedure.

Two implementation forms were rejected and remain replayable in Git history:

1. recursive dynamic-storage classification reached 82.51 s before it was
   terminated; sampling attributed the CPU to repeated recursive traversal;
2. eager materialization of all read/write components completed RexxCPS in
   0.78 s but peaked at about 305 MB, including individual procedures retaining
   about 245 MB and 257 MB.

Generation-marked traversal removed the repeated recursion.  Restricting
diagnostic materialization to real derivation sites removed the dense
point/component behavior without narrowing the on-demand query API.

## Ordinary assembler-cost result

Two warmups and 30 balanced/interleaved elapsed rounds compared the frozen
Gate 0 binary with the final Stage 5 ordinary Release binary.  Peak RSS used
ten separately interleaved samples.  Pre/post load average was
`{ 7.24 9.69 6.86 }` / `{ 2.37 7.24 6.23 }`; the high historical load reflects
the immediately preceding Debug fixture build, while pairing controls the
same live session.

| Workload | Frozen median | Stage 5 median | Elapsed delta | Frozen RSS | Stage 5 RSS | RSS delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards | 0.057711482 s | 0.061113476 s | +5.895% (+3.402 ms) | 8,519,680 B | 9,076,736 B | +557,056 B (+6.538%) |
| Towers | 0.019920468 s | 0.019938945 s | +0.093% | 4,669,440 B | 5,054,464 B | +385,024 B (+8.246%) |
| RexxCPS | 0.054520964 s | 0.054525971 s | +0.009% | 9,740,288 B | 9,912,320 B | +172,032 B (+1.766%) |

The Richards increase is disclosed rather than treated as zero; it is 3.402 ms
absolute and all ordinary cells remain in the tens-of-milliseconds range.  No
RSS cell crosses the combined greater-than-5%-and-1-MiB escalation rule.
Ordinary assembly does not instantiate the Stage 5 cache.

## Gate 5 verdict

Gate 5 passes.  Storage and component identities are sparse, write-once,
epoch-cached, edge-specific, demand-driven and fail closed.  The canonical
inlined RexxCPS image completes within the explicitly agreed seconds-scale
analysis budget, output remains byte-identical, and Stage 6 may add proof
queries plus old/new decision parity without selecting a broader rewrite.
