# PERF3-12D generated-binding register-reuse verdict

## Verdict

The controlled rerun removes the product-contamination error from the first
comparison.  It freezes one profiling-off Release `rxtvm`, `rxbvm` and linked
library, then crosses the optimized compiled-PARSE RXC output before and after
the internal-binding metadata fix with the old and new RXAS behavior.

Direct register reuse is the best structural form and the fastest median cell
on both VMs.  Against the matching metadata-free old-RXAS control, new RXAS
direct reuse improves median CPS by 0.571154% on `rxtvm` and 0.595225% on
`rxbvm`.  Against the original metadata-retaining old-RXAS form, the complete
RXC-metadata plus RXAS-reuse change improves median CPS by 0.436060% and
0.146506%.  These are small gains: the paired interval is clear only for the
isolated `rxbvm` direct-reuse comparison, so the timing claim remains a modest
positive tendency rather than a broad throughput result.

Removing generated-variable metadata is not a runtime regression.  With old
RXAS, the two RXC outputs have identical executable instruction streams and the
paired timing is neutral.  The metadata fix matters because it tells RXAS that
the generated PARSE assignment is internal, allowing all later exact-string
uses to consume the dominating register directly.

`link` is not proved faster than `scopy`.  An enlarged dedicated 80-pair panel
puts the ratio-of-medians result at +0.305177%/+0.085937% for
`rxtvm`/`rxbvm`, but paired estimates remain mixed and both 95% intervals cross
zero.  Combining that panel with the 40-pair matrix gives 120 pairs per VM:
absolute medians favor `link` by 0.268590%/0.228740%, while paired geometric
mean changes are +0.092393%/-0.114199%.  The evidence therefore supports
direct reuse, but does not support a claim that a remaining `link` dispatch is
faster than `scopy` in RexxCPS.

The earlier cross-product -2.198028%/-2.618265% regression is retracted and is
not used anywhere in this verdict.

Adrian accepted the production choice on 2026-08-08: use direct register reuse
for internal bindings and prefer `link` over `scopy` where metadata requires an
exact destination register.  This is an architectural scalability choice for
large strings and later binary/object values, not a claim that the current
small-string RexxCPS panel proves `link` faster.  RXAS must prove that the
persistent alias is safe across both bindings' lifetimes; when it cannot, it
retains the original materialisation.

## Controlled variants

| ID | optimized RXC state | RXAS state | hot PARSE pattern form |
| --- | --- | --- | --- |
| A | after compiled-PARSE fix, before internal metadata fix | old, no H02 reuse | four `load *,"b"` operations; metadata present |
| B | after internal metadata fix | old, no H02 reuse | four `load *,"b"` operations; metadata absent |
| C | before internal metadata fix | new H02 | one `load` plus three `link`; metadata blocks direct reuse |
| D | after internal metadata fix | new H02 | one `load`; all four consumers use the same register |
| E | before internal metadata fix | new H02 `scopy` control | one `load` plus three `scopy` |

The RXC outputs for A/C/E and B/D differ only by the generated `.meta` records.
A and B have identical 1,069-instruction executable streams.  C and E also
have 1,069 instructions and differ only in the three `link`/`scopy` opcodes.
D has 1,066 instructions and no replacement dispatch.

The canonical source is unchanged and has SHA-256
`2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`.
Every recorded process reported `effective_count=500`, `calibrated=1`, and
`PASS: RexxCPS 2.2d cREXX port`.

## Frozen-product 40-round matrix

Two warmups per variant and VM were discarded.  Forty recorded rounds used
balanced cyclic and reverse-cyclic orders: every variant occupied every
position eight times, and VM order alternated.  All 400 recorded processes
passed; no sample was removed or replaced.

| ID | form | `rxtvm` median CPS | change from A | `rxbvm` median CPS | change from A |
| --- | --- | ---: | ---: | ---: | ---: |
| A | metadata + old RXAS: four loads | 42,658,006.0 | control | 45,141,390.0 | control |
| B | no metadata + old RXAS: four loads | 42,600,705.0 | -0.134326% | 44,940,030.5 | -0.446064% |
| C | metadata + new RXAS: three links | 42,691,038.0 | +0.077434% | 45,181,608.0 | +0.089093% |
| D | no metadata + new RXAS: direct reuse | 42,844,020.5 | +0.436060% | 45,207,525.0 | +0.146506% |
| E | metadata + new RXAS: three string copies | 42,707,909.0 | +0.116984% | 45,036,119.0 | -0.233203% |

