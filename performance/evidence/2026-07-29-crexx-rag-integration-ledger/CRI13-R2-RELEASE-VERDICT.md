# CRI-13 Option B R2 ordinary Release verdict

Status: **accepted by Adrian on 2026-07-30 — proportional closeout authorized**

Date: 2026-07-30

Governed ID: `CAP-01-J02`

## Outcome first

The approved parse-time classification countermeasure is favorable. Optimized
f32 projection is 295 us on `rxvm` and 326 us on `rxbvm`, or 1.14x and 1.22x
the same-cell prototype. Both are comfortably inside the predeclared 2x
ceiling. One parse, one production projection and one packed scan is
1.772/2.1695 ms, 9.60%/11.03% of the retained current-composition total and
inside the 25% rule.

The implementation also closes a newly minimized status edge. The current
`STOF` path signals on some `strtod` `ERANGE` cases. One signal handler encloses
the whole projection loop and translates that condition to the public method's
documented `-6`, cleared output and element diagnostic. Optimized RXAS contains
one `sigpush` before the element loop, not one per element. `node_float`
similarly translates the same conversion condition to its documented `-4`.

All focused and sampled checksums, bytes and behavior pass. No C wrapper class,
new VM operation, public RXAS/RXBIN encoding, ABI change, broad closeout or
CRI-14 work has begun.

## Frozen implementation

While `_json_scan_number` is already consuming each token, it now derives:

- whether any significant digit is nonzero; and
- whether the adjusted decimal exponent lies in the existing binary64
  preclassification band.

The two facts are encoded as private flags in the already-zeroed u32 node field
at offset 36. The node remains 40 bytes, `index_bytes()` is unchanged, and the
private ephemeral index remains neither an ABI nor a serialized format.

`node_f32_array` reads one flag word, materializes the required source token
once, converts to binary64, checks f32 range, writes f32, and retains the
nonzero-to-stored-zero underflow test. The adverse R1 projection-time byte loop
and its two whole-source copies per element are gone.

Frozen hashes:

| Artifact | SHA-256 |
| --- | --- |
| `lib/rxfnsb/rexx/rxjson.crexx` | `7916d23df7cc488adfee54d4b25e504fa3afd47a8746fb7d15a6f401bde83d77` |
| projection contract | `394e49daea830a9a8ace9921775b6f1a17c34ac0252710c1c482e22e1a1a6395` |
| numeric benchmark | `ffe920aed41293f767356a12db398b5a9a10272b50f2a7d1ed2815e15f784932` |
| unchanged JSON benchmark | `f4ce6bcd3b53f9be3a7ffab3357e99d39b5687562bed43dac1b385276718257b` |
| optimized `node_f32_array` RXAS excerpt | `b0a88c154ddfa36a11bd44248bc95e045e7970eccb73693993963a0cc3c18387` |

The production hash includes the scoped conversion catch added after Adrian
observed that the existing signal can be caught.

## Correctness and RXAS proof

The maintained projection matrix now includes `1e-320`, `1e-324`, and
`1.8e308`. The frozen R1 product lets all three escape as `CONVERSION_ERROR` on
both VMs. R2 returns `-6`, clears the packed output and reports the affected
element. Matching `node_float` cases return `-4`.

Results:

- focused Debug projection matrix: 5/5;
- broader Debug document/noisy/projection/numeric matrix: 17/17;
- focused ordinary Release projection matrix: 5/5;
- formal Release numeric samples: 40/40;
- formal unchanged-parser samples: 24/24.

Raw focused logs and SHA-256 values:

