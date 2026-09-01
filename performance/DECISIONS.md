# cREXX performance decisions and lessons

Status: **enduring decision record; initially seeded with the completed
DECIMAL-01 Stage 3 decision; broader Stage 6 consolidation remains plan-only**

This file records accepted mechanisms, rejected experiments, lessons and
explicit reopening triggers. It prevents a later performance review from
repeating a closed experiment merely because its evidence is no longer in the
immediate working set. Current measurements remain in `performance/RESULTS.md`
when that Stage 6 authority is created; raw evidence remains in dated bundles.

## DECIMAL-01-DECISION-2026-08-18 — retain current `mc_decimal`

Decision: **no change**

Keep the current Mike Cowlishaw `decNumber`-based `mc_decimal` provider with
the existing `DECDPUN=8`, `DECNUMDIGITS=64`, `DECBUFFER=64` configuration. Do
not select libmpdec, retuned decNumber, decQuad, a fixed/arbitrary hybrid, a
new plugin ABI or another production provider from this panel.

This decision closes the complete first candidate panel on the Apple M5
profiling-off Release product. All candidates were correctness-qualified for
their claimed boundary before their performance result was interpreted.

### Rejected experiments

| Candidate | Correctness/capability boundary | Decisive performance result | Disposition |
| --- | --- | --- | --- |
| D2 retuned `decNumber` | All 48 composed builds passed the full provider contract | Best balanced build was 0.23% slower at Common-18 and 1.67% slower at Classic-9; best isolated gain was only 2.70% and lost 1.74% in the other context | Closed before formal L1 and L2/L3 |
| D3 `decQuad` | Common-18 and Classic-9 checksums and lifecycle passed; fixed at 34 digits; contexts above 34 unsupported; native power unavailable | Adapter arithmetic was 44.40%/64.76% slower and direct-core arithmetic 49.28%/66.73% slower at Common-18/Classic-9 | Closed before formal L1 and L2/L3 |
| D4 libmpdec 4.0.1 | BSD-2-Clause, supported macOS/Linux/Windows toolchains, provider contract and lifecycle passed | Formal adapter arithmetic was 41.66%/26.28% slower; matched-capacity direct core remained 21.90%/5.30% slower at Common-18/Classic-9 | Rejected at formal L1; no L2/L3 |

The D2 48-build grid already covered `DECDPUN` 3, 4, 8 and 9;
`DECNUMDIGITS` 18, 34 and 64; and `DECBUFFER` 20, 36, 64 and 128. Do not
repeat this same grid under the same implementation, compiler class, VM value
contract and host class.

The D3 compact 16-byte representation improved isolated copy/clear and some
context-sync work. That is not evidence for a hybrid: conversion and comparison
were slower, arithmetic was materially slower, the provider is capped at 34
digits, and decQuad supplies no native power operation.

Libmpdec's licence and platform support were acceptable. Its rejection is a
measured performance decision, not a licensing, portability or correctness
rejection.

### Lessons learned

1. A modern library, good maintenance record or favourable published
   microbenchmark is useful for prioritising a test, but does not predict its
   cREXX result. The VM value contract, context mapping, conversion and
   allocation path must be measured through the real adapter.
2. Direct-core and adapter measurements answer different questions. The
   libmpdec double-check showed both a library/kernel disadvantage and extra
   view/commit cost; the decQuad direct-core result showed that removing
   adapter suspicion did not rescue its arithmetic.
3. Smaller decimal values can improve movement without improving the product.
   Copy/clear or sync wins cannot compensate for large arithmetic, conversion
   or semantic-coverage losses.
4. The present decNumber configuration is already near the useful local
   optimum for the tested Common-18 and Classic-9 arithmetic. All `DECDPUN=8`
   variants ranked above the other limb sizes, while changing value capacity
   or temporary-buffer size within that group produced only small mixed or
   adverse results. Smaller or larger limb choices were materially slower,
   not hidden opportunities.
5. Correctness checksums are gates, not performance evidence. They establish
   comparable work; current elapsed/throughput samples determine the speed
   verdict.
6. Low-cost calibration should precede a formal campaign. When no candidate
   approaches the predeclared 10% credible-headroom or two-mode 15% L1 gate,
   more rounds and L2/L3 work are not justified.
7. A contaminated host interval invalidates the whole balanced block. The
   XProtect-overlapped D2 attempt was not trimmed or statistically repaired;
   the decision uses the subsequent clear-host replacement only.
8. Negative evidence is a product result. Retain the compact decision and
   decisive raw tables, while allowing superseded calibration/build noise to
   be removed during the separately governed Stage 7 clean-down.

### Reopening triggers

Reopen decimal-provider performance work only if at least one of these is
true and the candidate can still meet the existing correctness and progression
gates:

- a materially new upstream arithmetic implementation or algorithm is
  available, rather than a packaging or minor-version refresh;
- a separately approved VM value/ownership contract removes the measured
  adapter view/commit, allocation or copying cost;
- supported target hardware or a compiler gains relevant native decimal or
  wide-integer acceleration, with evidence that changes the direct-core
  expectation;
- current application profiling shows decimal work has become a material
  end-to-end bottleneck and defines a representative workload absent from this
  panel;
- a new fixed/arbitrary design supplies a correctness-preserving fallback and
  demonstrates enough direct-core and adapter headroom to repay tagging,
  promotion, fallback, lifecycle and code-size cost; or
- independently reproduced results on a supported platform show a credible
  path to the predeclared L1/L3 thresholds.

The following are not reopening triggers by themselves:

- another run of the same D2 macro grid;
- a larger sample count intended to promote a 0-3% calibration signal;
- a favourable comparison-only, copy-only or sync-only microkernel;
- smaller value size without arithmetic and semantic coverage;
- a library's name, age, popularity or published result on different work;
- unchanged-output or historical checksum identity; or
- the absence of a production integration attempt for a candidate that failed
  its progression gate.

### Evidence

- [Current-provider baseline](evidence/2026-08-18-decimal-01-gate1-current-provider/)
- [D4 libmpdec screen and double-check](evidence/2026-08-18-decimal-01-libmpdec-screen/)
- [D2/D3 calibration and Stage 3 verdict](evidence/2026-08-18-decimal-01-stage3-calibration/)
- [DECIMAL-01 engineering plan](decimal/DECIMAL-01-ENGINEERING-PLAN.md)
- [DECIMAL-01 worklist](decimal/DECIMAL-01-WORKLIST.md)
