# Level C `ABS`

The standalone direct entry point is:

```rexx
.rexxclassicbifabs..rexxclassicbif_abs(
    context = reference .RexxBifCallContext) = .RexxValue
```

It implements `ABS(number)`. The `rNUM` CheckArgs rule accepts Classic numeric
text, including outer blanks and blanks between a leading sign and the number,
normalizes it under the RexxValue numeric context, and returns its non-negative
decimal value.

Standard context errors are `RXC-LC-40.3`, `40.4`, and `40.5` for argument
count/presence and `RXC-LC-40.11` for nonnumeric text.

The implementation converts the checked RexxValue to decimal once, performs
one comparison and at most one subtraction, and constructs the result
RexxValue. It calls neither the name controller nor the Level B ABS helper.

`lib/rxfnsc/tests_functional/testRexxClassicBifAbs.crexx` calls the standalone
entry directly in optimized and unoptimized overlays. It covers ordinary,
signed, exponent, blank-separated-sign, zero, non-mutation, and standard error
behavior.
