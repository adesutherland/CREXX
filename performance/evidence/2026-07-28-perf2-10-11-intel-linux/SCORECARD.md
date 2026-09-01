# PERF2-10/11 Intel Linux scorecard

Status: **initial Linux x86-64 campaign complete; no candidate selected**

This scorecard consolidates the retained GCC/Clang formal baseline, schema-5
profiles and ordinary-Release Intel PMU evidence. Raw files remain
authoritative. Absolute Mac/Linux timing and the separate GCC/Clang capture
sessions are not paired before/after comparisons.

## Correctness and products

| Lane | Result |
| --- | ---: |
| GCC Debug | 1,925/1,925 |
| Clang Debug | 1,925/1,925 |
| GCC Release | 1,925/1,925 |
| Clang Release | 1,925/1,925 |
| GCC ASan/LSan | 1,925/1,925 |
| Clang UBSan diagnostic | 1,925/1,925 |

| Ordinary Release product | GCC bytes | Clang bytes |
| --- | ---: | ---: |
| `rxvm` | 3,514,896 | 1,197,872 |
| `rxbvm` | 3,448,240 | 1,181,488 |

The exact source patch, product hashes, CMake caches, compile commands and
suite logs are retained in `provenance/` and `logs/`.

## Formal timing

Higher ratios are better. The common aggregate contains Sieve, Permute,
Bounce, Richards and Base64 only.

| Compiler | VM | ooRexx geometric mean | NetRexx geometric mean |
| --- | --- | ---: | ---: |
| GCC | `rxvm` | 2.264305x | 0.703810x |
| GCC | `rxbvm` | 2.198584x | 0.683381x |
| Clang | `rxvm` | 2.548345x | 0.808085x |
| Clang | `rxbvm` | 2.582703x | 0.818980x |

| Selected cell | GCC `rxvm` | GCC `rxbvm` | Clang `rxvm` | Clang `rxbvm` |
| --- | ---: | ---: | ---: | ---: |
| Richards / ooRexx | 0.350662x | 0.348219x | 0.342747x | 0.341700x |
| Richards / NetRexx | 0.172139x | 0.170940x | 0.172875x | 0.172347x |
| Base64 / ooRexx | 0.723087x | 0.710393x | 0.875639x | 0.838559x |
| Base64 / NetRexx | 0.671923x | 0.660127x | 0.844723x | 0.808953x |
| Towers / ooRexx | 0.477614x | 0.466631x | 0.467988x | 0.462112x |
| RexxCPS / ooRexx | 1.175370x | 1.140760x | 1.164950x | 1.168880x |

Towers is outside the common aggregate and NetRexx Towers is a labelled
object/JVM control. RexxCPS is the separate cREXX 2.2d versus canonical
Classic rate row.

The two compiler lanes were captured in separate sessions. ooRexx and NetRexx
medians show material session drift, so comparing GCC and Clang ratios does not
select a compiler. A future compiler/layout verdict requires paired,
balanced/interleaved same-session samples.

## Native PMU

The GCC lane is the complete diagnostic authority. The table below reports the
optimized `rxvm` focus cells; `rxbvm` raw rows and both compiler controls are in
`native-perf/stat-summary.csv`.

| Workload | IPC | Branch miss | Retiring | Front-end bound | Back-end bound | Indirect miss |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards | 1.766 | 0.58% | 29.8% | 7.5% | 50.6% | 6.43% |
| Base64 | 2.053 | 0.26% | 38.0% | 4.8% | 51.0% | 2.09% |
| Towers | 2.087 | 0.50% | 37.9% | 12.4% | 37.6% | 8.92% |
| RexxCPS | 2.321 | 0.36% | 45.9% | 42.0% | 2.5% | 2.92% |

Top-down category totals can differ slightly from 100% because each kernel
event reports its hardware estimate. Every event pass was isolated,
non-multiplexed and 100% scheduled.

