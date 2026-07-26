# Current Level B BIF and callable census

This census is for exact product commit
`6567f0ba23f20623e01322f5a62323b2347ab09d`, compiled with the accepted
PERF2-03 inliner. Historical PERF2-01 counts are not used to select candidates.

## Complete callable and image surface

The freshly linked Level B library exports 561 callables. One is the scratch
path-bearing `_elapsed` bootstrap entry; the 560 Level B source callables are
the maintained census population:

| Population | Count |
| --- | ---: |
| Level B exported callables | 560 |
| top-level procedures | 280 |
| member methods/factories | 280 |
| callables with versioned I6 inline bodies | 238 |
| executable Level B bodies in the linked image | 630 |
| declarations/import stubs excluded from executable bodies | 74 |
| Level B source/RXBIN modules | 142 |
| modules with a Markdown API/RexxDoc companion | 134 |

The linked library contains 54,224 executable bytecode instructions and is
859,969 bytes. All 142 freshly compiled Level B module RXBINs are byte-identical
to the retained accepted product. The freshly linked library differs by 16
bytes only because `_elapsed` embeds a longer detached scratch path; its
instructions and semantics are identical.

The exact signatures, source modules and I6 availability are retained in
`census/exported-callable-signatures.tsv`. Per-module source, instruction,
locals, RXAS and RXBIN measurements are in `census/library-module-census.tsv`.

## Seed-family static product

"Instructions" below are executable bytecode instructions in the standalone
module. Peak locals is the maximum procedure register declaration. I6 is the
current imported-body payload availability; it is not a claim that every call
shape can bind or profitably inline.

| Family | Modules | opt instructions | opt peak locals | opt RXAS bytes | opt RXBIN bytes | no-opt instructions | no-opt peak locals | no-opt RXAS bytes | no-opt RXBIN bytes | I6 modules |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| LEN | `length` | 8 | 2 | 2,663 | 2,040 | 8 | 2 | 2,662 | 2,040 | 1/1 |
| SLC | `substr,left,right` | 159 | 17 | 91,536 | 28,848 | 159 | 17 | 91,524 | 28,848 | 3/3 |
| WRD | `word,words,wordpos` | 181 | 27 | 51,889 | 21,336 | 181 | 27 | 51,878 | 21,344 | 1/3 |
| SRC | `pos,lastpos` | 75 | 10 | 18,240 | 8,384 | 76 | 10 | 18,259 | 8,392 | 1/2 |
| CAS | `upper,lower` | 12 | 2 | 5,346 | 4,056 | 12 | 2 | 5,346 | 4,056 | 2/2 |
| DAT | `datatype` | 620 | 49 | 88,308 | 30,271 | 503 | 38 | 71,085 | 25,561 | 0/1 |
| CNV | `b2d,b2x,c2d,c2x,d2b,d2c,d2x,x2b,x2c,x2d` | 647 | 30 | 171,901 | 70,167 | 700 | 23 | 180,510 | 73,827 | 5/10 |

These 22 seed modules account for 1,702 executable instructions and 5,528
linked code bytes, 3.139% and 3.110% of the complete library respectively.
Large non-seed modules such as `regex`, `trace`, `rxjson`, `rxhttp` and
`_address` dominate source/image size, but source size without current dynamic
attribution is not a PERF2-04 selection rule.

## Current workload images

| Image | Mode | executable instructions | peak locals | RXAS bytes | RXBIN bytes | residual selected calls |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Base64 | optimized | 621 | 43 | 115,082 | 38,249 | none |
| Base64 | no-opt | 321 | 24 | 46,933 | 19,161 | `LENGTH` 4, `SUBSTR` 6, `POS` 1 static sites |
| RexxCPS | optimized | 1,402 | 105 | 220,731 | 77,438 | `UPPER` 4 timed; `WORD` 1 setup; `LEFT` 1 reporting; `POS` in TRACE support |
| RexxCPS | no-opt | 934 | 104 | 127,280 | 48,931 | `LENGTH` 1, `SUBSTR` 2, `WORD` 2, `UPPER` 4, `LEFT` 1, `RIGHT` 2 plus helpers |

The optimized images are larger because inlined bodies and trace/source
metadata replace calls. The comparison is therefore machine work and decisive
wall time, not source or RXBIN size alone.

## Timed-site ownership

RexxCPS's final profile trial is adaptive. Counts below use the disclosed
`(initial_count + effective_count) * 100` denominator and are also stated per
one 1,000-clause timed iteration so the two VMs are comparable.