The comparisons that isolate each requested change are:

| comparison | meaning | `rxtvm` median change | `rxbvm` median change |
| --- | --- | ---: | ---: |
| B versus A | metadata suppression alone under old RXAS | -0.134326% | -0.446064% |
| C versus A | new RXAS `link` versus old repeated loads, metadata retained | +0.077434% | +0.089093% |
| D versus B | new RXAS direct reuse versus old repeated loads, metadata absent | +0.571154% | +0.595225% |
| D versus A | complete metadata plus RXAS reuse implementation | +0.436060% | +0.146506% |
| C versus E | `link` versus `scopy` inside the matrix | -0.039503% | +0.323050% |

B-versus-A paired geometric-mean changes are -0.192390% and -0.256885%, with
95% intervals spanning zero; its median paired changes are +0.029554% and
-0.004404%.  The absolute-median differences in the table must therefore not
be described as a metadata regression.

D-versus-B paired geometric-mean changes are +0.514487% and +0.509702%.  The
95% intervals are [-0.023020%, +1.267171%] on `rxtvm` and
[+0.190047%, +0.832742%] on `rxbvm`, with 25/40 and 28/40 favourable pairs.

## Enlarged `link` versus `scopy` panel

The dedicated follow-up discarded two warmups per form and VM, then recorded
80 paired rounds per VM.  Order alternated every round and VM order alternated.
All 320 recorded processes passed; no sample was removed or replaced.

| VM | `link` median CPS | `scopy` median CPS | ratio of medians | paired geometric mean change | 95% paired interval | `link`-favourable pairs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxtvm` | 42,795,889.5 | 42,665,683.5 | +0.305177% | +0.227154% | [-0.017549%, +0.457293%] | 49/80 |
| `rxbvm` | 45,159,632.5 | 45,120,857.0 | +0.085937% | -0.129669% | [-0.724374%, +0.392974%] | 40/80 |

As a sensitivity check, the 40 matrix pairs and 80 dedicated pairs can be
combined because they use the same frozen binaries and linked library:

| VM | pairs | ratio of combined medians | paired geometric mean change | 95% paired interval | `link`-favourable pairs |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxtvm` | 120 | +0.268590% | +0.092393% | [-0.098142%, +0.276330%] | 67/120 |
| `rxbvm` | 120 | +0.228740% | -0.114199% | [-0.552539%, +0.279173%] | 64/120 |

The intervals are deterministic paired-bootstrap intervals over log CPS
ratios.  They are descriptive uncertainty bounds, not permission to remove
samples; the two wide `rxbvm` pairs remain included.

## Frozen product and inputs

| artifact | SHA-256 |
| --- | --- |
| `rxtvm` | `2c4b785017d67edb86798cd3f2503d0bcc9011faf8b8dacb7c035c2a38eb68ab` |
| `rxbvm` | `29d635397f8f62ff08bd40d7e5002a37e773f4d0705c37de9b1a8873f6c44610` |
| `library.rxbin` | `ba590b27c1a25f8fc6e124ddaf4eef323caf3c87742e71e1723c540237ac0333` |
| RXC output before metadata fix | `4b64d7448cf37970722a7ec1fbfa6b565ef3384831b397721bace2523f43fdbb` |
| RXC output after metadata fix | `4bbba440b22c03a1257f310072181f919dfd60585cae50df2f1c4035b9adfcc2` |
| old RXAS, H02 absent | `6ec46f27ba3e0ac9ba891acdee323cb495edfdf4d5db0606157b029e75b4ac51` |
| new RXAS, `link` or direct | `82224ce6068ffb6a6be2f369cba563e3cad86a9798737a4b980c4a7a89e06730` |
| new RXAS, `scopy` control or direct | `383a9f3ece76f5dc941066ed42f8ebd6d32695ab6cb40b3abd1133c2de0f9bdc` |

