# Verdict

## Decision result

**Accepted; R1a and PERF2-05 closeout complete.**

The corrected R1a product removes one dispatch for the exact canonical
`UNLINK destination; LINKREF destination,source` shape in normal valid-reference
execution. Canonical RXAS and RXBIN remain unchanged. Debug/breakpoint
execution and invalid/non-reference sources resume at the canonical `LINKREF`,
preserving instruction visibility, signal attribution and resume behavior after
the already-observed `UNLINK`.

| VM | Pairs | Before median | After median | Paired median | Favorable | Mean 95% interval | Result |
| --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| `rxvm` | 36 | 84.4605 ms | 83.7145 ms | -1.151991% | 32/36 | -2.337441% to -0.187062% | clear favorable |
| `rxbvm` | 36 | 93.0945 ms | 90.2810 ms | -3.022743% | 36/36 | -3.204016% to -2.814217% | clear favorable |

## Capture interpretation

The initial 12-pair `rxvm` interval crossed zero and both initial `rxvm`
absolute cells exceeded the 10% span rule, so a balanced 12-pair append was
required. At 24 pairs both paired intervals were clear favorable, but the
appended R2a `rxvm` cell still had an 11.317845% span. A final balanced append
therefore took the series to the 36-pair cap.

The last R1a `rxvm` absolute cell had a 13.981496% span, driven by a high tail,
but no further append is allowed at the cap. The complete paired population
still has a below-zero mean interval and 32/36 favorable pairs. No sample was
removed, replaced or reclassified.

## Correctness gate

- Focused Debug core/reference/TRACE dual-VM suite: 12/12 pass.
- Compiler/import/optimized/no-opt reference matrix: 49/49 pass.
- Fresh ordinary Release exact-relink guard: 2/2 pass.
- The same canonical guard RXBIN passes on both preserved R2a VMs.
- The retained List RXBIN/library passes on both corrected R1a VMs at work 100.
- Release configure/build logs contain no compiler warning or error.

## Accepted closeout

Adrian accepted the clear-favorable verdict on 2026-07-26. The full Debug and
ordinary profiling-off Release products then rebuilt cleanly. The focused
12-test core/reference/TRACE set and 49-test compiler/import/optimized/no-opt
set pass in each configuration; broad CTest passes 1,924/1,924 in Debug and
1,924/1,924 in Release.

No disposable PoC or superseded result was committed. In line with the shortest
approved closeout path, sanitizer, install/package, cross-platform, expanded
portfolio and repeated-baseline work were not added. R2b still lacks separate
post-R2a `copy_value` attribution and full payload/lifetime proof; the neutral
B1 control still lacks stable multi-workload evidence. They remain future
ledger entries rather than open PERF2-05 work.