| Evidence | SHA-256 |
| --- | --- |
| `/tmp/cri13-r2-focused-build.log` | `d129c02633b3bbe5673085cd5f89236a13fbe80c241f0cc6c69ffeb890a58a72` |
| `/tmp/cri13-r2-focused-ctest.log` | `7fc70836dc0baf068d03ca484c7f3b7b820b0397611d173b8312511646d70143` |
| `/tmp/cri13-r2-focused-matrix-ctest.log` | `b2e4003115dfdeb747451008a8e7839716785d1c53ed518d3b0674e90244af03` |
| `/tmp/cri13-r2-release-build.log` | `d129c02633b3bbe5673085cd5f89236a13fbe80c241f0cc6c69ffeb890a58a72` |
| `/tmp/cri13-r2-release-focused-ctest.log` | `caa9eb6fd581b3fee11d7ef42114f316bacd4ad7e193e8f598ee482627090340` |

The optimized RXAS excerpt has one `sigpush "CONVERSION_ERROR"` before the
`do while child > 0` label and one corresponding handler after the loop. The
old `_json_f32_span_classify` procedure is absent. Each successful element now
uses node type/start/end/flags reads, one token slice conversion, f32 range and
store checks, and the sibling link. This matches the intended mechanism.

## Ordinary Release product and sampling

The profiling-off Release product remains
`/tmp/crexx-cri09-release-a2.N4ELYs/build`, configured with
`CMAKE_BUILD_TYPE=Release` and `CREXX_VM_PROFILING=OFF`. VM executables are
unchanged from accepted A2. The updated linked library SHA-256 is
`97c9646b0b2a183165d3832a9ceb91aa63becee0907d74395625d7e32e5a2d45`.

The numeric verdict used two warmups and ten serial recorded samples in each
opt/no-opt x VM cell with rotating starts and no removed outliers. The parser
guard used three warmups and twelve alternating optimized samples per VM. All
40 numeric and 24 parser samples passed. The Apple M5 host was on AC and
low-power mode was off.

Evidence root: `/private/tmp/crexx-cri13-r2-release.3XfqaW/`.

| Evidence | SHA-256 |
| --- | --- |
| `numeric-raw.log` | `048e76e03f286d5762e9031315f0fcf20c2a744abd9930b5387e6761f5d8a44c` |
| `numeric-medians.tsv` | `ed48c7a00f3b7a6dd01b11f14bcee23579de3f0402b1e752ba0bf325fce43279` |
| `parser-raw.log` | `c8b9012d82a170f76834ce29ac1cda664b1b61d887f5a01478db906ce905593d` |
| `parser-medians.tsv` | `5c860deb0232a4bace9fb3f027e087f058e8fdb32e19e8a4efd02896cc2841aa` |
| `verdict-calculations.txt` | `7b81d3d5ccb154493fe85587b1a28026fca2fbd35cd30251093b8117c057c187` |
| `manifest.txt` | `1ca3cef3ce48c4f5160d945234926859a52dba6adf742e62e725f26efc1e2980` |

## Numeric medians

Times are microseconds. Retained current totals and prototype medians are the
unchanged acceptance references used by the first B and R1 verdicts.

