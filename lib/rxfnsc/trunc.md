# Level C `TRUNC`

The standalone direct entry point is:

```rexx
.rexxclassicbiftrunc..rexxclassicbif_trunc(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `TRUNC(number [,digits])`. `number` uses the shared `rNUM`
normalization contract. `digits` is an optional non-negative whole number and
defaults to zero, including when its argument position is omitted.

The result is fixed, non-exponent text with exactly `digits` fractional places.
It is truncated without rounding and padded with zeros when necessary:

```rexx
TRUNC("127.09782", 3) /* "127.097" */
TRUNC("00127.1", 3)   /* "127.100" */
TRUNC("1E-12", 12)    /* "0.000000000001" */
TRUNC("-0E999999999", 2) /* "0.00" */
```

Standard context errors are `RXC-LC-40.3`, `40.4`, and `40.5` for argument
count/presence, `RXC-LC-40.11` for invalid numeric text,
`RXC-LC-40.12` for a non-whole `digits`, and `RXC-LC-40.13` for a negative
value.

The standalone implementation parses the validated RexxValue text once and
assembles the fixed result directly. It does not call the name controller, the
typed Level B helper, binary floating-point conversion, or the RexxValue decimal
cache; this preserves long Classic mantissas. Leading zeroes are scanned once,
and result work is linear in the input plus output lengths.

`lib/rxfnsc/tests_functional/testRexxClassicBifTrunc.crexx` calls the qualified
entry directly in optimized and unoptimized overlays. It covers the documented
forms, signs, leading zeroes, large mantissas, positive/negative exponents,
signed zero with a huge exponent, omission, non-mutation, and all standard
argument errors.
