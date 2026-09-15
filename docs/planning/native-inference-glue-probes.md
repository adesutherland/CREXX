# NI-S4-P01 — Bounded glue probes

Authorized by Adrian's follow-up on 14 September 2026: investigate the noisy
first Release comparison with phase probes, including the suspected float
conversion/output cost. The [first panel](../../performance/evidence/2026-09-14-ni-s4-first-release/README.md)
remains retained unchanged. This direction replaces its proposed quiet replay
with bounded attribution first; it does not authorize model/backend tuning.

## Numbered vision and outcomes

1. **P-OUT-01:** Determine whether admission/tokenization, batch construction,
   normalization/float conversion or packed-output publication accounts for a
   material part of request time, separately from decode/synchronization.
2. **P-OUT-02:** Verify actual contiguous output volume and copy count. BGE emits
   384 coordinates per row: 3,072 bytes for one packed double row, 24,576 bytes
   for eight. The current `.packedfloat` owner stores one `.binary` payload.
3. **P-OUT-03:** Keep diagnostic timing distinct from the ordinary first Release
   verdict, retain noise and exact inputs, and preserve every parent OUT/NI/AC.

## Numbered checkable acceptance criteria

1. **P-AC-01:** Probes are opt-in, VM-local and bounded; no per-request logging,
   VM pointer retention or upstream/model parameter change. Existing public
   procedure signatures and packed result representation remain unchanged.
2. **P-AC-02:** Record admission, submit/tokenization, batch setup, upstream
   decode/synchronization, normalization/conversion and host packed-copy time.
   Separate phase timers from end-to-end request time; neither overlap nor omit
   costs silently. Identify unmeasured remainder and probe overhead honestly.
3. **P-AC-03:** A warm request performs one decode and one packed publication of
   exactly rows × 384 × 8 bytes; counters are checked outside timed loops.
   Complete-text/numeric/ownership controls pass with probes enabled.
4. **P-AC-04:** Use the same fixed model/text/threads/context/one/eight-row settings
   and Level B capture/reduction. No inference quality work, upstream tuning or
   sanitizer/full-suite testing. Report bounded findings before any performance
   repair; NI-S4-P01 is not closed merely because noise is plausible.

## Numbered plan steps

1. **P-01:** Preserve first-panel source patch, identities, raw data and checksum
   manifest; record this authorized direction. Complete.
2. **P-02:** Add opt-in aggregate phase/copy counters and ordinary positive/negative
   controls before measurement. Keep model/request behavior unchanged.
3. **P-03:** Run focused normal checks and a bounded Release diagnostic using
   snapshots outside the measured request loop. Retain uninstrumented evidence.
4. **P-04:** Explain measured conversion/copy contribution and where variation
   occurs; stop for direction before an implementation repair or broader work.

## Current handoff — probes complete, disposition accepted

P-01–03 and P-AC-01–04 are complete for this bounded local diagnostic. The
[retained Release probes](../../performance/evidence/2026-09-14-ni-s4-glue-probes/README.md)
show that normalize/convert plus packed publication is below 0.2% of request
time in all four fixed cases. Eight-row Metal medians are 4.18 µs normalization/
conversion and 0.50 µs packed publication versus 4,102 µs per request. Exactly
one packed copy/decode and the expected byte count are checked for every warm
run. Approximately 99% of request time is inside decode/synchronization; most
variation occurs there. This locates the time boundary, not an upstream defect.

Adrian conditionally accepted the cost of required conversion. The existing
packedfloat interface requires doubles, but the measured conversion does not
explain the first panel's GPU difference. The
[representation review](native-inference-vector-representation-review.md)
confirms float32-LE SQLite storage and sidecar centroids, with widening for current
rxvector computation. No representation change is approved or implemented.

P-04 reported the recommendation: retain the current packed result, make no
conversion/copy or upstream performance edit, and continue STEP-04 functional
closeout after acceptance, which Adrian has now given below. NI-S4-P01 retains the original
noisy GPU comparison; these four-pair instrumented observations do not replace a
formal performance pass. No broad or sanitizer work occurred. All original
OUT/NI/ACs remain intact. The owner of the next action and STEP-06 release QA is
Codex under Adrian's direction.

Adrian subsequently requested the [12-pair probes-disabled replay](../../performance/evidence/2026-09-14-ni-s4-quiet-release/README.md)
and accepted its indicative overhead/variation on 2026-09-14. P-04 is complete;
NI-S4-P01 is an accepted observation with unresolved cause. S4-05 is active and
no additional performance investigation is selected. The original data and
numeric tripwires are unchanged.
