# `D2X` (Level C Classic BIF)

```text
D2X(number [, length])
CheckArgs without length: rWHOLENUM>=0
CheckArgs with length:    rWHOLENUM rWHOLE>=0
```

Classic Level C D2X converts a whole-number RexxValue, interpreted under the
caller's numeric settings, to uppercase hexadecimal digit text. Without
`length`, the number must be non-negative and the result uses the minimum
width. With a non-negative `length`, negative numbers use twos-complement; the
result is zero- or F-padded, or truncated from the left, to exactly that many
digits.

```rexx
D2X(129)       /* "81" */
D2X(129, 4)    /* "0081" */
D2X(257, 2)    /* "01" */
D2X(-127, 2)   /* "81" */
D2X(-127, 4)   /* "FF81" */
D2X(12, 0)     /* empty */
```

Standard context errors are `RXC-LC-40.3`/`40.4` for argument count,
`RXC-LC-40.5` for an omitted required argument, `RXC-LC-40.12` for a value
that is not whole, and `RXC-LC-40.13` for a negative length or a negative
number without `length`.

The standalone `rexxclassicbifd2x.rexxclassicbif_d2x(context_ref)` entry uses
shared `WHOLENUM` validation and converts arbitrary normalized decimal
magnitudes through mutable base-1e9 limbs. It does not narrow the number to a
native `.int`, use the compatibility name controller, or depend on compiler
lowering.

The distinct native signed-64-bit API is documented in
[`lib/rxfnsb/rexx/d2x.md`](../rxfnsb/rexx/d2x.md). The direct Level C harness is
`lib/rxfnsc/tests_functional/testRexxClassicBifD2x.crexx`.
