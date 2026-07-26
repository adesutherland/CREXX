# PERF2-04 P04-WRD1 first ordinary Release verdict

P04-WRD1 extends the accepted general certified-call constant evaluator with
an independently proved `rxfnsb.word` entry. It also schedules the existing
general constant fold/propagate fixed point before certified-call evaluation,
then repeats it afterwards so flow-proved actuals and consumers can compose.
There is no `WORD`-specific consumer rule: the certified call produces the
constant word and the ordinary folder removes the false comparison. The
current Level B body remains the complete runtime fallback and behavior
documentation.

## Qualification

- The entry pins the exact callable, typed I6 summary and normalized body
  fingerprint before its evaluator may run.
- Both actuals must be constants, the source must be valid Unicode and the
  word number must be at least one. Invalid, dynamic, unproved, body-drifted,
  no-opt or otherwise rejected cells retain the ordinary Level B path and its
  signal behavior.
- Word boundaries use the exact VM Unicode blank set and public one-based word
  numbering. The result is a new owned string, including an owned empty result
  for empty, blank-only or missing-word cases.
- The evaluator is a general constant `WORD` implementation; the current
  RexxCPS win happens to be `WORD("Key Bee", 1)` consumed by equality with
  `"?"`. The setup-only `WORD(version_info, 1)` remains dynamic.

Focused Debug comprises the certificate/effect/import tests, optimized and
no-opt `WORD`, both retained PERF2-03 reference-accessor guards and the linked
runtime fixture. All seven selected tests pass. The certificate test covers
empty, ASCII and mixed Unicode whitespace, missing words, strict equality,
nested folding, invalid word-number signals, dynamic fallback, source import,
same-summary/opposite-body contradiction, optimized/no-opt behavior and both
VMs.

## Exact generated-machine result

| Metric | Accepted CEX1 | P04-WRD1 | Delta |
| --- | ---: | ---: | ---: |
| whole-module executable RXAS | 1,387 | 1,319 | -68 (-4.902668%) |
| `main` executable RXAS | 470 | 402 | -68 (-14.468085%) |
| `main` locals | 100 | 103 | +3 |
| RXAS bytes | 204,309 | 195,576 | -8,733 (-4.274408%) |
| RXBIN bytes | 72,106 | 68,446 | -3,660 (-5.075861%) |

The timed `word("Key Bee", 1) = "?"` path disappears completely, including
one each of `strlen`, `fndnblnk`, `fndblnk`, `setstrpos` and `substring`, its
result copy/comparison and the impossible `Failed6` branch. The remaining
`rxfnsb.word` call is the deliberately dynamic setup/reporting use over
`version_info`; it is not in the timed kernel. The three-extra-local peak is
explicit, but no added local executes in place of the removed timed path.

The ordinary Release `rxvm`, `rxbvm` and linked `library.rxbin` are
byte-identical to accepted CEX1. The compiler grows by 160 bytes. No Level B,
public RXAS, RXBIN format, ABI, VM or native implementation changes.

## Profiling-off Release wall verdict

The accepted CEX1 RexxCPS RXAS/RXBIN are byte-identical to accepted SLC1, so
the retained SLC1 medians are the exact valid predecessor baseline. The
maintained Level B matrix runner executed the frozen WRD1 product serially with
two warmups and seven recorded samples per VM. All four warmups and all 14
recorded executions exit zero, emit the required PASS and retain no stderr.

| VM | Accepted CEX1 median | P04-WRD1 median | Increment | Relative MAD | Rerun |
| --- | ---: | ---: | ---: | ---: | --- |
| `rxvm` | 36,229,324 | 39,000,952 | +7.650234% | 0.881617% | no |
| `rxbvm` | 33,904,454 | 36,892,167 | +8.812155% | 0.547875% | no |

Every WRD1 recorded sample exceeds the matching accepted-baseline maximum:
38,516,387 exceeds 36,586,950 on `rxvm`, and 36,197,195 exceeds 34,100,161 on
`rxbvm`. The dual-VM movement, exact scan/slice/branch removal and low noise
make the first verdict unambiguously favorable.

## Verdict and mandatory stop

Recommend accepting P04-WRD1 and retaining the exact `WORD` certificate plus
the general pre/post certified-call fold ordering. The implementation is
frozen and remains provisional. Stop here for Adrian's decision. Do not begin
broad QA, closeout, local slice commits or push until this verdict is accepted.