| Cell | retained current total | R2 f32 projection | R2 i64 projection | R2 total | R2/current | prototype projection | R2/prototype | peak RSS bytes |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm -n` | 18,503.5 | 289 | 1,585.5 | 1,787.5 | 9.66% | 256.65 | 1.13x | 28,606,464 |
| `rxvm opt` | 18,460.5 | 295 | 1,575.5 | 1,772 | 9.60% | 258.65 | 1.14x | 28,672,000 |
| `rxbvm -n` | 19,702 | 309.5 | 1,814.5 | 2,164 | 10.98% | 285.65 | 1.08x | 28,663,808 |
| `rxbvm opt` | 19,665.5 | 326 | 1,787.5 | 2,169.5 | 11.03% | 266.35 | 1.22x | 28,672,000 |

Against adverse R1, optimized projection falls 94.57% on `rxvm` and 94.44% on
`rxbvm`. Optimized total is 0.87% faster than non-optimized on `rxvm` and 0.25%
slower on `rxbvm`, inside the 10% inversion rule. Optimized RSS is 4.51%/4.57%
above the retained pre-B diagnostic, not a material regression. Bytes and
checksums are exact in all samples.

## Unchanged JSON benchmark

Every legacy compatibility cell remains inside its 25% pre-edit guard:

| VM | valid | deep get | tail get | count | members |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | +20.88% | +10.54% | +17.45% | -37.74% | +16.45% |
| `rxbvm` | +22.42% | +13.26% | +16.23% | -36.67% | +15.33% |

Construction plus retained path use remains 78.65%/81.91% faster than matched
legacy repeated parsing; once-resolved-node use remains 94.46%/94.85% faster.
Scanner medians are 401.5/502.5 us, 1.77%/4.58% above accepted A2 and well
inside the guard. The expected small first-parse cost is the price of deriving
the flags in the one existing scan.

## Verdict against unchanged rules

| Rule | Verdict | Evidence |
| --- | --- | --- |
| CRI-09 behavior remains correct | pass | focused 17/17; parser 24/24 |
| Byte/count/type/range/failure contract | pass | exact boundary matrix, including caught conversion edges |
| Optimized total <=25% retained current total | pass | 9.60% `rxvm`; 11.03% `rxbvm` |
| f32 projection <=2x same-cell prototype | pass | 1.08x--1.22x |
| No optimizer inversion >10% | pass | -0.87% / +0.25% total |
| Exact bytes/checksums and no material RSS regression | pass | all 40/40 exact; <4.6% RSS |
| Unchanged JSON guards | pass | every legacy and retained-document cell passes |

## Alternative D and the bounded converter follow-on

R2 confirms that Alternative D belongs to the standard family of fused lexical
classification/cached parse metadata approaches. Its exact private flags are a
CREXX specialization.

The bounded binary ASCII span-to-float idea remains technically sound, and
offset plus length is the correct range contract. It is not required to accept
CRI-13: R2 is already close to the prototype and passes every rule. Adding the
helper now would turn a closed local mechanism into a public RXAS/RXBIN and
converter architecture decision.

The current conversion implementation still merits independent review:
`rx_string_to_double` copies to a NUL-terminated local or heap buffer, delegates
to locale-sensitive `strtod`, and collapses `ERANGE`; `string2integer` always
heap-allocates. That generic work is queued, not active, as `PERF2-07-C01`.
It must inventory current `STOF` compatibility including signs, whitespace,
`inf` and `nan`, compare a bounded correctly rounded converter, and decide a
public instruction only from retained correctness and performance evidence.
The JSON tokenizer and decimal-to-binary conversion engine are deliberately
treated as different problems.

## Recommendation and smallest exact decision

Decision: **accepted by Adrian on 2026-07-30**.

Accept frozen R2 at `rxjson.crexx` SHA-256
`7916d23df7cc488adfee54d4b25e504fa3afd47a8746fb7d15a6f401bde83d77`
as the CRI-13 Option B production candidate. Acceptance authorizes
proportional CRI-13 Debug/ASan/documentation closeout and then the previously
approved measured Option C class comparison. It does not authorize
`PERF2-07-C01`, a new VM instruction, general conversion changes, CRI-14 or
crexx-rag work.

## Paste-ready continuation prompt

```text
Resume /Users/adrian/CLionProjects/CREXX at the CRI-13 R2 ordinary Release
stop using performance/CREXX-RAG-INTEGRATION-WORKLIST.md and
performance/evidence/2026-07-29-crexx-rag-integration-ledger/CRI13-R2-RELEASE-VERDICT.md.

I accept frozen CRI-13 R2 at rxjson.crexx SHA-256
7916d23df7cc488adfee54d4b25e504fa3afd47a8746fb7d15a6f401bde83d77.
Complete proportional CRI-13 Debug/ASan/documentation validation, then perform
the previously approved measured Option C class comparison with typed by-value
reads and writes and stop at its required verdict. Keep PERF2-07-C01 queued and
inactive: do not add a binary span conversion opcode or change general STOF,
integer conversion, public RXAS/RXBIN, ABI or serialized formats. Do not start
CRI-14 or touch crexx-rag.
```
