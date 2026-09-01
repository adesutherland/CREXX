# Level C `SIGN`

The standalone direct entry point is:

```rexx
.rexxclassicbifsign..rexxclassicbif_sign(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `SIGN(number)`. The required argument uses the shared `rNUM`
contract, including Classic outer blanks and a blank between a leading sign and
the number. The result is a RexxValue integer: `1` for positive, `-1` for
negative, and `0` for either signed zero.

Standard context errors are `RXC-LC-40.3`, `40.4`, and `40.5` for argument
count/presence, and `RXC-LC-40.11` for invalid numeric text.

The implementation validates and converts once, then performs at most two
decimal comparisons. It calls neither the name controller nor the typed Level B
helper and creates no numeric result string.

`lib/rxfnsc/tests_functional/testRexxClassicBifSign.crexx` calls the qualified
entry directly in optimized and unoptimized overlays. It covers signs, signed
zero, exponent extremes, Classic leading-sign blanks, non-mutation, and every
documented argument error.