RexxCPS absolute counters are normalized per 10 million clauses because its
canonical default may self-calibrate separately in each pass. Raw counts,
effective counts and scales are retained in
`native-perf/rexxcps-counter-normalization.csv`.

The bounded compiler control exposes direction changes but is not a formal
verdict. Relative to the separate GCC formal medians, Clang points faster for
Richards and Base64, roughly neutral for Sieve, and slower for Towers.
Instructions, I-cache behavior and helper outlining also change materially.

## Hot symbols

| Compiler/cell | Dominant sampled symbols |
| --- | --- |
| GCC Richards `rxvm` | `copy_value` 55.39%, `run` 44.03% |
| GCC Richards `rxbvm` | `copy_value` 56.63%, `run` 42.69% |
| Clang Richards `rxvm` | `copy_value` 77.15%, `maybe_trim_attribute_storage` 16.54%, `run` 5.44% |
| GCC Towers `rxvm` | `copy_value` 42.96%, `run` 23.33%, `clear_value_contents` 10.24%, storage reset 5.84% |
| Clang Towers `rxvm` | `copy_value` 46.50%, `clear_value_contents` 17.85%, storage reset 8.89%, `run` 6.51% |
| GCC Base64 `rxvm` | `run` 95.59%; each libc conversion/string helper below 1.5% |
| Clang Base64 `rxvm` | `run` 92.97%; each libc conversion/string helper below 1.9% |
| RexxCPS, all four cells | `run` 35.00-44.34%; decimal parse/format/arithmetic, string movement and formatted output dominate the remaining samples |

All 16 focus profiles have zero lost samples. `hot-symbols.csv`,
`hot-offsets.csv`, `hot-instructions.csv` and `annotations/*-disassembly.txt`
retain symbol and instruction-level evidence.

## Schema-5 mechanism

The optimized bounded profile rows show:

| Workload | Dynamic VM instructions | Copy operations/bytes | Clear/reset/destroy | Allocation requests/bytes |
| --- | ---: | ---: | ---: | ---: |
| Richards | 8,615,245 | 56,902,732 / 451,730,841 | 860,895 / 3,978,206 | 735,010 / 172,258,936 |
| Base64 | 46,724,369 | 2,221,507 / 18,789,128 | 1,382 / 1,075,088 | 1,700 / 608,160 |
| Towers | 7,143,456 | 26,789,582 / 206,454,410 | 31,345,134 / 41,935,002 | 17,243,733 / 5,864,230,528 |
| RexxCPS | 6,110,601 | 243,278 / 10,330,331 | 353,939 / 15,273,748 | 330,174 / 58,629,368 |

For Richards the copy count is much larger than the top-level opcode count,
showing recursive attribute/value copying rather than dispatch alone. Towers
combines recursive copies with teardown and allocation pressure. Base64 is a
large dispatch/string-copy loop with very little call-frame churn after
optimization. RexxCPS adds a distinct conversion/allocation and decimal/string
lane: the same optimized GCC row records 673,511 conversions.

## Decision

The initial Linux campaign has done enough. It identified mechanisms and
compiler sensitivity without selecting an implementation:

1. Test scalar/no-payload value-copy fast paths against compiler/RXAS copy
   specialization in disposable macOS harnesses.
2. Isolate Richards/Towers attribute trim and teardown shapes without reviving
   previously rejected reset-list, slab or broad-layout ideas absent new proof.
3. Isolate Base64 `SCOPY_REG_REG`, string-state and dispatch/code-layout
   ceilings.
4. Rank the alternatives, batch one coherent production slice and apply the
   mandatory first ordinary Release verdict on macOS.
5. Return to Linux only for an accepted selected candidate, Linux-specific PMU
   or sanitizer work, or final platform validation.

Linux ARM64, same-machine Windows, candidate selection and the final
cross-platform/default-VM decision remain open. No Gate E or final superiority
claim is made.
