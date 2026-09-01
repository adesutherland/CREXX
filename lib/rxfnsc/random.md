# Level C `RANDOM`

The standalone direct Level C BIF entry point is:

```rexx
rexxclassicbif_random(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `RANDOM([max])` and
`RANDOM([min [,max [,seed]]])`. Omitted bounds select `0..999`, while one
argument is the maximum. All supplied arguments must be non-negative whole
numbers. A supplied seed resets the `RexxClassicConfig` sequence before the
value is drawn; subsequent contexts using that same configuration continue the
sequence. Separate evaluator/configuration instances remain isolated.

The difference between minimum and maximum may not exceed `100000`. The direct
BIF reports standard Level C errors through its context: `40.31` for an
oversized one-argument maximum, `40.32` for an oversized selected range, and
`40.33` for reversed bounds. CheckArgs reports argument count, omission,
whole-number, and non-negative failures.

The configured generator uses deterministic Park-Miller state and unbiased
rejection sampling. It does not call the VM `irand` instruction or process-
global C `rand()` state. RexxScript owns one configuration per evaluator and
injects it into every direct BIF context. Compiler lowering is unchanged.
