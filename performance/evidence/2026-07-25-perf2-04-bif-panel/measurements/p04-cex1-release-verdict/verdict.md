# PERF2-04 P04-CEX1 first ordinary Release verdict

P04-CEX1 extends the accepted general certified-call constant evaluator with
four independently proved core Level B entries: `rxfnsb.lower`,
`rxfnsb.length`, `rxfnsb.left` and `rxfnsb.right`. The current Level B bodies
remain the complete runtime fallback and behavior documentation. The compiler
does not infer eligibility from a BIF category or name: each entry pins the
exact callable, typed I6 summary and normalized body fingerprint before its
canonical evaluator may run.

## Qualification

- `LOWER`: all validated constant strings, using the canonical VM simple
  Unicode lowercase mapping.
- `LENGTH`: all validated constant strings, returning Unicode codepoint count,
  including combining codepoints and embedded U+0000.
- `LEFT`/`RIGHT`: constant source, width and default/explicit pad; non-negative
  width; exactly one padding codepoint; exact codepoint truncation or padding.
  Invalid/signalling domains retain the Level B call. Results above 1,048,576
  codepoints also retain the call, bounding compiler allocation without
  changing runtime semantics.
- All entries require deterministic/no-I/O/no-random/no-ambient, owned-result,
  success-only policy. Dynamic, aliasing, unproved, body-drifted, malformed or
  no-opt calls retain the ordinary Level B path.
- `LOWER` has a new same-summary/opposite-body contradiction guard. The
  existing `UPPER` guard independently proves the registry's body check.

Focused Debug comprises the general certificate/effect/import tests; direct
optimized/no-opt `LEFT`, `LENGTH`, `LOWER`, `RIGHT`, `UPPER`/`LOWER` and
`SUBSTR` tests; both retained PERF2-03 reference-accessor guards; and both VM
engines. All 21 tests pass. The certificate test covers local contradiction,
source import, binary import, no-opt, Unicode, combining codepoints, empty and
width-zero results, explicit multibyte padding, padding-validation order,
invalid signals, nested certified calls, dynamic fallback and the compiler
result-size bound.

## Exact machine-work result

The focused semantic cell contains two `LOWER`, three `LENGTH`, three `LEFT`
and three `RIGHT` constant uses in addition to the already accepted
`UPPER`/`SUBSTR` cases.

| Metric | Accepted SLC1 compiler | P04-CEX1 | Delta |
| --- | ---: | ---: | ---: |
| executable RXAS | 286 | 20 | -266 (-93.007%) |
| `main` locals | 12 | 0 | -12 |
| RXAS bytes | 45,567 | 2,234 | -43,333 (-95.097%) |
| RXBIN bytes | 16,336 | 2,400 | -13,936 (-85.309%) |

The candidate removes two `strlower`, fifteen `strlen`, six `setstrpos`, six
`substring`, six `padstr`, six `append`, twelve impossible `signal` paths and
31 scalar/string copies from this exact admitted cell. Both `rxvm` and
`rxbvm` produce the same output under both compilers and all stderr is empty.

The repeated end-to-end timing cell is smaller because it contains only the
four new families. It falls from 234 to 45 executable RXAS instructions,
20 to 7 locals, 40,084 to 6,746 RXAS bytes and 14,592 to 3,760 RXBIN bytes.

## Ordinary profiling-off Release wall verdict

The maintained Level B runner executed the frozen SLC1 and CEX1 images
serially and rotated their order. The final equal-work cell uses 5,000,000
iterations. The first block used two warmups and seven recorded samples. Since
the short candidate process crossed the retained 3% relative-MAD/10% span
rule, ten more serial samples were appended unchanged; the table reports all
17 samples. No sample was removed. All 76 timed invocations (eight warmups and
68 recorded) exit zero, emit the required PASS and have empty stderr.

| VM | Accepted SLC1 median | P04-CEX1 median | Speedup | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 1,092,735,000 ns | 51,803,000 ns | 21.094049x | 95.259326% |
| `rxbvm` | 1,250,143,000 ns | 56,995,000 ns | 21.934257x | 95.440922% |

The combined candidate remains noisy on `rxvm` and its short-process spans
remain above 10%; that uncertainty affects the precise multiplier, not the
verdict direction. Every candidate maximum is below every SLC1 minimum: the
least favorable separation is 18.156x on `rxvm` and 19.289x on `rxbvm`.

## Current-portfolio boundary

The linked Level B library, canonical RexxCPS RXAS/RXBIN, `rxpp.rxbin`, linked
`rxpp` image and packed `rxpp.c` are all byte-identical to accepted SLC1. CEX1
therefore makes no claim for the current Tier A portfolio or RexxCPS. It is a
general use-site win: when these certified core functions receive proved
constant inputs, their entire runtime body disappears; dynamic workloads are
unchanged.

## Verdict and mandatory stop

P04-CEX1 is favorable and the four entries should be retained. No public
RXAS/RXBIN/ABI/VM/native surface changes. The accepted Level B sources remain
unchanged. Stop here for Adrian's first-verdict acceptance. P04-WRD1 is already
selected as the successor, but does not begin until this verdict is accepted.
Broad QA, closeout, commit and push also remain stopped.