| Family/site | Phase | Static sites | Dynamic work per timed iteration | current optimized result | exact census observation |
| --- | --- | ---: | ---: | --- | --- |
| `UPPER`, RexxCPS line 191 | timed kernel | 4 | 56 calls/scans | not inlined: computed/literal actual cannot bind exposed formal | counts bundle: 280,000 `rxvm`, 212,800 `rxbvm`; timing bundle: 229,600/224,000 calls |
| `LENGTH`, RexxCPS line 163 | timed kernel | 1 | 28 `strlen` uses | inlined | no residual call; direct-result cleanup is the ceiling question |
| `SUBSTR`, RexxCPS lines 159,165 | timed kernel | 2 | 14 three-argument slices plus 28 one-character slices | inlined | validation/result scaffolding remains; exact arguments are compile-time constants at both sites |
| `WORD`, RexxCPS line 166 | timed kernel | 1 | 28 first-word extractions/comparisons | inlined | `strlen`/blank scans/slice/result remain; the one residual `WORD` call is setup at line 107 |
| `LEFT`, RexxCPS line 226 | reporting | 1 | zero timed-kernel work | residual normal call | one dynamic call after the timer |
| `POS`, TRACE helper | setup/control | library support | zero clause-kernel work | residual normal call | 400 calls in the fixed TRACE controller path before clause timing; not a RexxCPS cause |
| `LENGTH`/`SUBSTR`/`POS`, Base64 | whole decoder | 4/6/1 source sites | input-dependent repeated scan/slice/search | all inlined | no residual BIF call; current 500-repetition product profile executes 1,370,502 `strlen`, 685,000 `setstrpos`, 685,000 `substring`, 683,000 `strpos` |

The current Base64 control retires 46,726,464 instructions and performs
2,907,504 string-copy operations transferring 1,877,756,578 bytes at 500
repetitions. Its decoder self time is 1.363 s (`rxvm`) and 1.446 s (`rxbvm`);
call entry/exit overhead is below 0.02%. The body algorithm and value movement,
not BIF dispatch, own the gap.

For UPPER's timing-profile cell, the exact procedure decomposition is:

| VM | calls | elapsed ms | self/body ms | entry ms | exit ms | `strupper` instruction ms |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 229,600 | 83.731 | 20.975 | 49.373 | 13.382 | 5.237 |
| `rxbvm` | 224,000 | 81.723 | 20.604 | 47.900 | 13.220 | 4.956 |

This makes the call boundary material, while also showing that merely removing
the call leaves result scaffolding and the case scan to optimize.

## Inlining result by family

- `LENGTH`, `SUBSTR`, `LEFT`, `RIGHT`, `WORD`, `POS`, `UPPER` and `LOWER` have
  current I6 bodies. Local/source-import/binary-import eligibility is guarded by
  the retained focused tests.
- The timed `LENGTH`, `SUBSTR` and `WORD` calls accept existing imported body
  proof and inline in the optimized RexxCPS image.
- Base64 has no residual seed-family call in its optimized image.
- `UPPER`'s exact body accepts a direct variable-symbol actual and becomes one
  inlined `strupper`, but a literal or computed actual fails
  `inline.bind.actuals`. The four current RexxCPS calls are exactly that rejected
  form. `diagnostics/upper-inline-diagnostic.txt` retains the compiler proof.
- `WORDS`, `WORDPOS`, `LASTPOS` and the larger typed conversion helpers do not
  have current imported body payloads in all forms, but no current optimized
  Tier A dynamic evidence justifies opening that transport work here.

## Ranked disposition

| Rank | Family | Disposition | Current owner/evidence | PERF2-04 decision |
| ---: | --- | --- | --- | --- |
| 1 | `UPPER` | inline/cleanup opportunity | residual hot calls; exact `strupper` body; direct and constant ceilings win on both VMs | select compiler exact-body composition, then trace-safe constant fold; no new opcode/native BIF |
| 2 | `SUBSTR` timed constants | inline/cleanup opportunity | current body already inline; exact sites still validate/slice constants | select compiler constant/value composition: +11.55%/+8.08% ordinary Release ceiling; no opcode |
| 3 | `WORD` timed predicate | inline/cleanup opportunity | current body inline; exact use needs only first-word equality and has constant operands | select compiler consumer/value composition (+6.71%/+6.81% exact ceiling); no benchmark-shaped assist |
| 4 | `LENGTH` timed dynamic value | inline/cleanup opportunity | current one-primitive body already inline but carries result scaffolding | reject dynamic F03 result-placement slice as neutral; retain exact constant `LENGTH` certificate through P04-CEX1 |
| 5 | Base64 string position/copy | algorithm opportunity | BIFs already inline; repeated slicing/search/copy owns >90% removable work | route a separately specified Level B Base64 API to CAP-03; common benchmark unchanged |
| 6 | `POS` and related search | clean Level B already at ceiling | one `strpos` primitive after validation; no timed residual wrapper | no PERF2-04 change |
| 7 | `LEFT`/`RIGHT` | not currently material in Tier A; general constant-use opportunity | reporting/no current Tier A timed use; constant bodies contain validation, scans, slices and padding | retain Level B fallback and add exact success-domain certificates through P04-CEX1 |
| 8 | `WORDS`/`WORDPOS`/`LASTPOS` | not currently material | no optimized Tier A dynamic site | retain; reopen only with current multi-site evidence |
| 9 | `DATATYPE` and typed BIF conversions | not currently material | size-significant, but zero optimized Tier A BIF calls | no PERF2-04 assist; hot language conversions route to PERF2-07 |
| 10 | `LOWER` | not currently material in Tier A; general constant-use opportunity | simple primitive body, no timed site; exact constant result removes the complete scan | retain Level B fallback and add exact certificate through P04-CEX1 |

The final placement decisions and the neutral/negative controls are expanded in
`DESIGN-PANEL.md`. Machine-readable site and disposition rows are retained in
`census/selected-workload-sites.tsv` and `census/current-dispositions.tsv`.