The host remained on AC with low-power mode off.  Post-panel load averages
were 1.37/1.27/1.25.  Raw samples are retained in
`raw/rxc-rxas-matrix-40.csv` and `raw/link-scopy-dedicated-80.csv`.

## Counts and implementation boundary

The previously retained counts establish the structural ceiling:

| form | executed instructions | relevant hot operation |
| --- | ---: | --- |
| old repeated loads | 31,660,402 | 420,000 later literal loads above direct reuse; 56 calibration/control-path instructions differ |
| `scopy` | 31,660,346 | 420,000 `SCOPY_REG_REG` above direct reuse |
| `link` | 31,660,346 | 420,000 `LINK_REG_REG` above direct reuse |
| direct reuse | 31,240,346 | no replacement operation |

Direct reuse removes 420,000 executed instructions (-1.326581%) and reduces
the retained canonical RXBIN from 76,033 to 73,161 bytes (-3.777307%).  The
direct source RXAS has no variable `.meta` record beginning `__rxcpx_`; all
four `strpos` sites use the same dominating literal register.

This verdict covers compiler-generated PARSE string temporaries.  String,
binary and object values remain the intended storage-bearing families for the
same internal-binding/direct-reuse principle.  A general `link` rewrite still
requires proof that the destination is an internal binding or otherwise cannot
be observed or independently written for the alias lifetime.  Scalar integer
and float typed copies remain independent value copies.

## Closeout QA

The production `link` selection adds a fail-closed lifetime proof.  Both locals
must have independent initial storage, and neither binding may be independently
rewritten or exposed through a call/opaque window after linking.  A safe
metadata-visible case emits exactly one `link r2,r1`; two negative cases that
write the candidate or seed later retain both literal loads and report
`alias-lifetime-unsafe`.

The retained closeout logs record:

| check | result |
| --- | ---: |
| affected Debug build and 14-test matrix | pass, 14/14 |
| affected profiling-off Release build and 14-test matrix | pass, 14/14 |
| full fresh Debug build | pass |
| full Debug CTest, `--parallel 30` | pass, 2,000/2,000 in 216.15 s |

A final fresh compile reproduced the metadata-fixed RXC output hash
`4bbba440b22c03a1257f310072181f919dfd60585cae50df2f1c4035b9adfcc2`.
The optimized disassembly contains one `load r68,"b"`, no `load`, `link` or
`scopy` for `r69`-`r71`, and all four `strpos` sites consume `r68`.  Thus the
canonical path still uses direct register reuse; `link` is only the proved
metadata-visible fallback.

Logs are retained under `qa-closeout/`.  No additional timing was run after
the already accepted frozen-product verdict because the canonical path remains
the exact direct-reuse form and the selected fallback was already measured in
the 40-round matrix and 80-pair panel.

### Develop integration

The isolated implementation was committed as `ef6e3fd77` and integrated with
the newer local and remote `develop` histories.  Local `develop` already
contained `6cde2c509`, which fixes sparse queue-batch snapshot relocation by
keeping snapshots inline and re-pinning record mappings after entry-array
growth.  The PARSE branch had independently used heap-stable snapshots.  The
integration retains the existing inline/re-pin implementation and its
no-per-record-allocation property, while adapting the new batch consumer and
tests to the four-argument relocation contract.

An initial automatic combination passed a pointer's address as an instruction
snapshot and reproducibly crashed optimized Debug `rxas` on `rxjson.rxas`.
The corrected representation assembles that exact reproducer and passes:

| integrated check | result |
| --- | ---: |
| focused Debug RXAS proof/metadata matrix | pass, 4/4 |
| full profiling-off Debug build | pass |
| full profiling-off Release build | pass |
| focused Release PARSE, RXAS and RXQUEUE matrix | pass, 20/20 |
| full merged Debug CTest, `--parallel 30` | pass, 2,002/2,002 in 216.57 s |

The retained integration logs are under `qa-integration/`.  No post-selection
performance timing was repeated because the merge changes only snapshot
ownership/repinning and test integration; the accepted canonical direct-reuse
instruction form is unchanged.
